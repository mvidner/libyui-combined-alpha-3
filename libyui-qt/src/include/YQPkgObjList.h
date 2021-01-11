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

  File:	      YQPkgObjList.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

// -*- c++ -*-


#ifndef YQPkgObjList_h
#define YQPkgObjList_h

#include <qpixmap.h>
#include <qaction.h>

#include <QY2ListView.h>
#include "YQZypp.h"
#include <zypp/Edition.h>
#include <y2util/FSize.h>

class YQPkgObjListItem;
class QPopupMenu;
using std::string;


/**
 * @short Abstract base class to display a list of zypp::ResObjects.
 * Handles most generic stuff like setting status etc.
 **/
class YQPkgObjList : public QY2ListView
{
    Q_OBJECT

protected:
    /**
     * Constructor. Does not add any QListView columns!
     **/
    YQPkgObjList( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgObjList();


public:

    // Column numbers

    int statusCol()		const	{ return _statusCol;		}
    int nameCol()		const	{ return _nameCol;		}
    int summaryCol()		const	{ return _summaryCol;		}
    int sizeCol()		const	{ return _sizeCol;		}
    int versionCol()		const	{ return _versionCol;		}
    int instVersionCol()	const	{ return _instVersionCol;	}
    int brokenIconCol()		const	{ return _brokenIconCol;	}
    int satisfiedIconCol()	const	{ return _satisfiedIconCol;	}

    /**
     * Return whether or not items in this list are generally editable,
     * i.e. the user can change their status. Note that individual items can be
     * set to non-editable even if the list is generally editable.
     * Lists are editable by default.
     **/
    bool editable() const { return _editable; }

    /**
     * Set the list's editable status.
     **/
    void setEditable( bool editable = true ) { _editable = editable; }

    /**
     * Sets the currently selected item's status.
     * Automatically selects the next item if 'selectNextItem' is 'true'.
     **/
    void setCurrentStatus( ZyppStatus	newStatus,
			   bool			selectNextItem = false );

    /**
     * Sets the status of all (toplevel) list items to 'newStatus', if possible.
     * Only one single statusChanged() signal is emitted.
     *
     * 'force' overrides sensible defaults like setting only zypp::ResObjects to
     * 'update' that really come with a newer version.
     **/
    void setAllItemStatus( ZyppStatus newStatus, bool force = false );

    /**
     * Add a submenu "All in this list..." to 'menu'.
     * Returns the newly created submenu.
     **/
    virtual QPopupMenu * addAllInListSubMenu( QPopupMenu * menu );

    /**
     * Returns the suitable icon for a zypp::ResObject status - the regular icon if
     * 'enabled' is 'true' or the insensitive icon if 'enabled' is 'false.
     * 'bySelection' is relevant only for auto-states: This uses the icon for
     * 'auto-by-selection" rather than the default auto-icon.
     **/
    virtual QPixmap statusIcon( ZyppStatus status,
				bool 		enabled     = true,
				bool		bySelection = false );

    /**
     * Returns a short ( one line ) descriptive text for a zypp::ResObject status.
     **/
    virtual QString statusText( ZyppStatus status ) const;


public slots:

    /**
     * Add a zypp::ResObject to the list. Connect a filter's filterMatch() signal to
     * this slot. Remember to connect filterStart() to clear() (inherited from
     * QListView).
     *
     * 'zyppObj' has to be one of the objects of 'selectable'. If it is 0,
     * selectable->theObject() will be used.
     *
     * Intentionally NOT named addItem() so the calling class cannot confuse
     * this method with overlaid methods of the same name that were simply
     * forgotten to implement!
     **/
    void addPkgObjItem( ZyppSel	selectable,
			ZyppObj 	zyppObj = 0 );

    /**
     * Add a purely passive list item that has a name and optional summary and
     * size.
     **/
    void addPassiveItem( const QString & name,
			 const QString & summary = QString::null,
			 FSize 		 size    = -1 );

    /**
     * Dispatcher slot for mouse click: cycle status depending on column.
     **/
    virtual void pkgObjClicked( int		button,
				QListViewItem * item,
				int		col,
				const QPoint &	pos );

    /**
     * Reimplemented from QY2ListView:
     * Emit selectionChanged() signal after clearing the list.
     **/
    virtual void clear();

    /**
     * Update the internal actions: What actions are available for 'item'?
     **/
    virtual void updateActions( YQPkgObjListItem * item );

    /**
     * Update the internal actions for the currently selected item ( if any ).
     * This only calls updateActions( YQPkgObjListItem * ) with the currently
     * selected item as argument, so there is normally no need to reimplement
     * this method, too, if the other one is reimplemented.
     **/
    virtual void updateActions();

    /**
     * Emit an updatePackages() signal.
     **/
    void sendUpdatePackages() { emit updatePackages(); }

    /**
     * Select the next item, i.e. move the selection one item further down the
     * list.
     **/
    void selectNextItem();

    /**
     * Emit a statusChanged() signal for the specified zypp::ResObject.
     **/
    void sendStatusChanged() { emit statusChanged(); }

    /**
     * Display a one-line message in the list.
     **/
    virtual void message( const QString & text );


    // Direct access to some states for menu actions

    void setCurrentInstall()	   { setCurrentStatus( S_Install	); }
    void setCurrentDontInstall()   { setCurrentStatus( S_NoInst	     	); }
    void setCurrentKeepInstalled() { setCurrentStatus( S_KeepInstalled	); }
    void setCurrentDelete()	   { setCurrentStatus( S_Del	     	); }
    void setCurrentUpdate()	   { setCurrentStatus( S_Update	     	); }
    void setCurrentTaboo()	   { setCurrentStatus( S_Taboo	     	); }
    void setCurrentProtected()	   { setCurrentStatus( S_Protected	); }

    void setListInstall()	   { setAllItemStatus( S_Install	); }
    void setListDontInstall()	   { setAllItemStatus( S_NoInst		); }
    void setListKeepInstalled()	   { setAllItemStatus( S_KeepInstalled	); }
    void setListDelete()	   { setAllItemStatus( S_Del		); }
    void setListUpdate()	   { setAllItemStatus( S_Update		); }
    void setListUpdateForce()	   { setAllItemStatus( S_Update, true  	); }
    void setListTaboo()		   { setAllItemStatus( S_Taboo		); }
    void setListProtected()	   { setAllItemStatus( S_Protected     	); }


protected slots:

    /**
     * Dispatcher slot for selection change - internal only.
     **/
    virtual void selectionChangedInternal( QListViewItem * item );


signals:


    /**
     * Emitted when a zypp::ui::Selectable is selected.
     * May be called with a null poiner if no zypp::ResObject is selected.
     **/
    void selectionChanged( ZyppSel selectable );

    /**
     * Emitted when the status of a zypp::ResObject is changed.
     **/
    void statusChanged();

    /**
     * Emitted when it's time to update displayed package information,
     * e.g., package states.
     **/
    void updatePackages();


protected:

    /**
     * Event handler for keyboard input.
     * Only very special keys are processed here.
     *
     * Reimplemented from QListView / QWidget.
     */
    virtual void keyPressEvent( QKeyEvent * ev );

    /**
     * Returns the context menu for items that are not installed.
     * Creates the menu upon the first call.
     **/
    virtual QPopupMenu * installedContextMenu();

    /**
     * Returns the context menu for items that are installed.
     * Creates the menu upon the first call.
     **/
    virtual QPopupMenu * notInstalledContextMenu();

    /**
     * Create the context menu for items that are not installed.
     **/
    virtual void createNotInstalledContextMenu();

    /**
     * Create the context menu for installed items.
     **/
    virtual void createInstalledContextMenu();

    /**
     * Create the actions for the context menus.
     * Note: This is intentionally not virtual!
     **/
    void createActions();

    /**
     * Create an action based on a zypp::ResObject status - automatically retrieve the
     * corresponding status icons (both sensitive and insensitive) and text.
     * 'key' is only a descriptive text, no true accelerator.
     **/
    QAction * createAction( ZyppStatus 	status,
			    const QString &	key	= QString::null,
			    bool 		enabled = false );

    /**
     * Low-level: Create an action.
     * 'key' is only a descriptive text, no true accelerator.
     **/
    QAction * createAction( const QString & 	text,
			    const QPixmap & 	icon		= QPixmap(),
			    const QPixmap & 	insensitiveIcon	= QPixmap(),
			    const QString & 	key		= QString::null,
			    bool 		enabled		= false );



    // Data members

    int		_statusCol;
    int		_nameCol;
    int		_summaryCol;
    int		_sizeCol;
    int		_versionCol;
    int		_instVersionCol;
    int		_brokenIconCol;
    int		_satisfiedIconCol;
    bool	_editable;
    bool	_debug;


    QPopupMenu *	_installedContextMenu;
    QPopupMenu *	_notInstalledContextMenu;


public:

    QAction *		actionSetCurrentInstall;
    QAction *		actionSetCurrentDontInstall;
    QAction *		actionSetCurrentKeepInstalled;
    QAction *		actionSetCurrentDelete;
    QAction *		actionSetCurrentUpdate;
    QAction *		actionSetCurrentTaboo;
    QAction *		actionSetCurrentProtected;

    QAction *		actionSetListInstall;
    QAction *		actionSetListDontInstall;
    QAction *		actionSetListKeepInstalled;
    QAction *		actionSetListDelete;
    QAction *		actionSetListUpdate;
    QAction *		actionSetListUpdateForce;
    QAction *		actionSetListTaboo;
    QAction *		actionSetListProtected;
};



class YQPkgObjListItem: public QY2ListViewItem
{
public:

    /**
     * Constructor for root items: Creates a YQPkgObjList item that corresponds
     * to the ZYPP selectable that 'selectable' refers to. 'zyppObj' has to be
     * one object of 'selectable'.  If it is 0, selectable->theObject() will be
     * used.
     **/
    YQPkgObjListItem( YQPkgObjList *	pkgObjList,
		      ZyppSel 		selectable,
		      ZyppObj		zyppObj = 0 );

    YQPkgObjListItem( YQPkgObjList *	pkgObjList );


protected:
    /**
     * Constructor for non-root items.
     **/
    YQPkgObjListItem( YQPkgObjList *	pkgObjList,
		      QY2ListViewItem * parent,
		      ZyppSel 		selectable,
		      ZyppObj		zyppObj = 0 );

public:

    /**
     * Destructor
     **/
    virtual ~YQPkgObjListItem();

    /**
     * Returns the original selectable within the package manager backend.
     **/
    ZyppSel selectable() const { return _selectable; }

    /**
     * Returns the original object within the package manager backend.
     **/
    ZyppObj zyppObj() const { return _zyppObj; }

    /**
     * Return whether or not this items is editable, i.e. the user can change
     * its status. This requires the corresponding list to be editable, too.
     * Items are editable by default.
     **/
    bool editable() const { return _editable; }

    /**
     * Set this item's editable status.
     **/
    void setEditable( bool editable = true ) { _editable = editable; }

    /**
     * Returns the (binary RPM) package status
     **/
    ZyppStatus status() const;

    /**
     * Returns 'true' if this selectable's status is set by a selection
     * (rather than by the user or by the dependency solver).
     **/
    virtual bool bySelection() const;

    /**
     * Set the (binary RPM) package status.
     *
     * If 'sendSignals' is 'true' (default), the parent list will be requested
     * to send update signals. List operations might want to use this for
     * optimizations to send the signals only once after all changes are done.
     **/
    virtual void setStatus( ZyppStatus newStatus, bool sendSignals = true );

    /**
     * Set a status icon according to the package's status.
     **/
    virtual void setStatusIcon();

    /**
     * Update this item's status.
     * Triggered by QY2ListView::updateAllItemStates().
     * Overwritten from QY2ListViewItem.
     **/
    virtual void updateStatus();

    /**
     * Cycle the package status to the next valid value.
     **/
    virtual void cycleStatus();

    /**
     * Check if the candidate is newer than the installed version.
     **/
    bool candidateIsNewer() const { return _candidateIsNewer; }

    /**
     * Check if the installed version is newer than the candidate.
     **/
    bool installedIsNewer() const { return _installedIsNewer; }

    /**
     * Check if this item is satisfied, even though it is not installed.
     * This is useful for package collections, e.g., patterns and patches:
     * 'true' is returned if all requirements are fulfilled, but the object
     * itself is not installed.
     **/
    bool isSatisfied() const;

    /**
     * Check if this item is "broken": If it is installed, but any of its
     * dependencies are no longer satisfied.
     * This is useful for package collections, e.g., patterns and patches.
     **/
    bool isBroken() const;

    /**
     * Display this item's notify text (if there is any) that corresponds to
     * the specified status (S_Install, S_Del) in a pop-up window.
     **/
    void showNotifyTexts( ZyppStatus status );

    /**
     * Display a selectable's license agreement (if there is any) that
     * corresponds to its current status (S_Install, S_Update) in a pop-up
     * window.
     *
     * Returns 'true' if the user agreed to that license , 'false' otherwise.
     * The item's status may have changed to S_Taboo, S_Proteced or S_Del if
     * the user disagreed with the license.
     **/
    static bool showLicenseAgreement( ZyppSel sel );

    /**
     * Display this item's license agreement (if there is any) that corresponds
     * to its current status (S_Install, S_Update) in a pop-up window.
     **/
    bool showLicenseAgreement();

    /**
     * Comparison function used for sorting the list.
     * Returns:
     * -1 if this <  other
     *	0 if this == other
     * +1 if this >  other
     *
     * Reimplemented from QListViewItem
     **/
    virtual int compare( QListViewItem *	other,
			 int			col,
			 bool			ascending ) const;

    /**
     * Calculate a numerical value to compare versions, based on version
     * relations:
     *
     * - Installed newer than candidate (red)
     * - Candidate newer than installed (blue) - worthwhile updating
     * - Installed
     * - Not installed, but candidate available
     **/
    int versionPoints() const;

    /**
     * Update this item's data completely.
     * Triggered by QY2ListView::updateAllItemData().
     *
     * Reimplemented from QY2ListViewItem.
     **/
    virtual void updateData();

    /**
     * Returns a tool tip text for a specific column of this item.
     * 'column' is -1 if the mouse pointer is in the tree indentation area.
     *
     * Reimplemented from QY2ListViewItem.
     **/
    virtual QString toolTip( int column );


    // Handle Debug isBroken and isSatisfied flags

    bool debugIsBroken()    const		{ return _debugIsBroken;		}
    bool debugIsSatisfied() const		{ return _debugIsSatisfied;		}
    void setDebugIsBroken   ( bool val = true )	{ _debugIsBroken = val;			}
    void setDebugIsSatisfied( bool val = true ) { _debugIsSatisfied = val;		}
    void toggleDebugIsBroken()			{ _debugIsBroken = ! _debugIsBroken;	}
    void toggleDebugIsSatisfied()		{ _debugIsSatisfied = ! _debugIsSatisfied; }


    // Columns

    int statusCol()		const	{ return _pkgObjList->statusCol();	}
    int nameCol()		const	{ return _pkgObjList->nameCol();	}
    int summaryCol()		const	{ return _pkgObjList->summaryCol();	}
    int sizeCol()		const	{ return _pkgObjList->sizeCol();	}
    int versionCol()		const	{ return _pkgObjList->versionCol();	}
    int instVersionCol()	const	{ return _pkgObjList->instVersionCol(); }
    int brokenIconCol()		const	{ return _pkgObjList->brokenIconCol();	}
    int satisfiedIconCol()	const	{ return _pkgObjList->satisfiedIconCol(); }


protected:

    /**
     * Initialize internal data and set fields accordingly.
     **/
    void init();

    /**
     * Apply changes hook. This is called each time the user changes the status
     * of a list item manually (if the old status is different from the new
     * one). Insert code to propagate changes to other objects here, for
     * example to trigger a "small" solver run (Resolver::transactObjKind()
     * etc.).
     *
     * This default implementation does nothing.
     **/
    virtual void applyChanges() {}

    /**
     * Do a "small" solver run for all "resolvable collections", i.e., for
     * selections, patterns, languages, patches.
     **/
    void solveResolvableCollections();

    /**
     * Set a column text via STL string.
     * ( QListViewItem::setText() expects a QString! )
     **/
    void setText( int column, const string text );

    /**
     * Re-declare ordinary setText() method so the compiler doesn't get
     * confused which one to use.
     **/
    void setText( int column, const QString & text )
	{ QListViewItem::setText( column, text ); }

    /**
     * Set a column text via Edition.
     **/
    void setText( int column, const zypp::Edition & edition );


    //
    // Data members
    //

    YQPkgObjList *	_pkgObjList;
    ZyppSel		_selectable;
    ZyppObj		_zyppObj;
    bool		_editable:1;
    bool		_candidateIsNewer:1;
    bool		_installedIsNewer:1;

    bool		_debugIsBroken:1;
    bool		_debugIsSatisfied:1;
};



#endif // ifndef YQPkgObjList_h
