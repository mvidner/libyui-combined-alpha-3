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

  File:	      YQIconPool.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#define y2log_component "qt-ui"
#include <ycp/y2log.h>

#include "YQIconPool.h"


#include "icons/autodel.xpm"
#include "icons/autodel_disabled.xpm"
#include "icons/autoinstall.xpm"
#include "icons/autoinstall_disabled.xpm"
#include "icons/autoupdate.xpm"
#include "icons/autoupdate_disabled.xpm"
#include "icons/del.xpm"
#include "icons/del_disabled.xpm"
#include "icons/install.xpm"
#include "icons/install_disabled.xpm"
#include "icons/keepinstalled.xpm"
#include "icons/keepinstalled_disabled.xpm"
#include "icons/noinst.xpm"
#include "icons/noinst_disabled.xpm"
#include "icons/protected.xpm"
#include "icons/protected_disabled.xpm"
#include "icons/sel_autodel.xpm"
#include "icons/sel_autodel_disabled.xpm"
#include "icons/sel_autoinstall.xpm"
#include "icons/sel_autoinstall_disabled.xpm"
#include "icons/sel_autoupdate.xpm"
#include "icons/sel_autoupdate_disabled.xpm"
#include "icons/taboo.xpm"
#include "icons/taboo_disabled.xpm"
#include "icons/update.xpm"
#include "icons/update_disabled.xpm"

#include "icons/normal_conflict.xpm"
#include "icons/select_conflict.xpm"
#include "icons/delete_conflict.xpm"
#include "icons/unresolvable_conflict.xpm"

#include "icons/checklist_off.xpm"
#include "icons/checklist_on.xpm"



YQIconPool * YQIconPool::_iconPool = 0;

QPixmap YQIconPool::pkgTaboo()			{ return iconPool()->cachedIcon( taboo_xpm );			}
QPixmap YQIconPool::pkgDel()			{ return iconPool()->cachedIcon( del_xpm );			}
QPixmap YQIconPool::pkgUpdate()			{ return iconPool()->cachedIcon( update_xpm );			}
QPixmap YQIconPool::pkgInstall()		{ return iconPool()->cachedIcon( install_xpm );			}
QPixmap YQIconPool::pkgAutoInstall()		{ return iconPool()->cachedIcon( autoinstall_xpm );		}
QPixmap YQIconPool::pkgAutoUpdate()		{ return iconPool()->cachedIcon( autoupdate_xpm );		}
QPixmap YQIconPool::pkgAutoDel()		{ return iconPool()->cachedIcon( autodel_xpm );			}
QPixmap YQIconPool::pkgSelAutoInstall()		{ return iconPool()->cachedIcon( sel_autoinstall_xpm );		}
QPixmap YQIconPool::pkgSelAutoUpdate()		{ return iconPool()->cachedIcon( sel_autoupdate_xpm );		}
QPixmap YQIconPool::pkgSelAutoDel()		{ return iconPool()->cachedIcon( sel_autodel_xpm );		}
QPixmap YQIconPool::pkgKeepInstalled()		{ return iconPool()->cachedIcon( keepinstalled_xpm );		}
QPixmap YQIconPool::pkgNoInst()			{ return iconPool()->cachedIcon( noinst_xpm );			}
QPixmap YQIconPool::pkgProtected()		{ return iconPool()->cachedIcon( protected_xpm );		}

QPixmap YQIconPool::disabledPkgTaboo()		{ return iconPool()->cachedIcon( taboo_disabled_xpm );		}
QPixmap YQIconPool::disabledPkgDel()		{ return iconPool()->cachedIcon( del_disabled_xpm );		}
QPixmap YQIconPool::disabledPkgUpdate()		{ return iconPool()->cachedIcon( update_disabled_xpm );		}
QPixmap YQIconPool::disabledPkgInstall()	{ return iconPool()->cachedIcon( install_disabled_xpm );	}
QPixmap YQIconPool::disabledPkgAutoInstall()	{ return iconPool()->cachedIcon( autoinstall_disabled_xpm );	}
QPixmap YQIconPool::disabledPkgAutoUpdate()	{ return iconPool()->cachedIcon( autoupdate_disabled_xpm );	}
QPixmap YQIconPool::disabledPkgAutoDel()	{ return iconPool()->cachedIcon( autodel_disabled_xpm );	}
QPixmap YQIconPool::disabledPkgSelAutoInstall()	{ return iconPool()->cachedIcon( sel_autoinstall_disabled_xpm );}
QPixmap YQIconPool::disabledPkgSelAutoUpdate()	{ return iconPool()->cachedIcon( sel_autoupdate_disabled_xpm );	}
QPixmap YQIconPool::disabledPkgSelAutoDel()	{ return iconPool()->cachedIcon( sel_autodel_disabled_xpm );	}
QPixmap YQIconPool::disabledPkgKeepInstalled()	{ return iconPool()->cachedIcon( keepinstalled_disabled_xpm );	}
QPixmap YQIconPool::disabledPkgNoInst()		{ return iconPool()->cachedIcon( noinst_disabled_xpm );		}
QPixmap YQIconPool::disabledPkgProtected()	{ return iconPool()->cachedIcon( protected_disabled_xpm );	}

QPixmap YQIconPool::checkMarkOn()		{ return iconPool()->cachedIcon( checklist_on_xpm );		}
QPixmap YQIconPool::checkMarkOff()		{ return iconPool()->cachedIcon( checklist_off_xpm );		}

QPixmap YQIconPool::normalPkgConflict()		{ return iconPool()->cachedIcon( normal_conflict_xpm );		}
QPixmap YQIconPool::severePkgConflict()		{ return iconPool()->cachedIcon( normal_conflict_xpm );		}
QPixmap YQIconPool::unresolvablePkgConflict()	{ return iconPool()->cachedIcon( unresolvable_conflict_xpm );	}
QPixmap YQIconPool::deletePkgConflict()		{ return iconPool()->cachedIcon( delete_conflict_xpm );		}
QPixmap YQIconPool::tabooPkgConflict()		{ return iconPool()->cachedIcon( normal_conflict_xpm );		}
QPixmap YQIconPool::selectPkgConflict()		{ return iconPool()->cachedIcon( select_conflict_xpm );		}


YQIconPool * YQIconPool::iconPool()
{
    if ( ! _iconPool )
	_iconPool = new YQIconPool();

    return _iconPool;
}


YQIconPool::YQIconPool()
{
    _iconCache.setAutoDelete( true );
}


YQIconPool::~YQIconPool()
{
    // NOP
}



QPixmap
YQIconPool::cachedIcon( const char ** xpm_data )
{
    QPixmap * iconPtr = _iconCache[ xpm_data ];

    if ( ! iconPtr )
    {
	iconPtr = new QPixmap( xpm_data );
	_iconCache.insert( xpm_data, iconPtr );
    }
	
    return *iconPtr;
}

