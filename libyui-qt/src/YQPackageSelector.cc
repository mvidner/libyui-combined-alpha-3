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

  File:	      YQPackageSelector.cc
  See also:   YQPackageSelectorHelp.cc

  Author:     Stefan Hundhammer <sh@suse.de>

  Textdomain "packages-qt"

/-*/

#define CHECK_DEPENDENCIES_ON_STARTUP	1
#define DEPENDENCY_FEEDBACK_IF_OK	1
#define SHOW_CHANGES_DIALOG		1
#define FAKE_DISK_USAGE_IN_TEST_MODE	0
#define AUTO_CHECK_DEPENDENCIES_DEFAULT	false

#include <qaction.h>
#include <qmap.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <qcursor.h>
#include <qdialog.h>
#include <qfiledialog.h>
#include <qhbox.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qstylefactory.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qvbox.h>

#include <Y2PM.h>
#include <y2pm/InstTarget.h>
#include <y2pm/InstYou.h>
#include <y2pm/PMManager.h>
#include <y2pm/PMPackageImEx.h>
#include <y2pm/PMPackageManager.h>
#include <y2pm/PMSelectionManager.h>
#include <y2pm/PMYouPatchManager.h>

#define y2log_component "qt-pkg"
#include <ycp/y2log.h>

#include "QY2FloppyMountDialog.h"
#include "QY2LayoutUtils.h"

#include "YQPackageSelector.h"
#include "YQPkgChangesDialog.h"
#include "YQPkgConflictDialog.h"
#include "YQPkgConflictList.h"
#include "YQPkgDependenciesView.h"
#include "YQPkgDescriptionView.h"
#include "YQPkgDiskUsageList.h"
#include "YQPkgDiskUsageWarningDialog.h"
#include "YQPkgLangFilterView.h"
#include "YQPkgLangList.h"
#include "YQPkgList.h"
#include "YQPkgRpmGroupTagsFilterView.h"
#include "YQPkgSearchFilterView.h"
#include "YQPkgSelList.h"
#include "YQPkgSelectionsFilterView.h"
#include "YQPkgStatusFilterView.h"
#include "YQPkgTechnicalDetailsView.h"
#include "YQPkgTextDialog.h"
#include "YQPkgUpdateProblemFilterView.h"
#include "YQPkgVersionsView.h"
#include "YQPkgYouPatchFilterView.h"
#include "YQPkgYouPatchList.h"

#include "QY2ComboTabWidget.h"
#include "YQDialog.h"
#include "utf8.h"
#include "YQUI.h"
#include "YEvent.h"
#include "YQi18n.h"


using std::max;
using std::string;

#define MIN_WIDTH			800
#define MIN_HEIGHT			600

#define SPACING			6	// between subwidgets
#define MARGIN			4	// around the widget



YQPackageSelector::YQPackageSelector( QWidget * 		parent,
				      const YWidgetOpt & 	opt,
				      const YCPString & 	floppyDevice )
    : QVBox( parent )
    , YPackageSelector( opt )
    , _floppyDevice( floppyDevice->value().c_str() )
{
    setWidgetRep( this );

    _autoDependenciesCheckBox	= 0;
    _checkDependenciesButton	= 0;
    _detailsViews		= 0;
    _diskUsageList		= 0;
    _filters			= 0;
    _langFilterView		= 0;
    _langList			= 0;
    _pkgDependenciesView	= 0;
    _pkgDescriptionView		= 0;
    _pkgList			= 0;
    _pkgTechnicalDetailsView	= 0;
    _pkgVersionsView		= 0;
    _rpmGroupTagsFilterView	= 0;
    _searchFilterView		= 0;
    _selConflictDialog		= 0;
    _selList			= 0;
    _selectionsFilterView	= 0;
    _statusFilterView		= 0;
    _updateProblemFilterView	= 0;
    _youPatchFilterView		= 0;
    _youPatchList		= 0;

    _searchMode	 = opt.searchMode.value();
    _testMode	 = opt.testMode.value();
    _updateMode	 = opt.updateMode.value();
    _youMode	 = opt.youMode.value();
    _summaryMode = opt.summaryMode.value();

    if ( _testMode )	fakeData();
    if ( _youMode )	y2milestone( "YOU mode" );
    if ( _updateMode )	y2milestone( "Update mode" );


    YQUI::setTextdomain( "packages-qt" );
    setFont( YQUI::ui()->currentFont() );
    YQUI::ui()->blockWmClose(); // Automatically undone after UI::RunPkgSelection()
    _installedPkgs = Y2PM::instTarget().numPackages();

    _pkgConflictDialog = new YQPkgConflictDialog( &( Y2PM::packageManager() ), this );
    CHECK_PTR( _pkgConflictDialog );

    basicLayout();
    addMenus();		// Only after all widgets are created!
    makeConnections();
    emit loadData();

    YQPkgConflict::loadIgnoredConflicts();
    Y2PM::packageManager().SaveState();

    if ( _youMode )
    {
	Y2PM::youPatchManager().SaveState();

	if ( _filters && _youPatchFilterView && _youPatchList )
	{
	    _filters->showPage( _youPatchFilterView );
	    _youPatchList->filter();
	    _youPatchList->clearSelection();
	    _youPatchList->selectSomething();
	}
    }
    else
    {
	Y2PM::selectionManager().SaveState();

	if ( _filters )
	{
	    if ( _updateProblemFilterView )
	    {
		_filters->showPage( _updateProblemFilterView );
		_updateProblemFilterView->filter();

	    }
	    else if ( _searchMode && _searchFilterView )
	    {
		_filters->showPage( _searchFilterView );
		_searchFilterView->filter();
		QTimer::singleShot( 0, _searchFilterView, SLOT( setFocus() ) );
	    }
	    else if ( _summaryMode && _statusFilterView )
	    {
		_filters->showPage( _statusFilterView );
		_statusFilterView->filter();
	    }
	    else if ( _selectionsFilterView && _selList )
	    {
		_filters->showPage( _selectionsFilterView );
		_selList->filter();
	    }
	}
    }


#if FAKE_DISK_USAGE_IN_TEST_MODE

    if ( _testMode && _diskUsageList )
    {
	y2milestone( "Using fake disk usage data" );
	_diskUsageList->clear();
	_diskUsageList->fakeData();
    }
#endif

    if ( _diskUsageList )
	_diskUsageList->updateDiskUsage();

    y2debug( "Floppy device: %s", (const char *) _floppyDevice );
    y2milestone( "PackageSelector init done" );


#if CHECK_DEPENDENCIES_ON_STARTUP

    if ( ! _youMode )
    {
	// Fire up the first dependency check in the main loop.
	// Don't do this right away - wait until all initializations are finished.
	QTimer::singleShot( 0, this, SLOT( resolvePackageDependencies() ) );
    }
#endif
}


void
YQPackageSelector::basicLayout()
{
    layoutMenuBar( this );

    QSplitter * outer_splitter = new QSplitter( QSplitter::Horizontal, this );
    CHECK_PTR( outer_splitter );

    QWidget * left_pane  = layoutLeftPane ( outer_splitter );
    QWidget * right_pane = layoutRightPane( outer_splitter );

    outer_splitter->setResizeMode( left_pane,  QSplitter::FollowSizeHint );
    outer_splitter->setResizeMode( right_pane, QSplitter::Stretch );
}


QWidget *
YQPackageSelector::layoutLeftPane( QWidget * parent )
{
    QSplitter * splitter = new QSplitter( QSplitter::Vertical, parent );
    CHECK_PTR( splitter );
    splitter->setMargin( MARGIN );

    QVBox * vbox = new QVBox( splitter );
    CHECK_PTR( vbox );
    layoutFilters( vbox );
    addVSpacing( vbox, MARGIN );

    vbox = new QVBox( splitter );
    addVSpacing( vbox, MARGIN );
    _diskUsageList = new YQPkgDiskUsageList( vbox );
    CHECK_PTR( _diskUsageList );

    return splitter;
}


void
YQPackageSelector::layoutFilters( QWidget * parent )
{
    _filters = new QY2ComboTabWidget( _( "Fi&lter:" ), parent );
    CHECK_PTR( _filters );


    //
    // Update problem view
    //

    if ( _updateMode )
    {
	if ( ! Y2PM::packageManager().updateEmpty()
	     || _testMode )
	{
	    _updateProblemFilterView = new YQPkgUpdateProblemFilterView( parent );
	    CHECK_PTR( _updateProblemFilterView );
	    _filters->addPage( _( "Update Problems" ), _updateProblemFilterView );
	}
    }


    //
    // YOU patches view
    //

    if ( _youMode )
    {
	_youPatchFilterView = new YQPkgYouPatchFilterView( parent );
	CHECK_PTR( _youPatchFilterView );
	_filters->addPage( _( "YOU Patches" ), _youPatchFilterView );

	_youPatchList = _youPatchFilterView->youPatchList();
	CHECK_PTR( _youPatchList );
    }


    //
    // Selections view
    //

    if ( ! _youMode )
    {
	_selectionsFilterView = new YQPkgSelectionsFilterView( parent );
	CHECK_PTR( _selectionsFilterView );
	_filters->addPage( _( "Selections" ), _selectionsFilterView );

	_selList = _selectionsFilterView->selList();
	CHECK_PTR( _selList );

	_selConflictDialog = new YQPkgConflictDialog( &( Y2PM::selectionManager() ), this );
	CHECK_PTR( _selConflictDialog );

	connect( _selList, 		SIGNAL( statusChanged()	               	),
		 this,			SLOT  ( resolveSelectionDependencies()	) );

	connect( _selConflictDialog,	SIGNAL( updatePackages()      		),
		 _selList, 		SLOT  ( updateToplevelItemStates() 	) );

	connect( this,			SIGNAL( refresh()			),
		 _selList, 		SLOT  ( updateToplevelItemStates() 	) );
    }


    //
    // RPM group tags view
    //

    // if ( ! _youMode )
    {
	_rpmGroupTagsFilterView = new YQPkgRpmGroupTagsFilterView( parent );
	CHECK_PTR( _rpmGroupTagsFilterView );
	_filters->addPage( _( "Package Groups" ), _rpmGroupTagsFilterView );

	connect( this,    			SIGNAL( loadData() ),
		 _rpmGroupTagsFilterView,	SLOT  ( filter()   ) );
    }


    //
    // Languages view
    //

    if ( ! _youMode )
    {
	_langFilterView = new YQPkgLangFilterView( parent );
	CHECK_PTR( _langFilterView );
	_filters->addPage( _( "Languages" ), _langFilterView );

	_langList = _langFilterView->langList();
	CHECK_PTR( _langList );

	_selConflictDialog = new YQPkgConflictDialog( &( Y2PM::selectionManager() ), this );
	CHECK_PTR( _selConflictDialog );

	connect( _langList, 		SIGNAL( statusChanged()	               	),
		 this,			SLOT  ( resolveSelectionDependencies()	) );

	connect( _selConflictDialog,	SIGNAL( updatePackages()      		),
		 _langList, 		SLOT  ( updateToplevelItemStates() 	) );

	connect( this,			SIGNAL( refresh()			),
		 _langList, 		SLOT  ( updateToplevelItemStates() 	) );
    }


    //
    // Package search view
    //

    // if ( ! _youMode )
    {
	_searchFilterView = new YQPkgSearchFilterView( parent );
	CHECK_PTR( _searchFilterView );
	_filters->addPage( _( "Search" ), _searchFilterView );
    }


    //
    // Status change view
    //

    // if ( ! _youMode )
    {
	_statusFilterView = new YQPkgStatusFilterView( parent );
	CHECK_PTR( _statusFilterView );
	_filters->addPage( _( "Installation Summary" ), _statusFilterView );
    }


#if 0
    // DEBUG

    _filters->addPage( _( "Keywords" ), new QLabel( "Keywords\nfilter\n\nfor future use", 0 ) );
    _filters->addPage( _( "MIME Types" ), new QLabel( "MIME Types\nfilter\n\nfor future use" , 0 ) );
#endif

}


QWidget *
YQPackageSelector::layoutRightPane( QWidget * parent )
{
    QSplitter * splitter = new QSplitter( QSplitter::Vertical, parent );
    CHECK_PTR( splitter );
    splitter->setMargin( MARGIN );

    QVBox * vbox = new QVBox( splitter );
    CHECK_PTR( vbox );
    layoutPkgList( vbox );
    addVSpacing( vbox, MARGIN );

    layoutDetailsViews( splitter );

    return splitter;
}


void
YQPackageSelector::layoutPkgList( QWidget * parent )
{
    _pkgList= new YQPkgList( parent );
    CHECK_PTR( _pkgList );

    if ( _youMode )
    {
	_pkgList->setEditable( false );
    }

    connect( _pkgList, 	SIGNAL( statusChanged()	          ),
	     this,	SLOT  ( autoResolveDependencies() ) );
}


void
YQPackageSelector::layoutDetailsViews( QWidget * parent )
{
    QVBox * details_vbox = new QVBox( parent );
    CHECK_PTR( details_vbox );
    details_vbox->setMinimumSize( 0, 0 );

    addVSpacing( details_vbox, 8 );

    _detailsViews = new QTabWidget( details_vbox );
    CHECK_PTR( _detailsViews );
    _detailsViews->setMargin( MARGIN );

    // _detailsViews->setTabPosition( QTabWidget::Bottom );


    //
    // Description
    //

    _pkgDescriptionView = new YQPkgDescriptionView( _detailsViews );
    CHECK_PTR( _pkgDescriptionView );

    _detailsViews->addTab( _pkgDescriptionView, _( "D&escription" ) );
    _detailsViews->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) ); // hor/vert

    connect( _pkgList,			SIGNAL( selectionChanged    ( PMObjectPtr ) ),
	     _pkgDescriptionView,	SLOT  ( showDetailsIfVisible( PMObjectPtr ) ) );

    //
    // Technical details
    //

    _pkgTechnicalDetailsView = new YQPkgTechnicalDetailsView( _detailsViews, _youMode );
    CHECK_PTR( _pkgTechnicalDetailsView );

    _detailsViews->addTab( _pkgTechnicalDetailsView, _( "&Technical Data" ) );

    connect( _pkgList,			SIGNAL( selectionChanged    ( PMObjectPtr ) ),
	     _pkgTechnicalDetailsView,	SLOT  ( showDetailsIfVisible( PMObjectPtr ) ) );


    //
    // Dependencies
    //

    _pkgDependenciesView = new YQPkgDependenciesView( _detailsViews );
    CHECK_PTR( _pkgDependenciesView );

    _detailsViews->addTab( _pkgDependenciesView, _( "Dependencies" ) );
    _detailsViews->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) ); // hor/vert

    connect( _pkgList,			SIGNAL( selectionChanged    ( PMObjectPtr ) ),
	     _pkgDependenciesView,	SLOT  ( showDetailsIfVisible( PMObjectPtr ) ) );


    //
    // Versions
    //

    // if ( ! _youMode )
    {
	_pkgVersionsView = new YQPkgVersionsView( _detailsViews,
						  ! _youMode );	// userCanSwitchVersions
	CHECK_PTR( _pkgVersionsView );

	_detailsViews->addTab( _pkgVersionsView, _( "&Versions" ) );

	connect( _pkgList,		SIGNAL( selectionChanged    ( PMObjectPtr ) ),
		 _pkgVersionsView,	SLOT  ( showDetailsIfVisible( PMObjectPtr ) ) );
    }


    layoutButtons( details_vbox );
}


void
YQPackageSelector::layoutButtons( QWidget * parent )
{
    QHBox * button_box = new QHBox( parent );
    CHECK_PTR( button_box );
    button_box->setSpacing( SPACING );

    if ( ! _youMode )
    {
	_checkDependenciesButton = new QPushButton( _( "Check &Dependencies" ), button_box );
	CHECK_PTR( _checkDependenciesButton );
	_checkDependenciesButton->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) ); // hor/vert

	connect( _checkDependenciesButton,	SIGNAL( clicked() ),
		 this,         			SLOT  ( manualResolvePackageDependencies() ) );


	_autoDependenciesCheckBox = new QCheckBox( _( "A&utocheck" ), button_box );
	CHECK_PTR( _autoDependenciesCheckBox );
	_autoDependenciesCheckBox->setChecked( AUTO_CHECK_DEPENDENCIES_DEFAULT );
    }

    addHStretch( button_box );

    QPushButton * cancel_button = new QPushButton( _( "&Cancel" ), button_box );
    CHECK_PTR( cancel_button );
    cancel_button->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) ); // hor/vert

    connect( cancel_button, SIGNAL( clicked() ),
	     this,          SLOT  ( reject()   ) );


    QPushButton * accept_button = new QPushButton( _( "&Accept" ), button_box );
    CHECK_PTR( accept_button );
    accept_button->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) ); // hor/vert

    connect( accept_button, SIGNAL( clicked() ),
	     this,          SLOT  ( accept()   ) );

    button_box->setMinimumHeight( button_box->sizeHint().height() );
}


void
YQPackageSelector::layoutMenuBar( QWidget * parent )
{
    _menuBar = new QMenuBar( parent );
    CHECK_PTR( _menuBar );

    _fileMenu		= 0;
    _viewMenu		= 0;
    _pkgMenu 		= 0;
    _youPatchMenu 	= 0;
    _extrasMenu		= 0;
    _helpMenu 		= 0;

}


void
YQPackageSelector::addMenus()
{
    //
    // File menu
    //

    _fileMenu = new QPopupMenu( _menuBar );
    CHECK_PTR( _fileMenu );
    _menuBar->insertItem( _( "&File" ), _fileMenu );

    if ( ! _youMode )
    {
	_fileMenu->insertItem( _( "&Import..." ),	this, SLOT( pkgImport() ) );
	_fileMenu->insertItem( _( "&Export..." ),	this, SLOT( pkgExport() ) );

	_fileMenu->insertSeparator();
    }

    _fileMenu->insertItem( _( "E&xit -- Discard Changes" ), this, SLOT( reject() ) );
    _fileMenu->insertItem( _( "&Quit -- Save Changes" ), this, SLOT( accept() ) );


    if ( _pkgList && ! _youMode )
    {
	//
	// Package menu
	//

	_pkgMenu = new QPopupMenu( _menuBar );
	CHECK_PTR( _pkgMenu );
	_menuBar->insertItem( _( "&Package" ), _pkgMenu );

	_pkgList->actionSetCurrentInstall->addTo( _pkgMenu );
	_pkgList->actionSetCurrentDontInstall->addTo( _pkgMenu );
	_pkgList->actionSetCurrentKeepInstalled->addTo( _pkgMenu );
	_pkgList->actionSetCurrentDelete->addTo( _pkgMenu );
	_pkgList->actionSetCurrentUpdate->addTo( _pkgMenu );
	_pkgList->actionSetCurrentTaboo->addTo( _pkgMenu );
	_pkgList->actionSetCurrentProtected->addTo( _pkgMenu );

	_pkgMenu->insertSeparator();

	_pkgList->actionInstallSourceRpm->addTo( _pkgMenu );
	_pkgList->actionDontInstallSourceRpm->addTo( _pkgMenu );

	_pkgMenu->insertSeparator();
        QPopupMenu * submenu = _pkgList->addAllInListSubMenu( _pkgMenu );
	CHECK_PTR( submenu );

	submenu->insertSeparator();
	_pkgList->actionInstallListSourceRpms->addTo( submenu );
	_pkgList->actionDontInstallListSourceRpms->addTo( submenu );
    }


    if ( _youMode && _youPatchList )
    {
	//
	// YOU Patch menu
	//

	_youPatchMenu = new QPopupMenu( _menuBar );
	CHECK_PTR( _youPatchMenu );
	_menuBar->insertItem( _( "&YOU Patch" ), _youPatchMenu );

	_youPatchList->actionSetCurrentInstall->addTo( _youPatchMenu );
	_youPatchList->actionSetCurrentDontInstall->addTo( _youPatchMenu );
	_youPatchList->actionSetCurrentKeepInstalled->addTo( _youPatchMenu );
	_youPatchList->actionSetCurrentUpdate->addTo( _youPatchMenu );
	_youPatchList->actionSetCurrentTaboo->addTo( _youPatchMenu );

	_youPatchMenu->insertSeparator();
        _youPatchList->addAllInListSubMenu( _youPatchMenu );
    }


    //
    // Extras menu
    //

    _extrasMenu = new QPopupMenu( _menuBar );
    CHECK_PTR( _extrasMenu );
    _menuBar->insertItem( _( "&Extras" ), _extrasMenu );

    if ( ! _youMode )
	_extrasMenu->insertItem( _( "Show &Automatic Package Changes" ), this, SLOT( showAutoPkgList() ), CTRL + Key_A );

    if ( ! _youMode && _pkgConflictDialog )
	YQPkgConflict::actionResetIgnoredConflicts( _pkgConflictDialog )->addTo( _extrasMenu );

    if ( _youMode && _youPatchList )
	_youPatchList->actionShowRawPatchInfo->addTo( _extrasMenu );

    if ( ! _youMode )
	// Translators: This is about packages ending in "-devel", so don't translate that "-devel"!
	_extrasMenu->insertItem( _( "Install All Matching -&devel Packages" ), this, SLOT( installDevelPkgs() ) );

    if ( ! _youMode )
	// Translators: This is about packages ending in "-debuginfo", so don't translate that "-debuginfo"!
	_extrasMenu->insertItem( _( "Install All Matching -de&buginfo Packages" ), this, SLOT( installDebugInfoPkgs() ) );

    //
    // Help menu
    //

    _helpMenu = new QPopupMenu( _menuBar );
    CHECK_PTR( _helpMenu );
    _menuBar->insertSeparator();
    _menuBar->insertItem( _( "&Help" ), _helpMenu );

    // Note: The help functions and their texts are moved out
    // to a separate source file YQPackageSelectorHelp.cc

    // Menu entry for help overview
    _helpMenu->insertItem( _( "&Overview" ), this, SLOT( help() 	), Key_F1         );

    // Menu entry for help about used symbols ( icons )
    _helpMenu->insertItem( _( "&Symbols" ), this, SLOT( symbolHelp() 	), SHIFT + Key_F1 );

    // Menu entry for keyboard help
    _helpMenu->insertItem( _( "&Keys" ), this, SLOT( keyboardHelp() )                 );
}


void
YQPackageSelector::connectFilter( QWidget * filter,
				  QWidget * pkgList,
				  bool hasUpdateSignal )
{
    if ( ! filter  )	return;
    if ( ! pkgList )	return;

    if ( _filters )
    {
	connect( _filters,	SIGNAL( currentChanged(QWidget *) ),
		 filter,	SLOT  ( filterIfVisible()            ) );
    }

    connect( this,	SIGNAL( refresh()	  ),
	     filter,	SLOT  ( filterIfVisible() ) );

    connect( filter,	SIGNAL( filterStart() 	),
	     pkgList, 	SLOT  ( clear() 	) );

    connect( filter,	SIGNAL( filterMatch( PMPackagePtr ) ),
	     pkgList, 	SLOT  ( addPkgItem ( PMPackagePtr ) ) );

    connect( filter, 	SIGNAL( filterFinished()  ),
	     pkgList, 	SLOT  ( selectSomething() ) );


    if ( hasUpdateSignal )
    {
	connect( filter, 		SIGNAL( updatePackages()           ),
		 pkgList,		SLOT  ( updateToplevelItemStates() ) );

	if ( _diskUsageList )
	{
	    connect( filter,		SIGNAL( updatePackages()	   ),
		     _diskUsageList,	SLOT  ( updateDiskUsage()	   ) );
	}
    }
}


void
YQPackageSelector::makeConnections()
{
    connectFilter( _updateProblemFilterView,	_pkgList, false );
    connectFilter( _youPatchList, 		_pkgList );
    connectFilter( _selList, 			_pkgList );
    connectFilter( _rpmGroupTagsFilterView, 	_pkgList, false );
    connectFilter( _langList, 			_pkgList );
    connectFilter( _statusFilterView, 		_pkgList, false );
    connectFilter( _searchFilterView, 		_pkgList, false );

    if ( _searchFilterView && _pkgList )
    {
	connect( _searchFilterView, 	SIGNAL( message( const QString & ) ),
		 _pkgList,		SLOT  ( message( const QString & ) ) );
    }

    if ( _pkgList && _diskUsageList )
    {

	connect( _pkgList,		SIGNAL( statusChanged()   ),
		 _diskUsageList,	SLOT  ( updateDiskUsage() ) );
    }

    if ( _pkgList && _youPatchList )
    {
	connect( _youPatchList, SIGNAL( filterMatch   ( const QString &, const QString &, FSize ) ),
		 _pkgList,	SLOT  ( addPassiveItem( const QString &, const QString &, FSize ) ) );
    }

    //
    // Connect package conflict dialog
    //

    if ( _pkgConflictDialog && _pkgList )
    {
	connect( _pkgConflictDialog,	SIGNAL( updatePackages()      ),
		 _pkgList, 		SLOT  ( updateToplevelItemStates() ) );

	if ( _diskUsageList )
	{
	    connect( _pkgConflictDialog, SIGNAL( updatePackages()	   ),
		     _diskUsageList,	 SLOT  ( updateDiskUsage()	   ) );
	}
    }


    //
    // Connect selection conflict dialog
    //

    if ( _selConflictDialog && _selList )
    {
	connect( _selConflictDialog,	SIGNAL( updatePackages()      	),
		 _selList, 		SLOT  ( applyChanges() 		) );

	connect( _selConflictDialog,	SIGNAL( updatePackages()      	),
		 _selList, 		SLOT  ( updateToplevelItemStates() ) );

	if ( _diskUsageList )
	{
	    connect( _selConflictDialog, SIGNAL( updatePackages()	   ),
		     _diskUsageList,	 SLOT  ( updateDiskUsage()	   ) );
	}
    }



    //
    // Connect package versions view
    //

    if ( _pkgVersionsView && _pkgList )
    {
	connect( _pkgVersionsView, 	SIGNAL( candidateChanged( PMObjectPtr ) ),
		 _pkgList,		SLOT  ( updateToplevelItemData() ) );
    }



    //
    // Update actions just before opening menus
    //

    if ( _pkgMenu && _pkgList )
    {
	connect( _pkgMenu, 	SIGNAL( aboutToShow()   ),
		 _pkgList, 	SLOT  ( updateActions() ) );
    }

    if ( _youPatchMenu && _youPatchList )
    {
	connect( _youPatchMenu,	SIGNAL( aboutToShow()   ),
		 _youPatchList, SLOT  ( updateActions() ) );
    }


    //
    // Handle WM_CLOSE like "Cancel"
    //

    connect( YQUI::ui(),	SIGNAL( wmClose() ),
	     this,		SLOT  ( reject()   ) );
}


void
YQPackageSelector::autoResolveDependencies()
{
    if ( _autoDependenciesCheckBox && ! _autoDependenciesCheckBox->isChecked() )
	return;

    resolvePackageDependencies();
}


int
YQPackageSelector::manualResolvePackageDependencies()
{
    if ( ! _pkgConflictDialog )
    {
	y2error( "No package conflict dialog existing" );
	return QDialog::Accepted;
    }

    YQUI::ui()->busyCursor();

    int result = _pkgConflictDialog->solveAndShowConflicts();

    YQUI::ui()->normalCursor();

#if DEPENDENCY_FEEDBACK_IF_OK

    if ( result == QDialog::Accepted )
    {
	QMessageBox::information( this, "",
				  _( "All package dependencies are OK." ),
				  QMessageBox::Ok );
    }
#endif

    return result;
}


int
YQPackageSelector::resolvePackageDependencies()
{
    if ( ! _pkgConflictDialog )
    {
	y2error( "No package conflict dialog existing" );
	return QDialog::Accepted;
    }


    YQUI::ui()->busyCursor();

    QColor oldBackground;

    if ( _checkDependenciesButton )
    {
	oldBackground = _checkDependenciesButton->paletteBackgroundColor();
	_checkDependenciesButton->setPaletteBackgroundColor( QColor( 0xE0, 0xE0, 0xF8 ) );
	_checkDependenciesButton->repaint();
    }

    int result = _pkgConflictDialog->solveAndShowConflicts();

    if ( _checkDependenciesButton )
	_checkDependenciesButton->setPaletteBackgroundColor( oldBackground );

    YQUI::ui()->normalCursor();

    return result;
}


int
YQPackageSelector::resolveSelectionDependencies()
{
    if ( _selConflictDialog )
    {
	y2debug( "Resolving selection dependencies" );
	return _selConflictDialog->solveAndShowConflicts();
    }
    else
	return QDialog::Accepted;
}


int
YQPackageSelector::checkDiskUsage()
{
    if ( ! _diskUsageList )
    {
	y2warning( "No disk usage list existing, assuming disk usage is OK" );
	return QDialog::Accepted;
    }

    if ( ! _diskUsageList->overflowWarning.inRange() )
	return QDialog::Accepted;

    QString msg =
	// Translators: RichText ( HTML-like ) format
	"<p><b>" + _( "Error: Out of disk space!" ) + "</b></p>"
	+ _( "<p>"
	     "You can choose to install anyway if you know what you are doing, "
	     "but you risk getting a corrupted system that requires manual repairs. "
	     "If you are not absolutely sure how to handle such a case, "
	     "press <b>Cancel</b> now and deselect some packages."
	     "</p>" );

    return YQPkgDiskUsageWarningDialog::diskUsageWarning( msg,
							  100, _( "C&ontinue Anyway" ), _( "&Cancel" ) );

}


void
YQPackageSelector::pkgExport()
{
    QY2FloppyMountDialog fileDialog( this,				// parent
				      "user.sel",			// startWith
				      "*.sel;;*",			// filter
				      _( "Save Package List" ),		// caption
				      _floppyDevice,			// floppyDevice
				      "/media/floppy",			// floppyMountPoint
				      false );				// startWithFloppy

    QString filename = fileDialog.askForSaveFileName();

#if 0
    QString filename = YQUI::ui()->askForSaveFileName( QString( "user.sel" ),	// startsWith
							   QString( "*.sel;;*" ),	// filter
							   _( "Save Package List" ) );
#endif


    if ( ! filename.isEmpty() )
    {
	y2milestone( "Exporting package list to %s", (const char *) filename );
	PMPackageImEx exporter;
	exporter.getPMState();

	if ( exporter.doExport( Pathname( (const char *) filename ) ) )
	{
	    // Success

	    autoResolveDependencies();
	}
	else	// Error
	{
	    y2warning( "Error writing package list to %s", (const char *) filename );

	    // PMPackageImEx::doExport() might have left over a partially written file.
	    // Try to delete that. Don't care if it doesn't exist and unlink() fails.
	    ( void ) unlink( (const char *) filename );

	    // Post error popup
	    QMessageBox::warning( this,						// parent
				  _( "Error" ),					// caption
				  _( "Error writing package list to %1" ).arg( filename ),
				  QMessageBox::Ok | QMessageBox::Default,	// button0
				  QMessageBox::NoButton,			// button1
				  QMessageBox::NoButton );			// button2
	}

	fileDialog.unmount( true ); // manual call only necessary if verbose mode desired
    }
}


void
YQPackageSelector::pkgImport()
{
    QY2FloppyMountDialog fileDialog( this,				// parent
				      "user.sel",			// startWith
				      "*.sel;;*",			// filter
				      _( "Load Package List" ),		// caption
				      _floppyDevice,			// floppyDevice
				      "/media/floppy",			// floppyMountPoint
				      false );				// startWithFloppy

    QString filename = fileDialog.askForExistingFile();

#if 0
    QString filename = 	QFileDialog::getOpenFileName( "user.sel",			// startsWith
						      "*.sel;;*",			// filter
						      this,				// parent
						      0,				// name
						      _( "Load Package List" ) );	// caption
#endif

    if ( ! filename.isEmpty() )
    {
	y2milestone( "Importing package list from %s", (const char *) filename );
	PMPackageImEx importer;
	importer.getPMState();

	if ( importer.doImport( Pathname( (const char *) filename ) ) )
	{
	    // Success

	    importer.setPMState();
	    emit refresh();
	}
	else // Error
	{
	    y2warning( "Error reading package list from %s", (const char *) filename );

	    // Post error popup
	    QMessageBox::warning( this,						// parent
				  _( "Error" ),					// caption
				  _( "Error loading package list from %1" ).arg( filename ),
				  QMessageBox::Ok | QMessageBox::Default,	// button0
				  QMessageBox::NoButton,			// button1
				  QMessageBox::NoButton );			// button2
	}

 	fileDialog.unmount( true ); // manual call only necessary if verbose mode desired
   }
}


void
YQPackageSelector::showAutoPkgList()
{
    resolvePackageDependencies();

    // Show which packages are installed/deleted automatically
    QString msg =
	"<p><b>"
	// Dialog header
	+ _( "Automatic Changes" )
	+ "</b></p>"
	// Detailed explanation ( automatic word wrap! )
	+ "<p>"
	+ _( "In addition to your manual selections, the following packages"
	     " have been changed to resolve dependencies:" )
	+ "<p>";

    YQPkgChangesDialog::showChangesDialog( msg,
					   _( "&OK" ),
					   QString::null,	// rejectButtonLabel
					   true );		// showIfEmpty
}


void
YQPackageSelector::installDevelPkgs()
{
    installSubPkgs( "-devel" );
}


void
YQPackageSelector::installDebugInfoPkgs()
{
    installSubPkgs( "-debuginfo" );
}


void
YQPackageSelector::installSubPkgs( const QString suffix )
{
    // Find all matching packages and put them into a QMap

    QMap<QString, PMSelectablePtr> subPkgs;

    for ( PMManager::PMSelectableVec::const_iterator it = Y2PM::packageManager().begin();
	  it != Y2PM::packageManager().end();
	  ++it )
    {
	QString name = (*it)->name().asString().c_str();

	if ( name.endsWith( suffix ) )
	{
	    subPkgs[ name ] = *it;

	    y2debug( "Found subpackage: %s", (const char *) name );
	}
    }


    // Now go through all packages and look if there is a corresponding subpackage in the QMap

    for ( PMManager::PMSelectableVec::const_iterator it = Y2PM::packageManager().begin();
	  it != Y2PM::packageManager().end();
	  ++it )
    {
	QString name = (*it)->name().asString().c_str();

	if ( subPkgs.contains( name + suffix ) )
	{
	    PMSelectablePtr subPkg = subPkgs[ name + suffix ];
	    QString subPkgName;

	    switch ( (*it)->status() )
	    {
		case PMSelectable::S_AutoDel:
		case PMSelectable::S_NoInst:
		case PMSelectable::S_Protected:
		case PMSelectable::S_Taboo:
		case PMSelectable::S_Del:
		    // Don't install the subpackage
		    y2milestone( "Ignoring unwanted subpackage %s", (const char *) subPkgName );
		    break;

		case PMSelectable::S_AutoInstall:
		case PMSelectable::S_Install:
		case PMSelectable::S_KeepInstalled:

		    // Install the subpackage, but don't try to update it

		    if ( ! subPkg->installedObj() )
		    {
			subPkg->set_status( PMSelectable::S_Install );
			y2milestone( "Installing subpackage %s", (const char *) subPkgName );
		    }
		    break;


		case PMSelectable::S_Update:
		case PMSelectable::S_AutoUpdate:

		    // Install or update the subpackage

		    if ( ! subPkg->installedObj() )
		    {
			subPkg->set_status( PMSelectable::S_Install );
			y2milestone( "Installing subpackage %s", (const char *) subPkgName );
		    }
		    else
		    {
			subPkg->set_status( PMSelectable::S_Update );
			y2milestone( "Updating subpackage %s", (const char *) subPkgName );
		    }
		    break;

		    // Intentionally omitting 'default' branch so the compiler can
		    // catch unhandled enum states
	    }
	}
    }


    if ( _filters && _statusFilterView )
    {
	_filters->showPage( _statusFilterView );
	_statusFilterView->filter();
    }

    YQPkgChangesDialog::showChangesDialog( _( "Added Subpackages:" ),
					   QRegExp( ".*" + suffix + "$" ),
					   _( "&OK" ),
					   QString::null,	// rejectButtonLabel
					   true );		// showIfEmpty
}



void
YQPackageSelector::fakeData()
{
#if 0
    y2warning( "*** Using fake data ***" );

    if ( _youMode )
    {
	PMYouServer server( "dir:///8.1-patches" );
	InstYou &you = Y2PM::youPatchManager().instYou();
	you.settings()->setPatchServer( server );
	you.settings()->setReloadPatches( false );
        you.retrievePatchInfo();
	Y2PM::youPatchManager().instYou().selectPatches( PMYouPatch::kind_recommended |
							 PMYouPatch::kind_security     );
	y2milestone( "Fake YOU patches initialized" );
    }
    else
    {
	InstSrcManager & MGR( Y2PM::instSrcManager() );

	Url url( "dir:///8.1" );
	InstSrcManager::ISrcIdList nids;
	PMError err = MGR.scanMedia( nids, url );

	if ( nids.size() )
	{
	    err = MGR.enableSource( *nids.begin() );
	}

	y2milestone( "Fake installation sources initialized" );
    }
#endif
}


void
YQPackageSelector::reject()
{
    bool changes = Y2PM::packageManager().DiffState();

    if ( _youMode )
	changes |= Y2PM::youPatchManager().DiffState();
    else
	changes |= Y2PM::selectionManager().DiffState();

    if ( ! changes ||
	 ( QMessageBox::warning( this, "",
				 _( "Abandon all changes?" ),
				 _( "&OK" ), _( "&Cancel" ), "",
				 1, // defaultButtonNumber ( from 0 )
				 1 ) // escapeButtonNumber
	   == 0 )	// Proceed upon button #0 ( OK )
	 )
    {
	Y2PM::packageManager().RestoreState();

	if ( _youMode )
	    Y2PM::youPatchManager().RestoreState();
	else
	    Y2PM::selectionManager().RestoreState();

	YQUI::ui()->sendEvent( new YCancelEvent() );
    }
}


void
YQPackageSelector::accept()
{
    if ( ! _youMode )
    {
	// Force final dependency resolving
	if ( resolvePackageDependencies() == QDialog::Rejected )
	    return;
    }

#if SHOW_CHANGES_DIALOG

    if ( ! _youMode )
    {
	// Show which packages are installed/deleted automatically
	QString msg =
	    "<p><b>"
	    // Dialog header
	    + _( "Automatic Changes" )
	    + "</b></p>"
	    // Detailed explanation ( automatic word wrap! )
	    + "<p>"
	    + _( "In addition to your manual selections, the following packages"
		 " have been changed to resolve dependencies:" )
	    + "<p>";

	if ( YQPkgChangesDialog::showChangesDialog( msg, _( "C&ontinue" ), _( "&Cancel" ) )
	     == QDialog::Rejected )
	    return;
    }
#endif

    // Check disk usage
    if ( checkDiskUsage() == QDialog::Rejected )
	return;


    Y2PM::packageManager().ClearSaveState();
    YQPkgConflict::saveIgnoredConflicts();

    if ( _youMode )
	Y2PM::youPatchManager().ClearSaveState();
    else
	Y2PM::selectionManager().ClearSaveState();

    YQUI::ui()->sendEvent( new YMenuEvent( YCPSymbol( "accept" ) ) );
}


void
YQPackageSelector::notImplemented()
{
    QMessageBox::information( this, "",
			      _( "Not implemented yet. Sorry." ),
			      QMessageBox::Ok );
}


void
YQPackageSelector::keyPressEvent( QKeyEvent * event )
{
    if ( event )
    {
	unsigned special_combo = ( Qt::ControlButton | Qt::ShiftButton | Qt::AltButton );

	if ( ( event->state() & special_combo ) == special_combo )
	{
	    if ( event->key() == Qt::Key_A )
	    {
		showAutoPkgList();
		event->accept();
		return;
	    }
	}
	else if ( event->key() == Qt::Key_F5 )	// No matter if Ctrl/Alt/Shift pressed
	{
	    YQUI::ui()->easterEgg();
	    return;
	}
    }

    QWidget::keyPressEvent( event );
}


long
YQPackageSelector::nicesize( YUIDimension dim )
{
    if ( dim == YD_HORIZ )
    {
	int hintWidth = sizeHint().width();

	return max( MIN_WIDTH, hintWidth );
    }
    else
    {
	int hintHeight = sizeHint().height();

	return max( MIN_HEIGHT, hintHeight );
    }
}


void
YQPackageSelector::setSize( long newWidth, long newHeight )
{
    resize( newWidth, newHeight );
}


void
YQPackageSelector::setEnabling( bool enabled )
{
    setEnabled( enabled );
}


bool
YQPackageSelector::setKeyboardFocus()
{
    setFocus();

    return true;
}


#include "YQPackageSelector.moc"
