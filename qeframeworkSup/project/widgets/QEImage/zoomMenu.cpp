/*  zoomMenu.cpp
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
 This class manages the zoom menu for the QEImage widget
 */

#include "zoomMenu.h"

zoomMenu::zoomMenu( QWidget *parent) : QMenu(parent)
{
    areaSelectedAction = new QAction( "Selected area (Region 1)", this );
    areaSelectedAction->setData( imageContextMenu::ICM_ZOOM_SELECTED );
    addAction( areaSelectedAction );

    QAction* a;
    a = new QAction( "Fit",  this ); a->setData( imageContextMenu::ICM_ZOOM_FIT   ); addAction( a );
    a = new QAction( "+",    this ); a->setData( imageContextMenu::ICM_ZOOM_PLUS  ); addAction( a );
    a = new QAction( "-",    this ); a->setData( imageContextMenu::ICM_ZOOM_MINUS ); addAction( a );
    a = new QAction( "10%",  this ); a->setData( imageContextMenu::ICM_ZOOM_10    ); addAction( a );
    a = new QAction( "25%",  this ); a->setData( imageContextMenu::ICM_ZOOM_25    ); addAction( a );
    a = new QAction( "50%",  this ); a->setData( imageContextMenu::ICM_ZOOM_50    ); addAction( a );
    a = new QAction( "75%",  this ); a->setData( imageContextMenu::ICM_ZOOM_75    ); addAction( a );
    a = new QAction( "100%", this ); a->setData( imageContextMenu::ICM_ZOOM_100   ); addAction( a );
    a = new QAction( "150%", this ); a->setData( imageContextMenu::ICM_ZOOM_150   ); addAction( a );
    a = new QAction( "200%", this ); a->setData( imageContextMenu::ICM_ZOOM_200   ); addAction( a );
    a = new QAction( "300%", this ); a->setData( imageContextMenu::ICM_ZOOM_300   ); addAction( a );
    a = new QAction( "400%", this ); a->setData( imageContextMenu::ICM_ZOOM_400   ); addAction( a );

    setTitle( "Local Zoom" );
}

// Get a zoom factor from the user.
// Used when this menu is used independantly of the main context menu
imageContextMenu::imageContextMenuOptions zoomMenu::getZoom( const QPoint& pos )
{
    QAction* selectedItem = exec( pos );
    if( selectedItem )
    {
        return (imageContextMenu::imageContextMenuOptions)(selectedItem->data().toInt());
    }
    else
    {
        return imageContextMenu::ICM_NONE;
    }
}

void zoomMenu::enableAreaSelected( bool enable )
{
    areaSelectedAction->setEnabled( enable );
}

// end
