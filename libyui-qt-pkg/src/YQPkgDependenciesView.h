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

  File:	      YQPkgDependenciesView.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#ifndef YQPkgDependenciesView_h
#define YQPkgDependenciesView_h

#include "YQZypp.h"
#include "YQPkgGenericDetailsView.h"

using std::string;


/**
 * @short Display technical details ( very much like 'rpm -qi' ) for a zypp::Package
 * object - the installed instance, the candidate instance or both ( in two
 * columns ) if both exist. All other available instances are ignored.
 **/
class YQPkgDependenciesView : public YQPkgGenericDetailsView
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgDependenciesView( QWidget * parent );


    /**
     * Destructor
     **/
    virtual ~YQPkgDependenciesView();


protected:

    /**
     * Show details for the specified selectable:
     * In this case technical data, very much like "rpm -qi".
     * Overwritten from YQPkgGenericDetailsView.
     **/
    virtual void showDetails( ZyppSel selectable );

    /**
     * Returns a string containing a HTML table for technical details for one
     * package.
     **/
    QString simpleTable( ZyppObj pkg );


    /**
     * Returns a string containing a HTML table for technical details for two
     * package instances: The installed instance and an alternate instance.
     * (usually the candidate instance).
     **/
    QString complexTable( ZyppObj installed,
			  ZyppObj candidate );

    /**
     * Format a zypp::CapSet (describing zypp::Dep::REQUIRES etc.)
     * with a heading in HTML lines.
     *
     * Returns an empty string if capSet is empty.
     **/
    static QString row( const QString &		heading,
			const zypp::Capabilities & 	capSet );


    /**
     * Format two zypp::Capabilities (describing zypp::Dep::REQUIRES etc.)
     * with a heading in HTML lines.
     *
     * Returns an empty string both capSets are empty.
     **/
    static QString row( const QString & 	heading,
			const zypp::Capabilities & 	capSet1,
			const zypp::Capabilities & 	capSet2 );

    /**
     * Returns a string containing a HTML table row with 'contents'.
     *
     * Required here to avoid inheritance ambiguities.
     **/
    static QString row( const QString & contents )
	{ return YQPkgGenericDetailsView::row( contents ); }

    /**
     * Format a zypp::Capabilities (describing zypp::Dep::REQUIRES etc.)
     * in HTML lines, separated with <BR>.
     * Returns an empty string if capSet is empty.
     **/
    static QString htmlLines( const zypp::Capabilities & capSet );
};


#endif // ifndef YQPkgDependenciesView_h
