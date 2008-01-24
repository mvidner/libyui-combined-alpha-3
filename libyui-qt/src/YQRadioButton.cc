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

  File:	      YQRadioButton.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#include <qradiobutton.h>
#include <QMouseEvent>
#include <QBoxLayout>

#define YUILogComponent "qt-ui"
#include "YUILog.h"

#include "utf8.h"
#include "YQUI.h"
#include "YQApplication.h"
#include "YEvent.h"
#include "YQRadioButton.h"
#include "YRadioButtonGroup.h"
#include "YQSignalBlocker.h"

using std::string;

#define SPACING 8

// +----+----------------------------------+----+
// |	|(o) RadioButtonlabel		   |	|
// +----+----------------------------------+----+
// <----> SPACING			   <---->



YQRadioButton::YQRadioButton( YWidget * 	parent,
			      const string & 	label,
			      bool 		checked )
    : QRadioButton( fromUTF8( label ), ( QWidget *) (parent->widgetRep() ) )
    , YRadioButton( parent, label )
{
    setWidgetRep( this );

    setChecked( checked );

    installEventFilter(this);

    connect ( this,     SIGNAL( toggled ( bool ) ),
	      this,	SLOT  ( changed ( bool ) ) );
}


void
YQRadioButton::setUseBoldFont( bool useBold )
{
    setFont( useBold ?
             YQUI::yqApp()->boldFont() :
             YQUI::yqApp()->currentFont() );

    YRadioButton::setUseBoldFont( useBold );
}


int YQRadioButton::preferredWidth()
{
    return sizeHint().width();
}


int YQRadioButton::preferredHeight()
{
    return sizeHint().height();
}


void YQRadioButton::setSize( int newWidth, int newHeight )
{
    resize( newWidth, newHeight );
}


bool YQRadioButton::value()
{
    return isChecked();
}


void YQRadioButton::setValue( bool newValue )
{
    YQSignalBlocker sigBlocker( this );

    setChecked( newValue );

    if ( newValue )
    {
	YRadioButtonGroup * group = buttonGroup();

	if ( group )
	    group->uncheckOtherButtons( this );
    }
}


void YQRadioButton::setLabel( const string & label )
{
    setText( fromUTF8( label ) );
    YRadioButton::setLabel( label );
}


void YQRadioButton::setEnabled( bool enabled )
{
    QRadioButton::setEnabled( enabled );
    YWidget::setEnabled( enabled );
}


bool YQRadioButton::setKeyboardFocus()
{
    setFocus();

    return true;
}


// slots

void YQRadioButton::changed( bool newState )
{
    if ( notify() && newState )
	YQUI::ui()->sendEvent( new YWidgetEvent( this, YEvent::ValueChanged ) );
}


bool YQRadioButton::eventFilter( QObject * obj, QEvent * event )
{
    if ( event && event->type() == QEvent::MouseButtonRelease )
    {
        QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *> (event);

        if ( mouseEvent && mouseEvent->button() == Qt::RightButton )
        {
	    yuiMilestone() << "Right click on button detected" << endl;
	    YQUI::ui()->maybeLeftHandedUser();
        }
    }

    return QObject::eventFilter( obj, event );
}


#include "YQRadioButton.moc"