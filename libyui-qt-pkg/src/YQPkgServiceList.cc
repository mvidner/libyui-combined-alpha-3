/**************************************************************************
Copyright (C) 2018 SUSE LLC
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

Textdomain "qt-pkg"

/-*/

#include <algorithm>
#include <set>
#include <string>
#include <QDateTime>
#include <QHeaderView>
#include <QString>

#define YUILogComponent "qt-pkg"
#include "YUILog.h"
#include <zypp/PoolQuery.h>
#include <zypp/RepoManager.h>
#include <zypp/ServiceInfo.h>

#include <QTreeWidget>
#include "YQPkgServiceList.h"
#include "YQPkgFilters.h"
#include "YQi18n.h"
#include "YQUI.h"
#include "utf8.h"

using std::string;
using std::list;
using std::endl;
using std::set;
using std::vector;


YQPkgServiceList::YQPkgServiceList( QWidget * parent )
    : QY2ListView( parent )
{
    yuiDebug() << "Creating service list" << endl;

    QStringList headers;

    // TRANSLATORS: Column header for the service list
    headers <<  _("Name");
    _nameCol = 0;

    setHeaderLabels( headers );
    header()->setSectionResizeMode( _nameCol, QHeaderView::Stretch );

    setSelectionMode( QAbstractItemView::ExtendedSelection );	// allow multi-selection with Ctrl-mouse

    connect( this, 	SIGNAL( itemSelectionChanged() ),
	     this, 	SLOT  ( filterIfVisible()) );
    setIconSize(QSize(32,32));
    fillList();
    setSortingEnabled( true );
    sortByColumn( nameCol(), Qt::AscendingOrder );
    selectSomething();

    yuiDebug() << "Creating service list done" << endl;
}

YQPkgServiceList::~YQPkgServiceList()
{
    // NOP
}

void
YQPkgServiceList::fillList()
{
    clear();
    yuiDebug() << "Filling service list" << endl;

    std::set<std::string> added_services;
    zypp::RepoManager repo_manager;

    std::for_each(ZyppRepositoriesBegin(), ZyppRepositoriesEnd(), [&](const zypp::Repository& repo) {
        const std::string &service_name(repo.info().service());
        if (!service_name.empty())
        {
            bool found = std::any_of(added_services.begin(), added_services.end(), [&](const std::string& name) {
                return service_name == name;
            });

            if (!found)
            {
                addService(service_name, repo_manager);
                added_services.insert(service_name);
            }
        }
    });

    yuiDebug() << "Service list filled" << endl;
}

void
YQPkgServiceList::filterIfVisible()
{
    if ( isVisible() )
	   filter();
}

void
YQPkgServiceList::filter()
{
    emit filterStart();

    yuiMilestone() << "Collecting packages in selected services..." << endl;
    QTime stopWatch;
    stopWatch.start();

    //
    // Collect all packages from repositories belonging to this service
    //
    QTreeWidgetItem * item;
    QList<QTreeWidgetItem *> items = selectedItems();
    QListIterator<QTreeWidgetItem *> it(items);

    while ( it.hasNext() )
    {
      item = it.next();
      YQPkgServiceListItem * serviceItem = dynamic_cast<YQPkgServiceListItem *> (item);

        if ( serviceItem )
        {
            yuiMilestone() << "Selected service: " << serviceItem->zyppService() << endl;

	    zypp::PoolQuery query;
	    std::for_each(ZyppRepositoriesBegin(), ZyppRepositoriesEnd(), [&](const zypp::Repository& repo) {
            if (serviceItem->zyppService() == repo.info().service())
            {
                yuiMilestone() << "Adding repo filter: " << repo.info().alias() << endl;
                query.addRepo( repo.info().alias() );
            }
        });
	    query.addKind(zypp::ResKind::package);

        std::for_each(query.selectableBegin(), query.selectableEnd(), [&](const zypp::ui::Selectable::Ptr &selectable) {
            emit filterMatch( selectable, tryCastToZyppPkg( selectable->theObj() ) );
        });
	}
    }

    yuiDebug() << "Packages sent to package list. Elapsed time: "
	       << stopWatch.elapsed() / 1000.0 << " sec"
	       << endl;

    emit filterFinished();
}

void
YQPkgServiceList::addService( ZyppService service, const zypp::RepoManager &mgr )
{
    new YQPkgServiceListItem( this, service, mgr );
}


YQPkgServiceListItem *
YQPkgServiceList::selection() const
{
    QTreeWidgetItem * item = currentItem();
    return dynamic_cast<YQPkgServiceListItem *> (item);
}

YQPkgServiceListItem::YQPkgServiceListItem( YQPkgServiceList *	parentList,
				      ZyppService	service, const zypp::RepoManager &mgr )
    : QY2ListViewItem( parentList )
    , _serviceList( parentList )
    , _zyppService( service )
{

    zypp::ServiceInfo srvinfo = mgr.getService(service);
    _zyppServiceName = srvinfo.name();
    QString service_name(fromUTF8(_zyppServiceName));

    if ( nameCol() >= 0 && !service.empty() )
    {
        setText( nameCol(), service_name);
    }

    QString infoToolTip("<p><b>" + service_name.toHtmlEscaped() + "</b></p>");

    // TRANSLATORS: Tooltip item, followed by service URL
    infoToolTip += "<p><b>" + _("URL: ") + "</b>" + fromUTF8(srvinfo.url().asString()).toHtmlEscaped() + "</p>";

    ZyppProduct product = singleProduct( _zyppService );
    if ( product )
    {
        // TRANSLATORS: Tooltip item, followed by product name
        infoToolTip += ("<p><b>" + _("Product: ") + "</b>"
         + fromUTF8(product->summary()).toHtmlEscaped() +  "</p>");
    }

    // TRANSLATORS: Tooltip item, followed by the list of repositories inluded in the libzypp service
    infoToolTip += "<p><b>" + _("Repositories:") + "</b><ul>";
    std::for_each(ZyppRepositoriesBegin(), ZyppRepositoriesEnd(), [&](const zypp::Repository& repo) {
        if (service == repo.info().service())
            infoToolTip += "<li>" + fromUTF8(repo.name()).toHtmlEscaped() + "</li>";
    });
    infoToolTip += "</ul></p>";

    setToolTip( nameCol(), infoToolTip);

    setIcon( 0, YQUI::ui()->loadIcon( "yast-update" ) );
}

YQPkgServiceListItem::~YQPkgServiceListItem()
{
    // NOP
}

ZyppProduct
YQPkgServiceListItem::singleProduct( ZyppService zyppService )
{
    return YQPkgFilters::singleProductFilter([&](const zypp::PoolItem& item) {
        // filter the products from the requested service
        return item.resolvable()->repoInfo().service() == zyppService;
    });
}

bool
YQPkgServiceListItem::operator< ( const QTreeWidgetItem & other ) const
{
    const YQPkgServiceListItem * otherItem = dynamic_cast<const YQPkgServiceListItem *>(&other);

    // case insensitive compare
    return QString::compare(fromUTF8(zyppServiceName()), fromUTF8(otherItem->zyppServiceName()), Qt::CaseInsensitive) < 0;
}

