/*  QEPvLoadSaveButton.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2020-2025 Australian Synchrotron
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
 *    Andraz Pozar
 *  Contact details:
 *    andraz.pozar@ansto.gov.au
 */

#include "QEPvLoadSaveButton.h"

#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QApplication>

#include "QECommon.h"
#include "QEPvLoadSaveItem.h"
#include "QEPvLoadSaveModel.h"
#include "QEPvLoadSaveUtilities.h"

#define DEBUG  qDebug () << "QEPvLoadSaveButton" << __LINE__ << __FUNCTION__ << "  "

QEPvLoadSaveButton::QEPvLoadSaveButton (QWidget* parent) : QPushButton (parent), QEWidget (this) {

   // Set up data
   this->progressDialog = NULL;
   this->confirmRequired = false;
   this->loadSaveAction = LoadToPVs;
   this->showProgressDialog = true;
   this->setVariableAsToolTip(false);
   this->setAllowDrop(false);
   this->setDisplayAlarmStateOption(QE::Never);


   // Identify the type of button
   this->setText( "QEPvLoadSaveButton" );
   confirmText = "Do you want to perform this action?";

   // Create an essentially empty model.
   //
   this->model = new QEPvLoadSaveModel (new QTreeView ());  // not a widget

   // Set up a connection to recieve configuration file name property changes.
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   QObject::connect (&this->vnpm, SIGNAL (newVariableNameProperty (QString, QString, unsigned int)),
                     this, SLOT   (useNewConfigurationFileProperty (QString, QString, unsigned int)));

   QObject::connect (this->model, SIGNAL (reportActionComplete (const QEPvLoadSaveItem*, const QEPvLoadSaveCommon::ActionKinds, const bool)),
                     this, SLOT   (acceptActionComplete (const QEPvLoadSaveItem*, const QEPvLoadSaveCommon::ActionKinds, const bool)));

   QObject::connect( this, SIGNAL( clicked( bool ) ), this, SLOT( userClicked( bool ) ) );
}

QEPvLoadSaveButton::~QEPvLoadSaveButton() {
   delete(this->model);
   if (this->progressDialog != NULL) {
      delete(this->progressDialog);
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveButton::useNewConfigurationFileProperty (QString configurationFileIn,
                                                          QString configurationFileSubstitutionsIn,
                                                          unsigned int variableIndex)
{
   this->setVariableNameAndSubstitutions (configurationFileIn, configurationFileSubstitutionsIn, variableIndex);
}

// On user click on the button
//
void QEPvLoadSaveButton::userClicked(bool)
{
   const message_types mt = message_types (MESSAGE_TYPE_WARNING,
                                           MESSAGE_KIND_STANDARD);

   // Check that the configuration file is actually set
   //
   if (this->getConfigurationFile().isEmpty()) {
      this->sendMessage ("No configuration file defined for this widget. Won't do anything.",  mt);
      return;
   }

   // Try to read the config file and create a tree. If we can't read for any reason
   // we fail and print the message below.
   //
   QString configurationFile = this->getSubstitutedVariableName (0);
   QString errorMessage;
   this->rootItem = QEPvLoadSaveUtilities::readTree (configurationFile, QString(), errorMessage);
   if (!this->rootItem) {
       this->sendMessage (errorMessage, mt);
       return;
   }

   this->model->setupModelData (this->rootItem, configurationFile);

   // If we find any PVs saved in the file and
   if (model->leafCount() > 0 && actionIsPermitted(this->getAction())) {
      // If progress dialog is requested, create and configure it if it hasn't yet been created.
      // If it has simply reset the values.
      //
      if (this->showProgressDialog) {
         QString labelText = getAction() == SaveToFile ? "Saving PV values to file ..." : "Applying PV values to the system...";
         if (this->progressDialog == NULL) {
            this->progressDialog = new QProgressDialog(labelText, "Abort", 0, MAX (1, model->leafCount ()), this);
            this->progressDialog->setWindowTitle("Load/Save Progress");
            this->progressDialog->setWindowModality(Qt::WindowModal);
            this->progressDialog->setMinimumDuration(0);
            this->progressDialog->setAutoClose(false);
            this->progressDialog->setAutoReset(false);
            this->progressDialog->setValue(0);
            this->progressDialog->setMinimumWidth(300);
         } else {
            this->progressDialog->setLabelText(labelText);
            this->progressDialog->setCancelButtonText("Abort");
            this->progressDialog->setRange(0,MAX (1, model->leafCount ()));
            this->progressDialog->setValue(0);
            this->progressDialog->open();
         }
         QApplication::processEvents();
      } else {
         this->setEnabled(false);
      }
      // Wait for one second to allow for all channels to connect and data to become available. Disable the
      // button while we wait.
      //
      if (getAction() == SaveToFile) {
         QTimer::singleShot (1000, this, SLOT (delayedSaveToFile()));
      } else if (getAction() == LoadToPVs) {
         QTimer::singleShot (1000, this, SLOT (delayedLoadToPVs()));
      }
   }
}


void QEPvLoadSaveButton::delayedSaveToFile () {
   QString configurationFile = this->getConfigurationFile();
   this->model->extractPVData ();
   if (QEPvLoadSaveUtilities::writeTree (configurationFile, this->rootItem)) {
      this->model->setHeading (configurationFile);
   }
   if (!this->showProgressDialog) {
      this->setEnabled(true);
   }
}


void QEPvLoadSaveButton::delayedLoadToPVs () {
   this->model->applyPVData ();
   if (!this->showProgressDialog) {
      this->setEnabled(true);
   }
}


bool QEPvLoadSaveButton::actionIsPermitted (Actions action)
{
   bool result = false;
   QString message;
   if (this->confirmRequired) {
      if (!this->getConfirmText().isEmpty()) {
         message = this->getConfirmText();
      } else {
         if (action == LoadToPVs) {
            message = "You are about to write to one or more system Process\n"
                  "Variables. This may adversely affect the operation of\n"
                  "the system. Are you sure you wish to processed?\n"
                  "Click OK to proceed or Cancel for no change.";
         } else if (action == SaveToFile) {
            message = "You are about to save the values of oneor more system\n"
                  "Process Variables. This will override any previously\n"
                  "saved values in the file. Are you sure you wish to processed?\n"
                  "Click OK to proceed or Cancel for no change.";
         }
      }
      int confirm = QMessageBox::warning
               (this, QString("PV %1 Confirmation").arg(action == LoadToPVs ? "Load" : "Save"), message,
                QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
      result = (confirm == QMessageBox::Ok);
   } else {
      result = true;
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveButton::acceptActionComplete (const QEPvLoadSaveItem*,
                                               const QEPvLoadSaveCommon::ActionKinds,
                                               const bool okay)
{
   if (okay && this->showProgressDialog) {
      int v = this->progressDialog->value () + 1;
      this->progressDialog->setValue (v);
      int n = this->progressDialog->maximum();
      QString status = QString ("Processed %2 of %3 items")
            .arg (v).arg (n);
      this->progressDialog->setLabelText (status);
      if (v == n) {
         this->progressDialog->setCancelButtonText("Done");
      }
      QApplication::processEvents();
   }
}

// end
