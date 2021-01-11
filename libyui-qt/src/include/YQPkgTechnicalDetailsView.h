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

  File:	      YQPkgTechnicalDetailsView.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

// -*- c++ -*-


#ifndef YQPkgTechnicalDetailsView_h
#define YQPkgTechnicalDetailsView_h

#include <y2pm/PMPackage.h>

#include "YQPkgGenericDetailsView.h"


/**
 * @short Display technical details ( very much like 'rpm -qi' ) for a PMPackage
 * object - the installed instance, the candidate instance or both ( in two
 * columns ) if both exist. All other available instances are ignored.
 **/
class YQPkgTechnicalDetailsView : public YQPkgGenericDetailsView
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgTechnicalDetailsView( QWidget * parent, bool youMode = false );


    /**
     * Destructor
     **/
    virtual ~YQPkgTechnicalDetailsView();


protected:

    /**
     * Show details for the specified PMObject:
     * In this case technical data, very much like "rpm -qi".
     * Overwritten from YQPkgGenericDetailsView.
     **/
    virtual void showDetails( PMObjectPtr pmObj );

    /**
     * Returns a string containing a HTML table for technical details for one
     * package.
     **/
    QString simpleTable( PMPackagePtr pkg );


    /**
     * Returns a string containing a HTML table for technical details for two
     * package instances: The installed instance and an alternate instance.
     * ( usually the candidate instance ).
     **/
    QString complexTable( PMPackagePtr installedPkg,
			  PMPackagePtr candidatePkg );

    /**
     * Returns a string containing HTML code for a package's authors list.
     **/
    QString authorsListCell( PMPackagePtr pkg ) const;

    /**
     * Format an RPM group. Retrieves the translated ( ! ) version.
     **/
    QString formatRpmGroup( PMPackagePtr pkg ) const;


    // Data members

    bool	_youMode;
};


#endif // ifndef YQPkgTechnicalDetailsView_h
