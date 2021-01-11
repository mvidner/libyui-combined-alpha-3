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

  File:		YQPkgChangeLogView.cc

  Author:	Stefan Hundhammer <sh@suse.de>

  Textdomain	"qt-pkg"

/-*/

#define YUILogComponent "qt-pkg"
#include "YUILog.h"

#include <qregexp.h>
#include "YQPkgChangeLogView.h"
#include "YQPkgDescriptionDialog.h"
#include "YQi18n.h"
#include "utf8.h"

// how many change log entries should be displayed at most,
// displaying huge changes takes too much time (bsc#1044777)
static const int MAX_DISPLAYED_CHANGES = 512;

YQPkgChangeLogView::YQPkgChangeLogView( QWidget * parent )
    : YQPkgGenericDetailsView( parent )
{
}


YQPkgChangeLogView::~YQPkgChangeLogView()
{
    // NOP
}


void
YQPkgChangeLogView::showDetails( ZyppSel selectable )
{
    _selectable = selectable;

    if ( ! selectable )
    {
	clear();
	return;
    }

    yuiDebug() << "Generating changelog..." << std::endl;

    QString html = htmlStart();
    html += htmlHeading( selectable, false );

    ZyppPkg installed = tryCastToZyppPkg( selectable->installedObj() );

    if ( installed )
    {
        html += changeLogTable( installed->changelog() );

        int not_displayed = installed->changelog().size() - MAX_DISPLAYED_CHANGES;
        if (not_displayed > 0)
        {
            yuiWarning() << "Changelog size limit reached, ignoring last "
                << not_displayed << " items" << std::endl;
            html.append("<p class='note'>"
                + notDisplayedChanges(not_displayed, installed->name() + "-" + installed->edition().asString())
                + "</p>");
        }
    }
    else
    {
	html += "<p><i>" + _( "Information only available for installed packages." ) + "</i></p>";
    }
    html += htmlEnd();

    yuiDebug() << "Changelog HTML size: " << html.size() << std::endl;
    setHtml( html );
    yuiDebug() << "Changes displayed" << std::endl;
}



QString YQPkgChangeLogView::changeLogTable( const zypp::Changelog & changeLog ) const
{
    yuiDebug() << "Changelog size: " << changeLog.size() << " entries" << std::endl;
    QString html;

    int index = 0;
    for ( zypp::Changelog::const_iterator it = changeLog.begin();
	  it != changeLog.end();
	  ++it )
    {
	QString changes = htmlEscape( fromUTF8( (*it).text() ) );
	changes.replace( "\n", "<br>" );
	changes.replace( " ", "&nbsp;" );

	html += row(
		    cell( (*it).date()   ) +
		    cell( (*it).author() ) +
		    "<td valign='top'>" + changes + "</td>" // cell() calls htmlEscape() !
		    );

    if (++index == MAX_DISPLAYED_CHANGES)
        break;
    }

    return html.isEmpty() ? "" : table( html );
}

QString YQPkgChangeLogView::notDisplayedChanges(int missing, const std::string &pkg)
{
    // TRANSLATORS: The package change log is too long to display, only the latest
    // changes are displayed. %1 is the number of the items which are not displayed,
    // %2 contains a command for getting the full changes manually.
    QString msg = _("(%1 more change entries are not displayed. Run \""
        "%2\" to see the complete change log.)");

    QString cmd = QString("rpm -q --changelog %1").arg(pkg.c_str());
    return msg.arg(QString::number(missing), cmd);
}

#include "YQPkgChangeLogView.moc"
