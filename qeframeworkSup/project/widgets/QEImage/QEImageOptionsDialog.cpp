/*  QEImageOptionsDialog.cpp
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

#include "QEImageOptionsDialog.h"
#include "ui_QEImageOptionsDialog.h"
#include <QDebug>
#include <QECommon.h>
#include "imageMarkup.h"
#include "videowidget.h"

#define DEBUG qDebug () << "QEImageOptionsDialog" << __LINE__ << __FUNCTION__  << "  "

// Maps the local dialog colour index value to the corresponding image markup id.
//
static const imageMarkup::markupIds
s_indexToMarkupIdMap [QEImageOptionsDialog::NumberOfColorSelections] =
{
   imageMarkup::MARKUP_ID_V1_SLICE,
   imageMarkup::MARKUP_ID_V2_SLICE,
   imageMarkup::MARKUP_ID_V3_SLICE,
   imageMarkup::MARKUP_ID_V4_SLICE,
   imageMarkup::MARKUP_ID_V5_SLICE,

   imageMarkup::MARKUP_ID_H1_SLICE,
   imageMarkup::MARKUP_ID_H2_SLICE,
   imageMarkup::MARKUP_ID_H3_SLICE,
   imageMarkup::MARKUP_ID_H4_SLICE,
   imageMarkup::MARKUP_ID_H5_SLICE,

   imageMarkup::MARKUP_ID_LINE,
   imageMarkup::MARKUP_ID_REGION1,
   imageMarkup::MARKUP_ID_REGION2,
   imageMarkup::MARKUP_ID_REGION3,
   imageMarkup::MARKUP_ID_REGION4,
   imageMarkup::MARKUP_ID_TARGET,
   imageMarkup::MARKUP_ID_BEAM,
   imageMarkup::MARKUP_ID_ELLIPSE
};

//------------------------------------------------------------------------------
//
QEImageOptionsDialog::QEImageOptionsDialog (QWidget *parent) :
   QEDialog (parent),
   ui (new Ui::QEImageOptionsDialog)
{
   ui->setupUi (this);
   this->m_colourDialog = new QColorDialog (this);
   this->m_videoWidget = NULL;

   // Create an array of line frame widgets.
   //
   this->m_lineFrames[0] = this->ui->lineFrame_00;
   this->m_lineFrames[1] = this->ui->lineFrame_01;
   this->m_lineFrames[2] = this->ui->lineFrame_02;
   this->m_lineFrames[3] = this->ui->lineFrame_03;
   this->m_lineFrames[4] = this->ui->lineFrame_04;
   this->m_lineFrames[5] = this->ui->lineFrame_05;
   this->m_lineFrames[6] = this->ui->lineFrame_06;
   this->m_lineFrames[7] = this->ui->lineFrame_07;
   this->m_lineFrames[8] = this->ui->lineFrame_08;
   this->m_lineFrames[9] = this->ui->lineFrame_09;
   this->m_lineFrames[10] = this->ui->lineFrame_10;
   this->m_lineFrames[11] = this->ui->lineFrame_11;
   this->m_lineFrames[12] = this->ui->lineFrame_12;
   this->m_lineFrames[13] = this->ui->lineFrame_13;
   this->m_lineFrames[14] = this->ui->lineFrame_14;
   this->m_lineFrames[15] = this->ui->lineFrame_15;
   this->m_lineFrames[16] = this->ui->lineFrame_16;
   this->m_lineFrames[17] = this->ui->lineFrame_17;
}

//------------------------------------------------------------------------------
// Initialise options.
// Set default values (emits optionChange() signal for each option).
// Note, can't be done during construction as the signals will not be connected yet.
//
void QEImageOptionsDialog::initialise (VideoWidget* videoWidgetIn)
{
   this->m_videoWidget = videoWidgetIn;

   // Initial default settings
   optionSet( imageContextMenu::ICM_ENABLE_TIME,                 false );
   optionSet( imageContextMenu::ICM_ENABLE_FALSE_COLOUR,         false );
   optionSet( imageContextMenu::ICM_ENABLE_PROFILE_AXES,         false );
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

   if (!this->m_videoWidget) {
      DEBUG << "nullptr videoWidget reference";
      return;
   }

   // Extract the current colours in use from the videoWidget
   //
   for (int j = 0; j < ARRAY_LENGTH (s_indexToMarkupIdMap); j++) {
      const imageMarkup::markupIds id = s_indexToMarkupIdMap[j];
      this->m_colours[j] = this->m_videoWidget->getMarkupColor (id);
      this->setLineColourStyle (j);
   }
}

//------------------------------------------------------------------------------
//
QEImageOptionsDialog::~QEImageOptionsDialog()
{
   delete ui;
}

//------------------------------------------------------------------------------
// Set an option in the dialog.
// Used when setting related properties.
//
void QEImageOptionsDialog::optionSet( const imageContextMenu::imageContextMenuOptions option,
                                      const bool checked )
{
   switch( option ) {
      case imageContextMenu::ICM_ENABLE_TIME:                 ui->checkBoxTime              ->setChecked( checked ); break;
      case imageContextMenu::ICM_ENABLE_FALSE_COLOUR:         ui->checkBoxFalseColour       ->setChecked( checked ); break;
      case imageContextMenu::ICM_ENABLE_PROFILE_AXES:         ui->checkBoxProfileAxes       ->setChecked( checked ); break;
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

//------------------------------------------------------------------------------
// Get a current setting in the dialog.
// Used when getting related properties.
//
bool QEImageOptionsDialog::optionGet( const imageContextMenu::imageContextMenuOptions option ) const
{
   switch( option ) {
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

//------------------------------------------------------------------------------
//
void QEImageOptionsDialog::setMarkupColour (const int index)
{
   if (index < 0 || index >= NumberOfColorSelections) {
      DEBUG << "unexpected index value:" << index;
      return;
   }

   if (!this->m_videoWidget) {
      DEBUG << "nullptr videoWidget reference";
      return;
   }

   const imageMarkup::markupIds id = s_indexToMarkupIdMap [index];
   const QColor colour = this->m_colours [index];
   this->m_videoWidget->setMarkupColor (id, colour);
}

//------------------------------------------------------------------------------
//
void QEImageOptionsDialog::setLineColourStyle (const int index)
{
   if (index < 0 || index >= NumberOfColorSelections) {
      DEBUG << "unexpected index value:" << index;
      return;
   }

   const QColor colour = this->m_colours [index];
   const QString style = QEUtilities::colourToStyle (colour);
   this->m_lineFrames [index] ->setStyleSheet (style);
}

//------------------------------------------------------------------------------
//
void QEImageOptionsDialog::colourSelectionClicked (const int index)
{
   if (index < 0 || index >= NumberOfColorSelections) {
      DEBUG << "unexpected index value:" << index;
      return;
   }

   this->m_colourDialog->setCurrentColor (this->m_colours [index]);
   int status = this->m_colourDialog->exec();
   if (status == 1) {      
      this->m_colours [index] = this->m_colourDialog->currentColor ();
      this->setMarkupColour (index);
      this->setLineColourStyle (index);
   }
}

//------------------------------------------------------------------------------
// Slots for acting on configuration check boxes
//
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
void QEImageOptionsDialog::on_checkBoxProfileAxes_clicked       (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_PROFILE_AXES,              checked ); }
void QEImageOptionsDialog::on_checkBoxInfo_clicked              (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_CURSOR_PIXEL,              checked ); }
void QEImageOptionsDialog::on_checkBoxArea1Selection_clicked    (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_AREA1,                     checked ); }
void QEImageOptionsDialog::on_checkBoxArea2Selection_clicked    (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_AREA2,                     checked ); }
void QEImageOptionsDialog::on_checkBoxArea3Selection_clicked    (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_AREA3,                     checked ); }
void QEImageOptionsDialog::on_checkBoxArea4Selection_clicked    (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_AREA4,                     checked ); }
void QEImageOptionsDialog::on_checkBoxTarget_clicked            (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_TARGET,                    checked ); }
void QEImageOptionsDialog::on_checkBoxBeam_clicked              (bool checked) { emit optionChange( imageContextMenu::ICM_ENABLE_BEAM,                      checked ); }

void QEImageOptionsDialog::on_selectionButton_00_clicked (bool) { this->colourSelectionClicked (0); }
void QEImageOptionsDialog::on_selectionButton_01_clicked (bool) { this->colourSelectionClicked (1); }
void QEImageOptionsDialog::on_selectionButton_02_clicked (bool) { this->colourSelectionClicked (2); }
void QEImageOptionsDialog::on_selectionButton_03_clicked (bool) { this->colourSelectionClicked (3); }
void QEImageOptionsDialog::on_selectionButton_04_clicked (bool) { this->colourSelectionClicked (4); }
void QEImageOptionsDialog::on_selectionButton_05_clicked (bool) { this->colourSelectionClicked (5); }
void QEImageOptionsDialog::on_selectionButton_06_clicked (bool) { this->colourSelectionClicked (6); }
void QEImageOptionsDialog::on_selectionButton_07_clicked (bool) { this->colourSelectionClicked (7); }
void QEImageOptionsDialog::on_selectionButton_08_clicked (bool) { this->colourSelectionClicked (8); }
void QEImageOptionsDialog::on_selectionButton_09_clicked (bool) { this->colourSelectionClicked (9); }
void QEImageOptionsDialog::on_selectionButton_10_clicked (bool) { this->colourSelectionClicked (10); }
void QEImageOptionsDialog::on_selectionButton_11_clicked (bool) { this->colourSelectionClicked (11); }
void QEImageOptionsDialog::on_selectionButton_12_clicked (bool) { this->colourSelectionClicked (12); }
void QEImageOptionsDialog::on_selectionButton_13_clicked (bool) { this->colourSelectionClicked (13); }
void QEImageOptionsDialog::on_selectionButton_14_clicked (bool) { this->colourSelectionClicked (14); }
void QEImageOptionsDialog::on_selectionButton_15_clicked (bool) { this->colourSelectionClicked (15); }
void QEImageOptionsDialog::on_selectionButton_16_clicked (bool) { this->colourSelectionClicked (16); }
void QEImageOptionsDialog::on_selectionButton_17_clicked (bool) { this->colourSelectionClicked (17); }

// end
