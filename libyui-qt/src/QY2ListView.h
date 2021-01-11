/*
  Copyright (C) 2000-2012 Novell, Inc
  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) version 3.0 of the License. This library
  is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
  License for more details. You should have received a copy of the GNU
  Lesser General Public License along with this library; if not, write
  to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
  Floor, Boston, MA 02110-1301 USA
*/


/*-/

  File:	      QY2ListView.h

  Author:     Stefan Hundhammer <sh@suse.de>

  This is a pure Qt widget - it can be used independently of YaST2.

/-*/


#ifndef QY2ListView_h
#define QY2ListView_h

#include <QTreeWidget>
#include <qtooltip.h>
#include <qpoint.h>
#include <qcolor.h>
#include <vector>

#define FIXME_TOOLTIP 0


class QY2ListViewItem;
class QY2ListViewToolTip;


/**
 * @short Enhanced QTreeWidget
 **/
class QY2ListView : public QTreeWidget
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    QY2ListView( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~QY2ListView();


public slots:

    /**
     * Select a list entry (if there is any).
     * Usually this will be the first list entry, but don't rely on that - this
     * might change without notice. Emits signal selectionChanged().
     **/
    virtual void selectSomething();

    /**
     * Reimplemented from Q3ListView:
     * Adjust header sizes after clearing contents.
     **/
    virtual void clear();

    /**
     * Update the status display of all list entries:
     * Call QY2ListViewItem::updateStatus() for each item.
     * This is an expensive operation.
     **/
    void updateItemStates();

    /**
     * Update the status display of all list entries:
     * Call QY2ListViewItem::updateData() for each item.
     * This is an expensive operation.
     **/
    void updateItemData();

    /**
     * Save the current column widths.
     **/
    void saveColumnWidths();

    /**
     * Restore the column widths to what was saved previously with
     * saveColumnWidths().
     **/
    void restoreColumnWidths();


signals:

    /**
     * Emitted for mouse clicks on an item
     **/
    void columnClicked		( int			button,
				  QTreeWidgetItem *	item,
				  int			col,
				  const QPoint &	pos );

    /**
     * Emitted for mouse double clicks on an item
     **/
    void columnDoubleClicked	( int			button,
				  QTreeWidgetItem *	item,
				  int			col,
				  const QPoint &	pos );


public:

    /**
     * Returns a tool tip text for a specific column of a list item.
     * 'column' is -1 if the mouse pointer is in the tree indentation area.
     *
     * This default implementation tries to call
     * QY2ListViewItem::toolTip( column ) or
     * QY2CheckListItem::toolTip( column ), respectively
     * if 'item' is a subclass of either.
     *
     * Derived classes may handle this differently.
     **/
    virtual QString toolTip( QTreeWidgetItem * item, int column );

    /**
     * Returns 'true' if the sort order should always be the item insertion
     * order, 'false' if the user can change the sort order by clicking on a
     * column header.
     **/
    bool sortByInsertionSequence() const { return _sortByInsertionSequence; }

    /**
     * Enforce sorting by item insertion order (true) or let user change
     * sorting by clicking on a column header (false).
     **/
    virtual void setSortByInsertionSequence( bool sortByInsertionSequence );

    /**
     * Returns the next free serial number for items that want to be ordered in
     * insertion sequence.
     **/
    int nextSerial() { return _nextSerial++; }

    /**
     * Returns the minimum size required for this widget.
     * Inherited from QWidget.
     **/
    virtual QSize minimumSizeHint() const;

    /**
     * Event filter - inherited from QWidget
     **/
    virtual bool eventFilter( QObject * obj, QEvent * event );


protected slots:

    /**
     * Internal: Handle manual column resize.
     * Save the user's preferred sizes so they don't get overwritten each time
     * the list is cleared and filled with new contents.
     **/
    void columnWidthChanged( int col, int oldSize, int newSize );

    /**
     * Internal notification that a tree item has been expanded
     */
    void treeExpanded( QTreeWidgetItem * listViewItem );

    /**
     * Internal notification that a tree item has been collapsed
     */
    void treeCollapsed( QTreeWidgetItem * listViewItem );


protected:

    /**
     * Handle mouse clicks.
     * Reimplemented from QScrollView.
     **/
    virtual void mousePressEvent( QMouseEvent * e );

    /**
     * Handle mouse clicks.
     * Reimplemented from QScrollView.
     **/
    virtual void mouseReleaseEvent( QMouseEvent * );

    /**
     * Handle mouse clicks.
     * Reimplemented from QScrollView.
     **/
    virtual void mouseDoubleClickEvent( QMouseEvent * );


    //
    // Data members
    //

    QTreeWidgetItem *		_mousePressedItem;
    int				_mousePressedCol;
    Qt::MouseButton		_mousePressedButton;

    std::vector<int> 		_savedColumnWidth;
    bool			_sortByInsertionSequence;
    int				_nextSerial;

    QY2ListViewToolTip *	_toolTip;
    bool			_mouseButton1PressedInHeader;
    bool			_finalSizeChangeExpected;
};



/**
 * Enhanced QTreeWidgetItem
 **/
class QY2ListViewItem: public QTreeWidgetItem
{
public:

    /**
     * Constructor for toplevel items.
     **/
    QY2ListViewItem( QY2ListView * 	parentListView,
		     const QString &	text = QString::null );


    /**
     * Constructor for deeper level items.
     **/
    QY2ListViewItem( QTreeWidgetItem * 	parentItem,
		     const QString &	text = QString::null );

    /**
     * Destructor
     **/
    virtual ~QY2ListViewItem();

    /**
     * Update this item's status.
     * Triggered by QY2ListView::updateAllItemStates().
     * Derived classes should overwrite this.
     * This default implementation does nothing.
     **/
    virtual void updateStatus() {}

    /**
     * Update this item's data completely.
     * Triggered by QY2ListView::updateAllItemData().
     * Derived classes should overwrite this.
     * This default implementation does nothing.
     **/
    virtual void updateData() {}

    /**
     * Comparison function used for sorting the list.
     * Reimplemented from QTreeWidgetItem
     **/
    virtual bool operator< ( const QTreeWidgetItem & other ) const;

    /**
     * Return this item's serial number.
     * Useful for comparison functions that order by insertion sequence.
     **/
    int serial() const { return _serial; }

    /**
     * Returns a tool tip text for a specific column of this item.
     * 'column' is -1 if the mouse pointer is in the tree indentation area.
     *
     * This default implementation does nothing.
     **/
    virtual QString toolTip( int column ) { return QString::null; }


protected:

    //
    // Data members
    //

    int		_serial;

    QColor	_textColor;
    QColor	_backgroundColor;
};



/**
 * Enhanced QCheckListItem
 **/
class QY2CheckListItem: public QY2ListViewItem
{
public:

    /**
     * Constructor for toplevel items.
     **/
    QY2CheckListItem( QY2ListView * 		parentListView,
		      const QString &		text );


    /**
     * Constructor for deeper level items.
     **/
    QY2CheckListItem( QTreeWidgetItem * 		parentItem,
		      const QString &		text );

    /**
     * Destructor
     **/
    virtual ~QY2CheckListItem();

    /**
     * Update this item's status.
     * Triggered by QY2ListView::updateAllItemStates().
     * Derived classes should overwrite this.
     * This default implementation does nothing.
     **/
    virtual void updateStatus() {}

    /**
     * Update this item's data completely.
     * Triggered by QY2ListView::updateAllItemData().
     * Derived classes should overwrite this.
     * This default implementation does nothing.
     **/
    virtual void updateData() {}

    /**
     * Return this item's serial number.
     * Useful for comparison functions that order by insertion sequence.
     **/
    int serial() const { return _serial; }

    /**
     * Set the text foreground color for all columns.
     * For more specific purposes reimiplement paintCell().
     **/
    void setTextColor( const QColor & col )
	{ _textColor = col; }

    /**
     * Set the text background color for all columns.
     * For more specific purposes reimiplement paintCell().
     **/
    void setBackgroundColor( const QColor & col )
	{ _backgroundColor = col; }

    /**
     * Returns a tool tip text for a specific column of this item.
     * 'column' is -1 if the mouse pointer is in the tree indentation area.
     *
     * This default implementation does nothing.
     **/
    virtual QString toolTip( int column ) { return QString(); }


protected:

    //
    // Data members
    //

    int		_serial;
};


#if FIXME_TOOLTIP
/**
 * Tool tip for a QY2ListView widget: Enables individual tool tips specific to
 * each list item and each column. Overwrite QY2ListViewItem::toolTip() to use
 * this.
 **/
class QY2ListViewToolTip : public QToolTip
{
public:

    /**
     * Constructor.
     **/
    QY2ListViewToolTip( QY2ListView * parent )
	: QToolTip( parent->viewport() )
	, _listView( parent )  {}

    /**
     * Destructor (to make gcc 4.x happy)
     **/
    virtual ~QY2ListViewToolTip() {}


protected:

    /**
     * Decide if there is a tool tip text at 'p' and display it if there is one.
     *
     * Reimplemented from QToolTip.
     **/
    virtual void maybeTip( const QPoint & p );


    //
    // Data members
    //

    QY2ListView * _listView;
};
#endif

#endif // ifndef QY2ListView_h
