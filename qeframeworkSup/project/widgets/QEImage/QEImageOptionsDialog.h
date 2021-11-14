/*  QEImageOptionsDialog.h
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

#ifndef QE_IMAGE_OPTIONS_DIALOG_H
#define QE_IMAGE_OPTIONS_DIALOG_H

#include <QEDialog.h>
#include <imageContextMenu.h>


namespace Ui {
    class QEImageOptionsDialog;
}

class QEImageOptionsDialog : public QEDialog
{
    Q_OBJECT

public:
    explicit QEImageOptionsDialog(QWidget *parent = 0);
    ~QEImageOptionsDialog();

    void initialise();                                                                // Set default values (emits optionChange() signal for each option)
    void optionSet( imageContextMenu::imageContextMenuOptions option, bool checked ); // Set an option in the dialog. Used when setting related properties
    bool optionGet( imageContextMenu::imageContextMenuOptions option );               // Get a current setting in the dialog. Used when getting related properties.

private:
    Ui::QEImageOptionsDialog *ui;

private slots:
    void on_checkBoxArea4Selection_clicked(bool checked);
    void on_checkBoxArea3Selection_clicked(bool checked);
    void on_checkBoxArea2Selection_clicked(bool checked);
    void on_checkBoxArea1Selection_clicked(bool checked);
    void on_checkBoxTarget_clicked(bool checked);
    void on_checkBoxBeam_clicked(bool checked);
    void on_checkBoxInfo_clicked(bool checked);
    void on_checkBoxTime_clicked(bool checked);
    void on_checkBoxFalseColour_clicked(bool checked);
    void on_checkBoxBrightnessContrast_clicked(bool checked);
    void on_checkBoxRecorder_clicked(bool checked);
    void on_checkBoxButtonBar_clicked(bool checked);
    void on_checkBoxArbitraryProfile_clicked(bool checked);
    void on_checkBoxHorizontalProfile1_clicked(bool checked);
    void on_checkBoxHorizontalProfile2_clicked(bool checked);
    void on_checkBoxHorizontalProfile3_clicked(bool checked);
    void on_checkBoxHorizontalProfile4_clicked(bool checked);
    void on_checkBoxHorizontalProfile5_clicked(bool checked);
    void on_checkBoxVerticalProfile1_clicked(bool checked);
    void on_checkBoxVerticalProfile2_clicked(bool checked);
    void on_checkBoxVerticalProfile3_clicked(bool checked);
    void on_checkBoxVerticalProfile4_clicked(bool checked);
    void on_checkBoxVerticalProfile5_clicked(bool checked);

signals:
    void optionChange( imageContextMenu::imageContextMenuOptions option, bool checked );    // Emitted when an option changed
};

#endif // QE_IMAGE_OPTIONS_DIALOG_H
