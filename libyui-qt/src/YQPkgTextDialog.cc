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

  File:	      YQPkgTextDialog.cc

  Author:     Stefan Hundhammer <sh@suse.de>

  Textdomain "packages-qt"

/-*/


#define y2log_component "qt-pkg"
#include <ycp/y2log.h>


#include <qtextbrowser.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qlayout.h>
#include <qhbox.h>

#include "YQPkgTextDialog.h"

#include "QY2LayoutUtils.h"
#include "YQi18n.h"
#include "utf8.h"

#define SPACING			6	// between subwidgets
#define MARGIN			4	// around the widget

using std::list;
using std::string;



YQPkgTextDialog::YQPkgTextDialog( const QString & text, QWidget * parent )
    : QDialog( parent )
{
    buildDialog( text, parent, _( "&OK" ) );
}


YQPkgTextDialog::YQPkgTextDialog( const QString & 	text,
				  QWidget * 		parent,
				  const QString & 	acceptButtonLabel,
				  const QString & 	rejectButtonLabel )
    : QDialog( parent )
{
    buildDialog( text, parent, acceptButtonLabel, rejectButtonLabel );
}


YQPkgTextDialog::~YQPkgTextDialog()
{
    // NOP
}


void YQPkgTextDialog::buildDialog( const QString & 	text,
				   QWidget * 		parent,
				   const QString & 	acceptButtonLabel,
				   const QString & 	rejectButtonLabel )
{
    // Enable dialog resizing even without window manager
    setSizeGripEnabled( true );

    // Dialog title
    setCaption( _( "YaST2" ) );

    // Layout for the dialog ( can't simply insert a QVBox )

    QVBoxLayout * layout = new QVBoxLayout( this, MARGIN, SPACING );
    CHECK_PTR( layout );


    // Text browser

    _textBrowser = new QTextBrowser( this );
    CHECK_PTR( _textBrowser );
    layout->addWidget( _textBrowser );
    layout->addSpacing(8);
    _textBrowser->setText( text );
    _textBrowser->setTextFormat( Qt::RichText );
    _textBrowser->installEventFilter( this );


    // Button box

    QHBox * buttonBox	= new QHBox( this );
    CHECK_PTR( buttonBox );
    buttonBox->setSpacing( SPACING );
    buttonBox->setMargin ( MARGIN  );
    layout->addWidget( buttonBox );

    addHStretch( buttonBox );

    // Accept (OK) button

    _acceptButton = new QPushButton( acceptButtonLabel, buttonBox );
    CHECK_PTR( _acceptButton );
    _acceptButton->setDefault( true );

    connect( _acceptButton,	SIGNAL( clicked() ),
	     this,      	SLOT  ( accept()  ) );

    addHStretch( buttonBox );

    if ( ! rejectButtonLabel.isEmpty() )
    {
	// Reject (Cancel) button

	_rejectButton = new QPushButton( rejectButtonLabel, buttonBox );
	CHECK_PTR( _rejectButton );
	_rejectButton->setDefault( true );

	connect( _rejectButton,	SIGNAL( clicked() ),
		 this,      	SLOT  ( reject()  ) );

	addHStretch( buttonBox );
    }
    else
    {
	_rejectButton = 0;
    }
}


QSize
YQPkgTextDialog::sizeHint() const
{
    return QSize( 500, 450 );
}


bool
YQPkgTextDialog::eventFilter( QObject * obj, QEvent * ev )
{
    if ( ev && ev->type() == QEvent::KeyPress )
    {
	QKeyEvent * keyEvent = dynamic_cast<QKeyEvent *> (ev);

	if ( keyEvent )
	{
	    if ( keyEvent->key() == Key_Return ||
		 keyEvent->key() == Key_Enter    )
	    {
		_acceptButton->animateClick();
		return true; // Stop event processing
	    }
	    else if ( keyEvent->key() == Key_Escape )
	    {
		if ( _rejectButton )
		{
		    _rejectButton->animateClick();
		    return true; // Stop event processing
		}
	    }
	}
    }

    return false;	// Don't stop event processing
}


void YQPkgTextDialog::setText( const QString & text )
{
    _textBrowser->setText( text );
}


void YQPkgTextDialog::setText( const string & text )
{
    setText( QString( text.c_str() ) );
}


void YQPkgTextDialog::setText( const list<string> & text )
{
    setText( htmlParagraphs( text ) );
}


void YQPkgTextDialog::setText( PMObjectPtr pmObj,
			       const list<string> & text )
{
    setText( htmlHeading( pmObj ) + htmlParagraphs( text ) );
}


void YQPkgTextDialog::showText( QWidget * parent, const QString & text )
{
    YQPkgTextDialog * dia = new YQPkgTextDialog( text, parent );
    CHECK_PTR( dia );
    dia->exec();
    delete dia;
}


void YQPkgTextDialog::showText( QWidget * parent, const string & text )
{
    showText( parent, QString( text.c_str() ) );
}


void YQPkgTextDialog::showText( QWidget * parent, const list<string> & text )
{
    showText( parent, htmlParagraphs( text ) );
}


void YQPkgTextDialog::showText( QWidget * parent,
				PMObjectPtr pmObj,
				const list<string> & text )
{
    showText( parent, htmlHeading( pmObj ) + htmlParagraphs( text ) );
}

void YQPkgTextDialog::showText( QWidget * parent,
				PMObjectPtr pmObj,
				const string & text )
{
    showText( parent, htmlHeading( pmObj ) + QString::fromUtf8( text.c_str() ) );
}


bool YQPkgTextDialog::confirmText( QWidget * 		parent,
				   const QString & 	text,
				   const QString & 	acceptButtonLabel,
				   const QString & 	rejectButtonLabel )
{
    YQPkgTextDialog * dia = new YQPkgTextDialog( text,
						 parent,
						 acceptButtonLabel,
						 rejectButtonLabel );
    CHECK_PTR( dia );
    bool confirmed = ( dia->exec() == QDialog::Accepted );
    delete dia;

    return confirmed;
}


bool YQPkgTextDialog::confirmText( QWidget * parent, const QString & text )
{
    return confirmText( parent, text, _( "&Accept" ), _( "&Cancel" ) );
}


bool YQPkgTextDialog::confirmText( QWidget * parent,
				   PMObjectPtr pmObj,
				   const list<string> & text )
{
    return confirmText( parent, htmlHeading( pmObj ) + htmlParagraphs( text ) );
}


bool YQPkgTextDialog::confirmText( QWidget * parent,
				   PMObjectPtr pmObj,
				   const string & text )
{
    return confirmText( parent, htmlHeading( pmObj ) + QString::fromUtf8( text.c_str() ) );
}



QString
YQPkgTextDialog::htmlParagraphs( const list<string> & text )
{
    bool preformatted = false;
    list<string>::const_iterator it = text.begin();
    
    if ( it != text.end()
	 && *it == "<!-- DT:Rich -->" )	// Special doctype for preformatted HTML
    {
	preformatted = true;
	++it;					// Discard doctype line
    }

    QString html = preformatted ? "" : "<p>";

    while ( it != text.end() )
    {
	QString line = fromUTF8( *it );

	if ( preformatted )
	    html += line + "\n";
	else
	{
	    line = htmlEscape( line );

	    if ( line.length() == 0 )	// Empty lines mean new paragraph
		html += "</p><p>";
	    else
		html += " " + line;
	}

	++it;
    }

    if ( ! preformatted )
	html += "</p>";

    return html;
}


QString
YQPkgTextDialog::htmlEscape( const QString & plainText )
{
    QString html = plainText;
    // y2debug( "Escaping '%s'", (const char *) plainText );

    html.replace( QRegExp( "&" ), "&amp;" );
    html.replace( QRegExp( "<" ), "&lt;"  );
    html.replace( QRegExp( ">" ), "&gt;"  );

    return html;
}


QString
YQPkgTextDialog::htmlHeading( const QString & text )
{
    QString html =
	"<table bgcolor=#E0E0F8><tr><td><b>"
	+ text
	+ "</b></td></tr></table><br>";

    return html;
}


QString
YQPkgTextDialog::htmlHeading( PMObjectPtr pmObj )
{
    QString summary = fromUTF8( pmObj->summary() );

    QString html =
	"<table bgcolor=#E0E0F8><tr><td><b>"
	+ fromUTF8( pmObj->name() )
	+ "</b>";

    if ( ! summary.isEmpty() )
	html += " - " + summary;

    html += "</td></tr></table><br>";

    return html;
}




#include "YQPkgTextDialog.moc"
