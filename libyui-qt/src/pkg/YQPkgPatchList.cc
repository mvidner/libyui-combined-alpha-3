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

  File:	      YQPkgPatchList.cc

  Author:     Stefan Hundhammer <sh@suse.de>

  Textdomain "packages-qt"

/-*/


#define y2log_component "qt-pkg"
#include <ycp/y2log.h>

#include <qpopupmenu.h>
#include <qaction.h>
#include <zypp/ui/PatchContents.h>
#include <set>

#include "YQi18n.h"
#include "utf8.h"

#include "YQPkgPatchList.h"
#include "YQPkgTextDialog.h"


#define VERBOSE_PATCH_LIST			1
#define DEBUG_SHOW_ALL_UPDATE_STACK_PATCHES	0


typedef zypp::ui::PatchContents			ZyppPatchContents;
typedef zypp::ui::PatchContents::const_iterator ZyppPatchContentsIterator;

using std::list;
using std::set;


YQPkgPatchList::YQPkgPatchList( QWidget * parent )
    : YQPkgObjList( parent )
{
    y2debug( "Creating patch list" );

    _filterCriteria = RelevantPatches;

    int numCol = 0;
    addColumn( "" );			_statusCol	= numCol++;
    addColumn( _( "Patch"	) );	_nameCol	= numCol++;
    addColumn( _( "Summary" 	) );	_summaryCol	= numCol++;
    addColumn( _( "Category"	) );	_categoryCol	= numCol++;
    addColumn( _( "Size"	) );	_sizeCol	= numCol++;
    addColumn( _( "Version"	) );	_versionCol	= numCol++;

    // Can use the same colum for "broken" and "satisfied":
    // Both states are mutually exclusive

    _satisfiedIconCol	= _summaryCol;
    _brokenIconCol	= _summaryCol;

    setAllColumnsShowFocus( true );
    setColumnAlignment( sizeCol(), Qt::AlignRight );

    connect( this,	SIGNAL( selectionChanged	( QListViewItem * ) ),
	     this,	SLOT  ( filter()				    ) );

    setSorting( categoryCol() );

    y2debug( "Creating patch list done" );
}


YQPkgPatchList::~YQPkgPatchList()
{
    // NOP
}


void
YQPkgPatchList::polish()
{
    // Delayed initialization after widget is fully created etc.

    // Only now send selectionChanged() signal so attached details views also
    // display something if their showDetailsIfVisible() slot is connected to
    // selectionChanged() signals.
    selectSomething();
}


void
YQPkgPatchList::setFilterCriteria( FilterCriteria filterCriteria )
{
    _filterCriteria = filterCriteria;
}


void
YQPkgPatchList::fillList()
{
    clear();
    y2debug( "Filling patch list" );

    for ( ZyppPoolIterator it = zyppPatchesBegin();
	  it != zyppPatchesEnd();
	  ++it )
    {
	ZyppSel	  selectable = *it;
	ZyppPatch zyppPatch = tryCastToZyppPatch( selectable->theObj() );

	if ( zyppPatch )
	{
	    bool displayPatch = false;

	    switch ( _filterCriteria )
	    {
		case UpdateStackPatches:

		    if ( zyppPatch->affects_pkg_manager() )
		    {
#if DEBUG_SHOW_ALL_UPDATE_STACK_PATCHES
			displayPatch = true;
#endif
			if ( selectable->hasCandidateObj() &&
			     selectable->candidatePoolItem().status().isNeeded() )
			{
			    displayPatch = true;

			    y2milestone( "Found YaST patch that should be installed: %s - %s",
					 zyppPatch->name().c_str(),
					 zyppPatch->summary().c_str() );
			}

			if ( selectable->hasInstalledObj() &&
			     selectable->installedPoolItem().status().isIncomplete() ) // patch broken?
			{
			    displayPatch = true;

			    y2warning( "Installed YaST patch is broken: %s - %s",
				       zyppPatch->name().c_str(),
				       zyppPatch->summary().c_str() );
			}
		    }
		    break;

		case RelevantPatches:	// needed + broken + satisfied (but not installed)

		    if ( selectable->hasInstalledObj() ) // installed?
		    {
			if ( selectable->installedPoolItem().status().isIncomplete() ) // patch broken?
			{
			    // The patch is broken: It had been installed, but the user somehow
			    // downgraded individual packages belonging to the patch to older versions.

			    displayPatch = true;

			    y2warning( "Installed patch is broken: %s - %s",
				       zyppPatch->name().c_str(),
				       zyppPatch->summary().c_str() );
			}
		    }
		    else // not installed
		    {
			if ( selectable->hasCandidateObj() &&
			     selectable->candidatePoolItem().status().isSatisfied() )
			{
			    // This is a pretty exotic case, but still it might happen:
			    //
			    // The patch itelf is not installed, but it is satisfied because the
			    // user updated all the packages belonging to the patch to the versions
			    // the patch requires. All that is missing now is to get the patch meta
			    // data onto the system. So let's display the patch to give the user
			    // a chance to install it (if he so chooses).

			    displayPatch = true;

			    y2milestone( "Patch satisfied, but not installed yet: %s - %s",
					 zyppPatch->name().c_str(),
					 zyppPatch->summary().c_str() );
			}
		    }

		    if ( selectable->hasCandidateObj() )	// candidate available?
		    {
			// The most common case: There is a candidate patch, i.e. one that could be
			// installed, but either no version of that patch is installed or there is a
			// newer one to which the patch could be updated.

			if ( selectable->candidatePoolItem().status().isNeeded() ) // patch really needed?
			{
			    // Patches are needed if any of the packages that belong to the patch
			    // are installed on the system.

			    displayPatch = true;
			}
			else
			{
			    // None of the packages that belong to the patch is installed on the system.

			    y2debug( "Patch not needed: %s - %s",
				     zyppPatch->name().c_str(),
				     zyppPatch->summary().c_str() );
			}
		    }
		    break;


		case RelevantAndInstalledPatches:	// needed + broken + installed

		    if ( selectable->hasInstalledObj() ) // installed?
		    {
			displayPatch = true;
		    }
		    else // not installed - display only if needed
		    {
			zypp::ResStatus candidateStatus = selectable->candidatePoolItem().status();

			if ( candidateStatus.isNeeded() ||
			     candidateStatus.isSatisfied() )
			{
			    displayPatch = true;
			}
			else
			{
			    y2milestone( "Patch not needed: %s - %s",
					 zyppPatch->name().c_str(),
					 zyppPatch->summary().c_str() );
			}
		    }
		    break;


		case AllPatches:
		    displayPatch = true;
		    break;

		// Intentionally omitting "default" so the compiler
		// can catch unhandled enum values
	    }

	    if ( displayPatch )
	    {
#if VERBOSE_PATCH_LIST
		y2debug( "Displaying patch %s - %s", zyppPatch->name().c_str(), zyppPatch->summary().c_str() );
#endif
		addPatchItem( *it, zyppPatch);
	    }
	}
	else
	{
	    y2error( "Found non-patch selectable" );
	}
    }


    if ( ! firstChild() )
	message( _( "No patches available." ) );

    y2debug( "patch list filled" );
}



void
YQPkgPatchList::message( const QString & text )
{
    QY2ListViewItem * item = new QY2ListViewItem( this );
    CHECK_PTR( item );

    item->setText( 1, text );
    item->setBackgroundColor( QColor( 0xE0, 0xE0, 0xF8 ) );
}



void
YQPkgPatchList::filterIfVisible()
{
    if ( isVisible() )
	filter();
}


void
YQPkgPatchList::filter()
{
    emit filterStart();
    std::set<ZyppSel> patchSelectables;

    if ( selection() )
    {
	ZyppPatch patch = selection()->zyppPatch();

	if ( patch )
	{
	    ZyppPatchContents patchContents( patch );

	    for ( ZyppPatchContentsIterator it = patchContents.begin();
		  it != patchContents.end();
		  ++it )
	    {
		ZyppPkg pkg = tryCastToZyppPkg( *it );

		if ( pkg )
		{
		    y2debug( "Found patch pkg: %s arch: %s", (*it)->name().c_str(), (*it)->arch().asString().c_str() );

		    ZyppSel sel = _selMapper.findZyppSel( pkg );

		    if ( sel )
		    {
			if ( contains( patchSelectables, sel ) )
			{
			    y2milestone( "Suppressing duplicate selectable %s-%s arch: %s",
					 (*it)->name().c_str(),
					 (*it)->edition().asString().c_str(),
					 (*it)->arch().asString().c_str() );
			}
			else
			{
			    patchSelectables.insert( sel );
			    emit filterMatch( sel, pkg );
			}
		    }
		    else
			y2error( "No selectable for pkg %s",  (*it)->name().c_str() );
		}
		else // No ZyppPkg - some other kind of object (script, message)
		{
		    if ( zypp::isKind<zypp::Script> ( *it ) )
		    {
			y2debug( "Found patch script %s", (*it)->name().c_str() );
			emit filterMatch( _( "Script" ),  fromUTF8( (*it)->name() ), -1 );
		    }
		    else if ( zypp::isKind<zypp::Message> ( *it ) )
		    {
			y2debug( "Found patch message %s (ignoring)", (*it)->name().c_str() );
		    }
		    else
		    {
			y2error( "Found unknown object of kind %s in patch: %s-%s arch: %s",
				 (*it)->kind().asString().c_str(),
				 (*it)->name().c_str(),
				 (*it)->edition().asString().c_str(),
				 (*it)->arch().asString().c_str() );
		    }
		}
	    }
	}
    }

    emit filterFinished();
}


void
YQPkgPatchList::addPatchItem( ZyppSel	selectable,
			      ZyppPatch zyppPatch )
{
    if ( ! selectable )
    {
	y2error( "NULL ZyppSel!" );
	return;
    }

    YQPkgPatchListItem * item = new YQPkgPatchListItem( this, selectable, zyppPatch );
    applyExcludeRules( item );
}


bool
YQPkgPatchList::haveUpdateStackPatches()
{
    for ( ZyppPoolIterator it = zyppPatchesBegin();
	  it != zyppPatchesEnd();
	  ++it )
    {
	ZyppSel selectable = *it;
	ZyppPatch zyppPatch = tryCastToZyppPatch( selectable->theObj() );

	if ( zyppPatch && zyppPatch->affects_pkg_manager() )
	{
#if DEBUG_SHOW_ALL_UPDATE_STACK_PATCHES
	    return true;
#else
	    if ( selectable->hasCandidateObj() &&
		 selectable->candidatePoolItem().status().isNeeded() )
	    {
		return true;
	    }
#endif
	}
    }

    return false;
}


YQPkgPatchListItem *
YQPkgPatchList::selection() const
{
    QListViewItem * item = selectedItem();

    if ( ! item )
	return 0;

    return dynamic_cast<YQPkgPatchListItem *> (item);
}


void
YQPkgPatchList::createNotInstalledContextMenu()
{
    _notInstalledContextMenu = new QPopupMenu( this );
    CHECK_PTR( _notInstalledContextMenu );

    actionSetCurrentInstall->addTo( _notInstalledContextMenu );
    actionSetCurrentDontInstall->addTo( _notInstalledContextMenu );
    actionSetCurrentTaboo->addTo( _notInstalledContextMenu );

    addAllInListSubMenu( _notInstalledContextMenu );
}


void
YQPkgPatchList::createInstalledContextMenu()
{
    _installedContextMenu = new QPopupMenu( this );
    CHECK_PTR( _installedContextMenu );

    actionSetCurrentKeepInstalled->addTo( _installedContextMenu );

#if ENABLE_DELETING_PATCHES
    actionSetCurrentDelete->addTo( _installedContextMenu );
#endif

    actionSetCurrentUpdate->addTo( _installedContextMenu );
    actionSetCurrentProtected->addTo( _installedContextMenu );

    addAllInListSubMenu( _installedContextMenu );
}


QPopupMenu *
YQPkgPatchList::addAllInListSubMenu( QPopupMenu * menu )
{
    QPopupMenu * submenu = new QPopupMenu( menu );
    CHECK_PTR( submenu );

    actionSetListInstall->addTo( submenu );
    actionSetListDontInstall->addTo( submenu );
    actionSetListKeepInstalled->addTo( submenu );

#if ENABLE_DELETING_PATCHES
    actionSetListDelete->addTo( submenu );
#endif

    actionSetListUpdate->addTo( submenu );
    actionSetListUpdateForce->addTo( submenu );
    actionSetListTaboo->addTo( submenu );
    actionSetListProtected->addTo( submenu );

    menu->insertItem( _( "&All in This List" ), submenu );

    return submenu;
}


void
YQPkgPatchList::keyPressEvent( QKeyEvent * event )
{
    if ( event )
    {
#if ! ENABLE_DELETING_PATCHES
	if ( event->ascii() == '-' )
	{
	    QListViewItem * selectedListViewItem = selectedItem();

	    if ( selectedListViewItem )
	    {
		YQPkgPatchListItem * item = dynamic_cast<YQPkgPatchListItem *> (selectedListViewItem);

		if ( item && item->selectable()->hasInstalledObj() )
		{
		    y2warning( "Deleting patches is not supported" );
		    return;
		}
	    }
	}
#endif
    }

    YQPkgObjList::keyPressEvent( event );
}




YQPkgPatchListItem::YQPkgPatchListItem( YQPkgPatchList *	patchList,
					ZyppSel			selectable,
					ZyppPatch		zyppPatch )
    : YQPkgObjListItem( patchList, selectable, zyppPatch )
    , _patchList( patchList )
    , _zyppPatch( zyppPatch )
{
    if ( ! _zyppPatch )
	_zyppPatch = tryCastToZyppPatch( selectable->theObj() );

    if ( ! _zyppPatch )
	return;

    setStatusIcon();
    _patchCategory = patchCategory( _zyppPatch->category() );

    if ( categoryCol() > -1 )
	setText( categoryCol(), asString( _patchCategory ) );

    if ( summaryCol() > -1 && _zyppPatch->summary().empty() )
	setText( summaryCol(), _zyppPatch->name() );		// use name as fallback

    switch ( _patchCategory )
    {
	case YQPkgYaSTPatch:		setTextColor( QColor( 0, 0, 0xC0 ) );	break;	// medium blue
	case YQPkgSecurityPatch:	setTextColor( Qt::red );		break;
	case YQPkgRecommendedPatch:	setTextColor( QColor( 0, 0, 0xC0 ) );	break;	// medium blue
	case YQPkgOptionalPatch:	break;
	case YQPkgDocumentPatch:	break;
	case YQPkgUnknownPatchCategory: break;
    }
}


YQPkgPatchListItem::~YQPkgPatchListItem()
{
    // NOP
}


YQPkgPatchCategory
YQPkgPatchListItem::patchCategory( QString category )
{
    category = category.lower();

    if ( category == "yast"		) return YQPkgYaSTPatch;
    if ( category == "security"		) return YQPkgSecurityPatch;
    if ( category == "recommended"	) return YQPkgRecommendedPatch;
    if ( category == "optional"		) return YQPkgOptionalPatch;
    if ( category == "document"		) return YQPkgDocumentPatch;

    y2warning( "Unknown patch category \"%s\"", (const char *) category );
    return YQPkgUnknownPatchCategory;
}


QString
YQPkgPatchListItem::asString( YQPkgPatchCategory category )
{
    switch ( category )
    {
	// Translators: These are patch categories
	case YQPkgYaSTPatch:		return _( "YaST"	);
	case YQPkgSecurityPatch:	return _( "security"	);
	case YQPkgRecommendedPatch:	return _( "recommended" );
	case YQPkgOptionalPatch:	return _( "optional"	);
	case YQPkgDocumentPatch:	return _( "document"	);
	case YQPkgUnknownPatchCategory: return "";
    }

    return "";
}


void
YQPkgPatchListItem::cycleStatus()
{
    YQPkgObjListItem::cycleStatus();

#if ! ENABLE_DELETING_PATCHES
    if ( status() == S_Del )	// Can't delete patches
	setStatus( S_KeepInstalled );
#endif
}


QString
YQPkgPatchListItem::toolTip( int col )
{
    QString text;

    if ( col == statusCol() )
    {
	text = YQPkgObjListItem::toolTip( col );
    }
    else
    {
	if (  ( col == brokenIconCol()	  && isBroken()	   ) ||
	      ( col == satisfiedIconCol() && isSatisfied() )   )
	{
	    text = YQPkgObjListItem::toolTip( col );
	}
	else
	{
	    text = fromUTF8( zyppPatch()->category() );

	    if ( ! text.isEmpty() )
		text += "\n";

	    text += fromUTF8( zyppPatch()->size().asString().c_str() );
	}
    }

    return text;
}


void
YQPkgPatchListItem::applyChanges()
{
    solveResolvableCollections();
}


/**
 * Comparison function used for sorting the list.
 * Returns:
 * -1 if this <	 other
 *  0 if this == other
 * +1 if this >	 other
 **/
int
YQPkgPatchListItem::compare( QListViewItem *	otherListViewItem,
			     int		col,
			     bool		ascending ) const
{
    YQPkgPatchListItem * other = dynamic_cast<YQPkgPatchListItem *> (otherListViewItem);

    if ( other )
    {
	if ( col == categoryCol() )
	{
	    if ( this->patchCategory() < other->patchCategory() ) return -1;
	    if ( this->patchCategory() > other->patchCategory() ) return  1;
	    return 0;
	}
    }
    return YQPkgObjListItem::compare( otherListViewItem, col, ascending );
}



#include "YQPkgPatchList.moc"
