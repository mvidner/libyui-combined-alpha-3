/*
  Copyright (C) 2020 SUSE LLC

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) version 3.0 of the License. This library
  is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
  License for more details. You should have received a copy of the GNU
  Lesser General Public License along with this library; if not, write
  to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
  Floor, Boston, MA 02110-1301 USA
*/

#include "YNCWidgetActionHandler.h"

void YNCWidgetActionHandler::activate_widget( YCheckBoxFrame * widget )
{
    activate_nc_widget( widget );
}

void YNCWidgetActionHandler::activate_widget( YComboBox * widget )
{
    activate_nc_widget( widget );
}

void YNCWidgetActionHandler::activate_widget( YDateField * widget )
{
    activate_nc_widget( widget );
}

void YNCWidgetActionHandler::activate_widget( YSelectionBox * widget )
{
    activate_nc_widget( widget );
}

void YNCWidgetActionHandler::activate_widget( YTimeField * widget )
{
    activate_nc_widget( widget );
}

void YNCWidgetActionHandler::activate_widget ( YMultiSelectionBox * widget, YItem * item )
{
    activate_nc_widget( widget, item );
}
