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

  File:	      YQPkgRepoList.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#ifndef YQPkgRepoList_h
#define YQPkgRepoList_h

#include "YQZypp.h"
#include "QY2ListView.h"
#include <zypp/Repository.h>
#include <zypp/Product.h>


class YQPkgRepoListItem;

typedef zypp::Repository	ZyppRepo;


/**
 * @short Display a list of zypp::Selection objects.
 **/
class YQPkgRepoList : public QY2ListView
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgRepoList( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgRepoList();

    /**
     * Returns the number of enabled repositories.
     **/
    static int countEnabledRepositories();


public slots:

    /**
     * Filter according to the view's rules and current selection.
     * Emits those signals:
     *    filterStart()
     *    filterMatch() for each pkg that matches the filter
     *    filterFinished()
     **/
    void filter();

    /**
     * Same as filter(), but only if this widget is currently visible.
     **/
    void filterIfVisible();

    /**
     * Add a repository to the list.
     **/
    void addRepo( ZyppRepo repo );


public:

    // Column numbers

    int nameCol()	const	{ return _nameCol;	}
    int urlCol()	const	{ return _urlCol;	}


    /**
     * Returns the currently selected item or 0 if there is none.
     **/
    YQPkgRepoListItem * selection() const;


signals:

    /**
     * Emitted when the filtering starts. Use this to clear package lists
     * etc. prior to adding new entries.
     **/
    void filterStart();

    /**
     * Emitted during filtering for each pkg that matches the filter
     * and the candidate package comes from the respective repository
     **/
    void filterMatch( ZyppSel	selectable,
		      ZyppPkg	pkg );

    /**
     * Emitted during filtering for each pkg that matches the filter
     * and the candidate package does not come from the respective repository
     **/
    void filterNearMatch( ZyppSel	selectable,
			  ZyppPkg	pkg );

    /**
     * Emitted when filtering is finished.
     **/
    void filterFinished();


protected slots:

    /**
     * Fill the list.
     **/
    void fillList();


private:

    //
    // Data members
    //

    int	_nameCol;
    int	_urlCol;

};



class YQPkgRepoListItem: public QY2ListViewItem
{
public:

    /**
     * Constructor
     **/
    YQPkgRepoListItem( YQPkgRepoList *parentList, ZyppRepo repo );

    /**
     * Destructor
     **/
    virtual ~YQPkgRepoListItem();

    /**
     * Returns the ZYPP repository this item corresponds to
     **/
    ZyppRepo zyppRepo() const { return _zyppRepo; }

    /**
     * Returns the parent list
     **/
    const YQPkgRepoList * repoList() const { return _repoList; }

    /**
     * Returns the product on a source if it has one single product
     * or 0 if there are no or multiple products.
     **/
    static ZyppProduct singleProduct( ZyppRepo repo );


    // Columns

    int nameCol()	const	{ return _repoList->nameCol();	}
    int urlCol()	const 	{ return _repoList->urlCol(); 	}

    virtual bool operator< ( const QTreeWidgetItem & other ) const;
protected:

    // Data members

    YQPkgRepoList *		_repoList;
    ZyppRepo			_zyppRepo;
};


#endif // ifndef YQPkgRepoList_h
