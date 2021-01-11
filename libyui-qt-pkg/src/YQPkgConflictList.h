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

  File:	      YQPkgConflictList.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#ifndef YQPkgConflictList_h
#define YQPkgConflictList_h


#include <stdio.h>
#include <QMap>
#include <QFile>
#include <QScrollArea>

#include <zypp/Resolver.h>
#include <zypp/ResolverProblem.h>
#include <zypp/ProblemSolution.h>

class QVBoxLayout;
class QRadioButton;
class QAction;
class YQPkgConflict;
class YQPkgConflictResolution;
class YQPkgConflictDialog;


/**
 * @short Display package dependency conflicts in a tree list and let the user
 * choose how to resolve each conflict.
 **/
class YQPkgConflictList : public QScrollArea
{
    Q_OBJECT

public:
    /**
     * Constructor.
     **/
    YQPkgConflictList( QWidget * parent );

    /**
     * Destructor.
     **/
    virtual ~YQPkgConflictList();

    /**
     * Fill the list with the specified problems.
     **/
    void fill( zypp::ResolverProblemList problemList );

    /**
     * Check if the conflict list is empty.
     **/
    bool isEmpty() const { return count() == 0; }

    /**
     * Returns the number of conflicts in the list.
     **/
    int count() const { return _conflicts.count(); }


public slots:

    /**
     * Apply the choices the user made.
     **/
    void applyResolutions();

    /**
     * Ask for a file name and save the current conflict list to file.
     **/
    void askSaveToFile() const;

    void clear();

    void relayout();

public:

    /**
     * Save the conflict list in its current state to a file. Retains the
     * current 'expanded' state, i.e. writes only those entries that are
     * currently open (not collapsed) in the tree.
     *
     * Posts error popups if 'interactive' is 'true' (only log entries
     * otherwise).
     **/
    void saveToFile( const QString filename, bool interactive ) const;

protected:

    QList<YQPkgConflict*> _conflicts;
    QVBoxLayout *_layout;

signals:

    /**
     * Update package states - they may have changed.
     **/
    void updatePackages();

};



/**
 * @short Root item for each individual conflict
 **/
class YQPkgConflict: public QFrame
{
    Q_OBJECT

public:

    /**
     * Constructor.
     **/
    YQPkgConflict( QWidget *			parent,
		   zypp::ResolverProblem_Ptr	problem );

    /**
     * Destructor.
     **/
    virtual ~YQPkgConflict() {}

    /**
     * Returns the corresponding ResolverProblem.
     **/
    zypp::ResolverProblem_Ptr problem() const { return _problem; }

    /**
     * Returns the resolution the user selected
     * or 0 if he didn't select one
     **/
    zypp::ProblemSolution_Ptr userSelectedResolution();

    /**
     * save one item to file.
     **/
    void saveToFile( QFile &file ) const;

					
protected slots:
    
    void detailsExpanded();

signals:
    
    void expanded();

protected:

    /**
     * Format the text heading line for this item.
     **/
    void formatHeading();

    /**
     * Add suggestions how to resolve this conflict.
     **/
    void addSolutions();

    //
    // Data members
    //

    zypp::ResolverProblem_Ptr		_problem;
    QLabel *				_resolutionsHeader;
    QList<zypp::ProblemSolution_Ptr>	_resolutions;
    QMap<QRadioButton *, zypp::ProblemSolution_Ptr> 	_solutions;
    QMap<QLabel *, zypp::ProblemSolution_Ptr> 		_details;
    QVBoxLayout *					_layout;
};

#endif // ifndef YQPkgConflictList_h
