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
