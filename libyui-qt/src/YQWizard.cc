/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|						     (c) SuSE Linux AG |
\----------------------------------------------------------------------/

  File:		YQWizard.cc

  Author:	Stefan Hundhammer <sh@suse.de>

  Textdomain	"packages-qt"

/-*/


#include "YQWizard.h"
#define y2log_component "qt-wizard"
#include <ycp/y2log.h>

// For the command parser

#include <ycp/YCPBoolean.h>
#include <ycp/YCPCode.h>
#include <ycp/YCPInteger.h>
#include "ycp/YCPInteger.h"
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPString.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPTerm.h>
#include <ycp/YCPValue.h>
#include <ycp/YCPVoid.h>
#include <YShortcut.h>

#include <string>

#include <qhbox.h>
#include <qheader.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qmenudata.h>
#include <qobjectlist.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qtabwidget.h>
#include <qtextbrowser.h>
#include <qtoolbutton.h>
#include <qwidgetstack.h>

#include "QY2ListView.h"

#include "utf8.h"
#include "YQi18n.h"
#include "YQUI.h"
#include "YQDialog.h"
#include "YQAlignment.h"
#include "YQReplacePoint.h"
#include "YQEmpty.h"
#include "YQLabel.h"
#include "YQWizardButton.h"
#include "QY2LayoutUtils.h"
#include "YEvent.h"

using std::string;


#define PIXMAP_DIR THEMEDIR "/wizard/"

#ifdef TEXTDOMAIN
#    undef TEXTDOMAIN
#endif

#define TEXTDOMAIN "packages-qt"

#define ENABLE_GRADIENTS		1
#define ENABLE_TITLEBAR			0

#define USE_SEPARATOR			1

#if ! ENABLE_TITLEBAR
#  define WORK_AREA_TOP_MARGIN		10
#endif

#if ENABLE_GRADIENTS
#  define WORK_AREA_BOTTOM_MARGIN	8
#  define WORK_AREA_RIGHT_MARGIN	8
#else
#  define WORK_AREA_BOTTOM_MARGIN	8
#  define WORK_AREA_RIGHT_MARGIN	6
#endif

#define BUTTON_BOX_TOP_MARGIN		10

#define SEPARATOR_MARGIN		6
#define STEPS_MARGIN			10
#define STEPS_SPACING			2
#define STEPS_HEADING_SPACING		8
#define MENU_BAR_MARGIN			8

#define USE_FIXED_STEP_FONTS		0
#define STEPS_FONT_FAMILY		"Sans Serif"
#define STEPS_FONT_SIZE			11
#define STEPS_HEADING_FONT_SIZE		11

#define USE_ICON_ON_HELP_BUTTON		0


YQWizard::YQWizard( QWidget *		parent,
		    const YWidgetOpt &	opt,
		    const YCPValue &	backButtonId,	const YCPString & backButtonLabel,
		    const YCPValue &	abortButtonId,	const YCPString & abortButtonLabel,
		    const YCPValue &	nextButtonId,	const YCPString & nextButtonLabel  )
    : QVBox( parent )
    , YWizard( opt,
	       backButtonId,	backButtonLabel,
	       abortButtonId,	abortButtonLabel,
	       nextButtonId,	nextButtonLabel	 )
{
    setWidgetRep( this );
    _stepsEnabled = opt.stepsEnabled.value();
    _treeEnabled  = opt.treeEnabled.value();

    if ( _stepsEnabled && _treeEnabled )
    {
	y2error( "Can't enable steps and tree at the same time - disabling steps" );
	_stepsEnabled = false;
    }

    _verboseCommands	= false;
    _protectNextButton	= false;
    _stepsDirty		= false;
    _direction		= YQWizard::Forward;
    _runningEmbedded	= YQUI::ui()->runningEmbedded() || YQUI::ui()->debugEmbedding();

    if ( _treeEnabled )
	_runningEmbedded = false;

    _sideBar		= 0;
    _stepsPanel		= 0;
    _stepsBox		= 0;
    _stepsGrid		= 0;
    _helpButton		= 0;
    _stepsButton	= 0;
    _treeButton		= 0;
    _releaseNotesButton = 0;
    _treePanel		= 0;
    _tree		= 0;
    _helpPanel		= 0;
    _helpBrowser	= 0;
    _clientArea		= 0;
    _menuBarBox		= 0;
    _menuBar		= 0;
    _dialogIcon		= 0;
    _dialogHeading	= 0;
    _contents		= 0;
    _backButton		= 0;
    _backButtonSpacer	= 0;
    _abortButton	= 0;
    _nextButton		= 0;
    _sendButtonEvents	= true;

    _stepsList.setAutoDelete( true );
    _stepsIDs.setAutoDelete( false );	// Only for one of both!

    setFont( YQUI::ui()->currentFont() );	// Make sure qApp->font() is initialized
    YQUI::setTextdomain( TEXTDOMAIN );


    //
    // Load graphics
    //

    if ( ! runningEmbedded() )
    {
#if ENABLE_GRADIENTS
	loadGradientPixmaps();
#endif

	if ( _stepsEnabled )
	    loadStepsIcons();
    }


    //
    // Create widgets
    //

    if ( ! runningEmbedded() )
    {
#if ENABLE_TITLEBAR
	layoutTitleBar( this );
#else
	QWidget * spacer = addVSpacing( this, WORK_AREA_TOP_MARGIN );
	CHECK_PTR( spacer );

#    if ENABLE_GRADIENTS
	spacer->setPaletteBackgroundColor( _gradientCenterColor );
#    endif
#endif
    }

    QHBox * hBox = new QHBox( this );
    CHECK_PTR( hBox );

    if ( ! runningEmbedded() )
	layoutSideBar( hBox );

    layoutWorkArea( hBox );

    y2debug( "Constructor finished." );
}



YQWizard::~YQWizard()
{
    deleteSteps();
}



void YQWizard::layoutTitleBar( QWidget * parent )
{
    if ( ! highColorDisplay() )		// 8 bit display or worse?
    {
	// No colorful title bar, just a spacing to match the one at the bottom.
	addVSpacing( parent, WORK_AREA_BOTTOM_MARGIN );

	return;
    }


    QHBox * titleBar = new QHBox( parent );
    CHECK_PTR( titleBar );

#if ENABLE_GRADIENTS
    setGradient( titleBar, _titleBarGradientPixmap );
#endif

    titleBar->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) ); // hor/vert

    //
    // Left logo
    //

    QLabel * left = new QLabel( titleBar );
    left->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) ); // hor/vert

    QPixmap leftLogo( PIXMAP_DIR "title-bar-left.png" );

    if ( ! leftLogo.isNull() )
    {
	left->setPixmap( leftLogo );
	left->setFixedSize( leftLogo.size() );
	left->setBackgroundOrigin( QWidget::ParentOrigin );
    }


    //
    // Center stretch space
    //

    addHStretch( titleBar );


    //
    // Right logo
    //

    QLabel * right = new QLabel( titleBar );
    CHECK_PTR( right );

    QPixmap rightLogo( PIXMAP_DIR "title-bar-right.png" );

    if ( ! rightLogo.isNull() )
    {
	right->setPixmap( rightLogo );
	right->setFixedSize( rightLogo.size() );
	right->setBackgroundOrigin( QWidget::ParentOrigin );
    }
}



void YQWizard::layoutSideBar( QWidget * parent )
{
    _sideBar = new QWidgetStack( parent );
    CHECK_PTR( _sideBar );
    _sideBar->setMinimumWidth( YQUI::ui()->defaultSize( YD_HORIZ ) / 5 );
    _sideBar->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred ) ); // hor/vert
    _sideBar->setMargin( 0 );


    layoutHelpPanel();

    if ( _treeEnabled )
    {
	layoutTreePanel();
	showTree();
    }
    else if ( _stepsEnabled )
    {
	layoutStepsPanel();
	showSteps();
    }
}



void YQWizard::layoutStepsPanel()
{
    _stepsPanel = new QVBox( _sideBar );
    CHECK_PTR( _stepsPanel );


#if ENABLE_GRADIENTS
#   if ENABLE_TITLEBAR

    // Top gradient

    QLabel * topGradient = new QLabel( _stepsPanel );
    CHECK_PTR( topGradient );
    setGradient( topGradient, _topGradientPixmap );

#   endif
#endif


    // Steps

    _sideBar->addWidget( _stepsPanel );

    _stepsBox = new QVBox( _stepsPanel );
    CHECK_PTR( _stepsBox );
#if ENABLE_GRADIENTS
    _stepsBox->setPaletteBackgroundColor( _gradientCenterColor );
#endif
    _stepsBox->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred ) ); // hor/vert

    QWidget * stretch = addVStretch( _stepsPanel );
    CHECK_PTR( stretch );
#if ENABLE_GRADIENTS
    stretch->setPaletteBackgroundColor( _gradientCenterColor );
#endif


    // Steps panel bottom buttons ("Help", "Release Notes")

    QLabel * helpButtonBox = new QLabel( _stepsPanel );

#if ENABLE_GRADIENTS
    CHECK_PTR( helpButtonBox );
    setGradient( helpButtonBox, _bottomGradientPixmap );
#endif



    // Layouts for the buttons

    QVBoxLayout * vbox = new QVBoxLayout( helpButtonBox, 0, 0 ); // parent, margin, spacing
    CHECK_PTR( vbox );
    vbox->addStretch( 99 );


    QHBoxLayout * hbox = new QHBoxLayout( vbox, 0 );	// parent, spacing
    hbox->addStretch( 99 );

    _releaseNotesButton = new QPushButton( _( "Release Notes..." ), helpButtonBox );
    hbox->addWidget( _releaseNotesButton );


    connect( _releaseNotesButton,	SIGNAL( clicked()  ),
	     this,			SLOT  ( releaseNotesClicked() ) );

    _releaseNotesButton->hide();	// hidden until showReleaseNotesButton() is called

    hbox->addStretch( 99 );
    vbox->addStretch( 99 );

    hbox = new QHBoxLayout( vbox, 0 );	// parent, spacing
    hbox->addStretch( 99 );

    // Help button - intentionally without keyboard shortcut
    _helpButton = new QPushButton( _( "Help" ), helpButtonBox );
    CHECK_PTR( _helpButton );

    hbox->addWidget( _helpButton );
    hbox->addStretch( 99 );

    connect( _helpButton, SIGNAL( clicked()  ),
	     this,	 SLOT  ( showHelp() ) );

#if USE_ICON_ON_HELP_BUTTON
    QPixmap pixmap = QPixmap( PIXMAP_DIR "help-button.png" );

    if ( ! pixmap.isNull() )
	_helpButton->setPixmap( pixmap );
#endif


    vbox->addSpacing( WORK_AREA_BOTTOM_MARGIN );
}



void YQWizard::addStep( const QString & text, const QString & id )
{
    if ( _stepsIDs[ id ] )
    {
	y2error( "Step ID \"%s\" (\"%s\") already used for \"%s\"",
		 (const char *) id,
		 (const char *) text,
		 (const char *) _stepsIDs[ id ]->name() );
	return;
    }

    if ( _stepsList.last() && _stepsList.last()->name() == text )
    {
	// Consecutive steps with the same name will be shown as one single step.
	//
	// Since steps are always added at the end of the list, it is
	// sufficient to check the last step of the list. If the texts are the
	// same, the other with the same text needs to get another (additional)
	// ID to make sure setCurrentStep() works as it should.
	_stepsList.last()->addID( id );
    }
    else
    {
	_stepsList.append( new YQWizard::Step( text, id ) );
	_stepsDirty = true;
    }

    _stepsIDs.insert( id, _stepsList.last() );
}



void YQWizard::addStepHeading( const QString & text )
{
    _stepsList.append( new YQWizard::StepHeading( text ) );
    _stepsDirty = true;
}



void YQWizard::updateSteps()
{
    if ( ! _stepsBox )
	return;

    //
    // Delete any previous step widgets
    //

    if ( _stepsGrid )
    {
	delete _stepsGrid->mainWidget();
	_stepsGrid = 0;
    }


    //
    // Create a new parent widget for the steps
    //

    QWidget * stepsParent = new QWidget( _stepsBox );
    CHECK_PTR( stepsParent );
    stepsParent->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) ); // hor/vert
#if ENABLE_GRADIENTS
    stepsParent->setPaletteBackgroundColor( _gradientCenterColor );
#endif

    // Create a grid layout for the steps

    _stepsGrid = new QGridLayout( stepsParent,			// parent
				  _stepsList.count(), 4,	// rows, cols
				  0, STEPS_SPACING );		// margin, spacing
    CHECK_PTR( _stepsGrid );

    const int statusCol = 1;
    const int nameCol	= 2;

    _stepsGrid->setColStretch( 0, 99 );		// Left margin column - stretch
    _stepsGrid->setColStretch( statusCol, 0 );	// Status column - don't stretch
    _stepsGrid->setColStretch( nameCol,	  0 );	// Name column - don't stretch
    _stepsGrid->setColStretch( 3, 99  );	// Left margin column - stretch


    // Work around Qt bug: Grid layout only works right if the parent widget isn't visible.
    stepsParent->hide();

    //
    // Add left and right (but not top and bottom) margins
    //

    int row = 0;

    QWidget * leftSpacer  = addHSpacing( stepsParent, STEPS_MARGIN );
    CHECK_PTR( leftSpacer );
    _stepsGrid->addWidget( leftSpacer, row, 0 );

    QWidget * rightSpacer = addHSpacing( stepsParent, STEPS_MARGIN );
    CHECK_PTR( rightSpacer );
    _stepsGrid->addWidget( rightSpacer, row, 3 );


    //
    // Create widgets for all steps and step headings in the internal list
    //

    YQWizard::Step * step = _stepsList.first();

    while ( step )
    {
	if ( step->isHeading() )
	{
	    if ( row > 0 )
	    {
		// Spacing

		QWidget * spacer = addVSpacing( stepsParent, STEPS_HEADING_SPACING );
		CHECK_PTR( spacer );
		_stepsGrid->addWidget( spacer, row++, nameCol );
	    }

	    //
	    // Heading
	    //

	    QLabel * label = new QLabel( step->name(), stepsParent );
	    CHECK_PTR( label );
	    label->setAlignment( Qt::AlignLeft | Qt::AlignTop );

#if USE_FIXED_STEP_FONTS
	    QFont font( STEPS_FONT_FAMILY, STEPS_HEADING_FONT_SIZE );
	    font.setWeight( QFont::Bold );
	    label->setFont( font );
#else
	    QFont font = YQUI::ui()->currentFont();

	    int size = font.pointSize();

	    if ( size > 1 )
		font.setPointSize( size + 2 );

	    font.setBold( true );
	    label->setFont( font );
#endif

	    step->setNameLabel( label );
	    _stepsGrid->addMultiCellWidget( label,
					    row, row,			// from_row, to_row
					    statusCol, nameCol );	// from_col, to_col
	}
	else	// No heading - ordinary step
	{
	    //
	    // Step status
	    //

	    QLabel * statusLabel = new QLabel( stepsParent );
	    CHECK_PTR( statusLabel );

	    step->setStatusLabel( statusLabel );
	    _stepsGrid->addWidget( statusLabel, row, statusCol );


	    //
	    // Step name
	    //

	    QLabel * nameLabel = new QLabel( step->name(), stepsParent );
	    CHECK_PTR( nameLabel );
	    nameLabel->setAlignment( Qt::AlignLeft | Qt::AlignTop );

#if USE_FIXED_STEP_FONTS
	    nameLabel->setFont( QFont( STEPS_FONT_FAMILY, STEPS_FONT_SIZE ) );
#else
	    nameLabel->setFont( YQUI::ui()->currentFont() );
#endif

	    step->setNameLabel( nameLabel );
	    _stepsGrid->addWidget( nameLabel, row, nameCol );
	}

	step = _stepsList.next();
	row++;
    }

    _stepsGrid->activate();
    stepsParent->show();
    _stepsDirty = false;
}


void YQWizard::updateStepStates()
{
    if ( _stepsDirty )
	updateSteps();

    YQWizard::Step * currentStep = findStep( _currentStepID );
    YQWizard::Step * step = _stepsList.first();

    if ( currentStep )
    {
	// Set status icon and color for the current step
	setStepStatus( currentStep, _stepCurrentIcon, _stepCurrentColor );


	//
	// Set all steps before the current to "done"
	//

	while ( step && step != currentStep )
	{
	    setStepStatus( step, _stepDoneIcon, _stepDoneColor );
	    step = _stepsList.next();
	}

	// Skip the current step - continue with the step after it

	if ( step )
	    step = _stepsList.next();
    }

    //
    // Set all steps after the current to "to do"
    //

    while ( step )
    {
	setStepStatus( step, _stepToDoIcon, _stepToDoColor );
	step = _stepsList.next();
    }
}


void YQWizard::setStepStatus( YQWizard::Step * step, const QPixmap & icon, const QColor & color )
{
    if ( step )
    {
	if ( step->nameLabel() )
	    step->nameLabel()->setPaletteForegroundColor( color );

	if ( step->statusLabel() )
	    step->statusLabel()->setPixmap( icon );
    }
}


void YQWizard::setCurrentStep( const QString & id )
{
    _currentStepID = id;
    updateStepStates();
}


void YQWizard::deleteSteps()
{
    _stepsList.clear();
    _stepsIDs.clear();
}


YQWizard::Step * YQWizard::findStep( const QString & id )
{
    if ( id.isEmpty() )
	return 0;

    return _stepsIDs[ id ];
}


void YQWizard::layoutHelpPanel()
{
    _helpPanel = new QHBox( _sideBar );
    CHECK_PTR( _helpPanel );
    _sideBar->addWidget( _helpPanel );

    addGradientColumn( _helpPanel );

    QVBox * vbox = new QVBox( _helpPanel );
    CHECK_PTR( vbox );


    // Help browser

    _helpBrowser = new QTextBrowser( vbox );
    CHECK_PTR( _helpBrowser );

    _helpBrowser->setMimeSourceFactory( 0 );
    _helpBrowser->setFont( YQUI::ui()->currentFont() );
    _helpBrowser->installEventFilter( this );
    _helpBrowser->setTextFormat( Qt::RichText );
    _helpBrowser->setMargin( 4 );
    _helpBrowser->setResizePolicy( QScrollView::Manual );

    if ( highColorDisplay() )
    {
	// Set fancy help browser background pixmap

	QPixmap bgPixmap( PIXMAP_DIR "help-background.png" );

	if ( ! bgPixmap.isNull() )
	    _helpBrowser->setPaletteBackgroundPixmap( bgPixmap );
    }



    //
    // Button box with bottom gradient
    //


    QLabel * buttonBox = new QLabel( vbox );
    CHECK_PTR( buttonBox );

    QPushButton * button;
    QPixmap pixmap;

    if ( _treeEnabled )
    {
	// "Tree" button - intentionally without keyboard shortcut
	button = new QPushButton( _( "Tree" ), buttonBox );
	CHECK_PTR( button );
	_treeButton = button;

#if USE_ICON_ON_HELP_BUTTON
	pixmap = QPixmap( PIXMAP_DIR "tree-button.png" );
#endif
    }
    else
	if ( _stepsEnabled )
    {
	// "Steps" button - intentionally without keyboard shortcut
	button = new QPushButton( _( "Steps" ), buttonBox );
	CHECK_PTR( button );
	_stepsButton = button;

#if USE_ICON_ON_HELP_BUTTON
	pixmap = QPixmap( PIXMAP_DIR "steps-button.png" );
#endif
    }
    else
    {
	// Create a dummy button just to find out how high it would become
	button = new QPushButton( "Dummy", buttonBox );
	CHECK_PTR( button );
    }


    if ( ! pixmap.isNull() )
	button->setPixmap( pixmap );

    layoutSideBarButtonBox( buttonBox, button );

    if ( _treeEnabled )
    {
	connect( button, SIGNAL( clicked()  ),
		 this,	 SLOT  ( showTree() ) );
    }
    else if ( _stepsEnabled )
    {
	connect( button, SIGNAL( clicked()   ),
		 this,	 SLOT  ( showSteps() ) );
    }
    else
    {
	// Hide the dummy button - the button box height is fixed now.
	button->hide();
    }

    addGradientColumn( _helpPanel );
}



void YQWizard::layoutSideBarButtonBox( QWidget * parent, QPushButton * button )
{
    QVBoxLayout * vbox = new QVBoxLayout( parent, 0, 0 );	// parent, margin, spacing
    CHECK_PTR( vbox );
    vbox->addSpacing( BUTTON_BOX_TOP_MARGIN );

    QHBoxLayout * hbox = new QHBoxLayout( vbox, 0 );		// parent, spacing
    CHECK_PTR( hbox );

    hbox->addStretch( 99 );
    hbox->addWidget( button );
    hbox->addStretch( 99 );

    vbox->addSpacing( WORK_AREA_BOTTOM_MARGIN );

    // For whatever strange reason, parent->sizeHint() does not return anything
    // meaningful yet - not even after vbox->activate() or parent->adjustSize()
    int height = button->sizeHint().height() + BUTTON_BOX_TOP_MARGIN + WORK_AREA_BOTTOM_MARGIN;

#if ENABLE_GRADIENTS
    if ( ! _bottomGradientPixmap.isNull() )
	setBottomCroppedGradient( parent, _bottomGradientPixmap, height );
#endif

    parent->setFixedHeight( height );
}



void YQWizard::layoutTreePanel()
{
    _treePanel = new QHBox( _sideBar );
    CHECK_PTR( _treePanel );
    _sideBar->addWidget( _treePanel );

    // Left margin (with gradients)
    addGradientColumn( _treePanel );

    QVBox * vbox = new QVBox( _treePanel );
    CHECK_PTR( vbox );


    // Selection tree

    _tree = new QY2ListView( vbox );
    CHECK_PTR( _tree );
    _tree->addColumn( "" );
    _tree->header()->hide();
    _tree->setRootIsDecorated( true );

    connect( _tree,	SIGNAL( selectionChanged     ( void ) ),
	     this,	SLOT  ( treeSelectionChanged ( void ) ) );

    connect( _tree,	SIGNAL( spacePressed  ( QListViewItem * ) ),
	     this,	SLOT  ( sendTreeEvent ( QListViewItem * ) ) );

    connect( _tree,	SIGNAL( doubleClicked ( QListViewItem * ) ),
	     this,	SLOT  ( sendTreeEvent ( QListViewItem * ) ) );


    // Bottom gradient

    QLabel * buttonBox = new QLabel( vbox );
    CHECK_PTR( buttonBox );


    // "Help" button - intentionally without keyboard shortcut
    QPushButton * button = new QPushButton( _( "Help" ), buttonBox );
    CHECK_PTR( button );

#if USE_ICON_ON_HELP_BUTTON
    QPixmap pixmap( PIXMAP_DIR "help-button.png" );

    if ( ! pixmap.isNull() )
	button->setPixmap( pixmap );
#endif

    layoutSideBarButtonBox( buttonBox, button );

    connect( button, SIGNAL( clicked()	),
	     this,   SLOT  ( showHelp() ) );


    // Right margin (with gradients)
    addGradientColumn( _treePanel );
}



void YQWizard::addTreeItem( const QString & parentID, const QString & text, const QString & id )
{
    if ( ! _tree )
    {
	y2error( "YQWizard widget not created with `opt(`treeEnabled) !" );
	return;
    }

    YQWizard::TreeItem * item	= 0;
    YQWizard::TreeItem * parent = 0;

    if ( ! parentID.isEmpty() )
    {
	parent = findTreeItem( parentID );
    }

    if ( parent )
    {
	item = new YQWizard::TreeItem( parent, text, id );
	CHECK_PTR( item );
    }
    else
    {
	item = new YQWizard::TreeItem( _tree, text, id );
	CHECK_PTR( item );
    }

    if ( ! id.isEmpty() )
	_treeIDs.insert( id, item );
}



void YQWizard::deleteTreeItems()
{
    if ( _tree )
	_tree->clear();

    _treeIDs.clear();
}



YQWizard::TreeItem * YQWizard::findTreeItem( const QString & id )
{
    if ( id.isEmpty() )
	return 0;

    return _treeIDs[ id ];
}


void YQWizard::selectTreeItem( const QString & id )
{
    if ( _tree )
    {
	YQWizard::TreeItem * item = findTreeItem( id );

	if ( item )
	{
	    _tree->blockSignals( true );
	    _tree->setSelected( item, true );
	    _tree->ensureItemVisible( item );
	    _tree->blockSignals( false );
	}
    }
}


void YQWizard::sendTreeEvent( QListViewItem * listViewItem )
{
    if ( listViewItem )
    {
	YQWizard::TreeItem * item = dynamic_cast<YQWizard::TreeItem *> ( listViewItem );

	if ( item && ! item->id().isEmpty() )
	    sendEvent( YCPString( toUTF8( item->id() ) ) );
    }
}


void YQWizard::treeSelectionChanged()
{
    if ( _tree )
	sendTreeEvent( _tree->selectedItem() );
}


YCPString YQWizard::currentTreeSelection()
{
    if ( _tree )
    {
	QListViewItem * sel = _tree->selectedItem();

	if ( sel )
	{
	    YQWizard::TreeItem * item = dynamic_cast<YQWizard::TreeItem *> (sel);

	    if ( item && ! item->id().isEmpty() )
		return YCPString( (const char *) item->id() );
	}
    }

    return YCPString( "" );
}




void YQWizard::layoutWorkArea( QHBox * parentHBox )
{
    QVBox * workAreaVBox = new QVBox( parentHBox );
    CHECK_PTR( workAreaVBox );

    // An extra QVBox inside the workAreaVBox is needed for frame and margin

    QVBox * workArea = new QVBox( workAreaVBox );
    CHECK_PTR( workArea );

#if ENABLE_GRADIENTS
    workArea->setFrameStyle( QFrame::Box | QFrame::Plain );
    workArea->setMargin( 4 );
#else
    workArea->setFrameStyle( QFrame::Box | QFrame::Sunken );
    // workArea->setFrameStyle( QFrame::TabWidgetPanel | QFrame::Sunken );
#endif


    //
    // Menu bar
    //

    // Placed directly inside workArea the menu bar positions itself at (0,0)
    // and so obscures any kind of frame there might be.
    _menuBarBox = new QVBox( workArea );
    CHECK_PTR( _menuBarBox );

    _menuBar = new QMenuBar( _menuBarBox );
    CHECK_PTR( _menuBar );

    _menuBarBox->hide(); // will be made visible when menus are added


    //
    // Dialog icon and heading
    //

    QHBox * headingHBox = new QHBox( workArea );
    CHECK_PTR( headingHBox );
    headingHBox->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum ) ); // hor/vert

    addHSpacing( headingHBox, SEPARATOR_MARGIN );

    _dialogIcon = new QLabel( headingHBox );
    CHECK_PTR( _dialogIcon );
    _dialogIcon->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) ); // hor/vert

    addHSpacing( headingHBox );

    _dialogHeading = new QLabel( headingHBox );
    CHECK_PTR( _dialogHeading );
    _dialogHeading->setFont( YQUI::ui()->headingFont() );
    _dialogHeading->setAlignment( Qt::AlignLeft | Qt::WordBreak );
    _dialogHeading->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum ) ); // hor/vert

#if 0
    addHStretch( headingHBox );
#endif
    addVSpacing( workArea );

#if USE_SEPARATOR

    QHBox * hbox = new QHBox( workArea );

    addHSpacing( hbox, SEPARATOR_MARGIN );

    QFrame * separator = new QFrame( hbox );
    CHECK_PTR( separator );
    separator->setFrameStyle( QFrame::HLine | QFrame::Sunken );

    addHSpacing( hbox, SEPARATOR_MARGIN );
    addVSpacing( workArea );
#endif

    //
    // Client area (the part that belongs to the YCP application)
    //

    layoutClientArea( workArea );


    //
    // Button box
    //

    layoutButtonBox( workAreaVBox );


    if ( ! runningEmbedded() )
    {
	//
	// Spacer (purely decorative) at the right of the client area
	//

	addGradientColumn( parentHBox, WORK_AREA_RIGHT_MARGIN );
    }
}



void YQWizard::layoutClientArea( QWidget * parent )
{
    _clientArea = new QVBox( parent );
    CHECK_PTR( _clientArea );
    _clientArea->setMargin( 4 );

#if 0
    _clientArea->setPaletteBackgroundColor( QColor( 0x60, 0x60, 0x60 ) );
#endif


    //
    // HVCenter for wizard contents
    //

    YWidgetOpt hvstretchOpt;
    hvstretchOpt.isHStretchable.setValue( true );
    hvstretchOpt.isVStretchable.setValue( true );
    _contents = new YQAlignment( _clientArea, hvstretchOpt, YAlignCenter, YAlignCenter );
    CHECK_PTR( _contents );

    addChild( _contents );
    _contents->setParent( this );
    _contents->installEventFilter( this );
    _contents->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) ); // hor/vert


    //
    // Replace point for wizard contents
    //

    YWidgetOpt widgetOpt;
    YQReplacePoint * replacePoint = new YQReplacePoint( _contents, widgetOpt );
    CHECK_PTR( replacePoint );

    replacePoint->setId( YCPSymbol( YWizardContentsReplacePointID ) ); // `id(`contents)
    _contents->addChild( replacePoint );
    replacePoint->setParent( _contents );


    //
    // Initial YEmpty widget contents of replace point
    //

    YQEmpty * empty = new YQEmpty( replacePoint, widgetOpt );
    empty->setParent( replacePoint );
    replacePoint->addChild( empty );
}



void YQWizard::layoutButtonBox( QWidget * parent )
{
    //
    // Button box and layout
    //

    QWidget * buttonBox = new QWidget( parent );
    CHECK_PTR( buttonBox );

    YQDialog * dialog = dynamic_cast<YQDialog *>( YQUI::ui()->currentDialog() );
    CHECK_PTR( dialog );

    // Using old-style layouts to enable a seamless background with the
    // gradient pixmap: Any sub-widgets (QVBox, QHBox) would have to get yet
    // another portion of that gradient as their backround pixmap, and it would
    // be very hard to cover all cases - resizing, hiding individual buttons, etc.

    QVBoxLayout * vbox = new QVBoxLayout( buttonBox, 0, 0 );	// parent, margin, spacing
    CHECK_PTR( vbox );

    vbox->addSpacing( BUTTON_BOX_TOP_MARGIN );


    //
    // QHBoxLayout for the buttons
    //

    QHBoxLayout * hbox = new QHBoxLayout( vbox, 2 );		// parent, spacing
    CHECK_PTR( hbox );


    //
    // "Back" button
    //

    _backButton	 = new YQWizardButton( this, dialog, buttonBox, _backButtonLabel, _backButtonId );
    CHECK_PTR( _backButton );

    hbox->addWidget( (QWidget *) _backButton->widgetRep() );
    addChild( _backButton );  // Enable shortcut checking for this button
    connect( _backButton,	SIGNAL( clicked()		),
	     this,		SLOT  ( slotBackClicked()	) );

    _backButtonSpacer = new QSpacerItem( 0, 0,				// width, height
					 QSizePolicy::Expanding,	// horizontal
					 QSizePolicy::Minimum );	// vertical
    CHECK_PTR( _backButtonSpacer );
    hbox->addItem( _backButtonSpacer );


    if ( _backButton->text().isEmpty() )
    {
	_backButton->hide();

	// Minimize _backButtonSpacer
	_backButtonSpacer->changeSize( 0, 0,				// width, height
				       QSizePolicy::Minimum,		// horizontal
				       QSizePolicy::Minimum );		// vertical
    }


    //
    // "Abort" button
    //

    _abortButton = new YQWizardButton( this, dialog, buttonBox, _abortButtonLabel, _abortButtonId );
    CHECK_PTR( _abortButton );

    hbox->addWidget( (QWidget *) _abortButton->widgetRep() );
    addChild( _abortButton ); // Enable shortcut checking for this button
    connect( _abortButton,	SIGNAL( clicked()		),
	     this,		SLOT  ( slotAbortClicked()	) );


    // Using spacer rather than addSpacing() since the default stretchability
    // of a QSpacerItem is undefined, i.e. centering the middle button could
    // not be guaranteed.

    QSpacerItem * spacer = new QSpacerItem( 0, 0,			// width, height
					    QSizePolicy::Expanding,	// horizontal
					    QSizePolicy::Minimum );	// vertical
    CHECK_PTR( spacer );
    hbox->addItem( spacer );


    //
    // "Next" button
    //

    _nextButton	 = new YQWizardButton( this, dialog, buttonBox, _nextButtonLabel, _nextButtonId );
    CHECK_PTR( _nextButton );

    hbox->addWidget( (QWidget *) _nextButton->widgetRep() );
    addChild( _nextButton );  // Enable shortcut checking for this button
    connect( _nextButton,	SIGNAL( clicked()		),
	     this,		SLOT  ( slotNextClicked()	) );


    //
    // Bottom margin and gradient
    //

    vbox->addSpacing( WORK_AREA_BOTTOM_MARGIN );

#if ENABLE_GRADIENTS
    if ( ! runningEmbedded() )
	setBottomCroppedGradient( buttonBox, _bottomGradientPixmap, buttonBox->sizeHint().height() );
#endif

    buttonBox->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) ); // hor/vert
}



void YQWizard::loadGradientPixmaps()
{
#if ENABLE_GRADIENTS
    if ( highColorDisplay() )
    {
	_topGradientPixmap	= QPixmap( PIXMAP_DIR "top-gradient.png"	);
	_bottomGradientPixmap	= QPixmap( PIXMAP_DIR "bottom-gradient.png"	);
	_titleBarGradientPixmap = QPixmap( PIXMAP_DIR "title-bar-gradient.png"	);
	_gradientCenterColor = pixelColor( _bottomGradientPixmap, 0, 0 );
    }
    else // 8 bit display or worse - don't use gradients
    {
	// Gradient pixmaps remain empty. The respecive widgets will retain the
	// default widget background (grey, depending on the widget theme).

	// Use deault widget background (some shade of grey) for the center
	// stretchable part of the side bar.
	_gradientCenterColor = paletteBackgroundColor();
    }
#endif
}


void YQWizard::loadStepsIcons()
{
    if ( highColorDisplay() )
    {
	_stepCurrentColor	= pixelColor( QPixmap( PIXMAP_DIR "color-step-current.png" ), 0, 0 );
	_stepToDoColor		= pixelColor( QPixmap( PIXMAP_DIR "color-step-todo.png"	   ), 0, 0 );
	_stepDoneColor		= pixelColor( QPixmap( PIXMAP_DIR "color-step-done.png"	   ), 0, 0 );
    }
    else
    {
	_stepCurrentColor	= paletteForegroundColor();
	_stepToDoColor		= paletteForegroundColor();
	_stepDoneColor		= paletteForegroundColor();
    }

    _stepCurrentIcon	= QPixmap( PIXMAP_DIR "step-current.png" );
    _stepToDoIcon	= QPixmap( PIXMAP_DIR "step-todo.png"	 );
    _stepDoneIcon	= QPixmap( PIXMAP_DIR "step-done.png"	 );
}



void YQWizard::setGradient( QWidget * widget, const QPixmap & pixmap )
{
#if ENABLE_GRADIENTS
    if ( widget && ! pixmap.isNull() )
    {
	widget->setFixedHeight( pixmap.height() );
	widget->setPaletteBackgroundPixmap( pixmap );
    }
#endif
}



void YQWizard::setBottomCroppedGradient( QWidget * widget, const QPixmap & pixmap, int croppedHeight )
{
#if ENABLE_GRADIENTS
    setGradient( widget, bottomCropPixmap( pixmap, croppedHeight ) );
#endif
}



QPixmap YQWizard::bottomCropPixmap( const QPixmap & full, int croppedHeight )
{
    QPixmap pixmap;

#if ENABLE_GRADIENTS

    if ( full.height() > croppedHeight )
    {
	pixmap = QPixmap( full.width(), croppedHeight );

	bitBlt( &pixmap, 0, 0,					// dest, dest_x, dest_y
		&full,	 0, full.height() - croppedHeight - 1,	// src, src_x, src_y
		full.width(), croppedHeight );			// src_width, src_height
    }
    else
    {
	pixmap = full;
    }
#endif

    return pixmap;
}



QColor YQWizard::pixelColor( const QPixmap & pixmap, int x, int y )
{
    // QPixmap doesn't allow direct access to pixel values (which makes some
    // sense since this requires a round-trip to the X server - pixmaps are X
    // server resources), so we need to convert the QPixmap to a QImage to get
    // that information. But since this conversion is expensive, we might save
    // some performance if we only convert the part we really need - so let's
    // cut out a tiny portion of the original pixmap and convert only that tiny
    // portion.

    QPixmap tiny( 1, 1 );

    bitBlt( &tiny, 0, 0,	// dest, dest_x, dest_y
	    &pixmap, x, y,	// src, src_x, src_y
	    1, 1 );		// src_width, src_height

    QImage image = tiny.convertToImage();


    return QColor( image.pixel( 0, 0 ) );
}



void YQWizard::addGradientColumn( QWidget * parent, int width )
{
    if ( ! parent )
	return;

    QVBox * vbox = new QVBox( parent );
    CHECK_PTR( vbox );

#if ENABLE_GRADIENTS
    QWidget * topGradient = addHSpacing( vbox, width );
    CHECK_PTR( topGradient );
    setGradient( topGradient, _topGradientPixmap );

    QWidget * centerStretch = new QWidget( vbox );
    CHECK_PTR( centerStretch );
    centerStretch->setPaletteBackgroundColor( _gradientCenterColor );


    QWidget * bottomGradient = new QWidget( vbox );
    CHECK_PTR( bottomGradient );
    setGradient( bottomGradient, _bottomGradientPixmap );
#else
    vbox->setFixedWidth( width );
#endif

}


void YQWizard::destroyButtons()
{
    if ( _backButton  )
    {
	delete _backButton;
	_backButton = 0;
    }

    if ( _abortButton )
    {
	delete _abortButton;
	_abortButton = 0;
    }

    if ( _nextButton  )
    {
	delete _nextButton;
	_nextButton = 0;
    }
}



bool YQWizard::highColorDisplay() const
{
    return QColor::numBitPlanes() > 8;
}


void YQWizard::connectNotify ( const char * signal )
{
    if ( QString( signal ).contains( "nextClicked()" ) )
    {
	y2debug( "nextClicked connected, no longer directly sending button events" );
	_sendButtonEvents = false;
    }
}


void YQWizard::disconnectNotify ( const char * signal )
{
    if ( QString( signal ).contains( "nextClicked()" ) )
    {
	y2debug( "nextClicked disconnected, directly sending button events again" );
	_sendButtonEvents = true;
    }
}


void YQWizard::setDialogIcon( const char * iconName )
{
    if ( _dialogIcon )
    {
	if ( iconName && *iconName )
	{
	    QPixmap icon( iconName );

	    if ( icon.isNull() )
		y2warning( "Couldn't load dialog icon \"%s\"", iconName );
	    else
	    {
		_dialogIcon->setPixmap( icon );
		topLevelWidget()->setIcon( icon );
	    }
	}
	else
	{
	    _dialogIcon->clear();
	    topLevelWidget()->setIcon( QPixmap() );
	}
    }
}


void YQWizard::setDialogHeading( const QString & headingText )
{
    if ( _dialogHeading )
    {
	if ( headingText )
	    _dialogHeading->setText( headingText );
	else
	    _dialogHeading->clear();
    }
}

string YQWizard::debugLabel()
{
    if ( _dialogHeading )
    {
	QString label = _dialogHeading->text();
	label.simplifyWhiteSpace(); // Replace any embedded newline with a single blank

	if ( ! label.isEmpty() )
	{
	    label.prepend( "YQWizard \"" );
	    label.append( "\"" );

	    return toUTF8( label );
	}
    }

    return "untitled YQWizard";
}

void YQWizard::setHelpText( QString helpText )
{
    if ( _helpBrowser )
    {
	if ( helpText )
	{
	    helpText.replace( "&product;", YQUI::ui()->productName() );
	    _helpBrowser->setText( helpText );
	}
	else
	    _helpBrowser->clear();
    }
}


void YQWizard::addChild( YWidget * child )
{
    if ( dynamic_cast<YQWizardButton *> (child)
	 || child == _contents )
    {
	YContainerWidget::addChild( child );
    }
    else
    {
	y2error( "Ignoring unwanted %s child", child->widgetClass() );
    }
}



void YQWizard::slotBackClicked()
{
    emit backClicked();

    if ( _sendButtonEvents )
	sendEvent( _backButton->id() );

    _direction = YQWizard::Backward;
}


void YQWizard::slotAbortClicked()
{
    emit abortClicked();

    if ( _sendButtonEvents )
	sendEvent( _abortButton->id() );
}


void YQWizard::slotNextClicked()
{
    emit nextClicked();

    if ( _sendButtonEvents )
	sendEvent( _nextButton->id() );

    _direction = YQWizard::Forward;
}


void YQWizard::showHelp()
{
    if ( _sideBar && _helpPanel )
    {
	_sideBar->raiseWidget( _helpPanel );
    }
}


void YQWizard::releaseNotesClicked()
{
    if ( ! _releaseNotesButtonId.isNull() )
    {
	y2milestone( "Release Notes button clicked" );
	sendEvent( _releaseNotesButtonId );
    }
}


void YQWizard::showSteps()
{
    if ( _sideBar && _stepsPanel )
    {
	_sideBar->raiseWidget( _stepsPanel );
    }
}


void YQWizard::showTree()
{
    if ( _sideBar && _treePanel )
    {
	_sideBar->raiseWidget( _treePanel );
    }
}



void YQWizard::addMenu( const QString & text,
			const QString & id )
{
    if ( _menuBar )
    {
	QPopupMenu * menu = new QPopupMenu( _menuBar );
	CHECK_PTR( menu );

	_menuIDs.insert( id, menu );
	_menuBar->insertItem( text, menu );

	connect( menu, SIGNAL( activated    ( int ) ),
		 this, SLOT  ( sendMenuEvent( int ) ) );

	if ( _menuBarBox && _menuBarBox->isHidden() )
	{
	    _menuBarBox->show();
	    _menuBarBox->setFixedHeight( _menuBar->sizeHint().height() + MENU_BAR_MARGIN );
	}
    }
}


void YQWizard::addSubMenu( const QString & parentMenuID,
			   const QString & text,
			   const QString & id )
{
    QPopupMenu * parentMenu = _menuIDs[ parentMenuID ];

    if ( parentMenu )
    {
	QPopupMenu * menu = new QPopupMenu( _menuBar );
	CHECK_PTR( menu );

	_menuIDs.insert( id, menu );
	parentMenu->insertItem( text, menu );

	connect( menu, SIGNAL( activated    ( int ) ),
		 this, SLOT  ( sendMenuEvent( int ) ) );
    }
    else
    {
	y2error( "Can't find menu with ID %s", (const char *) parentMenuID );
    }
}


void YQWizard::addMenuEntry( const QString & parentMenuID,
			     const QString & text,
			     const QString & idString )
{
    QPopupMenu * parentMenu = _menuIDs[ parentMenuID ];

    if ( parentMenu )
    {
	int id = _menuEntryIDs.size();
	_menuEntryIDs.push_back( idString );
	parentMenu->insertItem( text, id );
    }
    else
    {
	y2error( "Can't find menu with ID %s", (const char *) parentMenuID );
    }
}


void YQWizard::addMenuSeparator( const QString & parentMenuID )
{
    QPopupMenu * parentMenu = _menuIDs[ parentMenuID ];

    if ( parentMenu )
    {
	parentMenu->insertSeparator();
    }
    else
    {
	y2error( "Can't find menu with ID %s", (const char *) parentMenuID );
    }
}


void YQWizard::deleteMenus()
{
    if ( _menuBar )
    {
	_menuBarBox->hide();
	_menuBar->clear();
	_menuIDs.clear();
	_menuEntryIDs.clear();
    }
}


void YQWizard::sendMenuEvent( int numID )
{
    if ( numID >= 0 && numID < (int) _menuEntryIDs.size() )
    {
	sendEvent( YCPString( toUTF8( _menuEntryIDs[ numID ] ) ) );
    }
    else
    {
	y2error( "Invalid menu ID: %d", numID );
    }
}



void YQWizard::sendEvent( YCPValue id )
{
    // Wizard events are sent as menu events - the semantics are similar.
    //
    // Widget events wouldn't do since they use their widget's ID as the ID to
    // return (which would be inappropriate since that would be the ID of the
    // wizard widget). Another type of event (WizardEvent) could be introduced,
    // but it would add little more information (if any) than MenuEvent.
    //
    // YQPackageSelector uses the same approach. After all, one widget that can
    // return multiple IDs is roughly the semantics of MenuEvents.

    YQUI::ui()->sendEvent( new YMenuEvent( id ) );
}



long YQWizard::nicesize( YUIDimension dim )
{
    return dim == YD_HORIZ ? sizeHint().width() : sizeHint().height();
}



void YQWizard::setSize( long newWidth, long newHeight )
{
    resize( newWidth, newHeight );
    resizeClientArea();
}



void YQWizard::resizeClientArea()
{
    // y2debug( "resizing client area" );
    QRect contentsRect = _clientArea->contentsRect();
    _contents->setSize( contentsRect.width(), contentsRect.height() );
}



bool YQWizard::eventFilter( QObject * obj, QEvent * ev )
{
    if ( ev->type() == QEvent::Resize && obj == _contents )
    {
	resizeClientArea();
	return true;		// Event handled
    }

    return QWidget::eventFilter( obj, ev );
}


void YQWizard::setButtonLabel( YQWizardButton * button, const QString & newLabel )
{
    if ( button )
    {
	button->setLabel( newLabel );

	if ( YQUI::ui()->currentDialog() )
	    YQUI::ui()->currentDialog()->checkShortcuts();

	if ( newLabel.isEmpty() )
	{
	    button->hide();

	    if ( button == _backButton && _backButtonSpacer )
	    {
		// Minimize _backButtonSpacer

		_backButtonSpacer->changeSize( 0, 0,				// width, height
					       QSizePolicy::Minimum,		// horizontal
					       QSizePolicy::Minimum );		// vertical
	    }
	}
	else
	{
	    button->show();

	    if ( button == _backButton && _backButtonSpacer )
	    {
		// Restore _backButtonSpacer to normal size

		_backButtonSpacer->changeSize( 0, 0,				// width, height
					       QSizePolicy::Expanding,		// horizontal
					       QSizePolicy::Minimum );		// vertical
	    }
	}
    }
}


void YQWizard::setButtonID( YQWizardButton * button, const YCPValue & id )
{
    if ( button )
    {
	button->setId( id );
    }
}


void YQWizard::enableButton( YQWizardButton * button, bool enabled )
{
    if ( button == _nextButton && _protectNextButton && ! enabled )
	return;

    if ( button )
	button->setEnabling( enabled );
}


void YQWizard::setButtonFocus( YQWizardButton * button )
{
    if ( button )
	button->setKeyboardFocus();
}


void YQWizard::showReleaseNotesButton( string label, const YCPValue & id )
{
    if ( ! _releaseNotesButton )
    {
	y2error( "NULL Release Notes button" );
	if ( ! _stepsBox )
	    y2error( "This works only if there is a \"steps\" panel!" );

	return;
    }

    label = YShortcut::cleanShortcutString( label );	// no way to check the shortcut, so strip it
    _releaseNotesButton->setText( fromUTF8( label ) );
    _releaseNotesButtonId = id;


    if ( _releaseNotesButton->isHidden() )
	_releaseNotesButton->show();

}


void YQWizard::hideReleaseNotesButton()
{
    if ( _releaseNotesButton && _releaseNotesButton->isShown() )
	_releaseNotesButton->hide();
}


void YQWizard::retranslateInternalButtons()
{
    YQUI::setTextdomain( TEXTDOMAIN );

    if ( _helpButton )
	// "Help" button - intentionally without keyboard shortcut
	_helpButton->setText( _( "Help" ) );

    if ( _stepsButton )
	// "Steps" button - intentionally without keyboard shortcut
	_stepsButton->setText( _( "Steps" ) );

    if ( _treeButton )
	// "Tree" button - intentionally without keyboard shortcut
	_treeButton->setText( _( "Tree" ) );
}


void YQWizard::ping()
{
    y2debug( "YQWizard is active" );
}


bool YQWizard::isCommand( QString declaration, const YCPTerm & term )
{
    declaration = declaration.simplifyWhiteSpace();

    // Check command name

    QString command = declaration;
    command.remove( QRegExp( "\\s*\\(.*$" ) );	// remove arguments

    if ( term->name().c_str() != command )
	return false;

    //
    // Check arguments
    //

    QString arg_decl = declaration;
    arg_decl.remove( QRegExp( "^.*\\(" ) );	// remove "command ("
    arg_decl.remove( QRegExp( "\\).*$" ) );	// remove ")"

    QStringList argDeclList = QStringList::split( ",", arg_decl );

    //
    // Check number of arguments
    //

    if ( argDeclList.size() != (unsigned) term->size() )
    {
	y2error( "Bad arguments for wizard command %s : %s",
		 (const char *) declaration, term->toString().c_str() );
	return false;
    }


    //
    // Check each individual argument
    //

    bool ok = true;

    for ( unsigned i=0; i < argDeclList.size() && ok; i++ )
    {
	QString wanted = argDeclList[ i ].stripWhiteSpace();
	YCPValue seen  = term->value( i );

	if	( wanted == "string"	)	ok = seen->isString();
	else if ( wanted == "boolean"	)	ok = seen->isBoolean();
	else if ( wanted == "bool"	)	ok = seen->isBoolean();
	else if ( wanted == "list"	)	ok = seen->isList();
	else if ( wanted == "map"	)	ok = seen->isMap();
	else if ( wanted == "integer"	)	ok = seen->isInteger();
	else if ( wanted == "int"	)	ok = seen->isInteger();
	else if ( wanted == "any"	)	ok = true;
	else
	{
	    y2error( "Bad declaration for wizard command %s : Unknown type \"%s\"",
		     (const char *) declaration, (const char *) wanted );
	}
    }

    if ( ! ok )
    {
	y2error( "Bad arguments for wizard command %s : %s",
		 (const char *) declaration, term->toString().c_str() );
    }

    if ( ok && _verboseCommands )
    {
	// Intentionally logging as milestone because a YCP app just explicitly
	// requested this log level
	y2milestone( "Recognized wizard command %s : %s",
		     (const char *) declaration, term->toString().c_str() );
    }

    return ok;
}


QString YQWizard::qStringArg( const YCPTerm & term, int argNo )
{
    return fromUTF8( stringArg( term, argNo ).c_str() );
}


string YQWizard::stringArg( const YCPTerm & term, int argNo )
{
    if ( term->size() > argNo )
    {
	YCPValue arg( term->value( argNo ) );

	if ( arg->isString() )
	    return arg->asString()->value();
    }

    y2error( "Couldn't convert arg #%d of '%s' to string", argNo, term->toString().c_str() );
    return "";
}


bool YQWizard::boolArg( const YCPTerm & term, int argNo )
{
    if ( term->size() > argNo )
    {
	YCPValue arg( term->value( argNo ) );

	if ( arg->isBoolean() )
	    return arg->asBoolean()->value();
    }

    y2error( "Couldn't convert arg #%d of '%s' to bool", argNo, term->toString().c_str() );
    return false;
}


YCPValue YQWizard::anyArg( const YCPTerm & term, int argNo )
{
    if ( term->size() > argNo )
    {
	return term->value( argNo );
    }

    return YCPVoid();
}



YCPValue YQWizard::command( const YCPTerm & cmd )
{
#define OK YCPBoolean( true );


    if ( isCommand( "SetHelpText	  ( string )", cmd ) )	{ setHelpText	( qStringArg( cmd, 0 ) );		return OK; }
    if ( isCommand( "SetDialogIcon	  ( string )", cmd ) )	{ setDialogIcon ( qStringArg( cmd, 0 ) );		return OK; }
    if ( isCommand( "SetDialogHeading	  ( string )", cmd ) )	{ setDialogHeading( qStringArg( cmd, 0 ) );		return OK; }

    if ( isCommand( "SetCurrentStep	  ( string )", cmd ) )	{ setCurrentStep( qStringArg( cmd, 0 ) );		return OK; }
    if ( isCommand( "AddStep ( string, string )"     , cmd ) )	{ addStep( qStringArg( cmd, 0 ), qStringArg( cmd, 1 )); return OK; }
    if ( isCommand( "AddStepHeading	  ( string )", cmd ) )	{ addStepHeading( qStringArg( cmd, 0 ) );		return OK; }
    if ( isCommand( "DeleteSteps()"		     , cmd ) )	{ deleteSteps();					return OK; }
    if ( isCommand( "UpdateSteps()"		     , cmd ) )	{ updateSteps();					return OK; }

    if ( isCommand( "SetAbortButtonLabel  ( string )", cmd ) )	{ setButtonLabel( _abortButton, qStringArg( cmd, 0 ) ); return OK; }
    if ( isCommand( "SetBackButtonLabel	  ( string )", cmd ) )	{ setButtonLabel( _backButton,	qStringArg( cmd, 0 ) ); return OK; }
    if ( isCommand( "SetNextButtonLabel	  ( string )", cmd ) )	{ setButtonLabel( _nextButton,	qStringArg( cmd, 0 ) ); return OK; }
    if ( isCommand( "SetCancelButtonLabel ( string )", cmd ) )	{ setButtonLabel( _abortButton, qStringArg( cmd, 0 ) ); return OK; }
    if ( isCommand( "SetAcceptButtonLabel ( string )", cmd ) )	{ setButtonLabel( _nextButton,	qStringArg( cmd, 0 ) ); return OK; }

    if ( isCommand( "SetAbortButtonID	  ( any )"   , cmd ) )	{ setButtonID( _abortButton,	anyArg( cmd, 0 ) );	return OK; }
    if ( isCommand( "SetBackButtonID	  ( any )"   , cmd ) )	{ setButtonID( _backButton,	anyArg( cmd, 0 ) );	return OK; }
    if ( isCommand( "SetNextButtonID	  ( any )"   , cmd ) )	{ setButtonID( _nextButton,	anyArg( cmd, 0 ) );	return OK; }

    if ( isCommand( "EnableBackButton	  ( bool )"  , cmd ) )	{ enableButton( _backButton,	boolArg( cmd, 0 ) );	return OK; }
    if ( isCommand( "EnableNextButton	  ( bool )"  , cmd ) )	{ enableButton( _nextButton,	boolArg( cmd, 0 ) );	return OK; }
    if ( isCommand( "EnableAbortButton	  ( bool )"  , cmd ) )	{ enableButton( _abortButton,	boolArg( cmd, 0 ) );	return OK; }
    if ( isCommand( "ProtectNextButton	  ( bool )"  , cmd ) )	{ _protectNextButton = boolArg( cmd, 0 );		return OK; }

    if ( isCommand( "SetFocusToNextButton ()"	     , cmd ) )	{ setButtonFocus( _nextButton );			return OK; }
    if ( isCommand( "SetFocusToBackButton ()"	     , cmd ) )	{ setButtonFocus( _backButton );			return OK; }


    if ( isCommand( "SetVerboseCommands	  ( bool )"  , cmd ) )	{ setVerboseCommands( boolArg( cmd, 0 ) );		return OK; }

    if ( isCommand( "DeleteTreeItems()"		     , cmd ) )	{ deleteTreeItems();					return OK; }
    if ( isCommand( "SelectTreeItem( string )"	     , cmd ) )	{ selectTreeItem( qStringArg( cmd, 0 ) );		return OK; }
    if ( isCommand( "AddTreeItem( string, string, string )", cmd ) )	{ addTreeItem	( qStringArg( cmd, 0 ),
											  qStringArg( cmd, 1 ),
											  qStringArg( cmd, 2 )	);	return OK; }

    if ( isCommand( "AddMenu	  ( string, string )"	      , cmd ) ) { addMenu	( qStringArg( cmd, 0 ),
											  qStringArg( cmd, 1 ) );	return OK; }

    if ( isCommand( "AddSubMenu	  ( string, string, string )" , cmd ) ) { addSubMenu	( qStringArg( cmd, 0 ),
											  qStringArg( cmd, 1 ),
											  qStringArg( cmd, 2 ) );	return OK; }

    if ( isCommand( "AddMenuEntry ( string, string, string )" , cmd ) ) { addMenuEntry	( qStringArg( cmd, 0 ),
											  qStringArg( cmd, 1 ),
											  qStringArg( cmd, 2 ) );	return OK; }

    if ( isCommand( "AddMenuSeparator ( string )"	     , cmd ) )	{ addMenuSeparator( qStringArg( cmd, 0 ) );	return OK; }
    if ( isCommand( "DeleteMenus ()"			     , cmd ) )	{ deleteMenus();				return OK; }
    if ( isCommand( "ShowReleaseNotesButton( string, any )"  , cmd ) )	{ showReleaseNotesButton( stringArg( cmd, 0 ),
												  anyArg   ( cmd, 1 )); return OK; }
    if ( isCommand( "HideReleaseNotesButton()"		     , cmd ) )	{ hideReleaseNotesButton();			return OK; }
    if ( isCommand( "RetranslateInternalButtons()"	     , cmd ) )	{ retranslateInternalButtons() ;		return OK; }
    if ( isCommand( "Ping()"				     , cmd ) )	{ ping() ;					return OK; }
    y2error( "Undefined wizard command: %s", cmd->toString().c_str() );
    return YCPBoolean( false );

#undef OK
}



#include "YQWizard.moc"
