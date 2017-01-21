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
 *  Copyright (c) 2014 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 * This class is used to present the user with full screen options
 * After creating and executing the dialog, getScreenNum() can be called which will return
 * a number from zero up if a particular screen was selected, or -1 if all screens
 */

#ifndef SCREENSELECTDIALOG_H
#define SCREENSELECTDIALOG_H

#include <QDialog>
#include <QRect>

namespace Ui {
    class screenSelectDialog;
}

class screenSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit screenSelectDialog( int numScreens, QWidget *parent = 0);
    ~screenSelectDialog();

    static bool getFullscreenGeometry( QWidget* target, QRect& geom  ); // Get the geometry of the selected screen or screens

    int getScreenNum();     // Screen number selected by user (negative if one of the enum options)
    enum screens{ PRIMARY_SCREEN = -3, THIS_SCREEN = -2, ALL_SCREENS = -1 }; // Descriptive options (non negative numbers are screen IDs)

private:
    Ui::screenSelectDialog *ui;
};

#endif // SCREENSELECTDIALOG_H
