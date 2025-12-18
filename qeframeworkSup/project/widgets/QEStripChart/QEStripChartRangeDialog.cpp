/*  QEStripChartRangeDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEStripChartRangeDialog.h"
#include <QVariant>
#include <ui_QEStripChartRangeDialog.h>
#include <QDebug>

#define DEBUG  qDebug () << "QEStripChartRangeDialog" << __LINE__ <<  __FUNCTION__  << "  "

//------------------------------------------------------------------------------
//
QEStripChartRangeDialog::QEStripChartRangeDialog (QWidget *parent) :
      QEDialog (parent),
      ui (new Ui::QEStripChartRangeDialog)
{
   this->ui->setupUi (this);

   this->minimum = 0.0;
   this->maximum = 0.0;
   this->returnIsMasked = false;

   QObject::connect (this->ui->minimumEdit, SIGNAL  (returnPressed ()),
                     this,                  SLOT (minReturnPressed ()));

   QObject::connect (this->ui->maximumEdit, SIGNAL  (returnPressed ()),
                     this,                  SLOT (maxReturnPressed ()));
}

//------------------------------------------------------------------------------
//
QEStripChartRangeDialog::~QEStripChartRangeDialog ()
{
   delete ui;
}


//------------------------------------------------------------------------------
//
void QEStripChartRangeDialog::setRange (const double minIn, const double maxIn)
{
   QString text;

   this->minimum = minIn;
   this->maximum = maxIn;

   text = QString::asprintf (" %.15g", this->minimum);
   this->ui->minimumEdit->setText(text);

   text = QString::asprintf (" %.15g", this->maximum);
   this->ui->maximumEdit->setText(text);

   this->ui->minimumEdit->setFocus ();
   }

//------------------------------------------------------------------------------
//
double QEStripChartRangeDialog::getMinimum ()
{
   return this->minimum;
}

//------------------------------------------------------------------------------
//
double QEStripChartRangeDialog::getMaximum ()
{
   return this->maximum;
}

//------------------------------------------------------------------------------
//
void QEStripChartRangeDialog::minReturnPressed ()
{
   this->returnIsMasked = true;
   this->ui->maximumEdit->setFocus ();
}

//------------------------------------------------------------------------------
//
void QEStripChartRangeDialog::maxReturnPressed ()
{
   this->returnIsMasked = true;
   this->ui->buttonBox->setFocus();
}


//------------------------------------------------------------------------------
// User has pressed OK
//
void QEStripChartRangeDialog::on_buttonBox_accepted ()
{
   if (this->returnIsMasked) {
      this->returnIsMasked = false;
      return;
   }

   // Extract and validate user entry.
   //
   bool ok1, ok2;
   double valMin = this->ui->minimumEdit->text ().toDouble(&ok1);
   double valMax = this->ui->maximumEdit->text ().toDouble(&ok2);

   if (ok1 && ok2 && (valMax > valMin)) {
      // All okay - assign values to object.
      //
      this->minimum = valMin;
      this->maximum = valMax;

      // Proceed with 'good' dialog exit
      //
      QDialog::accept();
   }
}

//------------------------------------------------------------------------------
// User has pressed cancel
//
void QEStripChartRangeDialog::on_buttonBox_rejected ()
{
   this->close ();
}

// end
