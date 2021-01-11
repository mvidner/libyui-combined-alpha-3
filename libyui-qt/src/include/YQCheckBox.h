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

  File:	      YQCheckBox.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

// -*- c++ -*-


#ifndef YQCheckBox_h
#define YQCheckBox_h

#include <qgroupbox.h>
#include <ycp/YCPString.h>

#include "YCheckBox.h"


class QCheckBox;

class YQCheckBox : public QGroupBox, public YCheckBox
{
    Q_OBJECT

public:
    
    /**
     * Constructor.
     */
    YQCheckBox( QWidget *		parent,
		const YWidgetOpt &	opt,
		const YCPString & 	label,
		bool 			initiallyChecked );

    /**
     * Inherited from YWidget: Sets the enabled state of the
     * widget. All new widgets are enabled per definition. Only
     * enabled widgets can take user input.
     */
    void setEnabling( bool enabled );

    /**
     * Minimum size the widget should have to make it look and feel
     * nice.
     */
    long nicesize( YUIDimension dim );

    /**
     * Sets the new size of the widget.
     */
    void setSize( long newWidth, long newHeight );

    /**
     * Sets the checked-state of the checkbox
     */
    void setValue( const YCPValue & checked );

    /**
     * Returns whether the checkbox is checked.
     * This may return 'true' or 'false' or 'nil' for a tristate check box.
     */
    YCPValue getValue();

    /**
     * Changes the label of the text entry.
     */
    void setLabel( const YCPString & label );

    /**
     * Accept the keyboard focus.
     */
    virtual bool setKeyboardFocus();

    /**
     * Returns whether tristate condition is set ( i.e. neither on nor off )
     */
    bool isTristate();

    /**
     * Set tristate condition
     */
    void setTristate( bool tristate );

    
private slots:

    /**
     * Triggered when the on/off status is changed
     */
    void stateChanged ( int newState );

    
protected:
    
    /**
     * Pointer to the qt widget that actually does the job
     */
    QCheckBox * _qt_checkbox;

    /**
     * Flag that indicates tristate condition: neither on nor off
     */
    bool _dont_care;

};

#endif // YQCheckBox_h
