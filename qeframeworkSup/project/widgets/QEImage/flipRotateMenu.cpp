/*  flipRotateMenu.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2021 Australian Synchrotron
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
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 This class manages the flip/rotate menu for the QEImage widget
 Functions are available to set the initial state of checkable actions.
 */

#include "flipRotateMenu.h"
#include "QEImage.h"

flipRotateMenu::flipRotateMenu( QWidget *parent) : QMenu(parent)
{
    // Macro to create the buttons
#define NEW_FLIP_ROTATE_MENU_BUTTON( TITLE, ID, ACTION ) \
    ACTION = new QAction( TITLE, this );                 \
    ACTION->setData( imageContextMenu::ID );                  \
    ACTION->setCheckable( true );                        \
    addAction( ACTION );

    // Create the buttons
    NEW_FLIP_ROTATE_MENU_BUTTON( "Local Rotate 90 Clockwise",     ICM_ROTATE_RIGHT,    rotation90RAction )
    NEW_FLIP_ROTATE_MENU_BUTTON( "Local Rotate 90 Anticlockwise", ICM_ROTATE_LEFT,     rotation90LAction )
    NEW_FLIP_ROTATE_MENU_BUTTON( "Local Rotate 180",              ICM_ROTATE_180,      rotation180Action )
    NEW_FLIP_ROTATE_MENU_BUTTON( "Local Flip Horizontal",         ICM_FLIP_HORIZONTAL, flipHAction )
    NEW_FLIP_ROTATE_MENU_BUTTON( "Local Flip Vertical",           ICM_FLIP_VERTICAL,   flipVAction )

    // Set the title
    setTitle( "Local Flip / Rotate" );
}

// Get a flip/rotate option from the user.
// Used when this menu is used independantly of the main context menu
imageContextMenu::imageContextMenuOptions flipRotateMenu::getFlipRotate( const QPoint& pos )
{
    QAction* selectedItem = exec( pos );
    if( selectedItem )
    {
        // !!! Doesn't return fliped state
        qDebug() << (imageContextMenu::imageContextMenuOptions)(selectedItem->data().toInt());
        return (imageContextMenu::imageContextMenuOptions)(selectedItem->data().toInt());
    }
    else
    {
        return imageContextMenu::ICM_NONE;
    }
}

// Set the initial state of the menu to reflect the current state of the image
void flipRotateMenu::setChecked( const int rotation, const bool flipH, const bool flipV )
{
    // Check the appropriate rotation option
    rotation90RAction ->setChecked( rotation == imageProperties::ROTATION_90_RIGHT );
    rotation90LAction ->setChecked( rotation == imageProperties::ROTATION_90_LEFT );
    rotation180Action ->setChecked( rotation == imageProperties::ROTATION_180 );

    // Check the appropriate flip options
    flipHAction->setChecked( flipH );
    flipVAction->setChecked( flipV );
}

// end
