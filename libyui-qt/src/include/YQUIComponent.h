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

  File:		YQUIComponent.h

  Authors:	Stefan Hundhammer <sh@suse.de>

  Maintainer:	Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YQUIComponent_h
#define YQUIComponent_h

#include <YUIComponent.h>
#include "YQUI.h"


class YQUIComponent :  public YUIComponent
{
public:

    YQUIComponent(): YUIComponent() {}

    /**
     * Create a Qt UI. This is called from within setServerOptions().
     *
     * Reimplemented from YUIComponent.
     **/
    virtual YUI * createUI( int argc, char **argv, bool with_threads, const char * macro_file )
    {
	return new YQUI( argc, argv, with_threads, macro_file );
    }

    /**
     * Returns the name of this YaST2 component.
     *
     * Reimplemented from YUIComponent.
     */
    virtual string name() const { return "qt"; }

};


#endif // YQUIComponent_h
