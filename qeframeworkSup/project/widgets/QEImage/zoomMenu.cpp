/*  zoomMenu.cpp
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
 This class manages the zoom menu for the QEImage widget
 */

#include "zoomMenu.h"

zoomMenu::zoomMenu( QWidget *parent) : QMenu(parent)
{
    areaSelectedAction = new QAction( "Selected area (Region 1)", this );
    areaSelectedAction->setData( imageContextMenu::ICM_ZOOM_SELECTED );
    addAction( areaSelectedAction );

    QAction* a;
    a = new QAction(tr("Fit"),this); a->setData( imageContextMenu::ICM_ZOOM_FIT   ); addAction( a );
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

    setTitle( tr("Local Zoom") );
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
