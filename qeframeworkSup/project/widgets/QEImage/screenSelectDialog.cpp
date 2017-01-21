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

#include <QString>
#include <QDesktopWidget>
#include <QApplication>
#include <QRect>

#include "screenSelectDialog.h"
#include "ui_screenSelectDialog.h"

// Return geometry of user's screen selection
bool screenSelectDialog::getFullscreenGeometry( QWidget* target, QRect& geom )
{
    // Determine number of screens
    QDesktopWidget* desktop = QApplication::desktop();
    int numScreens = desktop->screenCount();

    // If more than one screen, ask the user where to go fullscreen
    if( numScreens > 1 )
    {
        // construct the dialog
        screenSelectDialog* screenSelect = new screenSelectDialog( numScreens, target );

        // Get user's choice of screen
        if( screenSelect->exec() == QDialog::Rejected )
        {
            return false;
        }

        // Determine the geometry of the user's choice
        int screenId = screenSelect->getScreenNum();
        switch( screenId )
        {
            case screenSelectDialog::PRIMARY_SCREEN:
                geom = desktop->screenGeometry( desktop->primaryScreen() );
                break;

            case screenSelectDialog::THIS_SCREEN:
                geom = desktop->screenGeometry( target );
                break;

            case screenSelectDialog::ALL_SCREENS:
                geom = desktop->geometry();
                break;

            default:
                geom = desktop->screenGeometry( screenId );
                break;
        }
    }

    // If only one screen, just get it's geometry
    else
    {
        geom = desktop->screenGeometry( desktop->primaryScreen() );
    }

    // All done
    return true;
}

// Screen selection dialog.
// May be called directly, but is intended to be used from static function getFullscreenGeometry()
screenSelectDialog::screenSelectDialog( int numScreens, QWidget *parent ) :
    QDialog(parent),
    ui(new Ui::screenSelectDialog)
{
    ui->setupUi(this);
    ui->comboBox->addItem( "Primary screen" );
    ui->comboBox->addItem( "This screen" );
    ui->comboBox->addItem( "All screens" );
    for( int i = 0; i < numScreens; i++ )
    {
        ui->comboBox->addItem( QString( "Screen %1" ).arg( i+1 ) );
    }
}

// destruction
screenSelectDialog::~screenSelectDialog()
{
    delete ui;
}

// Get the option selected
int screenSelectDialog::getScreenNum()
{
    return ui->comboBox->currentIndex() - 3;
}
