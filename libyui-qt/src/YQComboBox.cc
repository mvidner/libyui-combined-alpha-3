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

  File:	      YQComboBox.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#define SEND_SELECTION_CHANGED_EVENT 0

#include <qstring.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpixmap.h>
#define YUILogComponent "qt-ui"
#include "YUILog.h"

#include "utf8.h"
#include "YQUI.h"
#include "YEvent.h"
#include "QY2CharValidator.h"
#include "YQComboBox.h"
#include "YQSignalBlocker.h"
#include "YQWidgetCaption.h"
#include <QVBoxLayout>
#include <QDebug>

YQComboBox::YQComboBox( YWidget * 	parent,
			const string &	label,
			bool		editable )
    : QFrame( (QWidget *) parent->widgetRep() )
    , YComboBox( parent, label, editable )
    , _validator(0)
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    setLayout( layout );

    setWidgetRep( this );
    layout->setSpacing( YQWidgetSpacing );
    layout->setMargin ( YQWidgetMargin  );

    _caption = new YQWidgetCaption( this, label );
    YUI_CHECK_NEW( _caption );
    layout->addWidget( _caption );

    _qt_comboBox = new QComboBox(this);
    _qt_comboBox->setEditable(editable);
    YUI_CHECK_NEW( _caption );
    layout->addWidget( _qt_comboBox );

    _caption->setBuddy( _qt_comboBox );

#if SEND_SELECTION_CHANGED_EVENT
    connect( _qt_comboBox,	SIGNAL( highlighted (int) ),
	     this,		SLOT  ( slotSelected(int) ) );
#endif

    connect( _qt_comboBox,	SIGNAL( activated  ( QString ) ),
	     this,		SLOT  ( textChanged( QString ) ) );

    connect( _qt_comboBox,	SIGNAL( editTextChanged( QString ) ),
	     this,		SLOT  ( textChanged( QString ) ) );
}


YQComboBox::~YQComboBox()
{
    // NOP
}


string YQComboBox::text()
{
    return toUTF8( _qt_comboBox->currentText() );
}


void YQComboBox::setText( const string & newValue )
{
    QString text = fromUTF8( newValue );

    if ( isValidText( text ) )
    {
	YQSignalBlocker sigBlocker( _qt_comboBox );
        int index = _qt_comboBox->findText( text );
        if ( index < 0 )
            _qt_comboBox->setItemText(_qt_comboBox->currentIndex(), text );
        else {
            _qt_comboBox->setCurrentIndex( index );
            _qt_comboBox->setItemText(index, text );
        }
    }
    else
    {
	yuiError() << this << ": Rejecting invalid value \"" << newValue << "\"" << endl;
    }
}


void YQComboBox::addItem( YItem * item )
{
    YComboBox::addItem( item );
    QIcon icon;

    if ( item->hasIconName() )
    {
	string iconName = iconFullPath( item );
	icon = QIcon( iconName.c_str() );

	if ( icon.isNull() )
	    yuiWarning() << "Can't load icon \"" << iconName << "\"" << endl;
    }

    if ( icon.isNull() )
	_qt_comboBox->addItem( fromUTF8( item->label() ) );
    else
	_qt_comboBox->addItem( icon, fromUTF8( item->label() ) );

    if ( item->selected() )
    {
	YQSignalBlocker sigBlocker( _qt_comboBox );
	setText( item->label() );
    }
}


void YQComboBox::deleteAllItems()
{
    YQSignalBlocker sigBlocker( _qt_comboBox );

    _qt_comboBox->clear();
    YComboBox::deleteAllItems();
}


void YQComboBox::setLabel( const string & label )
{
    _caption->setText( label );
    YComboBox::setLabel( label );
}


void YQComboBox::setValidChars( const string & newValidChars )
{
    if ( ! _qt_comboBox->isEditable() )
    {
	yuiWarning() << this << ": Setting ValidChars is useless on a combo box that isn't editable! (%s)" << endl;
	return;
    }

    if ( _validator )
    {
	_validator->setValidChars( fromUTF8( newValidChars ) );
    }
    else
    {
	_validator = new QY2CharValidator( fromUTF8( newValidChars ), this );
	_qt_comboBox->setValidator( _validator );

	// No need to delete the validator in the destructor - Qt will take
	// care of that since it's a QObject with a parent!
    }

    if ( ! isValidText( _qt_comboBox->currentText() ) )
    {
	yuiError() << this << ": Old value \"" << _qt_comboBox->currentText()
		   << " \" invalid according to new ValidChars \""<< newValidChars << "\" - deleting"
		   << endl;
	_qt_comboBox->setItemText(_qt_comboBox->currentIndex(), "");
    }

    YComboBox::setValidChars( newValidChars );
}


bool YQComboBox::isValidText( const QString & txt ) const
{
    if ( ! _validator )
	return true;

    int pos = 0;
    QString text( txt );	// need a non-const QString &

    return _validator->validate( text, pos ) == QValidator::Acceptable;
}


void YQComboBox::slotSelected( int i )
{
    if ( notify() )
    {
	if ( ! YQUI::ui()->eventPendingFor( this ) )
	{
	    // Avoid overwriting a (more important) ValueChanged event with a SelectionChanged event

	    YQUI::ui()->sendEvent( new YWidgetEvent( this, YEvent::SelectionChanged ) );
	}
    }
}


void YQComboBox::textChanged( QString )
{
    if ( notify() )
	YQUI::ui()->sendEvent( new YWidgetEvent( this, YEvent::ValueChanged ) );
}


void YQComboBox::setInputMaxLength( int len )
{
    _qt_comboBox->lineEdit()->setMaxLength( len );
    YComboBox::setInputMaxLength( len );
}


int YQComboBox::preferredWidth()
{
    return sizeHint().width();
}


int YQComboBox::preferredHeight()
{
    return sizeHint().height();
}


void YQComboBox::setSize( int newWidth, int newHeight )
{
    resize( newWidth, newHeight );
}


void YQComboBox::setEnabled( bool enabled )
{
    _caption->setEnabled( enabled );
    _qt_comboBox->setEnabled( enabled );
    YWidget::setEnabled( enabled );
}


bool YQComboBox::setKeyboardFocus()
{
    _qt_comboBox->setFocus();

    return true;
}


#include "YQComboBox.moc"