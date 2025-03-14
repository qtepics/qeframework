/*  QEStripChartRangeDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2024 Australian Synchrotron
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
