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

  File:	      YQPkgList.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#ifndef YQPkgList_h
#define YQPkgList_h

#include <YQPkgObjList.h>
#include <QMenu>
#include <QResizeEvent>

class YQPkgListItem;


/**
 * @short Display a list of zypp::Package objects.
 **/
class YQPkgList : public YQPkgObjList
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgList( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgList();


    // Column numbers

    int srpmStatusCol() 	const	{ return _srpmStatusCol; 	}

    /**
     * Save the pkg list to a file.
     *
     * Posts error popups if 'interactive' is 'true' ( only log entries
     * otherwise ).
     **/
    void exportList( const QString filename, bool interactive ) const;

    /**
     * Add a submenu "All in this list..." to 'menu'.
     * Returns the newly created submenu.
     *
     * Reimplemented from YQPkgObjList.
     **/
    virtual QMenu * addAllInListSubMenu( QMenu * menu );

    /**
     * Returns 'true' if there are any installed packages.
     **/
    static bool haveInstalledPkgs();

    /**
     * Set the status of all packages in the pool to a new value.
     * This is not restricted to the current content of this package list.
     * All selectables in the ZYPP pool are affected.
     *
     * 'force' indicates if it should be done even if it is not very useful,
     * e.g., if packages should be updated even if there is no newer version.
     *
     * If 'countOnly' is 'true', the status is not actually changed, only the
     * number of packages that would be affected is return.
     *
     * Return value: The number of status changes
     **/
    int globalSetPkgStatus( ZyppStatus newStatus, bool force, bool countOnly );


public slots:

    /**
     * Add a pkg to the list. Connect a filter's filterMatch() signal to this
     * slot. Remember to connect filterStart() to clear() (inherited from
     * QListView).
     **/
    void addPkgItem	( ZyppSel	selectable,
			  ZyppPkg	zyppPkg	);

    /**
     * Add a pkg to the list, but display it dimmed (grey text foreground
     * rather than normal black).
     **/
    void addPkgItemDimmed( ZyppSel	selectable,
			   ZyppPkg 	zyppPkg );

    /**
     * Add a pkg to the list
     **/
    void addPkgItem	( ZyppSel	selectable,
			  ZyppPkg	zyppPkg,
			  bool 		dimmed );


    /**
     * Dispatcher slot for mouse click: Take care of source RPM status.
     * Let the parent class handle the normal status.
     * Reimplemented from YQPkgObjList.
     **/
    virtual void pkgObjClicked( int		button,
				QTreeWidgetItem *	item,
				int		col,
				const QPoint &	pos );

    /**
     * Update the internal actions: What actions are available for 'item'?
     *
     * Reimplemented from YQPkgObjList
     **/
    virtual void updateActions( YQPkgObjListItem * item );

    /**
     * Reimplemented from QListView / QWidget:
     * Reserve a reasonable amount of space.
     **/
    virtual QSize sizeHint() const;

    /**
     * Ask for a file name and save the current pkg list to file.
     **/
    void askExportList() const;


    // Direct access to some states for menu actions

    void setInstallCurrentSourceRpm()	  { setInstallCurrentSourceRpm( true  ); }
    void setDontInstallCurrentSourceRpm() { setInstallCurrentSourceRpm( false ); }

    void setInstallListSourceRpms()	  { setInstallListSourceRpms( true  ); }
    void setDontInstallListSourceRpms()	  { setInstallListSourceRpms( false ); }


    // No separate currentItemChanged( ZyppPkg ) signal:
    // Use YQPkgObjList::currentItemChanged( ZyppObj ) instead
    // and dynamic_cast to ZyppPkg if required.
    // This saves duplicating a lot of code.

    /**
     * Clears the tree-widgets content, resets the optimal column width values
     *
     * Reimplemented from QPkgObjList, calls QPkgObjList::reset()
     **/
    void clear();
    
    /**
     * Sort the tree widget again according to the column selected and
     * its current sort order.
     **/
    void resort();

    
protected:

    /**
     * Create ( additional ) actions for the context menus.
     **/
    void createActions();

    /**
     * Create the context menu for items that are not installed.
     *
     * Reimplemented from YQPkgObjList.
     **/
    virtual void createNotInstalledContextMenu();

    /**
     * Create the context menu for installed items.
     *
     * Reimplemented from YQPkgObjList.
     **/
    virtual void createInstalledContextMenu();

    /**
     * Create context menu for source RPMs.
     **/
    void createSourceRpmContextMenu();

    /**
     * Sets the currently selected item's source RPM status.
     * Automatically selects the next item if 'selectNextItem' is 'true'.
     **/
    void setInstallCurrentSourceRpm( bool inst, bool selectNextItem = false );

    /**
     * Sets the source RPM status of all items in this list.
     **/
    void setInstallListSourceRpms( bool inst );

    /**
     * Resets the optimal column width values.
     * Needed for empty list.
     **/
    void resetOptimalColumnWidthValues();
    
    /**
     * Set and save optimal column widths depending on content only
     * There is currently no way to get the optimal widths without setting them, so we have to do it.
     **/
    void updateOptimalColumnWidthValues(ZyppSel selectable, ZyppPkg zyppPkg);

    /**
     * Optimizes the column widths depending on content and the available horizontal space.
     **/
    void optimizeColumnWidths();

    /**
     * Handler for resize events.
     * Triggers column width optimization.
     **/
    void resizeEvent(QResizeEvent *event);


    // *** Data members:

    int			_srpmStatusCol;
    QMenu *		_sourceRpmContextMenu;
    // Optimal (sized-to-content) column widths:
    int _optimalColWidth_statusIcon;
    int _optimalColWidth_name;
    int _optimalColWidth_summary;
    int _optimalColWidth_version;
    int _optimalColWidth_instVersion;
    int _optimalColWidth_size;


public:

    QAction *		actionInstallSourceRpm;
    QAction *		actionDontInstallSourceRpm;
    QAction *		actionInstallListSourceRpms;
    QAction *		actionDontInstallListSourceRpms;
};



class YQPkgListItem: public YQPkgObjListItem
{
public:

    /**
     * Constructor. Creates a YQPkgList item that corresponds to the package
     * manager object that 'pkg' refers to.
     **/
    YQPkgListItem( YQPkgList *	pkgList,
		   ZyppSel	selectable,
		   ZyppPkg 	zyppPkg );

    /**
     * Destructor
     **/
    virtual ~YQPkgListItem();

    /**
     * Returns the parent package list.
     **/
    YQPkgList * pkgList() { return _pkgList; }

    /**
     * Returns the original object within the package manager backend.
     **/
    ZyppPkg zyppPkg() const { return _zyppPkg; }

    /**
     * Returns the source RPM package status:
     * Should the source RPM be installed?
     **/
    bool installSourceRpm() const;

    /**
     * Set the source RPM status
     **/
    void setInstallSourceRpm( bool installSourceRpm );

    /**
     * Cycle the source package status to the next valid value.
     **/
    void toggleSourceRpmStatus();

    /**
     * Returns whether or not a source RPM is available for this package.
     **/
    bool hasSourceRpm() const;

    /**
     * sorting function
     */
    virtual bool operator< ( const QTreeWidgetItem & other ) const;

    /**
     * Update this item's data completely.
     * Triggered by QY2ListView::updateAllItemData().
     *
     * Reimplemented from YQPkgObjList.
     **/
    virtual void updateData();

    /**
     * Returns a tool tip text for a specific column of this item.
     * 'column' is -1 if the mouse pointer is in the tree indentation area.
     *
     * Reimplemented from YQPkgObjList.
     **/
    virtual QString toolTip( int column );

    /**
     * Returns true if this package is to be displayed dimmed,
     * i.e. with grey text foreground rather than the normal black.
     **/
    bool isDimmed() const { return _dimmed; }

    /**
     * Set the 'dimmed' flag.
     **/
    void setDimmed( bool d = true ) { _dimmed = d; }


    // Columns

    int srpmStatusCol() const { return _pkgList->srpmStatusCol(); }


protected:

    /**
     * Set the suitable icon for the source RPM status.
     **/
    void setSourceRpmIcon();


    // Data members

    YQPkgList *			_pkgList;
    ZyppPkg	_zyppPkg;
    bool			_dimmed;
};


#endif // ifndef YQPkgList_h
