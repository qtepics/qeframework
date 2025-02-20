/*  QEPvLoadSaveValueEditDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2025 Australian Synchrotron
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

#include "QEPvLoadSaveValueEditDialog.h"
#include <QDebug>
#include <QMetaType>
#include <QStringList>
#include <QECommon.h>
#include <QEPlatform.h>
#include <QEVectorVariants.h>
#include <ui_QEPvLoadSaveValueEditDialog.h>

static const QVariant nilValue = QVariant();

//------------------------------------------------------------------------------
//
QEPvLoadSaveValueEditDialog::QEPvLoadSaveValueEditDialog (QWidget *parent) :
      QEDialog (parent),
      ui (new Ui::QEPvLoadSaveValueEditDialog)
{
   this->ui->setupUi (this);

   QObject::connect (this->ui->elementIndexEdit, SIGNAL (valueChanged (int)),
                     this,                       SLOT   (elementIndexChanged (int)));

   QObject::connect (this->ui->numberElementsEdit, SIGNAL (valueChanged (int)),
                     this,                         SLOT   (numberElementsChanged (int)));
}

//------------------------------------------------------------------------------
//
QEPvLoadSaveValueEditDialog::~QEPvLoadSaveValueEditDialog ()
{
   delete ui;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveValueEditDialog::setPvName (const QString& pvName)
{
   this->ui->nameLabel->setText (pvName);
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveValueEditDialog::setValue (const QVariant& valueIn)
{
   const QMetaType::Type mtype = QEPlatform::metaType (valueIn);

   if (QEVectorVariants::isVectorVariant (valueIn)) {
      bool okay;
      this->valueList = QEVectorVariants::convertToVariantList (valueIn, okay);
   } else if (mtype == QMetaType::QVariantList) {
      this->valueList = valueIn.toList ();
   } else {
      this->valueList.clear ();
      this->valueList.append (valueIn);
   }

   const int n = this->valueList.size ();

   this->ui->numberElementsEdit->setValue (n);
   this->ui->elementIndexEdit->setMaximum (n);
   this->ui->elementIndexEdit->setValue (1);

   this->currentIndex = 1;
   this->outputText ();
}

//------------------------------------------------------------------------------
//
QVariant QEPvLoadSaveValueEditDialog::getValue () const
{
   QVariant result;

   if (this->valueList.count () == 1) {
      // Convert array with just one element to a scalar.
      //
      result = this->valueList.value (0);
   } else {
      // Just return an array variant.
      //
      result = this->valueList;
   }

   return result;
}


//------------------------------------------------------------------------------
//
void QEPvLoadSaveValueEditDialog::outputText ()
{
   QString text = this->valueList.value (this->currentIndex - 1).toString ();
   this->ui->valueEdit->setText (text);
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveValueEditDialog::captureText ()
{
   int j = this->currentIndex - 1;

   if (j >= 0 && j < this->valueList.size ()) {
      QVariant v = this->ui->valueEdit->text ();
      this->valueList.replace (j, v);
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveValueEditDialog::elementIndexChanged (int newIndex)
{
   // Capture current text value, set index, then update text value.
   //
   this->captureText ();
   this->currentIndex = newIndex;
   this->outputText ();
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveValueEditDialog::numberElementsChanged (int numberOfElements)
{
   // Limit currently selected index if needs be.
   //
   if (this->ui->elementIndexEdit->value() > numberOfElements) {
      this->ui->elementIndexEdit->setValue (numberOfElements);
      // this is cause the elementIndexChanged slot to be called.
   }

   // Limit spin box edit maximum.
   //
   this->ui->elementIndexEdit->setMaximum (numberOfElements);

   // Truncate/expand list
   //
   while (this->valueList.size () > numberOfElements) {
      this->valueList.removeLast ();
   }

   while (this->valueList.size () < numberOfElements) {
      this->valueList.append (nilValue);
   }
}

//------------------------------------------------------------------------------
// User has pressed OK
//
void QEPvLoadSaveValueEditDialog::on_buttonBox_accepted ()
{
   this->captureText ();  //ensure valueList is up-to-date
   this->accept ();
}

//------------------------------------------------------------------------------
// User has pressed Cancel
//
void QEPvLoadSaveValueEditDialog::on_buttonBox_rejected ()
{
   this->close ();
}

// end
