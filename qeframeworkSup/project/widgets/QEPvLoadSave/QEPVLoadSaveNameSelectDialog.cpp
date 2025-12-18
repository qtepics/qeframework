/*  QEPVLoadSaveNameSelectDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2021-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEPVLoadSaveNameSelectDialog.h"
#include <QDebug>
#include <QRegularExpression>
#include <QStringList>

#include <ui_QEPVLoadSaveNameSelectDialog.h>
#include <QEPvNameSearch.h>
#include <QEArchiveManager.h>
#include <QEPVNameSelectDialog.h>
#include <QEScaling.h>

#define DEBUG  qDebug () << "QEPVLoadSaveNameSelectDialog:" << __LINE__ << __FUNCTION__ << "  "


//------------------------------------------------------------------------------
//
QEPVLoadSaveNameSelectDialog::QEPVLoadSaveNameSelectDialog (QWidget *parent) :
   QEDialog (parent),
   ui (new Ui::QEPVLoadSaveNameSelectDialog)
{
   this->ui->setupUi (this);

   // Form array of combo box edits
   this->pvNameEdit [ptWrite] = this->ui->pvWriteNameEdit;
   this->pvNameEdit [ptRead]  = this->ui->pvReadNameEdit;
   this->pvNameEdit [ptArch]  = this->ui->pvArchNameEdit;

   // Ensure the dialog centres using this widget as reference point.
   //
   this->setSourceWidget (this->ui->pvWriteNameEdit);

   this->returnIsMasked = false;

   this->ui->help_frame->setVisible (false);
   this->setFixedHeight (this->ui->frame_1->minimumHeight() +
                         this->ui->frame_2a->minimumHeight() +
                         this->ui->frame_2b->minimumHeight() +
                         this->ui->frame_2c->minimumHeight() +
                         this->ui->frame_3->minimumHeight() );

   QObject::connect (this->ui->filterEdit,  SIGNAL  (returnPressed ()),
                     this,                  SLOT    (filterEditReturnPressed ()));

   QObject::connect (this->ui->filterEdit,  SIGNAL (editingFinished       ()),
                     this,                  SLOT   (filterEditingFinished ()));

   QObject::connect (this->ui->helpButton,  SIGNAL (clicked       (bool)),
                     this,                  SLOT   (helpClicked   (bool)));

   QObject::connect (this->ui->clearButton, SIGNAL (clicked       (bool)),
                     this,                  SLOT   (clearClicked  (bool)));

#if QT_VERSION < 0x060000
#ifndef QT_NO_COMPLETER
   // Could not get completer to work - yet.
   for (int j = 0; j < PT_NUMBER; j++) {
      this->pvNameEdit[j]->setAutoCompletion (true);
      this->pvNameEdit[j]->setAutoCompletionCaseSensitivity (Qt::CaseSensitive);
   }
#endif
#endif

}

//------------------------------------------------------------------------------
//
QEPVLoadSaveNameSelectDialog::~QEPVLoadSaveNameSelectDialog ()
{
   this->filteredNames.clear ();
   delete ui;
}

//------------------------------------------------------------------------------
//
void QEPVLoadSaveNameSelectDialog::setPvNames (const QString& setPointPvNameIn,
                                               const QString& readBackPvNameIn,
                                               const QString& archiverPvNameIn)
{
   this->originalPvName [ptWrite] = setPointPvNameIn.trimmed ();
   this->originalPvName [ptRead]  = readBackPvNameIn.trimmed ();
   this->originalPvName [ptArch]  = archiverPvNameIn.trimmed ();

   // Number may be zero - no special check required - just do it.
   //
   for (int j = 0; j < PT_NUMBER; j++) {
      this->pvNameEdit [j]->clear();
      this->pvNameEdit [j]->insertItems (0, this->filteredNames);

      if (!this->originalPvName[j].isEmpty ()) {
         this->pvNameEdit[j]->insertItem (0, this->originalPvName[j], QVariant ());
      }
      this->pvNameEdit [j]->setCurrentIndex (0);
   }

   // setPvName typically invoked just before exec () call.
   // Maybe we should override exec?
   //
   this->ui->pvWriteNameEdit->setFocus ();
   this->returnIsMasked = false;
}

//------------------------------------------------------------------------------
//
void QEPVLoadSaveNameSelectDialog::getPvNames (QString& setPointPvNameOut,
                                               QString& readBackPvNameOut,
                                               QString& archiverPvNameOut)
{
   setPointPvNameOut = this->pvNameEdit[ptWrite]->currentText ().trimmed ();
   readBackPvNameOut = this->pvNameEdit[ptRead]->currentText ().trimmed ();
   archiverPvNameOut = this->pvNameEdit[ptArch]->currentText ().trimmed ();
}

//------------------------------------------------------------------------------
// User has moved away form the filterEdit widget.
// Re-evaluate the set of filtered PVs names that match the filter.
//
void QEPVLoadSaveNameSelectDialog::applyFilter ()
{
   const QString pattern = this->ui->filterEdit->text ().trimmed ();
   const QRegularExpression re (pattern, QRegularExpression::NoPatternOption);

   // Form list of PV names from both the user defined arbitary list
   // and the list extarcted from the QEArchiveAccess.
   //
   QEPvNameSearch findNames (QEPVNameSelectDialog::getPvNameList());

   // addPvNameList ensures overall set of names is sorted and unique.
   //
   findNames.addPvNameList (QEArchiveAccess::getAllPvNames ());
   const int m = findNames.count ();

   this->filteredNames.clear ();
   this->filteredNames = findNames.getMatchingPvNames (re, true);
   const int n = this->filteredNames.count ();

   for (int j = 0; j < PT_NUMBER; j++) {
      this->pvNameEdit[j]->clear ();
      this->pvNameEdit[j]->insertItems (0, this->filteredNames);

      if ((n == 0) && (!this->originalPvName[j].isEmpty ())) {
         this->pvNameEdit[j]->insertItem (0, this->originalPvName[j], QVariant ());
         this->pvNameEdit[j]->setCurrentIndex (0);
      }
   }

   this->ui->matchCountLabel->setText (QString ("%1 / %2").arg (n).arg (m));
}

//------------------------------------------------------------------------------
//
void QEPVLoadSaveNameSelectDialog::filterEditReturnPressed ()
{
   // This return also picked up by on_buttonBox_accepted, mask this return.
   //
   this->returnIsMasked = true;

   // This will cause filterEditingFinished to be invoked - no need
   // to apply filter here.
   //
   this->ui->pvWriteNameEdit->setFocus ();
}

//------------------------------------------------------------------------------
//
void QEPVLoadSaveNameSelectDialog::filterEditingFinished ()
{
   this->applyFilter ();
}

//------------------------------------------------------------------------------
//
void QEPVLoadSaveNameSelectDialog::helpClicked (bool /* checked */ )
{
   // Toggle visibility.
   //
   const bool helpIsVis = !this->ui->help_frame->isVisible();

   this->ui->help_frame->setVisible (helpIsVis);

   const int requiredHelpHeight =
         helpIsVis ? this->ui->help_frame->minimumHeight() : 0;

   this->setFixedHeight (this->ui->frame_1->minimumHeight() +
                         this->ui->frame_2a->minimumHeight() +
                         this->ui->frame_2b->minimumHeight() +
                         this->ui->frame_2c->minimumHeight() +
                         this->ui->frame_3->minimumHeight() +
                         requiredHelpHeight);

   // Expand width if needs be.
   //
   if (helpIsVis) {
      QRect geo = this->geometry();
      int m = this->ui->help_frame->minimumWidth();
      if (m > geo.width()) {
         geo.setWidth (m);
         this->setGeometry (geo);
      }
   }

   // And update button text.
   //
   const QString helpButtonText = helpIsVis ? "Hide" : "Help";
   this->ui->helpButton->setText (helpButtonText);
}

//------------------------------------------------------------------------------
//
void QEPVLoadSaveNameSelectDialog::clearClicked (bool /* checked */ )
{
   for (int j = 0; j < PT_NUMBER; j++) {
      this->pvNameEdit[j]->clear();
      this->pvNameEdit[j]->setCurrentIndex (0);
   }
}

//------------------------------------------------------------------------------
// User close closed the dialog.
//
void QEPVLoadSaveNameSelectDialog::closeEvent (QCloseEvent * event)
{
   QEDialog::closeEvent (event);
}

//------------------------------------------------------------------------------
// User has pressed OK (or return)
//
void QEPVLoadSaveNameSelectDialog::on_buttonBox_accepted ()
{
   if (this->returnIsMasked) {
      this->returnIsMasked = false;
      return;
   }

   QString w,r,a;
   this->getPvNames (w,r,a);
   if (!w.isEmpty ()) {
      this->accept ();
   }
}

//------------------------------------------------------------------------------
// User has pressed Cancel
//
void QEPVLoadSaveNameSelectDialog::on_buttonBox_rejected ()
{
   this->close ();
}

// end
