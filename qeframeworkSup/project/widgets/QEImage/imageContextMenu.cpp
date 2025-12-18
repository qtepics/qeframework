/*  imageContextMenu.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
