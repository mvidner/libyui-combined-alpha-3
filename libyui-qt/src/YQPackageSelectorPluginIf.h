/**************************************************************************
Copyright (C) 2000 - 2010 Novell, Inc.
All Rights Reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

**************************************************************************/


/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|						     (c) SuSE Linux AG |
\----------------------------------------------------------------------/

  File:		YQPackageSelectorPluginIf.h

  Author:	kmachalkova@suse.cz


/-*/

#ifndef YQPackageSelectorPluginIf_h
#define YQPackageSelectorPluginIf_h

#include <YDialog.h>
#include <YEvent.h>
#include <YPackageSelectorPlugin.h>

class YQPackageSelectorPluginIf
{

  public:

    virtual ~YQPackageSelectorPluginIf() {}

    virtual YPackageSelector *createPackageSelector( YWidget *parent, long modeFlags ) = 0 ;
    virtual YWidget *createPatternSelector( YWidget * parent, long modeFlags ) = 0;
    virtual YWidget *createSimplePatchSelector( YWidget * parent, long modeFlags ) = 0;
};

#endif
