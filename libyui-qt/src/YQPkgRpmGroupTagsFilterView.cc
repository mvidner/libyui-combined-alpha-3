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

  File:	      YQPkgRpmGroupTagsFilterView.cc

  Author:     Stefan Hundhammer <sh@suse.de>

  Textdomain "packages-qt"

/-*/


#define y2log_component "qt-pkg"
#include <ycp/y2log.h>

#include "YQPkgRpmGroupTagsFilterView.h"
#include "YQi18n.h"
#include "utf8.h"



YRpmGroupsTree * YQPkgRpmGroupTagsFilterView::_rpmGroupsTree = 0;


YQPkgRpmGroupTagsFilterView::YQPkgRpmGroupTagsFilterView( QWidget * parent )
    : QListView( parent )
{
    addColumn( _( "Package Groups" ) );
    setRootIsDecorated( true );
    cloneTree( rpmGroupsTree()->root(), 0 );

    new YQPkgRpmGroupTag( this, _( "zzz All" ), 0 );

    connect( this, SIGNAL( selectionChanged     ( QListViewItem * ) ),
	     this, SLOT  ( slotSelectionChanged	( QListViewItem * ) ) );

    selectSomething();
}


YQPkgRpmGroupTagsFilterView::~YQPkgRpmGroupTagsFilterView()
{
}


YRpmGroupsTree *
YQPkgRpmGroupTagsFilterView::rpmGroupsTree()
{
    if ( ! _rpmGroupsTree )
    {
	_rpmGroupsTree = new YRpmGroupsTree();
	CHECK_PTR( _rpmGroupsTree );

	fillRpmGroupsTree();
    }

    return _rpmGroupsTree;
}


void
YQPkgRpmGroupTagsFilterView::fillRpmGroupsTree()
{
    y2debug( "Filling RPM groups tree" );

    for ( ZyppPoolIterator it = zyppPkgBegin();
	  it != zyppPkgEnd();
	  ++it )
    {
	ZyppPkg zyppPkg = tryCastToZyppPkg( (*it)->theObj() );

	if ( zyppPkg )
	    rpmGroupsTree()->addRpmGroup( zyppPkg->group() );
    }

    y2debug( "Filling RPM groups tree done" );
}


void
YQPkgRpmGroupTagsFilterView::cloneTree( YStringTreeItem * 	parentRpmGroup,
					YQPkgRpmGroupTag * 	parentClone )
{
    YStringTreeItem * 	child = parentRpmGroup->firstChild();
    YQPkgRpmGroupTag * 	clone;

    while ( child )
    {
	if ( parentClone )
	    clone = new YQPkgRpmGroupTag( this, parentClone, child );
	else
	    clone = new YQPkgRpmGroupTag( this, child );

	CHECK_PTR( clone );
	clone->setOpen( clone->depth() < 1 );

	cloneTree( child, clone );
	child = child->next();
    }
}


void
YQPkgRpmGroupTagsFilterView::selectSomething()
{
    QListViewItem * item = firstChild();

    if ( item )
	setSelected( item, true );
}


void
YQPkgRpmGroupTagsFilterView::filterIfVisible()
{
    if ( isVisible() )
	filter();
}


void
YQPkgRpmGroupTagsFilterView::filter()
{
    emit filterStart();
    // y2debug( "Filtering packages for RPM group \"%s\"", selectedRpmGroup().c_str() );
    
    if ( selection() )
    {
	for ( ZyppPoolIterator it = zyppPkgBegin();
	      it != zyppPkgEnd();
	      ++it )
	{
	    ZyppSel selectable = *it;

	    // Multiple instances of this package may or may not be in the same
	    // RPM group, so let's check both the installed version (if there
	    // is any) and the candidate version.
	    //
	    // Make sure we emit only one filterMatch() signal if both exist
	    // and both are in the same RPM group. We don't want multiple list
	    // entries for the same package!

	    bool match =
		check( selectable, tryCastToZyppPkg( selectable->candidateObj() ) ) ||
		check( selectable, tryCastToZyppPkg( selectable->installedObj() ) );

	    // If there is neither an installed nor a candidate package, check
	    // any other instance.

	    if ( ! match			&&
		 ! selectable->candidateObj()   &&
		 ! selectable->installedObj()	  )
		check( selectable, tryCastToZyppPkg( selectable->theObj() ) );
	}
    }

    emit filterFinished();
}


void
YQPkgRpmGroupTagsFilterView::slotSelectionChanged( QListViewItem * newSelection )
{
    YQPkgRpmGroupTag * sel = dynamic_cast<YQPkgRpmGroupTag *>( newSelection );

    if ( sel )
    {
	if ( sel->rpmGroup() )
	    _selectedRpmGroup = rpmGroupsTree()->rpmGroup( sel->rpmGroup() );
	else
	    _selectedRpmGroup = "*";	// "zzz_All"
    }
    else
    {
	_selectedRpmGroup = "";
    }

    filter();
}


bool
YQPkgRpmGroupTagsFilterView::check( ZyppSel	selectable,
				    ZyppPkg	pkg		)
{
    if ( ! pkg || ! selection() )
	return false;

    if ( selection()->rpmGroup() == 0 )		// Special case: All packages
    {
	emit filterMatch( selectable, pkg );
	return true;
    }

    if ( selectedRpmGroup().empty() )
	return false;
    
    if ( pkg->group() == selectedRpmGroup() ||			// full match?
	 pkg->group().find( selectedRpmGroup() + "/" ) == 0 )	// starts with selected?
    {
	emit filterMatch( selectable, pkg );
	return true;
    }

    return false;
}


YQPkgRpmGroupTag *
YQPkgRpmGroupTagsFilterView::selection() const
{
    QListViewItem * item = selectedItem();

    if ( ! item )
	return 0;

    return dynamic_cast<YQPkgRpmGroupTag *> ( selectedItem() );
}






YQPkgRpmGroupTag::YQPkgRpmGroupTag( YQPkgRpmGroupTagsFilterView * 	parentFilterView,
				    YStringTreeItem *			rpmGroup	)
    : QListViewItem( parentFilterView )
    , _filterView( parentFilterView )
    , _rpmGroup( rpmGroup )
{
    setText( 0,  fromUTF8( _rpmGroup->value().translation() ) );
}


YQPkgRpmGroupTag::YQPkgRpmGroupTag( YQPkgRpmGroupTagsFilterView * 	parentFilterView,
				    YQPkgRpmGroupTag * 			parentGroupTag,
				    YStringTreeItem *	 		rpmGroup	)
    : QListViewItem( parentGroupTag )
    , _filterView( parentFilterView )
    , _rpmGroup( rpmGroup )
{
    setText( 0,  fromUTF8( _rpmGroup->value().translation() )  );
}


YQPkgRpmGroupTag::YQPkgRpmGroupTag( YQPkgRpmGroupTagsFilterView * 	parentFilterView,
				    const QString &			rpmGroupName,
				    YStringTreeItem *			rpmGroup	)
    : QListViewItem( parentFilterView )
    , _filterView( parentFilterView )
    , _rpmGroup( rpmGroup )
{
    setText( 0,  rpmGroupName );
}


YQPkgRpmGroupTag::~YQPkgRpmGroupTag()
{
    // NOP
}



#include "YQPkgRpmGroupTagsFilterView.moc"
