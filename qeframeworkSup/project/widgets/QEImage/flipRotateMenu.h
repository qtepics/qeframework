/*  flipRotateMenu.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2015-2024 Australian Synchrotron
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

#ifndef QE_FLIP_ROTATE_MENU_H
#define QE_FLIP_ROTATE_MENU_H

#include <QMenu>
#include "imageContextMenu.h"

class flipRotateMenu : public QMenu
{
    Q_OBJECT
public:

    explicit flipRotateMenu( QWidget *parent = 0 );

    imageContextMenu::imageContextMenuOptions getFlipRotate( const QPoint& pos );

    // Set the initial state of the menu to reflect the current state of the image.
    //
    void setChecked( const int rotation, const bool flipH, const bool flipV );

signals:

public slots:

private:
    QAction* rotationNoneAction;
    QAction* rotation90RAction;
    QAction* rotation90LAction;
    QAction* rotation180Action;
    QAction* flipHAction;
    QAction* flipVAction;
};

#endif // QE_FLIP_ROTATE_MENU_H
