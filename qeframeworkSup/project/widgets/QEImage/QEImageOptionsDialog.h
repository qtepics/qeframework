/*  QEImageOptionsDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2026 Australian Synchrotron
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

#include <QColor>
#include <QColorDialog>
#include <QFrame>
#include <QEDialog.h>
#include <imageContextMenu.h>

namespace Ui {
   class QEImageOptionsDialog;
}

class VideoWidget;   // differed

class QEImageOptionsDialog : public QEDialog
{
   Q_OBJECT

public:
   enum Constansts {
      NumberOfColorSelections = 18
   };

   explicit QEImageOptionsDialog (QWidget *parent = 0);
   ~QEImageOptionsDialog();

   void initialise (VideoWidget* videoWidget);                                                   // Set default values (emits optionChange() signal for each option)
   void optionSet (const imageContextMenu::imageContextMenuOptions option, const bool checked); // Set an option in the dialog. Used when setting related properties
   bool optionGet (const imageContextMenu::imageContextMenuOptions option) const;               // Get a current setting in the dialog. Used when getting related properties.

private:
   Ui::QEImageOptionsDialog *ui;
   QColorDialog* m_colourDialog;
   VideoWidget* m_videoWidget;

   QFrame* m_lineFrames[NumberOfColorSelections];  // the line frames showing the currebt color selections.
   QColor m_colours[NumberOfColorSelections];

   void setMarkupColour (const int index);
   void setLineColourStyle (const int index);
   void colourSelectionClicked (const int index);

private slots:
   // These slot functio names must match the wigdget/signal names.
   //
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

   void on_selectionButton_00_clicked(bool checked);
   void on_selectionButton_01_clicked(bool checked);
   void on_selectionButton_02_clicked(bool checked);
   void on_selectionButton_03_clicked(bool checked);
   void on_selectionButton_04_clicked(bool checked);
   void on_selectionButton_05_clicked(bool checked);
   void on_selectionButton_06_clicked(bool checked);
   void on_selectionButton_07_clicked(bool checked);
   void on_selectionButton_08_clicked(bool checked);
   void on_selectionButton_09_clicked(bool checked);
   void on_selectionButton_10_clicked(bool checked);
   void on_selectionButton_11_clicked(bool checked);
   void on_selectionButton_12_clicked(bool checked);
   void on_selectionButton_13_clicked(bool checked);
   void on_selectionButton_14_clicked(bool checked);
   void on_selectionButton_15_clicked(bool checked);
   void on_selectionButton_16_clicked(bool checked);
   void on_selectionButton_17_clicked(bool checked);

signals:
   void optionChange( imageContextMenu::imageContextMenuOptions option, bool checked );    // Emitted when an option changed
};

#endif // QE_IMAGE_OPTIONS_DIALOG_H
