/*
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The EPICS QT Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2012 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 This class manages the context menu for the QEImage widget
 It extends the standard QEFramework context menu
 Functions are available to add menu options, and sub menus.
 A function is available to present the menu to the user and return a user selection
 */

#include "imageContextMenu.h"

// Construction
imageContextMenu::imageContextMenu( QWidget *parent) : QMenu(parent)
{
    setTitle( "Image options" );
}

// Present the context menu, and return whatever the user selects
void imageContextMenu::getContextMenuOption( const QPoint& pos, imageContextMenuOptions* option, bool* checked )
{
    QAction* selectedItem = exec( pos );
    if( selectedItem )
    {
        *option = (imageContextMenuOptions)(selectedItem->data().toInt());
        *checked = selectedItem->isChecked();
    }
    else
    {
        *option = ICM_NONE;
        *checked = false;
    }
}

// Add an item to the context menu
void imageContextMenu::addMenuItem( const QString& title, const bool checkable, const bool checked, const imageContextMenuOptions option )
{
    addMenuItemGeneric( this, title, checkable, checked, option);
}

// Add a menu item to the either the context menu, or one of its sub menus
void imageContextMenu::addMenuItemGeneric( QMenu* menu, const QString& title, const bool checkable, const bool checked, const imageContextMenuOptions option )
{
    QAction* a = new QAction( title, this );
    a->setCheckable( checkable );
    if( checkable )
    {
        a->setChecked( checked );
    }
    a->setData( option );
    menu->addAction( a );
}
