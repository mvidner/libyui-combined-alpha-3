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

  File:	      QY2DiskUsageList.cc

  Author:     Stefan Hundhammer <sh@suse.de>

  Textdomain "qt"

  This is a pure Qt widget - it can be used independently of YaST2.


/-*/

#include "QY2DiskUsageList.h"
#include "YQi18n.h"
#include <QPainter>
#include <QItemDelegate>
#include <QDebug>

#ifdef TEXTDOMAIN
#    undef TEXTDOMAIN
#endif

#define TEXTDOMAIN "qt"



/**
 * Stolen from KDirStat::KDirTreeView with the author's permission.
 **/
QColor
contrastingColor( const QColor & desiredColor,
					const QColor & contrastColor )
{
    if ( desiredColor != contrastColor )
    {
	return desiredColor;
    }

    if ( contrastColor != contrastColor.light() )
    {
	// try a little lighter
	return contrastColor.light();
    }
    else
    {
	// try a little darker
	return contrastColor.dark();
    }
}

/**
 * Interpolate ( translate ) a value 'from' in the range between 'minFrom'
 * and 'maxFrom'  to a range between 'minTo' and 'maxTo'.
 **/
static int
interpolate( int from,
				   int minFrom, int maxFrom,
				   int minTo, 	int maxTo 	)
{
    if ( minFrom > maxFrom )
    {
	// Swap min/max values

	int tmp = maxFrom;
	maxFrom = minFrom;
	minFrom = tmp;
    }

    long x = from - minFrom;
    x *= maxTo - minTo;
    x /= maxFrom - minFrom;
    x += minTo;

    if ( minTo < maxTo )
    {
	if ( x < minTo )	x = minTo;
	if ( x > maxTo )	x = maxTo;
    }
    else
    {
	if ( x < maxTo )	x = maxTo;
	if ( x > minTo )	x = minTo;
    }

    return (int) x;
}

/**
 * Interpolate ( in the HSV color space ) a color between 'minColor' and
 * 'maxColor' for a current value 'val' so that 'minVal' corresponds to
 * 'minColor' and 'maxVal' to 'maxColor'.
 *
 * Returns the interpolated color.
 **/
static QColor
interpolateColor( int 		val,
					int 		minVal,
					int 		maxVal,
					const QColor & 	minColor,
					const QColor & 	maxColor )
{
    int minH, maxH;
    int minS, maxS;
    int minV, maxV;

    minColor.getHsv( &minH, &minS, &minV );
    maxColor.getHsv( &maxH, &maxS, &maxV );

    return QColor::fromHsv( interpolate( val, minVal, maxVal, minH, maxH ),
		   interpolate( val, minVal, maxVal, minS, maxS ),
		   interpolate( val, minVal, maxVal, minV, maxV ) );
}


class QY2DiskUsagePercentageItem : public QItemDelegate
{
    QY2DiskUsageList *_view;

public:
    QY2DiskUsagePercentageItem( QY2DiskUsageList *parent ) : QItemDelegate( parent ), _view( parent ) {
    }

    virtual void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
    {
        painter->save();
        QColor background = option.palette.color(QPalette::Window);
        painter->setBackground( background );

        QY2DiskUsageListItem *item = dynamic_cast<QY2DiskUsageListItem *>(_view->itemFromIndex(index));
        if ( item )
        {
          item->paintPercentageBar( painter,
                                    option,
                                    interpolateColor( item->usedPercent(),
                                                      60, 95,
                                                      QColor( 0, 0xa0, 0 ),	// Medium dark green
                                                      QColor( 0xFF, 0, 0 ) ) );	// Bright red
        }
        painter->restore();
    }
};

QY2DiskUsageList::QY2DiskUsageList( QWidget * parent, bool addStdColumns )
    : QY2ListView( parent )
{
    _nameCol		= -42;
    _percentageBarCol	= -42;
    _usedSizeCol	= -42;
    _freeSizeCol	= -42;
    _totalSizeCol	= -42;
    _deviceNameCol	= -42;

    // set temporary textdomain to enable translations
    // in inherit classed (e.g. YQPkgDiskUsageList)
    // see bnc #445716
    QString savedtextdomain = textdomain(NULL);
    textdomain(TEXTDOMAIN);

    QStringList columnLabels;
    if ( addStdColumns )
    {
        int numCol = 0;
        columnLabels << _( "Name" 		);	_nameCol 		= numCol++;
        // Translators: Please keep this short!
        columnLabels <<  _("Disk Usage");	_percentageBarCol	= numCol++;
        setItemDelegateForColumn( _percentageBarCol, new QY2DiskUsagePercentageItem( this ) );
        //columnLabels << _("Used"); _usedSizeCol		= numCol++;
        columnLabels << _( "Free"); _freeSizeCol		= numCol++;
        columnLabels << _("Total"); _totalSizeCol		= numCol++;
#if 0
        addColumn( _( "Device" 		) );	_deviceNameCol		= numCol++;
#endif
        // needed?
        setColumnCount(numCol);
        setHeaderLabels(columnLabels);

        sortItems( percentageBarCol(), Qt::AscendingOrder );
        setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    }

    textdomain(savedtextdomain.toLatin1());

    saveColumnWidths();
    setSelectionMode(QAbstractItemView::NoSelection);
}


QY2DiskUsageList::~QY2DiskUsageList()
{
}


void QY2DiskUsageList::drawRow( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    // Intentionally bypassing the direct parent class method, use the grandparent's:
    // Don't let QY2ListViewItem::_textColor / _backgroundColor interfere with our colors.

    QTreeWidget::drawRow( painter, option, index );
}


QY2DiskUsageListItem::QY2DiskUsageListItem( QY2DiskUsageList * parent )
    : QY2ListViewItem( parent )
    , _diskUsageList( parent )
{
}




QY2DiskUsageListItem::~QY2DiskUsageListItem()
{
    // NOP
}




void
QY2DiskUsageListItem::init( bool allFields )
{
    setSizeHint( percentageBarCol(), QSize( 20, 10 ) );

    setTextAlignment( usedSizeCol(), Qt::AlignRight );
    setTextAlignment( freeSizeCol(), Qt::AlignRight );
    setTextAlignment( totalSizeCol(), Qt::AlignRight );

    if ( usedSizeCol()		>= 0 ) setText( usedSizeCol(),		usedSize() 	);
    if ( freeSizeCol()		>= 0 ) setText( freeSizeCol(),		freeSize() 	);

    if ( allFields )
    {
	if ( totalSizeCol()	>= 0 ) setText( totalSizeCol(), 	totalSize() 	);
	if ( nameCol()		>= 0 ) setText( nameCol(),		name()	);
	if ( deviceNameCol()	>= 0 ) setText( deviceNameCol(),	deviceName()	);
    }

    if ( usedSizeCol() < 0 )
        setToolTip( freeSizeCol(), _( "Used %1" ).arg( usedSize().form( 0, 1, true ).c_str() ) );
}


void
QY2DiskUsageListItem::setText( int column, const FSize & size )
{
    QString sizeText = size.form( 0, 1, true ).c_str();
    setText( column, sizeText );
}


FSize
QY2DiskUsageListItem::freeSize() const
{
    return totalSize() - usedSize();
}


int
QY2DiskUsageListItem::usedPercent() const
{
    int percent = 0;

    if ( totalSize() != 0 )
	percent = ( 100 * usedSize() ) / totalSize();

    return percent;
}


void
QY2DiskUsageListItem::updateStatus()
{
    init( false );
}


void
QY2DiskUsageListItem::updateData()
{
    init( true );
}





/**
     * Comparison function used for sorting the list.
     * Reimplemented from QTreeWidgetItem
     **/
bool
QY2DiskUsageListItem::operator<( const QTreeWidgetItem & otherListViewItem ) const
{
    const QY2DiskUsageListItem * other = dynamic_cast<const QY2DiskUsageListItem *> (&otherListViewItem);
    int col = treeWidget()->sortColumn();

    if ( other )
    {
	if ( col == percentageBarCol()   )
	{
	    // Intentionally reverting sort order: Fullest first
            return ( this->usedPercent() < other->usedPercent() );
	}
	else if ( col == usedSizeCol() )
	{
	    return ( this->usedSize() < other->usedSize() );
	}
	else if ( col == freeSizeCol() )
	{
	    return ( this->freeSize() < other->freeSize() );
	}
	else if ( col == totalSizeCol() )
	{
	    return ( this->totalSize() < other->totalSize() );
	}
    }

    return QY2ListViewItem::operator<( otherListViewItem );
}

/**
 * Stolen from KDirStat::KDirTreeView with the author's permission.
 **/
void
QY2DiskUsageListItem::paintPercentageBar( QPainter *		painter,
					  QStyleOptionViewItem option,
					  const QColor &	fillColor )
{
    float percent = usedPercent();
    if ( percent > 100.0 )	percent = 100.0;
    if ( percent < 0.0   )	percent = 0.0;
    int x = option.rect.left() + 1;
    int y = option.rect.top() + 1;
    int w = option.rect.width() - 2;
    int h = option.rect.height() - 2;
    int fillWidth = 0;

    if ( w > 0 )
    {
 	fillWidth = (int) ( w * percent / 100.0 );

	// Fill the desired percentage.

	painter->fillRect( x, y,  fillWidth, h,
			   fillColor );

        QString percentageText;
	percentageText.sprintf( "%d%%", usedPercent() );

        if ( usedPercent() > 50 ) {
            painter->setPen( treeWidget()->palette().color( QPalette::Base ) );
            painter->drawText( QRect( x, y,
                                      fillWidth - 3, h ),
                               Qt::AlignRight, percentageText );
        } else {
            painter->setPen( treeWidget()->palette().color( QPalette::Text ) );
            painter->drawText( QRect( x + fillWidth + 3, y,
                                      w - fillWidth - 3, h ),
                               Qt::AlignLeft, percentageText );

        }
    }
}

#include "QY2DiskUsageList.moc"
