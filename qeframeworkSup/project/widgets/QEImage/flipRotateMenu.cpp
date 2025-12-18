/*  flipRotateMenu.cpp
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
 This class manages the flip/rotate menu for the QEImage widget
 Functions are available to set the initial state of checkable actions.
 */

#include "flipRotateMenu.h"
#include "QEImage.h"
#include <QEEnums.h>

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
    rotation90RAction->setChecked( rotation == QE::Rotate90Right );
    rotation90LAction->setChecked( rotation == QE::Rotate90Left );
    rotation180Action->setChecked( rotation == QE::Rotate180 );

    // Check the appropriate flip options
    flipHAction->setChecked( flipH );
    flipVAction->setChecked( flipV );
}

// end
