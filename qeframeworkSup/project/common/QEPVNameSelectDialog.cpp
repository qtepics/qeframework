/*  QEPVNameSelectDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2022 Australian Synchrotron
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#include "QEPVNameSelectDialog.h"
#include <QtGlobal>
#include <QDebug>
#include <QRegularExpression>
#include <QStringList>

#include <ui_QEPVNameSelectDialog.h>
#include <QEPvNameSearch.h>
#include <QEArchiveManager.h>
#include <QEScaling.h>

#define DEBUG  qDebug () << "QEPVNameSelectDialog:" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// static
QStringList QEPVNameSelectDialog::pvNameList;

//------------------------------------------------------------------------------
// static
void QEPVNameSelectDialog::setPvNameList (const QStringList& pvNameListIn)
{
   QEPVNameSelectDialog::pvNameList = pvNameListIn;
}

//------------------------------------------------------------------------------
// static
QStringList QEPVNameSelectDialog::getPvNameList ()
{
   return QEPVNameSelectDialog::pvNameList;
}

//------------------------------------------------------------------------------
//
QEPVNameSelectDialog::QEPVNameSelectDialog (QWidget *parent) :
   QEDialog (parent),
   ui (new Ui::QEPVNameSelectDialog)
{
   this->ui->setupUi (this);

   // Ensure the dialog centres using this widget as reference point.
   //
   this->setSourceWidget (this->ui->pvNameEdit);

   this->returnIsMasked = false;

   this->ui->help_frame->setVisible (false);
   this->setFixedHeight (this->ui->frame_1->minimumHeight() +
                         this->ui->frame_2->minimumHeight() +
                         this->ui->frame_2->minimumHeight() );

   QObject::connect (this->ui->filterEdit,  SIGNAL  (returnPressed ()),
                     this,                  SLOT    (filterEditReturnPressed ()));

   QObject::connect (this->ui->filterEdit,  SIGNAL (editingFinished       ()),
                     this,                  SLOT   (filterEditingFinished ()));

   QObject::connect (this->ui->pvNameEdit,  SIGNAL (editTextChanged (const QString&)),
                     this,                  SLOT   (editTextChanged (const QString&)));

   QObject::connect (this->ui->helpButton,  SIGNAL (clicked       (bool)),
                     this,                  SLOT   (helpClicked   (bool)));

   QObject::connect (this->ui->clearButton, SIGNAL (clicked       (bool)),
                     this,                  SLOT   (clearClicked  (bool)));

#if QT_VERSION < 0x060000
#ifndef QT_NO_COMPLETER
   // Could not get completer to work - yet.
   this->ui->pvNameEdit->setAutoCompletion (true);
   this->ui->pvNameEdit->setAutoCompletionCaseSensitivity (Qt::CaseSensitive);
#endif
#endif
}

//------------------------------------------------------------------------------
//
QEPVNameSelectDialog::~QEPVNameSelectDialog ()
{
   this->filteredNames.clear ();
   delete ui;
}

//------------------------------------------------------------------------------
//
void QEPVNameSelectDialog::setPvName (QString pvNameIn)
{
   this->originalPvName = pvNameIn.trimmed ();

   this->ui->pvNameEdit->clear ();

   // Number may be zero - no special check required - just do it.
   //
   this->ui->pvNameEdit->insertItems (0, this->filteredNames);
   if (!this->originalPvName.isEmpty ()) {
      this->ui->pvNameEdit->insertItem (0, this->originalPvName, QVariant ());
   }
   this->ui->pvNameEdit->setCurrentIndex (0);

   // setPvName typically invoked just before exec () call.
   // Maybe we should override exec?
   //
   this->ui->pvNameEdit->setFocus ();
   this->returnIsMasked = false;
}

//------------------------------------------------------------------------------
//
QString QEPVNameSelectDialog::getPvName ()
{
   return this->ui->pvNameEdit->currentText ().trimmed ();
}

//------------------------------------------------------------------------------
// Use has moved away form the filterEdit widget.
// Re-evaluate the set of filtered PVs names that match the filter.
//
void QEPVNameSelectDialog::applyFilter ()
{
   QString pattern = this->ui->filterEdit->text ().trimmed ();
   QRegularExpression re (pattern, QRegularExpression::NoPatternOption);

   // Form list of PV names from both the user defined arbitary list
   // and the list extarcted from the QEArchiveAccess.
   //
   QEPvNameSearch findNames (QEPVNameSelectDialog::pvNameList);

   // addPvNameList ensures overall set of names is sorted and unique.
   //
   findNames.addPvNameList (QEArchiveAccess::getAllPvNames ());
   const int m = findNames.count ();

   this->filteredNames.clear ();
   this->filteredNames = findNames.getMatchingPvNames (re, true);
   const int n = this->filteredNames.count ();

   this->ui->pvNameEdit->clear ();
   this->ui->pvNameEdit->insertItems (0, this->filteredNames);

   if ((n == 0) && (!this->originalPvName.isEmpty ())) {
      this->ui->pvNameEdit->insertItem (0, this->originalPvName, QVariant ());
      this->ui->pvNameEdit->setCurrentIndex (0);
   }

   this->ui->matchCountLabel->setText (QString ("%1 / %2").arg (n).arg (m));
}

//------------------------------------------------------------------------------
//
void QEPVNameSelectDialog::filterEditReturnPressed ()
{
   // This return also pick up by on_buttonBox_accepted, mask this return.
   //
   this->returnIsMasked = true;

   // This will cause filterEditingFinished to be invoked - no need
   // to apply filter here.
   //
   this->ui->pvNameEdit->setFocus ();
}

//------------------------------------------------------------------------------
//
void QEPVNameSelectDialog::filterEditingFinished ()
{
   this->applyFilter ();
}

//------------------------------------------------------------------------------
//
void QEPVNameSelectDialog::editTextChanged (const QString&)
{
   // NOTE: calling buttonBox->setStandardButtons causes a seg fault when cancel
   // eventually pressed and sometimes okay button as well, so do nothing for now.
   // Maybe do our own buttons instead of using a QDialogButtonBox.
}

//------------------------------------------------------------------------------
//
void QEPVNameSelectDialog::helpClicked (bool /* checked */ )
{
   // Toggle visibility.
   //
   const bool helpIsVis = !this->ui->help_frame->isVisible();

   this->ui->help_frame->setVisible (helpIsVis);

   const int requiredHelpHeight =
         helpIsVis ? this->ui->help_frame->minimumHeight() : 0;

   this->setFixedHeight (this->ui->frame_1->minimumHeight() +
                         this->ui->frame_2->minimumHeight() +
                         this->ui->frame_2->minimumHeight() +
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
void QEPVNameSelectDialog::clearClicked (bool /* checked */ )
{
   this->ui->pvNameEdit->clear();
   this->ui->pvNameEdit->setCurrentIndex (0);
}

//------------------------------------------------------------------------------
// User close closed the dialog.
//
void QEPVNameSelectDialog::closeEvent (QCloseEvent * event)
{
   QEDialog::closeEvent (event);
}

//------------------------------------------------------------------------------
// User has pressed OK (or return)
//
void QEPVNameSelectDialog::on_buttonBox_accepted ()
{
   if (this->returnIsMasked) {
      this->returnIsMasked = false;
      return;
   }

   if (!this->getPvName().isEmpty ()) {
      this->accept ();
   }
}

//------------------------------------------------------------------------------
// User has pressed Cancel
//
void QEPVNameSelectDialog::on_buttonBox_rejected ()
{
   this->close ();
}

// end
