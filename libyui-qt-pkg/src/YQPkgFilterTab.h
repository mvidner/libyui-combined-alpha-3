/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:       YQPkgFilterTab.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YQPkgFilterTab_h
#define YQPkgFilterTab_h

#include <QWidget>
#include <QHash>
#include <QTabWidget>
#include "ImplPtr.h"

class YQPkgFilterTabPrivate;
class YQPkgFilterPage;
class YQPkgDiskUsageList;
class QAction;


/**
 * Widget for "tabbed browsing" in packages:
 *
 *               /------\/------\/------\
 *     [View v]  | Tab1 || Tab2 || Tab3 |               [Close]
 *     +-----------------+------------------------------------+
 *     |                 |                                    |
 *     | QStackedWidget: |    Right pane                      |
 *     |                 |                                    |
 *     | Filter pages    |    (application defined)           |
 *     |                 |                                    |
 *     |                 |                                    |
 *     |                 |                                    |
 *     .                 .                                    .
 *     .                 .                                    .
 *     .                 .                                    .
 *     |                 |                                    |
 *     +-----------------+------------------------------------+
 *
 * Each filter page corresponds to one tab and a number of widgets in a
 * QStackedWidget in the left filter pane. When tabs are switched, the
 * corresponding filter page is raised to the top of the widget stack.
 * The right pane, however, remains unchanged.
 *
 * Only a small numbers of filter pages is displayed as open tabs right
 * away. Each of the other filter pages is shown in a new tabs when the user
 * requests it via the pop-up menu on [View] button. Similarly, the tabs for all
 * but the last filter pages can be closed with the [Close] button.
 *
 * The left (filter page) and right panes are separated with a user-moveable
 * splitter.
 **/
class YQPkgFilterTab: public QTabWidget
{
    Q_OBJECT

public:

    /**
     * Constructor. 'settingsName' is the name to use to save and load settings.
     **/
    YQPkgFilterTab( QWidget * parent, const QString & settingsName );

    /**
     * Destructor.
     **/
    virtual ~YQPkgFilterTab();

    /**
     * Add a page with a user-visible "pageLabel", a widget with the page
     * content and an internal name (or ID). 
     * 'pageContent' will be reparented to a subwidget of this class.
     **/
    void addPage( const QString &       pageLabel,
                  QWidget *             pageContent,
                  const QString &       internalName );

    /**
     * Return the right pane.
     **/
    QWidget * rightPane() const;

    /**
     * Return the disk usage list widget or 0 if there is none.
     **/
    YQPkgDiskUsageList * diskUsageList() const;

    /**
     * Find a filter page by its content widget (the widget that was passed
     * to addPage() ).
     * Return 0 if there is no such page.
     **/
    YQPkgFilterPage * findPage( QWidget * pageContent );

    /**
     * Find a filter page by its internal name.
     * Return 0 if there is no such page.
     **/
    YQPkgFilterPage * findPage( const QString & internalName );

    /**
     * Find a filter page by its tab index.
     * Return 0 if there is no such page.
     **/
    YQPkgFilterPage * findPage( int tabIndex );

    /**
     * Return the number of open tabs.
     **/
    int tabCount() const;

    /**
     * Event filter to catch mouse right clicks on open tabs for the tab
     * context menu. Returns 'true' if the event is processed and consumed,
     * 'false' if processed should be continued by the watched object itself.
     *
     * Reimplemented from QObject.
     **/
    virtual bool eventFilter ( QObject * watchedObj, QEvent * event );


signals:

    /**
     * Emitted when the current page changes.
     * NOT emitted initially for the very first page that is shown.
     **/
    void currentChanged( QWidget * newPageContent );


public slots:

    /**
     * Show a page. Create a tab for that page if it doesn't already exist.
     **/
    void showPage( QWidget * page );
    void showPage( const QString & internalName );

    /**
     * Close the current page unless this is the last visible page.
     **/
    void closeCurrentPage();

    /**
     * Load settings, including which tabs are to be opened and in which order.
     * Return 'true' if settings could be loaded, 'false' if not.
     *
     * Applications should call this after all pages have been added so the
     * open tabs can be restored the same way as the user left the program.
     * If tabCount() is still 0 afterwards, there were no settings, so it might
     * make sense to open a number of default pages.
     **/
    void loadSettings();

    /**
     * Save the current settings, including which tabs are currently open and
     * in which order. This is implicitly done in the destructor.
     **/
    void saveSettings();

    /**
     * Close all currently open pages.
     **/
    void closeAllPages();


protected slots:

    /**
     * Show the page with the specified tab index.
     **/
    void showPage( int tabIndex );

    /**
     * Show the page with the widget of this action's data().
     **/
    void showPage( QAction * action );

    /**
     * Move the current tab page (from the context menu) one position to the
     * left.
     **/
    void contextMovePageLeft();

    /**
     * Move the current tab page (from the context menu) one position to the
     * right.
     **/
    void contextMovePageRight();

    /**
     * Close the current tab page (from the context menu).
     **/
    void contextClosePage();


protected:

    /**
     * Show a page.
     **/
    void showPage( YQPkgFilterPage * page );

    /**
     * Open the tab context menu for the tab at the specified position.
     * Return 'true' upon success (i.e., there is a tab at that position),
     * 'false' upon failure.
     **/
    bool postTabContextMenu( const QPoint & pos );

    /**
     * Swap two tabs and adjust their tab indices accordingly.
     **/
    void swapTabs( YQPkgFilterPage * page1, YQPkgFilterPage * page2 );


private:

    ImplPtr<YQPkgFilterTabPrivate> priv;
};



/**
 * Helper class for filter pages
 **/
struct YQPkgFilterPage
{
    YQPkgFilterPage( const QString &	pageLabel,
		     QWidget *		content,
		     const QString &	internalName )
	: content( content )
	, label( pageLabel )
	, id( internalName )
	, closeEnabled( true )
	, tabIndex( -1 )
	{}

    QWidget *	content;
    QString	label;		// user visible text
    QString	id;		// internal name
    bool	closeEnabled;
    int		tabIndex;	// index of the corresponding tab or -1 if none
};


#endif // YQPkgFilterTab_h
