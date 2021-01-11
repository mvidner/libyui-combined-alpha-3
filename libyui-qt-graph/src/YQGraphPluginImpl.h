/*
 * Copyright (c) [2009-2012] Novell, Inc.
 *
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, contact Novell, Inc.
 *
 * To contact Novell about this file by physical or electronic mail, you may
 * find current contact information at www.novell.com.
 */

/*
 * File:	YQGraphPluginImpl.h
 * Author:	Arvin Schnell <aschnell@suse.de>
 */


#ifndef YQGraphPluginImpl_h
#define YQGraphPluginImpl_h

#include <graphviz/types.h>

#include "YQGraphPluginIf.h"

class YQGraphPluginImpl : public YQGraphPluginIf
{

public:

    virtual ~YQGraphPluginImpl() {}

    virtual YGraph * createGraph( YWidget * parent, const std::string & filename,
				  const std::string & layoutAlgorithm );

    virtual YGraph * createGraph( YWidget * parent, /* graph_t */ void * graph );

};

#endif
