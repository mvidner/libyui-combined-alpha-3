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

  File:	      QY2DiskUsageList.h

  Author:     Stefan Hundhammer <sh@suse.de>

  This is a pure Qt widget - it can be used independently of YaST2.

/-*/



#ifndef QY2DiskUsageList_h
#define QY2DiskUsageList_h

#include <QY2ListView.h>
#include <y2util/FSize.h>
#include <qcolor.h>


class QY2DiskUsageListItem;


/**
 * Generic scrollable list of disk usage for any number of partitions.
 **/
class QY2DiskUsageList : public QY2ListView
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * Adds a standard set of list columns if 'addStdColumns' is
     *'true'. Otherwise the caller is responsible for adding any columns.
     **/
    QY2DiskUsageList( QWidget * parent, bool addStdColumns = true );

    /**
     * Destructor
     **/
    virtual ~QY2DiskUsageList();


    // Column numbers

    int nameCol()		const	{ return _nameCol;		}
    int percentageBarCol()	const	{ return _percentageBarCol;	}
    int percentageCol()		const	{ return _percentageCol;	}
    int usedSizeCol()		const	{ return _usedSizeCol;		}
    int freeSizeCol()		const	{ return _freeSizeCol;		}
    int totalSizeCol()		const	{ return _totalSizeCol;		}
    int deviceNameCol()		const	{ return _deviceNameCol;	}


protected:

    // Data members

    int _nameCol;
    int _percentageBarCol;
    int _percentageCol;
    int _usedSizeCol;
    int _freeSizeCol;
    int _totalSizeCol;
    int _deviceNameCol;
};



/**
 * Abstract base class for one partition ( mount point ) to display in a
 * QY2DiskUsageList.
 *
 * This class contains pure virtuals, so it cannot be used directly.
 **/
class QY2DiskUsageListItem: public QY2ListViewItem
{
protected:
    /**
     * Constructor.
     *
     * Call updateData() after the constructor for the initial display
     * update. Unfortunately, this cannot be done automatically in the
     * constructor since it uses virtual methods that are not available yet at
     * this point.
     **/
    QY2DiskUsageListItem( QY2DiskUsageList * parent );


    /**
     * Destructor.
     **/
    virtual ~QY2DiskUsageListItem();


public:

    /**
     * The currently used size of this partition.
     *
     * Derived classes need to implement this method.
     **/

    virtual FSize usedSize() const = 0;

    /**
     * The total size of this partition.
     *
     * Derived classes need to implement this method.
     **/
    virtual FSize totalSize() const = 0;

    /**
     * The current free size of this partition.
     *
     * Derived classes can choose reimpmenent this if it is less expensive than
     * calculating this value each time from usedSize() and totalSize() which
     * is the default implementation.
     **/
    virtual FSize freeSize() const;

    /**
     * The currently used percentage ( 0..100 ) of this partition.
     *
     * Derived classes can choose reimpmenent this if it is less expensive than
     * calculating this value each time from usedSize() and totalSize() which
     * is the default implementation.
     **/
    virtual int usedPercent() const;

    /**
     * The name to display for this partition.
     * It makes most sense to use the mount point here ( but this is not a
     * requirement ). This is what will be displayed in the "Name" column.
     *
     * Derived classes need to implement this method.
     **/
    virtual QString name() const = 0;

    /**
     * The device name of this partition.
     *
     * Derived classes may choose to reimplement this method.
     * This default implementation returns an empty string.
     **/
    virtual QString deviceName() const { return ""; }


    /**
     * Update this item's status ( here: the numeric fields ).
     * Triggered by QY2ListView::updateAllItemStates().
     *
     * Reimplemented from QY2ListViewItem.
     **/
    virtual void updateStatus();

    /**
     * Update this item's data completely.
     * Triggered by QY2ListView::updateAllItemData().
     *
     * Reimplemented from QY2ListViewItem.
     **/
    virtual void updateData();

    /**
     * Re-declare ordinary setText() method so the compiler doesn't get
     * confused which one to use.
     **/
    void setText( int column, const QString & text )
	{ QListViewItem::setText( column, text ); }

    /**
     * Set a column text via FSize.
     **/
    void setText( int column, const FSize & size );

    /**
     * Comparison function used for sorting the list.
     * Returns:
     * -1 if this <  other
     *	0 if this == other
     * +1 if this >  other
     *
     * Reimplemented from QY2ListViewItem.
     **/
    virtual int compare( QListViewItem *	other,
			 int			col,
			 bool			ascending ) const;

    // Columns

    int nameCol()		const	{ return _diskUsageList->nameCol();		}
    int percentageBarCol()	const	{ return _diskUsageList->percentageBarCol();	}
    int percentageCol()		const	{ return _diskUsageList->percentageCol();	}
    int usedSizeCol()		const	{ return _diskUsageList->usedSizeCol();		}
    int freeSizeCol()		const	{ return _diskUsageList->freeSizeCol();		}
    int totalSizeCol()		const	{ return _diskUsageList->totalSizeCol();	}
    int deviceNameCol()		const	{ return _diskUsageList->deviceNameCol();	}


protected:

    /**
     * ( Re- ) initialize fields - all displayed fields ( if 'allFields' is
     * 'true' ) or only the varying fields ( used, free, percentage ).
     **/
    void init( bool allFields );

    /**
     * Paint method.
     *
     * Reimplemented from QY2ListViewItem.
     **/
    virtual void paintCell( QPainter *		painter,
			    const QColorGroup & colorGroup,
			    int			column,
			    int			width,
			    int			alignment );

    /**
     * Paint a percentage bar into a @ref QListViewItem cell.
     * 'width' is the width of the entire cell.
     * 'indent' is the number of pixels to indent the bar.
     *
     * Stolen from KDirStat::KDirTreeView with the author's permission.
     **/
    void paintPercentageBar( float		percent,
			     QPainter *		painter,
			     int		indent,
			     int		width,
			     const QColor &	fillColor,
			     const QColor &	barBackground	);

    /**
     * Return a color that contrasts to 'contrastColor'.
     *
     * Stolen from KDirStat::KDirTreeView with the author's permission.
     **/
    QColor contrastingColor ( const QColor &	desiredColor,
			      const QColor &	contrastColor );

    /**
     * Interpolate ( in the HSV color space ) a color between 'minColor' and
     * 'maxColor' for a current value 'val' so that 'minVal' corresponds to
     * 'minColor' and 'maxVal' to 'maxColor'.
     *
     * Returns the interpolated color.
     **/
    virtual QColor interpolateColor( int 		val,
				     int 		minVal,
				     int 		maxVal,
				     const QColor & 	minColor,
				     const QColor & 	maxColor );

    /**
     * Interpolate ( translate ) a value 'from' in the range between 'minFrom'
     * and 'maxFrom'  to a range between 'minTo' and 'maxTo'.
     **/
    int interpolate( int from,
		     int minFrom,	int maxFrom,
		     int minTo, 	int maxTo 	);


    // Data members

    QY2DiskUsageList * _diskUsageList;
};




#endif // ifndef QY2DiskUsageList_h
