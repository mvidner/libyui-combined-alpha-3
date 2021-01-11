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

  File:		YQPkgClassFilterView.cc

  Authors:	Duncan Mac-Vicar Prett <duncan@suse.de>
		Stefan Hundhammer <sh@suse.de>

  Textdomain "qt-pkg"

/-*/


#define YUILogComponent "qt-pkg"
#include "YUILog.h"

#include <QApplication>

#include "YQPackageSelector.h"

#include "YQPkgClassFilterView.h"
#include "YQi18n.h"
#include "utf8.h"

using std::string;


QString
translatedText( YQPkgClass pkgClass )
{
    switch ( pkgClass )
    {
	case YQPkgClassNone:               return _( "No Packages"           	    );
	case YQPkgClassRecommended:	   return _( "Recommended Packages"	    );
	case YQPkgClassSuggested:	   return _( "Suggested Packages"	    );
	case YQPkgClassOrphaned:	   return _( "Orphaned Packages"	    );
	case YQPkgClassUnneeded:	   return _( "Unneeded Packages"	    );
	case YQPkgClassMultiversion:	   return _( "Multiversion Packages"	    );
	case YQPkgClassRetracted:	   return _( "Retracted Packages"	    );
	case YQPkgClassRetractedInstalled: return _( "Retracted Installed Packages" );
	case YQPkgClassAll:		   return _( "All Packages"		    );

	// Intentionally omitting 'default' case so gcc can catch unhandled enums
    }

    // this should never be reached, not marked for translation
    return "Unknown PkgClass";
}


string
pkgClassIcon( YQPkgClass pkgClass )
{
    switch ( pkgClass )
    {
	case YQPkgClassAll:
	    return( "preferences-other" );

	default:
	    return( "preferences-desktop-locale" );
    }

    return( "" );
}


YQPkgClassFilterView::YQPkgClassFilterView( QWidget * parent )
    : QTreeWidget( parent )
{
    setIconSize( QSize( 32, 32 ) );
    setHeaderLabels( QStringList( _( "Package Classification" ) ) );
    setRootIsDecorated( false );
    setSortingEnabled( true );

    connect( this, SIGNAL( currentItemChanged	( QTreeWidgetItem *, QTreeWidgetItem * ) ),
	     this, SLOT	 ( slotSelectionChanged ( QTreeWidgetItem * ) ) );

    fillPkgClasses();
}


YQPkgClassFilterView::~YQPkgClassFilterView()
{
}


void
YQPkgClassFilterView::fillPkgClasses()
{
    new YQPkgClassItem( this, YQPkgClassRecommended	   );
    new YQPkgClassItem( this, YQPkgClassSuggested	   );
    new YQPkgClassItem( this, YQPkgClassOrphaned	   );
    new YQPkgClassItem( this, YQPkgClassUnneeded	   );
    new YQPkgClassItem( this, YQPkgClassRetracted	   );
    new YQPkgClassItem( this, YQPkgClassRetractedInstalled );

    if ( ! zypp::sat::Pool::instance().multiversion().empty() )
	new YQPkgClassItem( this, YQPkgClassMultiversion );

    new YQPkgClassItem( this, YQPkgClassAll );
}


void
YQPkgClassFilterView::filterIfVisible()
{
    if ( isVisible() )
	filter();
}


void
YQPkgClassFilterView::filter()
{
    emit filterStart();

    if ( selectedPkgClass() != YQPkgClassNone )
    {
	for ( ZyppPoolIterator it = zyppPkgBegin();
	      it != zyppPkgEnd();
	      ++it )
	{
	    ZyppSel selectable = *it;
	    bool match = false;

	    // If there is an installed obj, check this first. The bits are set for the installed
	    // obj only and the installed obj is not contained in the pick list if there in an
	    // identical candidate available from a repo.

	    if ( selectable->installedObj() )
	    {
		match = check( selectable, tryCastToZyppPkg( selectable->installedObj() ) );
	    }
	    if ( selectable->candidateObj() && ! match )
	    {
		match = check( selectable, tryCastToZyppPkg( selectable->candidateObj() ) );
	    }

	    // And then check the pick list which contain all availables and all objects for multi
	    // version packages and the installed obj if there isn't same version in a repo.

	    if ( ! match )
	    {
		zypp::ui::Selectable::picklist_iterator it = selectable->picklistBegin();

		while ( it != selectable->picklistEnd() && ! match )
		{
		    check( selectable, tryCastToZyppPkg( *it ) );
		    ++it;
		}
	    }
	}
    }

    emit filterFinished();
}


void
YQPkgClassFilterView::slotSelectionChanged( QTreeWidgetItem * newSelection )
{
    YQPkgClassItem * sel = dynamic_cast<YQPkgClassItem *>( newSelection );

    if ( sel )
    {
        bool needSolverRun;

        switch ( sel->pkgClass() )
        {
            case YQPkgClassRecommended:
            case YQPkgClassSuggested:
            case YQPkgClassOrphaned:
            case YQPkgClassUnneeded:
                needSolverRun = true;
                break;

            default:
                needSolverRun = false;
                break;
        }

	if ( needSolverRun )
	{
	    QApplication::setOverrideCursor(Qt::WaitCursor);
	    zypp::getZYpp()->resolver()->resolvePool();
	    QApplication::restoreOverrideCursor();
	}
    }

    filter();
}


bool
YQPkgClassFilterView::check( ZyppSel selectable, ZyppPkg pkg )
{
    bool match = checkMatch( selectable, pkg );

    if ( match )
	emit filterMatch( selectable, pkg );

    return match;
}


bool
YQPkgClassFilterView::checkMatch( ZyppSel selectable, ZyppPkg pkg )
{
    if ( ! pkg )
	return false;

    switch ( selectedPkgClass() )
    {
        case YQPkgClassNone:                    return false;
	case YQPkgClassRecommended:		return zypp::PoolItem( pkg ).status().isRecommended();
	case YQPkgClassSuggested:		return zypp::PoolItem( pkg ).status().isSuggested();
	case YQPkgClassOrphaned:		return zypp::PoolItem( pkg ).status().isOrphaned();
	case YQPkgClassUnneeded:		return zypp::PoolItem( pkg ).status().isUnneeded();
	case YQPkgClassMultiversion:		return selectable->multiversionInstall();
	case YQPkgClassRetracted:		return selectable->hasRetracted();
	case YQPkgClassRetractedInstalled:	return selectable->hasRetractedInstalled();
	case YQPkgClassAll:			return true;

        // No 'default' branch to let the compiler catch unhandled enum values
    }

    return false;
}


YQPkgClass
YQPkgClassFilterView::selectedPkgClass() const
{
    QTreeWidgetItem * qItem = currentItem();

    if ( ! qItem )
	return YQPkgClassNone;

    YQPkgClassItem * pkgClassItem = dynamic_cast<YQPkgClassItem *> ( qItem );

    if ( ! pkgClassItem )
        return YQPkgClassNone;
    else
        return pkgClassItem->pkgClass();
}


void
YQPkgClassFilterView::showPkgClass( YQPkgClass pkgClass )
{
    QTreeWidgetItemIterator it( this );

    while ( *it )
    {
        YQPkgClassItem * item = dynamic_cast<YQPkgClassItem *>( *it );

        if ( item && item->pkgClass() == pkgClass )
        {
            setCurrentItem( item );
            // This will also send the currentItemChanged() signal which will
            // start filtering, i.e. it will populate the package list.
        }

        ++it;
    }
}






YQPkgClassItem::YQPkgClassItem( YQPkgClassFilterView * parentFilterView,
				YQPkgClass pkgClass )
    : QTreeWidgetItem( parentFilterView )
    , _pkgClass( pkgClass )
{
    setText( 0, translatedText( pkgClass ) );
}


YQPkgClassItem::~YQPkgClassItem()
{
    // NOP
}


bool
YQPkgClassItem::operator< ( const QTreeWidgetItem & otherListViewItem ) const
{
    const YQPkgClassItem * otherCategoryItem =
	dynamic_cast<const YQPkgClassItem *>(&otherListViewItem);

    if ( otherCategoryItem )
	return pkgClass() > otherCategoryItem->pkgClass();
    else
	return true;
}


