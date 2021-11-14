/*  QEImageOptionsDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2021 Australian Synchrotron
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
 This class manages option selection for the QEImage widget
 */

#include "QEImageOptionsDialog.h"
#include "ui_QEImageOptionsDialog.h"
#include <QDebug>

#define DEBUG qDebug () << "QEImageOptionsDialog" << __LINE__ << __FUNCTION__  << "  "

QEImageOptionsDialog::QEImageOptionsDialog(QWidget *parent) :
    QEDialog(parent),
    ui(new Ui::QEImageOptionsDialog)
{
    ui->setupUi(this);
}

// Initialise options.
// Set default values (emits optionChange() signal for each option).
// Note, can't be done during construction as the signals will not be connected yet.
void QEImageOptionsDialog::initialise()
{
    // Initial default settings
    optionSet( imageContextMenu::ICM_ENABLE_TIME,                 false );
    optionSet( imageContextMenu::ICM_ENABLE_FALSE_COLOUR,         false );
    optionSet( imageContextMenu::ICM_ENABLE_CURSOR_PIXEL,         false );
    optionSet( imageContextMenu::ICM_ENABLE_HOZ1,                 false );
    optionSet( imageContextMenu::ICM_ENABLE_HOZ2,                 false );
    optionSet( imageContextMenu::ICM_ENABLE_HOZ3,                 false );
    optionSet( imageContextMenu::ICM_ENABLE_HOZ4,                 false );
    optionSet( imageContextMenu::ICM_ENABLE_HOZ5,                 false );
    optionSet( imageContextMenu::ICM_ENABLE_VERT1,                false );
    optionSet( imageContextMenu::ICM_ENABLE_VERT2,                false );
    optionSet( imageContextMenu::ICM_ENABLE_VERT3,                false );
    optionSet( imageContextMenu::ICM_ENABLE_VERT4,                false );
    optionSet( imageContextMenu::ICM_ENABLE_VERT5,                false );
    optionSet( imageContextMenu::ICM_ENABLE_LINE,                 false );
    optionSet( imageContextMenu::ICM_ENABLE_AREA1,                 true );
    optionSet( imageContextMenu::ICM_ENABLE_AREA2,                 true );
    optionSet( imageContextMenu::ICM_ENABLE_AREA3,                 true );
    optionSet( imageContextMenu::ICM_ENABLE_AREA4,                 true );
    optionSet( imageContextMenu::ICM_ENABLE_TARGET,               false );
    optionSet( imageContextMenu::ICM_ENABLE_BEAM,                 false );
    optionSet( imageContextMenu::ICM_DISPLAY_BUTTON_BAR,          false );
    optionSet( imageContextMenu::ICM_DISPLAY_IMAGE_DISPLAY_PROPERTIES, false );
    optionSet( imageContextMenu::ICM_DISPLAY_RECORDER,            false );
}

QEImageOptionsDialog::~QEImageOptionsDialog()
{
    delete ui;
}

// Set an option in the dialog.
// Used when setting related properties
void QEImageOptionsDialog::optionSet( imageContextMenu::imageContextMenuOptions option, bool checked )
{
    switch( option )
    {
        case imageContextMenu::ICM_ENABLE_TIME:                 ui->checkBoxTime              ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_FALSE_COLOUR:         ui->checkBoxFalseColour       ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_CURSOR_PIXEL:         ui->checkBoxInfo              ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_HOZ1:                 ui->checkBoxHorizontalProfile1->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_HOZ2:                 ui->checkBoxHorizontalProfile2->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_HOZ3:                 ui->checkBoxHorizontalProfile3->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_HOZ4:                 ui->checkBoxHorizontalProfile4->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_HOZ5:                 ui->checkBoxHorizontalProfile5->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_VERT1:                ui->checkBoxVerticalProfile1  ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_VERT2:                ui->checkBoxVerticalProfile2  ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_VERT3:                ui->checkBoxVerticalProfile3  ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_VERT4:                ui->checkBoxVerticalProfile4  ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_VERT5:                ui->checkBoxVerticalProfile5  ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_LINE:                 ui->checkBoxArbitraryProfile  ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_AREA1:                ui->checkBoxArea1Selection    ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_AREA2:                ui->checkBoxArea2Selection    ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_AREA3:                ui->checkBoxArea3Selection    ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_AREA4:                ui->checkBoxArea4Selection    ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_TARGET:               ui->checkBoxTarget            ->setChecked( checked ); break;
        case imageContextMenu::ICM_ENABLE_BEAM:                 ui->checkBoxBeam              ->setChecked( checked ); break;
        case imageContextMenu::ICM_DISPLAY_BUTTON_BAR:          ui->checkBoxButtonBar         ->setChecked( checked ); break;
        case imageContextMenu::ICM_DISPLAY_IMAGE_DISPLAY_PROPERTIES: ui->checkBoxBrightnessContrast->setChecked( checked ); break;
        case imageContextMenu::ICM_DISPLAY_RECORDER:            ui->checkBoxRecorder          ->setChecked( checked ); break;

            // This switch should cater for all check boxes in the dialog, but this is not all context menu options
            // so include a default to keep compiler from generating warnings
        default: break;
    }

    // Act on the option change.
    emit optionChange( option, checked );
}

// Get a current setting in the dialog.
// Used when getting related properties.
bool QEImageOptionsDialog::optionGet( imageContextMenu::imageContextMenuOptions option )
{
    switch( option )
    {
        case imageContextMenu::ICM_ENABLE_TIME:                 return ui->checkBoxTime              ->isChecked();
        case imageContextMenu::ICM_ENABLE_FALSE_COLOUR:         return ui->checkBoxFalseColour       ->isChecked();
        case imageContextMenu::ICM_ENABLE_CURSOR_PIXEL:         return ui->checkBoxInfo              ->isChecked();
        case imageContextMenu::ICM_ENABLE_HOZ1:                 return ui->checkBoxHorizontalProfile1->isChecked();
        case imageContextMenu::ICM_ENABLE_HOZ2:                 return ui->checkBoxHorizontalProfile2->isChecked();
        case imageContextMenu::ICM_ENABLE_HOZ3:                 return ui->checkBoxHorizontalProfile3->isChecked();
        case imageContextMenu::ICM_ENABLE_HOZ4:                 return ui->checkBoxHorizontalProfile4->isChecked();
        case imageContextMenu::ICM_ENABLE_HOZ5:                 return ui->checkBoxHorizontalProfile5->isChecked();
        case imageContextMenu::ICM_ENABLE_VERT1:                return ui->checkBoxVerticalProfile1  ->isChecked();
        case imageContextMenu::ICM_ENABLE_VERT2:                return ui->checkBoxVerticalProfile2  ->isChecked();
        case imageContextMenu::ICM_ENABLE_VERT3:                return ui->checkBoxVerticalProfile3  ->isChecked();
        case imageContextMenu::ICM_ENABLE_VERT4:                return ui->checkBoxVerticalProfile4  ->isChecked();
        case imageContextMenu::ICM_ENABLE_VERT5:                return ui->checkBoxVerticalProfile5  ->isChecked();
        case imageContextMenu::ICM_ENABLE_LINE:                 return ui->checkBoxArbitraryProfile  ->isChecked();
        case imageContextMenu::ICM_ENABLE_AREA1:                return ui->checkBoxArea1Selection    ->isChecked();
        case imageContextMenu::ICM_ENABLE_AREA2:                return ui->checkBoxArea2Selection    ->isChecked();
        case imageContextMenu::ICM_ENABLE_AREA3:                return ui->checkBoxArea3Selection    ->isChecked();
        case imageContextMenu::ICM_ENABLE_AREA4:                return ui->checkBoxArea4Selection    ->isChecked();
        case imageContextMenu::ICM_ENABLE_TARGET:               return ui->checkBoxTarget            ->isChecked();
        case imageContextMenu::ICM_ENABLE_BEAM:                 return ui->checkBoxBeam              ->isChecked();
        case imageContextMenu::ICM_DISPLAY_BUTTON_BAR:          return ui->checkBoxButtonBar         ->isChecked();
        case imageContextMenu::ICM_DISPLAY_IMAGE_DISPLAY_PROPERTIES: return ui->checkBoxBrightnessContrast->isChecked();
        case imageContextMenu::ICM_DISPLAY_RECORDER:            return ui->checkBoxRecorder->isChecked();

            // This switch should cater for all check boxes in the dialog, but this is not all context menu options
            // so include a default to keep compiler from generating warnings
        default: return 0;
    }
}

// Slots for acting on configuration check boxes
void QEImageOptionsDialog::on_checkBoxVerticalProfile1_clicked  (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_VERT1,                     checked ); }
void QEImageOptionsDialog::on_checkBoxVerticalProfile2_clicked  (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_VERT2,                     checked ); }
void QEImageOptionsDialog::on_checkBoxVerticalProfile3_clicked  (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_VERT3,                     checked ); }
void QEImageOptionsDialog::on_checkBoxVerticalProfile4_clicked  (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_VERT4,                     checked ); }
void QEImageOptionsDialog::on_checkBoxVerticalProfile5_clicked  (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_VERT5,                     checked ); }
void QEImageOptionsDialog::on_checkBoxHorizontalProfile1_clicked(bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_HOZ1,                      checked ); }
void QEImageOptionsDialog::on_checkBoxHorizontalProfile2_clicked(bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_HOZ2,                      checked ); }
void QEImageOptionsDialog::on_checkBoxHorizontalProfile3_clicked(bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_HOZ3,                      checked ); }
void QEImageOptionsDialog::on_checkBoxHorizontalProfile4_clicked(bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_HOZ4,                      checked ); }
void QEImageOptionsDialog::on_checkBoxHorizontalProfile5_clicked(bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_HOZ5,                      checked ); }
void QEImageOptionsDialog::on_checkBoxArbitraryProfile_clicked  (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_LINE,                      checked ); }
void QEImageOptionsDialog::on_checkBoxButtonBar_clicked         (bool checked) { emit optionChange( imageContextMenu::ICM_DISPLAY_BUTTON_BAR,               checked ); }
void QEImageOptionsDialog::on_checkBoxBrightnessContrast_clicked(bool checked) { emit optionChange( imageContextMenu::ICM_DISPLAY_IMAGE_DISPLAY_PROPERTIES, checked ); }
void QEImageOptionsDialog::on_checkBoxRecorder_clicked          (bool checked) { emit optionChange( imageContextMenu::ICM_DISPLAY_RECORDER,                 checked ); }
void QEImageOptionsDialog::on_checkBoxTime_clicked              (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_TIME,                      checked ); }
void QEImageOptionsDialog::on_checkBoxFalseColour_clicked       (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_FALSE_COLOUR,              checked ); }
void QEImageOptionsDialog::on_checkBoxInfo_clicked              (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_CURSOR_PIXEL,              checked ); }
void QEImageOptionsDialog::on_checkBoxArea1Selection_clicked    (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_AREA1,                     checked ); }
void QEImageOptionsDialog::on_checkBoxArea2Selection_clicked    (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_AREA2,                     checked ); }
void QEImageOptionsDialog::on_checkBoxArea3Selection_clicked    (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_AREA3,                     checked ); }
void QEImageOptionsDialog::on_checkBoxArea4Selection_clicked    (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_AREA4,                     checked ); }
void QEImageOptionsDialog::on_checkBoxTarget_clicked            (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_TARGET,                    checked ); }
void QEImageOptionsDialog::on_checkBoxBeam_clicked              (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_BEAM,                      checked ); }

// end
