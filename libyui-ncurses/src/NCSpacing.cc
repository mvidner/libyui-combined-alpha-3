/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       NCSpacing.cc

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/
#include "Y2Log.h"
#include "NCurses.h"
#include "NCSpacing.h"

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : NCSpacing::NCSpacing
//	METHOD TYPE : Constructor
//
//	DESCRIPTION :
//
NCSpacing::NCSpacing( YWidget * parent,
		      YUIDimension dim,
		      bool stretchable,
		      YLayoutSize_t layoutUnits )
    : YSpacing( parent, dim, stretchable, layoutUnits )
    , NCWidget( parent )
{
  WIDDBG << endl;
#if 0
  if ( horizontal && vertical ) {
    l = "NC(B)Spacing";
  }
  else if ( horizontal ) {
    l = "NC(H)Spacing";
  }
  else if ( vertical ) {
    l = "NC(V)Spacing";
  }
  else {
    l = "NC(N)Spacing";
  }
#endif
  
  if ( dim == YD_VERT )
     l = "NC(V)Spacing";
  else if ( dim == YD_HORIZ )
      l = "NC(H)Spacing";
  else
      l = "NC(N)Spacing";
  
  wstate = NC::WSdumb;
  skipNoDimWin = true;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : NCSpacing::~NCSpacing
//	METHOD TYPE : Destructor
//
//	DESCRIPTION :
//
NCSpacing::~NCSpacing()
{
  WIDDBG << endl;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : NCSpacing::setSize
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void NCSpacing::setSize( int newwidth, int newheight )
{
  wRelocate( wpos( 0 ), wsze( newheight, newwidth ) );
}

void NCSpacing::setEnabled( bool do_bv )
{
    NCWidget::setEnabled( do_bv );
    YSpacing::setEnabled( do_bv );
}