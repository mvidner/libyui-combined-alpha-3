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

   File:       NCPopupMenu.h

   Author:     Michael Andres <ma@suse.de>

/-*/

#ifndef NCPopupMenu_h
#define NCPopupMenu_h

#include <iosfwd>
#include <list>

#include "NCPopupTable.h"
#include "NCMenuButton.h"

using std::list;


class NCPopupMenu : public NCPopupTable
{
private:

    NCPopupMenu & operator=( const NCPopupMenu & );
    NCPopupMenu( const NCPopupMenu & );

    std::map<YTableItem *, YMenuItem *> itemsMap;

protected:

    virtual NCursesEvent wHandleInput( wint_t ch );
    virtual bool postAgain();

public:

    NCPopupMenu( const wpos at,
		 YItemIterator begin,
		 YItemIterator end );

    virtual ~NCPopupMenu();

};



#endif // NCPopupMenu_h