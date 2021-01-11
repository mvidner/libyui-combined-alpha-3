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

  File:	      YQPkgLangList.cc

  Author:     Stefan Hundhammer <sh@suse.de>

  Textdomain "qt-pkg"

/-*/


#define YUILogComponent "qt-pkg"
#include "YUILog.h"
#include <qregexp.h>

#include <QHeaderView>
#include "YQi18n.h"
#include "utf8.h"
#include "YQPkgLangList.h"

#include "zypp/sat/LocaleSupport.h"

using std::set;
using std::endl;


YQPkgLangList::YQPkgLangList( QWidget * parent )
    : YQPkgObjList( parent )
//FIXME "base class works with zypp::Resolvable, but zypp::Locale isn't one any longer!"
{
    yuiDebug() << "Creating language list" << endl;

    int numCol = 0;
    QStringList headers;
    headers <<  "";	_statusCol	= numCol++;

    // Translators: Table column heading for language ISO code like "de_DE", "en_US"
    // Please keep this short to avoid stretching the column too wide!
    headers <<  _( "Code"	);	_nameCol	= numCol++;

    // Full (human readable) language / country name like "German (Austria)"
    headers <<  _( "Language");	_summaryCol	= numCol++;
    setAllColumnsShowFocus( true );
    setHeaderLabels(headers);
    header()->setSectionResizeMode( _nameCol, QHeaderView::ResizeToContents );
    header()->setSectionResizeMode( _summaryCol, QHeaderView::Stretch );


    setSortingEnabled( true );
    header()->setSortIndicatorShown( true );
    header()->setSectionsClickable( true );

    sortByColumn( nameCol(), Qt::AscendingOrder );

    connect( this, 	SIGNAL( currentItemChanged        ( QTreeWidgetItem *, QTreeWidgetItem * ) ),
	     this, 	SLOT  ( filter()                                    ) );

    fillList();
    selectSomething();
    resizeColumnToContents(_statusCol);

    yuiDebug() << "Creating language list done" << endl;
}


YQPkgLangList::~YQPkgLangList()
{
    // NOP
}


void
YQPkgLangList::fillList()
{
    clear();
    yuiDebug() << "Filling language list" << endl;

    zypp::LocaleSet locales = zypp::getZYpp()->pool().getAvailableLocales();

    for ( zypp::LocaleSet::const_iterator it = locales.begin();
	  it != locales.end();
	  ++it )
    {
        addLangItem( *it );
    }

    yuiDebug() << "Language list filled" << endl;
}


void
YQPkgLangList::filterIfVisible()
{
    if ( isVisible() )
	filter();
}


void
YQPkgLangList::filter()
{
    emit filterStart();

    if ( selection() )
    {
        int total = 0;
        int installed = 0;

        zypp::Locale lang = selection()->zyppLang();

        zypp::sat::LocaleSupport myLocale( lang );
        for_( it, myLocale.selectableBegin(), myLocale.selectableEnd() )
        {
            ZyppPkg zyppPkg = tryCastToZyppPkg( (*it)->theObj() );
            if ( zyppPkg )
            {
                if ( (*it)->installedSize() > 0 )
                    ++installed;
                ++total;

                emit filterMatch( *it, zyppPkg );
            }
        }
    }
    emit filterFinished();
}

void
YQPkgLangList::addLangItem( const zypp::Locale &zyppLang )
{
    new YQPkgLangListItem( this, zyppLang );
}


YQPkgLangListItem *
YQPkgLangList::selection() const
{
    QTreeWidgetItem * item = currentItem();

    if ( ! item )
	return 0;

    return dynamic_cast<YQPkgLangListItem *> (item);
}


void
YQPkgLangList::updateActions( YQPkgObjListItem * item)
{
#if 0
    YQPkgLangListItem *litem;
    if ( !item)
        litem = dynamic_cast<YQPkgLangListItem *> ( currentItem() );
    else
        litem = dynamic_cast<YQPkgLangListItem *> ( item );
#endif

    actionSetCurrentInstall->setEnabled( true );
    actionSetCurrentDontInstall->setEnabled( true );
    actionSetCurrentTaboo->setEnabled( true );
    actionSetCurrentProtected->setEnabled( false );

    actionSetCurrentKeepInstalled->setEnabled( false );
    actionSetCurrentDelete->setEnabled( false );
    actionSetCurrentUpdate->setEnabled( false );
}


YQPkgLangListItem::YQPkgLangListItem( YQPkgLangList * 	   langList,
                                      const zypp::Locale & lang )
    : YQPkgObjListItem( langList )
    , _zyppLang( lang )
{
    init();
}



YQPkgLangListItem::~YQPkgLangListItem()
{
    // NOP
}


void
YQPkgLangListItem::applyChanges()
{
    solveResolvableCollections();
}


void
YQPkgLangListItem::init()
{
//FIXME this is utterly broken - see bug #370233
    // DO NOT CALL PARENT CLASS
    _debugIsBroken	= false;
    _debugIsSatisfied	= false;
    _candidateIsNewer	= false;
    _installedIsNewer 	= false;

    if ( nameCol()    >= 0 )	setText( nameCol(),	_zyppLang.code()	);
    if ( summaryCol() >= 0 )	setText( summaryCol(),	_zyppLang.name()	);

    setStatusIcon();
}


ZyppStatus
YQPkgLangListItem::status() const
{
    if ( zypp::getZYpp()->pool().isRequestedLocale( _zyppLang ) )
        return S_Install;
    else
        return S_NoInst;
}

void
YQPkgLangListItem::setStatus( ZyppStatus newStatus, bool sendSignals )
{
    ZyppStatus oldStatus = status();

    switch ( newStatus )
    {
        case S_Install:
            if ( ! zypp::getZYpp()->pool().isRequestedLocale( _zyppLang ) )
            {
                zypp::getZYpp()->pool().addRequestedLocale( _zyppLang );
            }
            break;
        case S_NoInst:
            if ( zypp::getZYpp()->pool().isRequestedLocale( _zyppLang ) )
            {
                zypp::getZYpp()->pool().eraseRequestedLocale( _zyppLang );
            }
            break;
        default:
            return;
    }

    if ( oldStatus != newStatus )
    {
	applyChanges();

	if ( sendSignals )
	{
	    _pkgObjList->updateItemStates();
	    _pkgObjList->sendUpdatePackages();
	}
    }

    setStatusIcon();
    _pkgObjList->sendStatusChanged();
}

bool
YQPkgLangListItem::bySelection() const
{
    return zypp::getZYpp()->pool().isRequestedLocale( _zyppLang );
}


void
YQPkgLangListItem::cycleStatus()
{
    if ( zypp::getZYpp()->pool().isRequestedLocale( _zyppLang ) )
    {
        zypp::getZYpp()->pool().eraseRequestedLocale( _zyppLang );
    }
    else
    {
        zypp::getZYpp()->pool().addRequestedLocale( _zyppLang );
    }
    setStatusIcon();
    _pkgObjList->sendStatusChanged();
}

bool YQPkgLangListItem::operator<( const QTreeWidgetItem & otherListViewItem ) const
{
    const YQPkgLangListItem * other = dynamic_cast<const YQPkgLangListItem *> (&otherListViewItem);
    int col = treeWidget()->sortColumn();

    if ( other )
    {
        if ( col == nameCol() )
	{
	    return ( strcoll( this->zyppLang().code().c_str(), other->zyppLang().code().c_str() ) < 0 );
	}
	if ( col == summaryCol() )
	{
            return ( strcoll( this->zyppLang().name().c_str(), other->zyppLang().name().c_str() ) < 0 );
	}
    }

    return QY2ListViewItem::operator<( otherListViewItem );
}
