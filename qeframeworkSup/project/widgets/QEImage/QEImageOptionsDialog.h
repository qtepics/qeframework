/*  QEImageOptionsDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
    void on_checkBoxProfileAxes_clicked(bool checked);
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
