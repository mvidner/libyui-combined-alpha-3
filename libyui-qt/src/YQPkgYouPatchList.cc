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

  File:	      YQPkgYouPatchList.cc

  Author:     Stefan Hundhammer <sh@suse.de>

  Textdomain "packages-qt"

/-*/


#define y2log_component "qt-pkg"
#include <ycp/y2log.h>

#include <qpopupmenu.h>
#include <qaction.h>

#include <Y2PM.h>
#include <y2pm/PMYouPatchManager.h>

#include "YQi18n.h"
#include "utf8.h"

#include "YQPkgYouPatchList.h"
#include "YQPkgTextDialog.h"

using std::list;


YQPkgYouPatchList::YQPkgYouPatchList( QWidget * parent )
    : YQPkgObjList( parent )
    , _patchCategory( InstallablePatches )
{
    y2debug( "Creating YOU patch list" );

    int numCol = 0;
    addColumn( "" );					_statusCol	= numCol++;
    addColumn( _( "YaST Online Update Patch"	) );	_summaryCol	= numCol++;
    addColumn( _( "Kind" 			) );	_kindCol	= numCol++;
    addColumn( _( "Size" 			) );	_sizeCol	= numCol++;
    setAllColumnsShowFocus( true );
    setColumnAlignment( sizeCol(), Qt::AlignRight );

    connect( this,	SIGNAL( selectionChanged	( QListViewItem * ) ),
	     this,	SLOT  ( filter()				    ) );

    fillList();
    setSorting( kindCol() );
    selectSomething();

    QString label = _( "Show &Raw Patch Info" );
    actionShowRawPatchInfo = new QAction( label,		// text
					  label + "\tr",	// menu text
					  ( QKeySequence ) 0,	// accel
					  ( QObject * ) 0 );	// parent

    connect( actionShowRawPatchInfo, SIGNAL( activated() ), SLOT( showRawPatchInfo() ) );

    y2debug( "Creating YOU patch list done" );
}


YQPkgYouPatchList::~YQPkgYouPatchList()
{
    // NOP
}


void
YQPkgYouPatchList::fillList()
{
    clear();
    y2debug( "Filling YOU patch list" );

    PMManager::PMSelectableVec::const_iterator it = Y2PM::youPatchManager().begin();

    while ( it != Y2PM::youPatchManager().end() )
    {
	PMYouPatchPtr		patch  = ( *it)->theObject();
	PMSelectable::UI_Status	status =( *it)->status();

	if ( patch )
	{
	    switch ( _patchCategory )
	    {
		case InstallablePatches:
		    if ( patch->installable() && status != PMSelectable::S_KeepInstalled )
			addYouPatchItem( patch );
		    break;

		case InstallableAndInstalledPatches:
		    if ( patch->installable() )
			addYouPatchItem( patch );
		    break;

		case AllPatches:
		    addYouPatchItem( patch );
	    }
	}

	++it;
    }

    if ( ! firstChild() )
	message( _( "No patches available." ) );

    y2debug( "YOU patch list filled" );
}



void
YQPkgYouPatchList::message( const QString & text )
{
    QY2ListViewItem * item = new QY2ListViewItem( this );
    CHECK_PTR( item );

    item->setText( 1, text );
    item->setBackgroundColor( QColor( 0xE0, 0xE0, 0xF8 ) );
}



void
YQPkgYouPatchList::filterIfVisible()
{
    if ( isVisible() )
	filter();
}


void
YQPkgYouPatchList::filter()
{
    emit filterStart();

    if ( selection() )
    {
	PMYouPatchPtr patch = selection()->pmYouPatch();

	if ( patch )
	{
	    //
	    // Check for a pre-script
	    //

	    if ( ! patch->preScript().empty() )
	    {
		// Translators: (Fixed) name for a script that is executed
		// at the start of installation of a YOU patch
		emit filterMatch( _( "[Pre-Script]" ), fromUTF8( patch->preScript() ), -1 );
	    }


	    //
	    // Add all packages
	    //

	    list<PMPackagePtr> pkgList = patch->packages();
	    list<PMPackagePtr>::const_iterator it = pkgList.begin();

	    while ( it != pkgList.end() )
	    {
		emit filterMatch( ( *it ) );
		++it;
	    }


	    //
	    // Check for extra files outside packages
	    //

	    list<PMYouFile> files = patch->files();

	    for ( list<PMYouFile>::iterator it = files.begin(); it != files.end(); ++it )
	    {
		// Translators: (Fixed) name for an extra file (outside packages)
                // that comes with a YOU patch
		emit filterMatch( _( "[File]" ), fromUTF8( (*it).name() ), (*it).size() );
	    }


	    //
	    // Check for a post-script
	    //

	    if ( ! patch->postScript().empty() )
	    {
		// Translators: (Fixed) name for a script that is executed
                // at the end of installation of a YOU patch
		emit filterMatch( _( "[Post-Script]" ), fromUTF8( patch->postScript() ), -1 );
	    }
	}
    }

    emit filterFinished();
}


void
YQPkgYouPatchList::addYouPatchItem( PMYouPatchPtr pmYouPatch )
{
    if ( ! pmYouPatch )
    {
	y2error( "NULL PMYouPatch!" );
	return;
    }

    new YQPkgYouPatchListItem( this, pmYouPatch );
}


YQPkgYouPatchListItem *
YQPkgYouPatchList::selection() const
{
    QListViewItem * item = selectedItem();

    if ( ! item )
	return 0;

    return dynamic_cast<YQPkgYouPatchListItem *> (item);
}


void
YQPkgYouPatchList::createNotInstalledContextMenu()
{
    _notInstalledContextMenu = new QPopupMenu( this );
    CHECK_PTR( _notInstalledContextMenu );

    actionSetCurrentInstall->addTo( _notInstalledContextMenu );
    actionSetCurrentDontInstall->addTo( _notInstalledContextMenu );
    actionSetCurrentTaboo->addTo( _notInstalledContextMenu );

    addAllInListSubMenu( _notInstalledContextMenu );
}


void
YQPkgYouPatchList::createInstalledContextMenu()
{
    _installedContextMenu = new QPopupMenu( this );
    CHECK_PTR( _installedContextMenu );

    actionSetCurrentKeepInstalled->addTo( _installedContextMenu );
    actionSetCurrentUpdate->addTo( _installedContextMenu );
    actionSetCurrentProtected->addTo( _installedContextMenu );

    addAllInListSubMenu( _installedContextMenu );
}


QPopupMenu *
YQPkgYouPatchList::addAllInListSubMenu( QPopupMenu * menu )
{
    QPopupMenu * submenu = new QPopupMenu( menu );
    CHECK_PTR( submenu );

    actionSetListInstall->addTo( submenu );
    actionSetListDontInstall->addTo( submenu );
    actionSetListKeepInstalled->addTo( submenu );
    actionSetListUpdate->addTo( submenu );
    actionSetListUpdateForce->addTo( submenu );
    actionSetListTaboo->addTo( submenu );
    actionSetListProtected->addTo( submenu );

    menu->insertItem( _( "&All in This List" ), submenu );

    return submenu;
}


void
YQPkgYouPatchList::showRawPatchInfo()
{
    if ( selection() )
    {
	PMYouPatchPtr patch = selection()->pmYouPatch();
	YQPkgTextDialog::showText( this, patch, Y2PM::youPatchManager().rawPatchInfo( patch ) );
    }
}


void
YQPkgYouPatchList::keyPressEvent( QKeyEvent * event )
{
    if ( event )
    {
	if ( event->ascii() == '-' )
	{
	    QListViewItem * selectedListViewItem = selectedItem();

	    if ( selectedListViewItem )
	    {
		YQPkgYouPatchListItem * item = dynamic_cast<YQPkgYouPatchListItem *> (selectedListViewItem);

		if ( item && item->pmObj()->hasInstalledObj() )
		{
		    y2warning( "Deleting patches is not supported" );
		    return;
		}
	    }
	}
	else if ( event->ascii() == 'r' )
	{
	    showRawPatchInfo();
	}
    }

    YQPkgObjList::keyPressEvent( event );
}




YQPkgYouPatchListItem::YQPkgYouPatchListItem( YQPkgYouPatchList * youPatchList, PMYouPatchPtr youPatch )
    : YQPkgObjListItem( youPatchList, youPatch )
    , _youPatchList( youPatchList )
    , _pmYouPatch( youPatch )
{
    setText( kindCol(), _pmYouPatch->kindLabel() );
    setStatusIcon();

    switch ( _pmYouPatch->kind() )
    {
	case PMYouPatch::kind_yast:		setTextColor( QColor( 0, 0, 0xC0 ) );	break;	// medium blue
	case PMYouPatch::kind_security:		setTextColor( Qt::red );		break;
	case PMYouPatch::kind_recommended:	setTextColor( QColor( 0, 0, 0xC0 ) );	break;	// medium blue
	case PMYouPatch::kind_optional:		break;
	case PMYouPatch::kind_document:		break;
	default:				break;
    }
}


YQPkgYouPatchListItem::~YQPkgYouPatchListItem()
{
    // NOP
}


void
YQPkgYouPatchListItem::setStatus( PMSelectable::UI_Status newStatus )
{
    YQPkgObjListItem::setStatus( newStatus );
    Y2PM::youPatchManager().updatePackageStates();
    _youPatchList->sendUpdatePackages();
}


void
YQPkgYouPatchListItem::cycleStatus()
{
    YQPkgObjListItem::cycleStatus();

    if ( status() == PMSelectable::S_Del )	// Can't delete YOU patches
	setStatus( PMSelectable::S_KeepInstalled );
}


QString
YQPkgYouPatchListItem::toolTip( int col )
{
    QString text;

    if ( col == statusCol() )
    {
	text = YQPkgObjListItem::toolTip( col );
    }
    else
    {
	text = fromUTF8( _pmYouPatch->kindLabel().c_str() );

	if ( ! text.isEmpty() )
	    text += "\n";

	text += fromUTF8( _pmYouPatch->size().form().c_str() );
    }

    return text;
}


/**
 * Comparison function used for sorting the list.
 * Returns:
 * -1 if this <	 other
 *  0 if this == other
 * +1 if this >	 other
 **/
int
YQPkgYouPatchListItem::compare( QListViewItem * otherListViewItem,
				int		col,
				bool		ascending ) const
{
    YQPkgYouPatchListItem * other = dynamic_cast<YQPkgYouPatchListItem *> (otherListViewItem);

    if ( other )
    {
	if ( col == kindCol() )
	{
	    if ( this->constPMYouPatch()->kind() < other->constPMYouPatch()->kind() ) return -1;
	    if ( this->constPMYouPatch()->kind() > other->constPMYouPatch()->kind() ) return 1;
	    return 0;
	}
    }
    return YQPkgObjListItem::compare( otherListViewItem, col, ascending );
}



#include "YQPkgYouPatchList.moc"
