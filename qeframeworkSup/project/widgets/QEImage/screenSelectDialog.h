/*  screenSelectDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
