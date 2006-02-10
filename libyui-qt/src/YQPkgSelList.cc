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

  File:	      YQPkgSelList.cc

  Author:     Stefan Hundhammer <sh@suse.de>

  Textdomain "packages-qt"

/-*/


#define y2log_component "qt-pkg"
#include <ycp/y2log.h>
#include <qregexp.h>

#include "YQi18n.h"
#include "utf8.h"
#include "YQPkgSelList.h"


YQPkgSelList::YQPkgSelList( QWidget * parent, bool autoFill )
    : YQPkgObjList( parent )
{
    y2debug( "Creating selection list" );

    int numCol = 0;
    addColumn( ""		);	_statusCol	= numCol++;
    addColumn( _( "Selection" ) );	_summaryCol	= numCol++;
    setAllColumnsShowFocus( true );

    connect( this, SIGNAL( selectionChanged( QListViewItem * )	),
	     this, SLOT  ( filter()				) );

    if ( autoFill )
    {
	fillList();
	selectSomething();
    }
    filter();

    y2debug( "Creating selection list done" );
}


YQPkgSelList::~YQPkgSelList()
{
    // NOP
}


void
YQPkgSelList::fillList()
{
    clear();
    y2debug( "Filling selection list" );


    for ( ZyppPoolIterator it = zyppSelectionsBegin();
	  it != zyppSelectionsEnd();
	  ++it )
    {
	ZyppSelection zyppSelection = tryCastToZyppSelection( (*it)->theObj() );

	if ( zyppSelection )
	{
	    if ( zyppSelection->visible() && ! zyppSelection->isBase() )
	    {
		addPkgSelItem( *it, zyppSelection );
	    }
	}
	else
	{
	    y2error( "Found non-Selection selectable" );
	}
    }

    y2debug( "Selection list filled" );
}


void
YQPkgSelList::filterIfVisible()
{
    if ( isVisible() )
	filter();
}


void
YQPkgSelList::filter()
{
    emit filterStart();

#ifdef FIXME
    if ( selection() )
    {
	ZyppSelection sel = selection()->zyppSelection();

	if ( sel )
	{

	    // FIXME
	}
    }
#else
    {
	{
#endif


	    for ( ZyppPoolIterator it = zyppPkgBegin();
		  it != zyppPkgEnd();
		  ++it )
	    {
		ZyppPkg zyppPkg = tryCastToZyppPkg( (*it)->theObj() );

		if ( zyppPkg )
		{
		    emit filterMatch( *it, zyppPkg );
		}
	    }
	}
    }

    emit filterFinished();
}


void
YQPkgSelList::addPkgSelItem( ZyppSel		selectable,
			     ZyppSelection	zyppSelection )
{
    if ( ! selectable )
    {
	y2error( "NULL zypp::ui::Selectable!" );
	return;
    }

    new YQPkgSelListItem( this, selectable, zyppSelection );
}


YQPkgSelListItem *
YQPkgSelList::selection() const
{
    QListViewItem * item = selectedItem();

    if ( ! item )
	return 0;

    return dynamic_cast<YQPkgSelListItem *> (item);
}


void
YQPkgSelList::applyChanges()
{
#ifdef FIXME
    Y2PM::selectionManager().activate( Y2PM::packageManager() );
#endif
    emit updatePackages();
}






YQPkgSelListItem::YQPkgSelListItem( YQPkgSelList *	pkgSelList,
				    ZyppSel		selectable,
				    ZyppSelection	zyppSelection )
    : YQPkgObjListItem( pkgSelList, selectable, zyppSelection )
    , _pkgSelList( pkgSelList )
    , _zyppSelection( zyppSelection )
{
    if ( ! _zyppSelection )
	_zyppSelection = tryCastToZyppSelection( selectable->theObj() );

#ifdef FIXME
    QString text = fromUTF8( _zyppSelection->summary( Y2PM::getPreferredLocale() ) );
#else
    QString text = fromUTF8( _zyppSelection->summary() );
#endif

    // You don't want to know why we need this.
    text.replace( QRegExp( "Graphical Basis System" ), "Graphical Base System" );
    text.replace( QRegExp( "Gnome" ), "GNOME" );

    setText( summaryCol(), text );

    setStatusIcon();
}



YQPkgSelListItem::~YQPkgSelListItem()
{
    // NOP
}


void
YQPkgSelListItem::setStatus( ZyppStatus newStatus )
{
    YQPkgObjListItem::setStatus( newStatus );
    _pkgSelList->applyChanges();
}



/**
 * Comparison function used for sorting the list.
 * Returns:
 * -1 if this <	 other
 *  0 if this == other
 * +1 if this >	 other
 **/
int
YQPkgSelListItem::compare( QListViewItem *	otherListViewItem,
			   int			col,
			   bool			ascending ) const
{
    YQPkgSelListItem * other = ( YQPkgSelListItem * ) otherListViewItem;

    if ( ! _zyppSelection || ! other || ! other->zyppSelection() )
	return 0;

    return _zyppSelection->order().compare( other->zyppSelection()->order() );
}



#include "YQPkgSelList.moc"
