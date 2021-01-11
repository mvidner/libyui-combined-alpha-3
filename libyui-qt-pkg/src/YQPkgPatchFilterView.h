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
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:	      YQPkgPatchFilterView.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#ifndef YQPkgPatchFilterView_h
#define YQPkgPatchFilterView_h

#include "YQZypp.h"
#include "YQPkgSelMapper.h"
#include <QLabel>


class YQPkgPatchList;
class YQPkgDescriptionView;
class QComboBox;
class QLabel;
class QSplitter;
class QTabWidget;


/**
 * @short Display a list of zypp::Patch objects and ( below ) details about the
 * currently selected patch.
 **/
class YQPkgPatchFilterView : public QWidget
{
    Q_OBJECT

public:

    /**
     * Constructor
     **/
    YQPkgPatchFilterView( QWidget * parent );

    /**
     * Destructor
     **/
    virtual ~YQPkgPatchFilterView();


    /**
     * Returns this view's selections list.
     **/
    YQPkgPatchList * patchList() const { return _patchList; }


public slots:

    /**
     * Update the "total download size" field.
     **/
    void updateTotalDownloadSize();


protected slots:

    /**
     * Fill the patch list with regard to the _patchCategory
     * combo box.
     **/
    void fillPatchList();


protected:


    // Data members

    QSplitter *			_splitter;
    YQPkgPatchList *		_patchList;
    QComboBox *			_patchFilter;
    QTabWidget *		_detailsViews;
    YQPkgDescriptionView *	_descriptionView;
    QLabel *			_totalDownloadSize;

    YQPkgSelMapper		_selMapper;
};



#endif // ifndef YQPkgPatchFilterView_h
