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

  File:	      NCApplication.h

  Author:     Gabriele Mohr <gs@suse.de>

/-*/


#ifndef NCApplication_h
#define NCApplication_h

#include "YApplication.h"


class NCApplication: public YApplication
{
    
protected:

    friend class YNCursesUI;
    
    /**
     * Constructor.
     *
     * Use YUI::app() to get the singleton for this class.
     **/
    NCApplication();

    /**
     * Destructor.
     **/
    virtual ~NCApplication();

public:
    
    /**
     * Set language and encoding for the locale environment ($LANG).
     *
     * 'language' is the ISO short code ("de_DE", "en_US", ...).
     *
     * 'encoding' an (optional) encoding ("utf8", ...) that will be appended if
     *  present.
     *
     * Reimplemented from YApplication.
     **/
    virtual void setLanguage( const string & language,
			      const string & encoding = string() );
    
    /**
     * Open a directory selection box and prompt the user for an existing
     * directory.
     *
     * 'startDir' is the initial directory that is displayed.
     *
     * 'headline' is an explanatory text for the directory selection box.
     * Graphical UIs may omit that if no window manager is running.
     *
     * Returns the selected directory name
     * or an empty string if the user canceled the operation.
     *
     * Implemented from YApplication.
     **/
    virtual string askForExistingDirectory( const string & startDir,
					    const string & headline );

    /**
     * Open a file selection box and prompt the user for an existing file.
     *
     * 'startWith' is the initial directory or file.
     *
     * 'filter' is one or more blank-separated file patterns, e.g.
     * "*.png *.jpg"
     *
     * 'headline' is an explanatory text for the file selection box.
     * Graphical UIs may omit that if no window manager is running.
     *
     * Returns the selected file name
     * or an empty string if the user canceled the operation.
     *
     * Implemented from YApplication.
     **/
    virtual string askForExistingFile( const string & startWith,
				       const string & filter,
				       const string & headline );

    /**
     * Open a file selection box and prompt the user for a file to save data
     * to.  Automatically asks for confirmation if the user selects an existing
     * file.
     *
     * 'startWith' is the initial directory or file.
     *
     * 'filter' is one or more blank-separated file patterns, e.g.
     * "*.png *.jpg"
     *
     * 'headline' is an explanatory text for the file selection box.
     * Graphical UIs may omit that if no window manager is running.
     *
     * Returns the selected file name
     * or an empty string if the user canceled the operation.
     *
     * Implemented from YApplication.
     **/
    virtual string askForSaveFileName( const string & startWith,
				       const string & filter,
				       const string & headline );
};


#endif // NCApplication_h