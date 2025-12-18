/*  selectMenu.h
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
 This class manages the selection mode menu for the QEImage widget
 */

#ifndef SELECTMENU_H
#define SELECTMENU_H

#include <QMenu>
#include <imageContextMenu.h>

class selectMenu : public QMenu
{
    Q_OBJECT
public:

    explicit selectMenu( QWidget *parent = 0 );

    imageContextMenu::imageContextMenuOptions getSelectOption( const QPoint& pos );

    void enable( imageContextMenu::imageContextMenuOptions option, bool state );
    bool isEnabled( imageContextMenu::imageContextMenuOptions option );
    void setChecked( const int mode );
    void setItemText( imageContextMenu::imageContextMenuOptions option, QString title );


signals:

public slots:

private:
    QAction* getAction( imageContextMenu::imageContextMenuOptions option );

    QAction* actionPan;
    QAction* actionHSlice1;
    QAction* actionHSlice2;
    QAction* actionHSlice3;
    QAction* actionHSlice4;
    QAction* actionHSlice5;
    QAction* actionVSlice1;
    QAction* actionVSlice2;
    QAction* actionVSlice3;
    QAction* actionVSlice4;
    QAction* actionVSlice5;
    QAction* actionArea1;
    QAction* actionArea2;
    QAction* actionArea3;
    QAction* actionArea4;
    QAction* actionProfile;
    QAction* actionTarget;
    QAction* actionBeam;
};

#endif // SELECTMENU_H
