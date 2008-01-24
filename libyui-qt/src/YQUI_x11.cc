/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:	      	YUIQt_x11.cc

  Author:     	Stefan Hundhammer <sh@suse.de>

  Textdomain "packages-qt"

/-*/

#include <QEvent>
#include <QCursor>
#include <QWidget>
#include <QTextCodec>
#include <QRegExp>
#include <QLocale>
#include <QMessageBox>
#include <QColorGroup>
#include <QDesktopWidget>


#define YUILogComponent "qt-ui"
#include "YUILog.h"

#include "YEvent.h"
#include "YQDialog.h"
#include "YQi18n.h"
#include "QY2Settings.h"
#include "YQUI.h"

// Include low-level X headers AFTER Qt headers:
// X.h pollutes the global namespace (!!!) with pretty useless #defines
// like "Above", "Below" etc. that clash with some Qt headers.
#include <X11/Xlib.h>


int YQUI::getDisplayWidth()
{
    return qApp->desktop()->width();
}


int YQUI::getDisplayHeight()
{
    return qApp->desktop()->height();
}


int YQUI::getDisplayDepth()
{
    return qApp->desktop()->depth();
}


long YQUI::getDisplayColors()
{
    return 1L << qApp->desktop()->depth();
}


int YQUI::getDefaultWidth()
{
    return _default_size.width();
}


int YQUI::getDefaultHeight()
{
    return _default_size.height();
}


int YQUI::defaultSize(YUIDimension dim) const
{
    return dim == YD_HORIZ ? _default_size.width() : _default_size.height();
}


void YQUI::beep()
{
    qApp->beep();
}

void
YQUI_Ui::slotBusyCursor()
{
    YQUI::ui()->busyCursor();
}

void
YQUI::busyCursor( void )
{
    qApp->setOverrideCursor( Qt::BusyCursor );
}


void
YQUI::normalCursor( void )
{
    if ( _busy_cursor_timer->isActive() )
	_busy_cursor_timer->stop();

    while ( qApp->overrideCursor() )
	qApp->restoreOverrideCursor();
}


void YQUI::toggleVisionImpairedPalette()
{
    if ( _usingVisionImpairedPalette )
    {
	qApp->setPalette( normalPalette());  // informWidgets

	_usingVisionImpairedPalette = false;
    }
    else
    {
	qApp->setPalette( visionImpairedPalette() );  // informWidgets

	_usingVisionImpairedPalette = true;
    }
}


QPalette YQUI::visionImpairedPalette()
{
    const QColor dark  ( 0x20, 0x20, 0x20 );
    QPalette pal;

    // for the active window (the one with the keyboard focus)
    pal.setColor( QPalette::Active, QPalette::Background,		Qt::black 	);
    pal.setColor( QPalette::Active, QPalette::Foreground,		Qt::cyan	);
    pal.setColor( QPalette::Active, QPalette::Text,		Qt::cyan	);
    pal.setColor( QPalette::Active, QPalette::Base,		dark		);
    pal.setColor( QPalette::Active, QPalette::Button,		dark		);
    pal.setColor( QPalette::Active, QPalette::ButtonText,		Qt::green	);
    pal.setColor( QPalette::Active, QPalette::Highlight,		Qt::yellow	);
    pal.setColor( QPalette::Active, QPalette::HighlightedText,	Qt::black	);

    // for other windows (those that don't have the keyboard focus)
    pal.setColor( QPalette::Inactive, QPalette::Background,	Qt::black 	);
    pal.setColor( QPalette::Inactive, QPalette::Foreground,	Qt::cyan	);
    pal.setColor( QPalette::Inactive, QPalette::Text,		Qt::cyan	);
    pal.setColor( QPalette::Inactive, QPalette::Base,		dark		);
    pal.setColor( QPalette::Inactive, QPalette::Button,		dark		);
    pal.setColor( QPalette::Inactive, QPalette::ButtonText,	Qt::green	);

    // for disabled widgets
    pal.setColor( QPalette::Disabled, QPalette::Background,	Qt::black 	);
    pal.setColor( QPalette::Disabled, QPalette::Foreground,	Qt::gray	);
    pal.setColor( QPalette::Disabled, QPalette::Text,		Qt::gray	);
    pal.setColor( QPalette::Disabled, QPalette::Base,		dark		);
    pal.setColor( QPalette::Disabled, QPalette::Button,		dark		);
    pal.setColor( QPalette::Disabled, QPalette::ButtonText,	Qt::gray	);

    return pal;
}


bool YQUI::close()
{
    sendEvent( new YCancelEvent() );
    return true;
}


/**
 * UI-specific conversion from logical layout spacing units (80x25)
 * to device dependent units (640x480).
 **/
int YQUI::deviceUnits( YUIDimension dim, float size )
{
    if ( dim==YD_HORIZ )	size *= ( 640.0/80 );
    else			size *= ( 480.0/25 );

    return (int) ( size + 0.5 );
}


/**
 * Default conversion from device dependent layout spacing units (640x480)
 * to logical layout units (80x25).
 *
 * This default function assumes 80x25 units.
 * Derived UIs may want to reimplement this.
 **/
float YQUI::layoutUnits( YUIDimension dim, int device_units )
{
    float size = (float) device_units;

    if ( dim==YD_HORIZ )	size *= ( 80/640.0 );
    else			size *= ( 25/480.0 );

    return size;
}


void YQUI::maybeLeftHandedUser()
{
    if ( _askedForLeftHandedMouse )
	return;


    QString message =
	_( "You clicked the right mouse button "
	   "where a left-click was expected."
	   "\n"
	   "Switch left and right mouse buttons?"
	   );
    int button = QMessageBox::question( 0,
					// Popup dialog caption
					_( "Unexpected Click" ),
					message,
					QMessageBox::Yes | QMessageBox::Default,
					QMessageBox::No,
					QMessageBox::Cancel | QMessageBox::Escape );

    if ( button == QMessageBox::Yes )
    {

	const char * command =
	    _leftHandedMouse ?
	    "xmodmap -e \"pointer = 1 2 3\"":	// switch back to right-handed mouse
	    "xmodmap -e \"pointer = 3 2 1\"";	// switch to left-handed mouse

	_leftHandedMouse	 = ! _leftHandedMouse; 	// might be set repeatedly!
	_askedForLeftHandedMouse = false;	// give the user a chance to switch back
	yuiMilestone() << "Switching mouse buttons: " << command << endl;

	system( command );
    }
    else if ( button == 1 )	// No
    {
	_askedForLeftHandedMouse = true;
    }
}



// EOF