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

  File:	      YQPkgGenericDetailsView.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

// -*- c++ -*-


#ifndef YQPkgGenericDetailsView_h
#define YQPkgGenericDetailsView_h

#include <qtextbrowser.h>
#include <YQZypp.h>
#include <zypp/Date.h>


class QTabWidget;
using std::string;


/**
 * @short Abstract base class for details views. Handles generic stuff like
 * HTML formatting, Qt slots and display only if this view is visible at all
 * ( it may be hidden if it's part of a QTabWidget ).
 **/
class YQPkgGenericDetailsView : public QTextBrowser
{
    Q_OBJECT

protected:

    /**
     * Constructor.
     **/
    YQPkgGenericDetailsView( QWidget * parent );

    /**
     * Destructor.
     **/
    virtual ~YQPkgGenericDetailsView();


public:

    /**
     * Returns the minimum size required for this widget.
     * Inherited from QWidget.
     **/
    virtual QSize minimumSizeHint() const;

    /**
     * Returns a uniform heading in HTML format for the specified selectable:
     * name and summary
     * or name, version and summary, if 'showVersion' is 'true'
     **/
    static QString htmlHeading( ZyppSel selectable, bool showVersion = false );

    /**
     * Escapes characters special to HTML in a ( plain text ) string, such as:
     * '<'   ->   '&lt;'
     * '>'   ->   '&gt;'
     * '&'   ->   '&amp;'
     *
     * Returns the escaped string.
     **/
    static QString htmlEscape( const QString & plainText );

    /**
     * Returns a string containing a HTML table with 'contents'.
     **/
    static QString table( const QString & contents );

    /**
     * Returns a string containing a HTML table row with 'contents'.
     **/
    static QString row( const QString & contents );

    /**
     * Returns a string containing a HTML table cell with 'contents'.
     **/
    static QString cell( QString contents		);
    static QString cell( int contents 			);
    static QString cell( const string & contents	);
    static QString cell( const zypp::Date & date	);

    /**
     * Returns a string containing a HTML table cell with 'contents'
     * for table headers.
     **/
    static QString hcell( QString contents );


public slots:

    /**
     * Show details for the specified package.
     * Delayed ( optimized ) display if this is embedded into a QTabWidget
     * parent: In this case, wait until this page becomes visible.
     **/
    void showDetailsIfVisible( ZyppSel selectable );

    // slot clear() inherited from QTextEdit

    /**
     * Show details for the specified package.
     * Reimplement this in derived classes.
     **/
    virtual void showDetails( ZyppSel selectable ) = 0;

    
protected slots:

    /**
     * Show data for the last package.
     **/
    void reload( QWidget * newCurrent );


protected:
    
    // Data members

    QTabWidget	* 	_parentTab;
    ZyppSel		_selectable;
};


#endif // ifndef YQPkgGenericDetailsView_h
