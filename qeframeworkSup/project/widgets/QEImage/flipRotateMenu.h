/*  flipRotateMenu.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2015-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
