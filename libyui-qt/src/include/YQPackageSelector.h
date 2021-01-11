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

  File:	      YQPackageSelector.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

// -*- c++ -*-

#ifndef YQPackageSelector_h
#define YQPackageSelector_h

#include <qvbox.h>
#include <ycp/YCPString.h>

#include "YPackageSelector.h"


class QCheckBox;
class QComboBox;
class QLabel;
class QListView;
class QProgressBar;
class QPushButton;
class QSplitter;
class QTabWidget;
class QPopupMenu;
class QMenuBar;

class QY2ComboTabWidget;

class YQPkgConflictDialog;
class YQPkgDependenciesView;
class YQPkgDescriptionView;
class YQPkgDiskUsageList;
class YQPkgLangFilterView;
class YQPkgLangList;
class YQPkgList;
class YQPkgRpmGroupTagsFilterView;
class YQPkgSearchFilterView;
class YQPkgSelList;
class YQPkgSelectionsFilterView;
class YQPkgStatusFilterView;
class YQPkgTechnicalDetailsView;
class YQPkgUpdateProblemFilterView;
class YQPkgVersionsView;
class YQPkgYouPatchFilterView;
class YQPkgYouPatchList;


class YQPackageSelector : public QVBox, public YPackageSelector
{
    Q_OBJECT

public:

    YQPackageSelector( QWidget * parent, const YWidgetOpt & opt, const YCPString & floppyDevice );

    /**
     * Inherited from YWidget: Sets the enabled state of the
     * widget. All new widgets are enabled per definition. Only
     * enabled widgets can take user input.
     **/
    void setEnabling( bool enabled );

    /**
     * Minimum size the widget should have to make it look and feel
     * nice.
     * @dim Dimension, either YD_HORIZ or YD_VERT
     **/
    long nicesize( YUIDimension dim );

    /**
     * Sets the new size of the widget.
     **/
    void setSize( long newWidth, long newHeight );

    /**
     * Accept the keyboard focus.
     **/
    virtual bool setKeyboardFocus();


public slots:

    /**
     * Resolve package dependencies ( unconditionally ).
     *
     * Returns QDialog::Accepted or QDialog::Rejected.
     **/
     int resolvePackageDependencies();

    /**
     * Resolve package dependencies manually.
     *
     * Returns QDialog::Accepted or QDialog::Rejected.
     **/
     int manualResolvePackageDependencies();

    /**
     * Resolve selection dependencies ( unconditionally ).
     * Can safely be called even if there is no selection conflict dialog.
     *
     * Returns QDialog::Accepted or QDialog::Rejected.
     **/
     int resolveSelectionDependencies();

    /**
     * Automatically resolve package dependencies if desired
     * ( if the "auto check" checkbox is on ).
     **/
    void autoResolveDependencies();

    /**
     * Check for disk overflow and post a warning dialog if necessary.
     * The user can choose to override this warning.
     *
     * Returns QDialog::Accepted if no warning is necessary or if the user
     * wishes to override the warning, QDialog::Rejected otherwise.
     **/
    int checkDiskUsage();

    /**
     * Export all current selection/package states
     **/
    void pkgExport();

    /**
     * Import selection/package states
     **/
    void pkgImport();

    /**
     * Display a list of automatically selected packages
     * (excluding packages contained in any selections that are to be installed)
     **/
    void showAutoPkgList();

    /**
     * Install any -devel package for packages that are installed or marked for
     * installation 
     **/
    void installDevelPkgs();

    /**
     * Install any -debuginfo package for packages that are installed or marked
     * for installation 
     **/
    void installDebugInfoPkgs();

    /**
     * Install any subpackage that ends with 'suffix' for packages that are
     * installed or marked for installation
     **/
    void installSubPkgs( const QString suffix );

    /**
     * Close processing and abandon changes
     **/
    void reject();

    /**
     * Close processing and accept changes
     **/
    void accept();

    /**
     * Display ( generic ) online help.
     **/
    void help();

    /**
     * Display online help about symbols ( package status icons ).
     **/
    void symbolHelp();

    /**
     * Display online help about magic keys.
     **/
    void keyboardHelp();

    /**
     * Inform user about a feature that is not implemented yet.
     * This should NEVER show up in the final version.
     **/
    void notImplemented();

signals:

    /**
     * Emitted once ( ! ) when the dialog is about to be shown, when all widgets
     * are created and all signal/slot connections are set up - when it makes
     * sense to load data.
     **/
    void loadData();

    /**
     * Emitted when the internal data base might have changed and a refresh of
     * all displayed data might be necessary - e.g., when saved ( exported ) pkgs
     * states are reimported.
     **/
    void refresh();


protected:

    // Layout methods - create and layout widgets

    void basicLayout();

    QWidget *	layoutLeftPane		( QWidget * parent );
    QWidget *	layoutRightPane		( QWidget * parent );
    void	layoutFilters		( QWidget * parent );
    void 	layoutPkgList		( QWidget * parent );
    void 	layoutDetailsViews	( QWidget * parent );
    void 	layoutButtons		( QWidget * parent );
    void 	layoutMenuBar		( QWidget * parent );

    /**
     * Establish Qt signal / slot connections.
     *
     * This really needs to be a separate method to make sure all affected
     * wigets are created at this point.
     **/
    void makeConnections();

    /**
     * Add pulldown menus to the menu bar.
     *
     * This really needs to be a separate method to make sure all affected
     * wigets are created at this point.
     **/
    void addMenus();

    /**
     * Provide some fake data for testing
     **/
    void fakeData();

    /**
     * Connect a filter view that provides the usual signals with a package
     * list. By convention, filter views provide the following signals:
     *	  filterStart()
     *	  filterMatch()
     *	  filterFinished()
     *	  updatePackages()  ( optional )
     **/
    void connectFilter( QWidget *	filter,
			QWidget *	pkgList,
			bool		hasUpdateSignal = true );
    /**
     * Event handler for keyboard input - for debugging and testing.
     * Changes the current item's percentage on the fly.
     *
     * Reimplemented from QListView / QWidget.
     */
    virtual void keyPressEvent( QKeyEvent * ev );

    /**
     * Return HTML code describing a symbol ( an icon ).
     **/
    QString symHelp( const QString & imgFileName,
		     const QString & summary,
		     const QString & explanation	);

    
    /**
     * Return HTML code describing a key.
     **/
    QString keyHelp( const QString & key,
		     const QString & summary,
		     const QString & explanation	);

    /**
     * Basic HTML formatting: Embed text into <p> ... </p>
     **/
    static QString para( const QString & text );
    
    /**
     * Basic HTML formatting: Embed text into <li> ... </li>
     **/
    static QString listItem( const QString & text );


    // Data members

    bool				_searchMode;
    bool				_testMode;
    bool				_updateMode;
    bool				_youMode;
    bool				_summaryMode;
    int					_installedPkgs;
    QString				_floppyDevice;

    QCheckBox *				_autoDependenciesCheckBox;
    QPushButton *			_checkDependenciesButton;
    QTabWidget *			_detailsViews;
    QY2ComboTabWidget *			_filters;
    YQPkgConflictDialog *		_pkgConflictDialog;
    YQPkgConflictDialog *		_selConflictDialog;
    YQPkgDependenciesView *		_pkgDependenciesView;
    YQPkgDescriptionView *		_pkgDescriptionView;
    YQPkgDiskUsageList *		_diskUsageList;
    YQPkgLangFilterView *		_langFilterView;
    YQPkgLangList *			_langList;
    YQPkgList *				_pkgList;
    YQPkgRpmGroupTagsFilterView *	_rpmGroupTagsFilterView;
    YQPkgSearchFilterView *		_searchFilterView;
    YQPkgSelList *			_selList;
    YQPkgSelectionsFilterView *		_selectionsFilterView;
    YQPkgStatusFilterView *		_statusFilterView;
    YQPkgTechnicalDetailsView *		_pkgTechnicalDetailsView;
    YQPkgUpdateProblemFilterView *	_updateProblemFilterView;
    YQPkgVersionsView *			_pkgVersionsView;
    YQPkgYouPatchFilterView *		_youPatchFilterView;
    YQPkgYouPatchList *			_youPatchList;

    QMenuBar *				_menuBar;
    QPopupMenu *			_fileMenu;
    QPopupMenu *			_viewMenu;
    QPopupMenu *			_pkgMenu;
    QPopupMenu *			_youPatchMenu;
    QPopupMenu *			_extrasMenu;
    QPopupMenu *			_helpMenu;
};



#endif // YQPackageSelector_h
