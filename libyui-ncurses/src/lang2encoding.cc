/*
  |****************************************************************************
  |
  | Copyright (c) 2000 - 2012 Novell, Inc.
  | All Rights Reserved.
  |
  | This program is free software; you can redistribute it and/or
  | modify it under the terms of version 2 of the GNU General Public License as
  | published by the Free Software Foundation.
  |
  | This program is distributed in the hope that it will be useful,
  | but WITHOUT ANY WARRANTY; without even the implied warranty of
  | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
  | GNU General Public License for more details.
  |
  | You should have received a copy of the GNU General Public License
  | along with this program; if not, contact Novell, Inc.
  |
  | To contact Novell about this file by physical or electronic mail,
  | you may find current contact information at www.novell.com
  |
  |****************************************************************************
*/


/*-/

   File:       lang2encoding.cc

   Author:     auto-generated

/-*/

#include <string>

std::string language2encoding( std::string lang )
{
  using std::string;
  lang = ":" + lang + ":";
  if ( string( ":ca:da:de:en:es:fi:fr:gl:is:it:nl:no:pt:sv:" ).find( lang ) != string::npos )
    return "ISO-8859-1";
  else if ( string( ":lt:" ).find( lang ) != string::npos )
    return "ISO-8859-13";
  else if ( string( ":cs:hr:hu:pl:ro:sk:sl:" ).find( lang ) != string::npos )
    return "ISO-8859-2";
  else if ( string( ":ru:" ).find( lang ) != string::npos )
    return "ISO-8859-5";
  else if ( string( ":el:" ).find( lang ) != string::npos )
    return "ISO-8859-7";
  else if ( string( ":iw:" ).find( lang ) != string::npos )
    return "ISO-8859-8";
  else if ( string( ":tr:" ).find( lang ) != string::npos )
    return "ISO-8859-9";
  else if ( string( ":ja:" ).find( lang ) != string::npos )
    return "eucJP";
  return "";
}
//-----------------------------------------------------------------
