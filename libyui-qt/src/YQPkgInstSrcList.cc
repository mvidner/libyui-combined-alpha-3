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

  File:	      YQPkgInstSrcList.cc

  Author:     Stefan Hundhammer <sh@suse.de>

  Textdomain "packages-qt"

/-*/


#define y2log_component "qt-pkg"
#include <ycp/y2log.h>
#include <qdatetime.h>
#include "YQi18n.h"
#include "utf8.h"
#include "YQPkgInstSrcList.h"
#include <zypp/SourceManager.h>
#include <algorithm>

using std::list;
using std::set;
using std::vector;



YQPkgInstSrcList::YQPkgInstSrcList( QWidget * parent )
    : QY2ListView( parent )
{
    y2debug( "Creating inst source list" );

    _nameCol	= -1;
    _urlCol	= -1;

    int numCol = 0;

    // Column headers for installation source list
    addColumn( _( "Name"	) );	_nameCol	= numCol++;
    addColumn( _( "URL"		) );	_urlCol		= numCol++;

    setAllColumnsShowFocus( true );
    setSelectionMode( QListView::Extended );	// allow multi-selection with Ctrl-mouse

    connect( this, 	SIGNAL( selectionChanged() ),
	     this, 	SLOT  ( filterIfVisible()) );

    fillList();
    selectSomething();

    y2debug( "Creating inst source list done" );
}


YQPkgInstSrcList::~YQPkgInstSrcList()
{
    // NOP
}


void
YQPkgInstSrcList::fillList()
{
    clear();
    y2debug( "Filling inst source list" );

    zypp::SourceManager_Ptr sourceManager = zypp::SourceManager::sourceManager();
    list<zypp::SourceManager::SourceId> sources = sourceManager->allSources();

    for ( list<zypp::SourceManager::SourceId>::const_iterator it = sources.begin();
	  it != sources.end();
	  ++it )
    {
	zypp::Source_Ref src = sourceManager->findSource( *it );

	if ( src && src.enabled() )
	    addInstSrc( src );
    }

    y2debug( "Inst source list filled" );
}


int
YQPkgInstSrcList::countEnabledSources()
{
    int count = 0;
    zypp::SourceManager_Ptr sourceManager = zypp::SourceManager::sourceManager();
    list<zypp::SourceManager::SourceId> sources = sourceManager->allSources();

    for ( list<zypp::SourceManager::SourceId>::const_iterator it = sources.begin();
	  it != sources.end();
	  ++it )
    {
	zypp::Source_Ref src = sourceManager->findSource( *it );

	if ( src && src.enabled() )
	    ++count;
    }

    return count;
}


void
YQPkgInstSrcList::filterIfVisible()
{
    if ( isVisible() )
	filter();
}


void
YQPkgInstSrcList::filter()
{
    emit filterStart();

    y2milestone( "Collecting packages on selected installation sources..." );
    QTime stopWatch;
    stopWatch.start();


    //
    // Collect all packages on this inst source
    //

    set<ZyppSel> exactMatches;
    set<ZyppSel> nearMatches;

    QListViewItem * item = firstChild();	// take multi selection into account

    while ( item )
    {
	if ( item->isSelected() )
	{
	    YQPkgInstSrcListItem * instSrcItem = dynamic_cast<YQPkgInstSrcListItem *> (item);

	    if ( instSrcItem )
	    {
		ZyppSrc currentSrc = instSrcItem->zyppSrc();

		for ( ZyppPoolIterator sel_it = zyppPkgBegin();
		      sel_it != zyppPkgEnd();
		      ++sel_it )
		{
		    if ( (*sel_it)->candidateObj() &&
			 (*sel_it)->candidateObj()->source() == currentSrc )
		    {
			exactMatches.insert( *sel_it );
		    }
		    else
		    {
			zypp::ui::Selectable::available_iterator pkg_it = (*sel_it)->availableBegin();

			while ( pkg_it != (*sel_it)->availableEnd() )
			{
			    if ( (*pkg_it)->source() == currentSrc )
				nearMatches.insert( *sel_it );

			    ++pkg_it;
			}
		    }
		}

	    }
	}

	item = item->nextSibling();
    }

    
    //
    // Send all exact matches to the list
    // (emit a filterMatch signal for each one)
    //

    set<ZyppSel>::const_iterator sel_it = exactMatches.begin();

    while ( sel_it != exactMatches.end() )
    {
	emit filterMatch( (*sel_it), tryCastToZyppPkg( (*sel_it)->theObj() ) );
	nearMatches.erase( *sel_it );
	++sel_it;
    }


    //
    // Send all near matches to the list
    // (emit a filterNearMatch signal for each one)
    //

    sel_it = nearMatches.begin();

    while ( sel_it != nearMatches.end() )
    {
	emit filterNearMatch( *sel_it, tryCastToZyppPkg( (*sel_it)->theObj() ) );
	++sel_it;
    }

    y2debug( "Packages sent to package list. Elapsed time: %f sec", stopWatch.elapsed() / 1000.0 );

    emit filterFinished();
}


void
YQPkgInstSrcList::addInstSrc( ZyppSrc src )
{
    new YQPkgInstSrcListItem( this, src );
}


YQPkgInstSrcListItem *
YQPkgInstSrcList::selection() const
{
    QListViewItem * item = selectedItem();

    if ( ! item )
	return 0;

    return dynamic_cast<YQPkgInstSrcListItem *> (item);
}






YQPkgInstSrcListItem::YQPkgInstSrcListItem( YQPkgInstSrcList *	instSrcList,
					    ZyppSrc		src 		)
    : QY2ListViewItem( instSrcList )
    , _instSrcList( instSrcList )
    , _zyppSrc( src )
{
    if ( nameCol() >= 0 )
    {
	string name;
	ZyppProduct product = singleProduct( _zyppSrc );

	if ( product )	// only if the source provides exactly one product
	{		// (which is the most common case)
	    name = product->summary();
	}

	if ( ! name.empty() )
	    setText( nameCol(), name.c_str() );
    }

    if ( urlCol() >= 0 )
    {
	setText( urlCol(), src.url().asString().c_str() );
    }
}



YQPkgInstSrcListItem::~YQPkgInstSrcListItem()
{
    // NOP
}


ZyppProduct
YQPkgInstSrcListItem::singleProduct( ZyppSrc zyppSrc )
{
    ZyppProduct product;

    zypp::ResStore::iterator it = zyppSrc.resolvables().begin();

    //
    // Find the first product on this inst src
    //

    while ( it != zyppSrc.resolvables().end() && ! product )
    {
	product = zypp::dynamic_pointer_cast<zypp::Product>( *it );
	++it;
    }

    //
    // Check if there is another product on this inst src
    //

    while ( it != zyppSrc.resolvables().end() )
    {
	if ( zypp::dynamic_pointer_cast<zypp::Product>( *it ) )
	{
	    y2milestone( "Multiple products on installation source %s",
			 zyppSrc.alias().c_str() );
	    ZyppProduct null;
	    return null;
	}

	++it;
    }

    if ( ! product )
	y2milestone( "No product on installation source %s",
		     zyppSrc.alias().c_str() );

    return product;
}



#include "YQPkgInstSrcList.moc"
