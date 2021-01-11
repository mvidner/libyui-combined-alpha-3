/**************************************************************************
Copyright (C) 2000 - 2010 Novell, Inc.
All Rights Reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

**************************************************************************/


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

  File:	      YQBusyIndicator.h

  Author:     Thomas Goettlicher <tgoettlicher@suse.de>

/-*/


#ifndef YQBusyIndicator_h
#define YQBusyIndicator_h

#include <QFrame>
#include <QTimer>
#include <QLabel>
#include "YBusyIndicator.h"


class QString;
class YQWidgetCaption;
class BusyBar;

class YQBusyIndicator : public QFrame, public YBusyIndicator
{
    Q_OBJECT

public:
    /**
     * Constructor.
     **/
    YQBusyIndicator( YWidget * 		parent,
		   const string &	label,
		   int			timeout = 1000 );
    /**
     * Destructor.
     **/
    virtual ~YQBusyIndicator();

    /**
     * Set the label (the caption above the progress bar).
     *
     * Reimplemented from YBusyIndicator.
     **/
    virtual void setLabel( const string & label );

    /**
     * Set the timeout is ms after that the widget shows 'stalled' when no
     * new tick is received.
     *
     * Reimplemented from YBusyIndicator.
     **/
    virtual void setTimeout( int newTimeout  );

    /**
     * Return the timeout is ms after that the widget shows 'stalled' when no
     * new tick is received.
     *
     * Reimplemented from YBusyIndicator.
     **/
    int timeout()   const   { return _timeout;  }

    /**
     * Send a keep allive message.
     *
     * Reimplemented from YBusyIndicator.
     **/
    virtual void setAlive( bool newAlive );

    /**
     * Set enabled/disabled state.
     *
     * Reimplemented from YWidget.
     **/
    virtual void setEnabled( bool enabled );

    /**
     * Preferred width of the widget.
     *
     * Reimplemented from YWidget.
     **/
    virtual int preferredWidth();

    /**
     * Preferred height of the widget.
     *
     * Reimplemented from YWidget.
     **/
    virtual int preferredHeight();

    /**
     * Set the new size of the widget.
     *
     * Reimplemented from YWidget.
     **/
    virtual void setSize( int newWidth, int newHeight );

    /**
     * Accept the keyboard focus.
     **/
    virtual bool setKeyboardFocus();

protected:

    YQWidgetCaption *	_caption;
    BusyBar *		_bar;
    QTimer *		_timer;
    int			_timeout;

private slots:
    void setStalled();

};





class BusyBar : public QFrame
{
    Q_OBJECT
public:
    BusyBar(QWidget * parent);


    /**
     * start moving bar animation
     **/
    void run();

    /**
     * stop moving bar animation 
     **/
    void stop();

protected:
    void paintEvent(QPaintEvent *event);

private:
    double 	_position;		// the position of the bar 
    bool 	_rightwards;		// direction the bar moves
    QTimer *	_timer;
    bool	_alive;

private slots:
    void	update();

};


#endif // YQBusyIndicator_h
