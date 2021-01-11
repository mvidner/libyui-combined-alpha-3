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

  File:	      YQPkgChangesDialog.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#ifndef YQPkgChangesDialog_h
#define YQPkgChangesDialog_h

#include <QDialog>
#include <QComboBox>
#include <QRegExp>
#include <QFlags>

class YQPkgList;

/**
 * Changes dialog: Show a dialog with a list of packages that are changed.
 * By default, only packages with an "auto" status ( not set via selections )
 * are displayed.
 **/
class YQPkgChangesDialog : public QDialog
{
    Q_OBJECT

public:

  /**
   * filter combobox entries
   */
  enum FilterIndex
  {
    FilterIndexAll = 0,
    FilterIndexUser = 1,
    FilterIndexAutomatic = 2
  };

    /**
     * Filters
     */
    enum Filter
    {
      FilterNone = 0x0,
      FilterUser = 0x1,
      FilterAutomatic = 0x2,
      FilterAll = 0x1 | 0x2
    };
    Q_DECLARE_FLAGS(Filters, Filter);

    /**
     * Options
     */
    enum Option
    {
      OptionNone = 0x0,
      OptionAutoAcceptIfEmpty = 0x1
    };
    Q_DECLARE_FLAGS(Options, Option);

    /**
     * Set the current filter
     * This will change the combo box current selected
     * filter and update the list
     */
    void setFilter( Filters f );

    /**
     * Set the current filter
     * This will change the combo box current selected
     * filter and update the list
     */
    void setFilter( const QRegExp &regexp, Filters f );

    /**
     * Static convenience method: Post a changes dialog with text
     * 'message', a list of changed packages and one ( default ) or two buttons.
     *
     * Returns 'true' if the user accepted ( i.e. clicked the 'accept' button )
     * and 'false' if the user rejected ( i.e. clicked the 'reject' button or
     * the window manager close button ).
     *
     * If the list is empty ( i.e., there are no packages with an "auto"
     * status ), the dialog is not shown at all ( and returns 'true' ) - unless
     *'showIfListEmpty' is 'true'.
     **/
    static bool showChangesDialog( QWidget *		parent,
				   const QString & 	message,
				   const QString &	acceptButtonLabel,
				   const QString &	rejectButtonLabel = QString(),
                                   Filters f = FilterAutomatic,
                                   Options o = OptionAutoAcceptIfEmpty );

    /**
     * Static convenience method: Post a changes dialog with text 'message', a
     * list of changed packages whose names match the specified regular
     * expression 'regexp' and one ( default ) or two buttons.
     *
     * Returns 'true' if the user accepted ( i.e. clicked the 'accept' button )
     * and 'false' if the user rejected ( i.e. clicked the 'reject' button or
     * the window manager close button ).
     *
     * If the list is empty ( i.e., there are no packages with an "auto"
     * status ), the dialog is not shown at all ( and returns 'true' ) - unless
     *'showIfListEmpty' is 'true'.
     **/
    static bool showChangesDialog( QWidget *		parent,
				   const QString & 	message,
				   const QRegExp & 	regexp,
				   const QString &	acceptButtonLabel,
				   const QString &	rejectButtonLabel = QString(),
                                   Filters f = FilterAutomatic,
                                   Options o = OptionAutoAcceptIfEmpty );

    /**
     * Returns the preferred size.
     *
     * Reimplemented from QWidget to limit the dialog to the screen dimensions.
     **/
    virtual QSize sizeHint () const;

protected slots:
    /**
     * called when the filter is changed
     */
    void slotFilterChanged( int index );

protected:
    /**
     * Constructor: Creates a changes dialog with text 'message' on
     * top, a list packages with an "auto" status that is not set via selections
     * and one ( default ) or two buttons.
     *
     * Not meant for public use. Applications should use the static
     * 'showChangesDialog' method instead.
     *
     * This constructor does not call filter() yet - this is the caller's
     * responsibility.
     **/
    YQPkgChangesDialog( QWidget *		parent,
			const QString & 	message,
			const QString &		acceptButtonLabel,
			const QString &		rejectButtonLabel = QString() );

    /**
     * Apply the filter criteria: Fill the pkg list with pkgs that have a
     * "modify" status ( install, update, delete ) set by automatic ( i.e. via the
     * dependency solver ), by application ( i.e. via software selections ) or
     * manually by the user.
     **/
    void filter( Filters f = FilterAutomatic );

    /**
     * Apply the filter criteria: Fill the pkg list with pkgs that have a
     * "modify" status ( install, update, delete ) set by automatic ( i.e. via the
     * dependency solver ), by application ( i.e. via software selections ) or
     * manually by the user and whose name matches 'regexp'.
     **/
    void filter( const QRegExp & regexp, Filters f = FilterAutomatic );

    /** 
     * extra filter for child classes 
     */
    virtual bool extraFilter( ZyppSel sel, ZyppPkg pkg );

    /**
     * Returns 'true' if the pkg list is empty.
     * This is only meaningful after calling 'filter()' !
     **/
    bool isEmpty() const;


    // Data members
    QComboBox *_filter;
    YQPkgList *		_pkgList;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(YQPkgChangesDialog::Filters);
Q_DECLARE_METATYPE(YQPkgChangesDialog::Filters);


class YQPkgUnsupportedPackagesDialog : public YQPkgChangesDialog
{
public:
    /**
     * Constructor: Creates a changes dialog with text 'message' on
     * top, a list packages with an "auto" status that is not set via selections
     * and one ( default ) or two buttons.
     *
     * Not meant for public use. Applications should use the static
     * 'showChangesDialog' method instead.
     *
     * This constructor does not call filter() yet - this is the caller's
     * responsibility.
     **/
    YQPkgUnsupportedPackagesDialog( QWidget * parent,
                                    const QString &message,
                                    const QString &acceptButtonLabel,
                                    const QString &rejectButtonLabel = QString() );

  /**
   * Static convenience method: Post a changes dialog with text
   * 'message', a list of changed packages and one ( default ) or two buttons.
   *
   * Returns 'true' if the user accepted ( i.e. clicked the 'accept' button )
   * and 'false' if the user rejected ( i.e. clicked the 'reject' button or
   * the window manager close button ).
   *
   * If the list is empty ( i.e., there are no packages with an "auto"
   * status ), the dialog is not shown at all ( and returns 'true' ) - unless
   *'showIfListEmpty' is 'true'.
   **/
  static bool showUnsupportedPackagesDialog( QWidget *		parent,
                                             const QString & 	message,
                                             const QString &	acceptButtonLabel,
                                             const QString &	rejectButtonLabel = QString(),
                                             Filters f = FilterAutomatic,
                                             Options o = OptionAutoAcceptIfEmpty );
protected:
  /** 
   * leave supported packages out.
   */
  virtual bool extraFilter( ZyppSel sel, ZyppPkg pkg );

};


#endif // ifndef YQPkgChangesDialog_h
