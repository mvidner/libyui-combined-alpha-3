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

  File:	      YQPkgGenericDetailsView.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#define YUILogComponent "qt-pkg"
#include "YUILog.h"
#include <QTabWidget>
#include <QRegExp>
#include <QDateTime>

#include "YQPkgGenericDetailsView.h"
#include "YQi18n.h"
#include "YQUI.h"
#include "utf8.h"

using std::string;

YQPkgGenericDetailsView::YQPkgGenericDetailsView( QWidget * parent )
    : QTextBrowser( parent )
{
    _selectable = 0;
    _parentTab  = dynamic_cast<QTabWidget *> (parent);

    if ( _parentTab )
    {
	connect( parent, SIGNAL( currentChanged(QWidget *) ),
		 this,   SLOT  ( reload        (QWidget *) ) );
    }

    QString css;
    css = "table.stats"
        "{text-align: center;"
        "font-family: Verdana, Geneva, Arial, Helvetica, sans-serif ;"
        "font-weight: normal;"
        "font-size: small;"
        ";color: #fff;"
        "width: 100%;"
        ""
        "border: 1px;"
        "border-collapse: collapse;"
        "border-spacing: 4px;}"
        ""
        "table.stats td"
        "{"
        ";color: #000;"
        "padding: 4px;"
        "text-align: left;"
        "border: 1px #fff solid;}"
        ""
        "table.stats td.hed"
        "{"
        ";color: #fff;"
        "padding: 4px;"
        "text-align: left;"
        "border-bottom: 2px #fff solid;"
        "font-size: small;"
        "font-weight: bold;} ";

    document()->addResource( QTextDocument::StyleSheetResource, QUrl( "format.css" ), css );
}


YQPkgGenericDetailsView::~YQPkgGenericDetailsView()
{
    // NOP
}


void
YQPkgGenericDetailsView::reload( QWidget * newCurrent )
{
    if ( newCurrent == this )
    {
	showDetailsIfVisible( _selectable );
    }
}


void
YQPkgGenericDetailsView::showDetailsIfVisible( ZyppSel selectable )
{
    _selectable = selectable;

    if ( _parentTab )		// Is this view embedded into a tab widget?
    {
	if ( _parentTab->currentWidget() == this )  // Is this page the topmost?
	{
	    showDetails( selectable );
	}
    }
    else	// No tab parent - simply show data unconditionally.
    {
	showDetails( selectable );
    }
}


QSize
YQPkgGenericDetailsView::minimumSizeHint() const
{
    return QSize( 0, 0 );
}


QString
YQPkgGenericDetailsView::htmlStart()
{
    return "<html><head>"
        "<link rel='stylesheet' type='text/css' href='format.css'>"
        "</head><body>";
}

QString
YQPkgGenericDetailsView::htmlEnd()
{
    return "</body></html>";
}


QString
YQPkgGenericDetailsView::htmlHeading( ZyppSel selectable, bool showVersion )
{
    if ( ! selectable )
	return "";

    ZyppObj zyppObj = selectable->theObj();

    if ( ! zyppObj )
	return "";

    QString summary = fromUTF8( zyppObj->summary() );

    QString html = "<table";

    if ( ! YQUI::ui()->usingVisionImpairedPalette() )
        html +=  " class=\"stats\"";

    html += "><tr><td><b>"
	+ fromUTF8( zyppObj->name() )
	+ "</b>";

    if ( showVersion )
	html += QString( "<b>-" ) + zyppObj->edition().asString().c_str() + "</b>";

    if ( ! summary.isEmpty() )
	html += " - " + summary;

    html += "</td></tr></table>";

    return html;
}



QString
YQPkgGenericDetailsView::htmlEscape( const QString & plainText )
{
    QString html = plainText;
    // yuiDebug() << "Escaping \"" << plainText << "\"" << endl;

    html.replace( QRegExp( "&" ), "&amp;" );
    html.replace( QRegExp( "<" ), "&lt;"  );
    html.replace( QRegExp( ">" ), "&gt;"  );

    return html;
}


QString
YQPkgGenericDetailsView::table( const QString & contents )
{
    QString html = "<table";
    if ( ! YQUI::ui()->usingVisionImpairedPalette() )
        html +=  " class=\"stats\"";

    html += ">" + contents + "</table>";

    return html;
}


QString
YQPkgGenericDetailsView::row( const QString & contents )
{
    return "<tr>" + contents + "</tr>";
}


QString
YQPkgGenericDetailsView::cell( QString contents )
{
    contents = htmlEscape( contents );
    return "<td>" + contents + "</td>";
}


QString
YQPkgGenericDetailsView::cell( int contents )
{
    QString html;
    html.sprintf( "<td>%d</td>", contents );

    return html;
}


QString
YQPkgGenericDetailsView::cell( const zypp::Date & date )
{
    return cell( ( (time_t) date == (time_t) 0 ? "" : date.asString() ) );
}


QString
YQPkgGenericDetailsView::cell( const string & contents )
{
    return cell( fromUTF8( contents ) );
}


QString
YQPkgGenericDetailsView::hcell( QString contents )
{
    QString html = "<td";

    if ( ! YQUI::ui()->usingVisionImpairedPalette() )
	html += " bgcolor=#D0D0D0";

    html += ">" + contents + "</td>";

    return html;
}


#include "YQPkgGenericDetailsView.moc"
