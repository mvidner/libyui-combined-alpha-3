/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|			   contributed Qt widgets		       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:	      QY2ListView.h

  Author:     Stefan Hundhammer <sh@suse.de>

  This is a pure Qt widget - it can be used independently of YaST2.

/-*/

// -*- c++ -*-


#ifndef QY2ListView_h
#define QY2ListView_h

#include <qlistview.h>
#include <qtooltip.h>
#include <qpoint.h>
#include <qcolor.h>
#include <vector>


class QY2ListViewItem;
class QY2ListViewToolTip;


/**
 * @short Enhanced QListView
 **/
class QY2ListView : public QListView
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
     * Select a list entry ( if there is any ).
     * Usually this will be the first list entry, but don't rely on that - this
     * might change without notice. Emits signal selectionChanged().
     **/
    virtual void selectSomething();

    /**
     * Reimplemented from QListView:
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
				  QListViewItem *	item,
				  int			col,
				  const QPoint &	pos );

    /**
     * Emitted for mouse double clicks on an item
     **/
    void columnDoubleClicked	( int			button,
				  QListViewItem *	item,
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
    virtual QString toolTip( QListViewItem * item, int column );

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

protected:

    /**
     * Handle mouse clicks.
     * Reimplemented from QScrollView.
     **/
    virtual void contentsMousePressEvent( QMouseEvent * e );

    /**
     * Handle mouse clicks.
     * Reimplemented from QScrollView.
     **/
    virtual void contentsMouseReleaseEvent( QMouseEvent * );

    /**
     * Handle mouse clicks.
     * Reimplemented from QScrollView.
     **/
    virtual void contentsMouseDoubleClickEvent( QMouseEvent * );



    // Data members

    QListViewItem *		_mousePressedItem;
    int				_mousePressedCol;
    int				_mousePressedButton;

    std::vector<int> 		_savedColumnWidth;
    int				_nextSerial;

    QY2ListViewToolTip *	_toolTip;
    bool			_mouseButton1PressedInHeader;
    bool			_finalSizeChangeExpected;
};



/**
 * Enhanced QListViewItem
 **/
class QY2ListViewItem: public QListViewItem
{
public:

    /**
     * Constructor for toplevel items.
     *
     * 'sortByInsertionSequence' indicates if this item keeps the insertion
     * order ( true ) or leaves sorting to the user ( false - sort-by-click on
     * column headers ).
     **/
    QY2ListViewItem( QY2ListView * 		parentListView,
		     const QString &		text = QString::null,
		     bool 			sortByInsertionSequence = false );


    /**
     * Constructor for deeper level items.
     *
     * 'sortByInsertionSequence' indicates if this item keeps the insertion
     * order ( true ) or leaves sorting to the user ( false - sort-by-click on
     * column headers ).
     **/
    QY2ListViewItem( QListViewItem * 		parentItem,
		     const QString &		text = QString::null,
		     bool 			sortByInsertionSequence = false );

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
     * Return this item's serial number.
     * Useful for comparison functions that order by insertion sequence.
     **/
    int serial() const { return _serial; }

    /**
     * Returns true if this item sorts itself by insertion sequence.
     **/
    bool sortByInsertionSequence() const { return _sortByInsertionSequence; }

    /**
     * Set sort policy: 'true' to sort by insertion sequence, 'false' for user
     * sort-by-click on column headers.
     **/
    void setSortByInsertionSequence( bool doit )
	{ _sortByInsertionSequence = doit; }

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
    virtual QString toolTip( int column ) { return QString::null; }


protected:

    /**
     * Paint method. Reimplemented from @ref QListViewItem so different
     * colors can be used.
     *
     * Reimplemented from QListViewItem.
     **/
    virtual void paintCell( QPainter *		painter,
			    const QColorGroup &	colorGroup,
			    int			column,
			    int			width,
			    int			alignment );

    // Data members

    int		_serial;
    bool	_sortByInsertionSequence;

    QColor	_textColor;
    QColor	_backgroundColor;
};



/**
 * Enhanced QCheckListItem
 **/
class QY2CheckListItem: public QCheckListItem
{
public:

    /**
     * Constructor for toplevel items.
     *
     * 'sortByInsertionSequence' indicates if this item keeps the insertion
     * order ( true ) or leaves sorting to the user ( false - sort-by-click on
     * column headers ).
     **/
    QY2CheckListItem( QY2ListView * 		parentListView,
		      const QString &		text,
		      QCheckListItem::Type	type,
		      bool 			sortByInsertionSequence = false );


    /**
     * Constructor for deeper level items.
     *
     * 'sortByInsertionSequence' indicates if this item keeps the insertion
     * order ( true ) or leaves sorting to the user ( false - sort-by-click on
     * column headers ).
     **/
    QY2CheckListItem( QListViewItem * 		parentItem,
		      const QString &		text,
		      QCheckListItem::Type	type,
		      bool 			sortByInsertionSequence = false );


    /**
     * Constructor for deeper level items for QCheckListItem parents.
     *
     * 'sortByInsertionSequence' indicates if this item keeps the insertion
     * order ( true ) or leaves sorting to the user ( false - sort-by-click on
     * column headers ).
     **/
    QY2CheckListItem( QCheckListItem * 		parentItem,
		      const QString &		text,
		      QCheckListItem::Type	type,
		      bool 			sortByInsertionSequence = false );

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
     * Return this item's serial number.
     * Useful for comparison functions that order by insertion sequence.
     **/
    int serial() const { return _serial; }

    /**
     * Returns true if this item sorts itself by insertion sequence.
     **/
    bool sortByInsertionSequence() const { return _sortByInsertionSequence; }

    /**
     * Set sort policy: 'true' to sort by insertion sequence, 'false' for user
     * sort-by-click on column headers.
     **/
    void setSortByInsertionSequence( bool doit )
	{ _sortByInsertionSequence = doit; }

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

    /**
     * Paint method. Reimplemented from @ref QListViewItem so different
     * colors can be used.
     *
     * Reimplemented from QListViewItem.
     **/
    virtual void paintCell( QPainter *		painter,
			    const QColorGroup &	colorGroup,
			    int			column,
			    int			width,
			    int			alignment );

    // Data members

    int		_serial;
    bool	_sortByInsertionSequence;

    QColor	_textColor;
    QColor	_backgroundColor;
};


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
	: QToolTip( parent->viewport() ), _listView( parent )  {}

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

    
    // Data members
    
    QY2ListView * _listView;
};

#endif // ifndef QY2ListView_h
