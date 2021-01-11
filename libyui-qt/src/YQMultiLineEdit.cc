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

  File:	      YQMultiLineEdit.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#include <qmultilineedit.h>
#include <qlabel.h>
#define y2log_component "qt-ui"
#include <ycp/y2log.h>

using std::max;

#include "utf8.h"
#include "YQUI.h"
#include "YEvent.h"
#include "YQMultiLineEdit.h"

#define VISIBLE_LINES	3


YQMultiLineEdit::YQMultiLineEdit( QWidget * 		parent,
				  const YWidgetOpt & 	opt,
				  const YCPString & 	label,
				  const YCPString & 	initialText )
    : QVBox( parent )
    , YMultiLineEdit( opt, label )
    , InputMaxLength( -1 )
{
    setWidgetRep( this );
    setSpacing( YQWidgetSpacing );
    setMargin( YQWidgetMargin );

    if ( label->value().length() > 0 )
    {
	_qt_label = new QLabel( fromUTF8( label->value() ), this );
	_qt_label->setTextFormat( QLabel::PlainText );
	_qt_label->setFont( YQUI::ui()->currentFont() );
    }
    else
	_qt_label = 0;

    _qt_textedit = new QTextEdit( this );
    _qt_textedit->setTextFormat( Qt::PlainText );
    _qt_textedit->setFont( YQUI::ui()->currentFont() );
    _qt_textedit->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    _qt_textedit->setText( fromUTF8( initialText->value() ) );

    if ( _qt_label )
	_qt_label->setBuddy( _qt_textedit );

    connect( _qt_textedit, SIGNAL( textChanged( void ) ), this, SLOT( changed ( void ) ) );
}


void YQMultiLineEdit::setEnabling( bool enabled )
{
    _qt_textedit->setEnabled( enabled );
}


long YQMultiLineEdit::nicesize( YUIDimension dim )
{
    if ( dim == YD_HORIZ )
    {
	long minSize   	 = 30;
	long hintWidth 	 = sizeHint().width();

	return max( minSize, hintWidth );
    }
    else
    {
	long minSize     = 10;
	long hintHeight	 = VISIBLE_LINES * _qt_textedit->fontMetrics().lineSpacing();
	hintHeight	+= _qt_textedit->frameWidth() * 2 + YQWidgetMargin * 2;

	if ( _qt_label && _qt_label->isShown() )
	    hintHeight	+= _qt_label->sizeHint().height() + YQWidgetSpacing;

	return max( minSize, hintHeight );
    }
}


void YQMultiLineEdit::setSize( long newWidth, long newHeight )
{
    resize( newWidth, newHeight );
}


void YQMultiLineEdit::setText( const YCPString & text )
{
    _qt_textedit->blockSignals( true );
    _qt_textedit->setText( fromUTF8( text->value() ) );
    _qt_textedit->blockSignals( false );
}


YCPString YQMultiLineEdit::text()
{
    return YCPString( toUTF8( _qt_textedit->text() ) );
}


void YQMultiLineEdit::setLabel( const YCPString & label )
{
    if ( _qt_label )
	_qt_label->setText( fromUTF8( label->value() ) );
    YMultiLineEdit::setLabel( label );
}


bool YQMultiLineEdit::setKeyboardFocus()
{
    _qt_textedit->setFocus();

    return true;
}


void YQMultiLineEdit::changed()
{

    // if we reached the maximum number of characters which can be inserted
    if ( InputMaxLength >= 0 && InputMaxLength < _qt_textedit->length() ) {
	int index, para;
	_qt_textedit->getCursorPosition( &para, &index);
	
	QString text = _qt_textedit->text();
	
	int pos = 0; // current positon in text
	int section =0; // section in text;
	// iterate over the string
	while ( pos != (int)text.length()+1 ) {
	    // we reached the paragraph where the user entered
	    // a character
	    if ( section == para ) {
		// remove that character
		text.remove( pos+index-1, 1 );
		break;
	    }
	    
	    // new paragraph begins
	    if ( text[pos] == '\n' ) {
		section++;
	    }
	    pos++;
	}
	
	_qt_textedit->setText( text );
	
	// user removed a paragraph
	if ( index == 0 ) {
	    --para;
	    // the new index is the end of the previous paragraph
	    index = _qt_textedit->paragraphLength(para) + 1;
	}

	// adjust to new cursor position before the removed character
	_qt_textedit->setCursorPosition( para, index-1 );
    }

    if ( getNotify() )
	YQUI::ui()->sendEvent( new YWidgetEvent( this, YEvent::ValueChanged ) );
}

void YQMultiLineEdit::setInputMaxLength( const YCPInteger & numberOfChars)
{
    InputMaxLength = numberOfChars->asInteger()->value();
    
    QString text = _qt_textedit->text();

    // truncate the text if appropriate
    if ( InputMaxLength < (int)text.length() ) {
	text.truncate( InputMaxLength );
	_qt_textedit->setText(text);
    }

}

#include "YQMultiLineEdit.moc"

