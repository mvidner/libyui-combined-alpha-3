/*
  Copyright (C) 2000-2012 Novell, Inc
  Copyright (C) 2018 SUSE LLC

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) version 3.0 of the License. This library
  is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
  License for more details. You should have received a copy of the GNU
  Lesser General Public License along with this library; if not, write
  to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
  Floor, Boston, MA 02110-1301 USA
*/


/*-/

  File:       FSize.cc

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose:

/-*/

#include <iostream>
#include <sstream>

#include "FSize.h"

// arbitrary precision float for floating point division
#include <boost/multiprecision/cpp_bin_float.hpp>

using boost::multiprecision::cpp_int;

const cpp_int FSize::KB = 1024;
const cpp_int FSize::MB = FSize::KB * 1024;
const cpp_int FSize::GB = FSize::MB * 1024;
const cpp_int FSize::TB = FSize::GB * 1024;
const cpp_int FSize::PB = FSize::TB * 1024;
const cpp_int FSize::EB = FSize::PB * 1024;
const cpp_int FSize::ZB = FSize::EB * 1024;
const cpp_int FSize::YB = FSize::ZB * 1024;


FSize::FSize( const std::string &sizeStr, const Unit unit_r )
  : _size( cpp_int(sizeStr) * factor( unit_r ) )
{
}

//
//
//	METHOD NAME : FSize::fillBlock
//	METHOD TYPE : FSize &
//
//	DESCRIPTION :
//
FSize & FSize::fillBlock( FSize blocksize_r )
{
  if ( _size > 0 && cpp_int(blocksize_r) > 0) {
    cpp_int diff = _size % cpp_int(blocksize_r);
    if ( diff ) {
      if ( _size > 0 )
	_size += cpp_int(blocksize_r);
      _size -= diff;
    }
  }
  return *this;
}

//
//
//	METHOD NAME : FSize::bestUnit
//	METHOD TYPE : FSize::Unit
//
//	DESCRIPTION :
//
FSize::Unit FSize::bestUnit() const
{
  cpp_int usize = abs(_size);
  if ( usize < KB )
    return Unit::B;
  if ( usize < MB )
    return Unit::K;
  if ( usize < GB )
    return Unit::M;
  if ( usize < TB )
    return Unit::G;
  if ( usize < PB )
    return Unit::T;
  if ( usize < EB )
    return Unit::P;
  if ( usize < ZB )
    return Unit::E;
  if ( usize < YB )
    return Unit::Z;
  return Unit::Y;
}

//
//
//	METHOD NAME : FSize::form
//	METHOD TYPE : std::string
//
//	DESCRIPTION :
//
std::string FSize::form( const Unit unit_r, unsigned fw, unsigned prec, const bool showunit ) const
{
  if ( prec == bestPrec ) {
    switch ( unit_r )
    {
    case Unit::Y:  prec = 3; break;
    case Unit::Z:  prec = 3; break;
    case Unit::E:  prec = 3; break;
    case Unit::P:  prec = 3; break;
    case Unit::T:  prec = 3; break;
    case Unit::G:  prec = 2; break;
    case Unit::M:  prec = 1; break;
    case Unit::K:  prec = 1; break;
    case Unit::B:  prec = 0; break;
    }
} else if ( unit_r == Unit::B )
    prec = 0; // doesn't make sense for Byte

  std::ostringstream str;
  // set the precision and field width, use fixed notation (not the scientific Xe+Y)
  str << std::setprecision(prec) << std::setfill(' ') << std::setw(fw) << std::fixed;

  if (prec == 0)
    // no decimal part required, we can use integer division,
    // add one unit half for correct rounding
    str << (_size + (factor( unit_r ) / 2))/ factor( unit_r );
  else
    // otherwise convert to boost floats
    str << (boost::multiprecision::cpp_bin_float_50)( _size ) /
        (boost::multiprecision::cpp_bin_float_50)(factor( unit_r ) );

  if ( showunit )
    str << " " << unit( unit_r );

  return str.str();
}


//
//
//	METHOD NAME : FSize::asString
//	METHOD TYPE : std::string
//
//	DESCRIPTION :
//
std::string FSize::asString() const
{
  return form();
}

std::ostream& operator<<(std::ostream &ostr, const FSize &fsize)
{
    ostr << fsize.asString();
    return ostr;
}

std::ostream& operator<<(std::ostream &ostr, const FSize::Unit unit)
{
    ostr << FSize::unit(unit);
    return ostr;
}
