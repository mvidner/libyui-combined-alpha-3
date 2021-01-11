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

  File:	      YQPkgSearchFilterView.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

// -*- c++ -*-


#ifndef YQPkgSearchFilterView_h
#define YQPkgSearchFilterView_h

#include "YQZypp.h"
#include <qvbox.h>
#include <qregexp.h>


class QComboBox;
class QCheckBox;
class QPushButton;
class QRadioButton;

using std::list;
using std::string;


/**
 * @short Filter view for searching within packages
 **/
class YQPkgSearchFilterView : public QVBox
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgSearchFilterView( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgSearchFilterView();

    /**
     * Returns the minimum size required for this widget.
     * Inherited from QWidget.
     **/
    virtual QSize minimumSizeHint() const;

    /**
     * Check one ResObject against the currently selected values.
     * Returns true if the package matches, false if not.
     **/
    bool check( ZyppSel	selectable,
		ZyppObj zyppObj );


public slots:

    /**
     * Filter according to the view's rules and current selection.
     * Emits those signals:
     *	  filterStart()
     *	  filterMatch() for each pkg that matches the filter
     *	  filterFinished()
     **/
    void filter();

    /**
     * Same as filter(), but only if this widget is currently visible.
     **/
    void filterIfVisible();

    /**
     * Set the keyboard focus into this view's input field.
     **/
    void setFocus();


signals:

    /**
     * Emitted when the filtering starts. Use this to clear package lists
     * etc. prior to adding new entries.
     **/
    void filterStart();

    /**
     * Emitted during filtering for each pkg that matches the filter.
     **/
    void filterMatch( ZyppSel	selectable,
		      ZyppPkg	pkg );

    /**
     * Emitted when filtering is finished.
     **/
    void filterFinished();

    /**
     * Send a short message about unsuccessful searches.
     **/
    void message( const QString & text );


protected:

    // Caution: Enum order must match corresponding message strings in combo box!
    typedef enum SearchMode
    {
	Contains = 0,
	BeginsWith,
	ExactMatch,
	UseWildcards,
	UseRegExp
    };

    
    /**
     * Check if pkg matches the search criteria.
     **/
    bool check( ZyppSel	selectable,
		ZyppObj 	zyppObj,
		const QRegExp & 		regexp );

    /**
     * Check if a single pkg attribute matches the search criteria.
     **/
    bool check( const string & attribute, const QRegExp & regexp );

    /**
     * Check capability like
     * zypp::Resolvable::dep( zypp::Dep::PROVIDES ),
     * zypp::Resolvable::dep( zypp::Dep::REQUIRES )
     **/
    bool check( const zypp::Capabilities & capSet, const QRegExp & regexp );

    /**
     * Key press event: Execute search upon 'Return'
     * Reimplemented from QVBox / QWidget.
     **/
    virtual void keyPressEvent( QKeyEvent * event );


    // Data members

    QComboBox *		_searchText;
    QPushButton *	_searchButton;

    QCheckBox *_searchInName;
    QCheckBox *_searchInKeywords;
    QCheckBox *_searchInSummary;
    QCheckBox *_searchInDescription;
    QCheckBox *_searchInRequires;
    QCheckBox *_searchInProvides;
    QCheckBox *_searchInFileList;

    QComboBox *		_searchMode;
    QCheckBox *		_caseSensitive;

    int			_matchCount;
};



#endif // ifndef YQPkgSearchFilterView_h
