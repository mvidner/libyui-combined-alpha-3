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

  File:	      YQLogView.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#include <qlabel.h>
#include <qprogressbar.h>
#include <qtimer.h>
#define y2log_component "qt-ui"
#include <ycp/y2log.h>

#include "utf8.h"
#include "YQUI.h"
#include "YQDownloadProgress.h"


YQDownloadProgress::YQDownloadProgress( QWidget *		parent,
					const YWidgetOpt &	opt,
					const YCPString & 	label,
					const YCPString &	filename,
					int 			expectedSize )
    : QVBox( parent )
    , YDownloadProgress( opt, label, filename, expectedSize )
{
    setWidgetRep( this );
    setMargin( YQWidgetMargin );

    _qt_label = new QLabel( fromUTF8( label->value() ), this );
    _qt_label->setTextFormat( QLabel::PlainText );
    _qt_label->setFont( YQUI::ui()->currentFont() );

    _qt_progress = new QProgressBar( this );
    _qt_progress->setFont( YQUI::ui()->currentFont() );
    _qt_progress->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );

    _qt_progress->setTotalSteps( expectedSize );
    _qt_progress->setProgress( currentFileSize() );

    _timer = new QTimer( this );
    connect( _timer, SIGNAL( timeout() ), this, SLOT( pollFileSize() ) );

    _timer->start( 250,		// millisec
		   false );	// single shot?
}


void YQDownloadProgress::setEnabling( bool enabled )
{
    _qt_progress->setEnabled( enabled );
}


long YQDownloadProgress::nicesize( YUIDimension dim )
{
    if ( dim == YD_HORIZ ) return sizeHint().width();
    else			return sizeHint().height();
}


void YQDownloadProgress::setSize( long newWidth, long newHeight )
{
    resize( newWidth, newHeight );
}


void YQDownloadProgress::setLabel( const YCPString & label )
{
    _qt_label->setText( fromUTF8( label->value() ) );
    YDownloadProgress::setLabel( label );
}


void YQDownloadProgress::setExpectedSize( int expectedSize )
{
    _qt_progress->setTotalSteps( expectedSize );
    _qt_progress->setProgress( currentFileSize() );
    YDownloadProgress::setExpectedSize( expectedSize );
}


void YQDownloadProgress::pollFileSize()
{
    long size = currentFileSize();

    if ( size > (long) expectedSize() )
	size = (long) expectedSize();
    _qt_progress->setProgress( size );
}



#include "YQDownloadProgress.moc"
