/*  QEStripChartDurationDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2014 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#include <QDebug>
#include <QECommon.h>

#include <ui_QEStripChartDurationDialog.h>

#include <QEStripChartDurationDialog.h>

static const QString valid   ("QWidget { background-color: #c0e0c0; }");
static const QString invalid ("QWidget { background-color: #e0e0a0; }");

static const int secsPerDay = 86400;

//------------------------------------------------------------------------------
//
QEStripChartDurationDialog::QEStripChartDurationDialog (QWidget *parent) :
      QEDialog (parent),
      ui (new Ui::QEStripChartDurationDialog)
{
   this->ui->setupUi (this);

   QObject::connect (this->ui->spinBox,       SIGNAL (valueChanged  (const int)),
                     this,                    SLOT   (daysChanged   (const int)));

   QObject::connect (this->ui->endTimeEdit,   SIGNAL (timeChanged   (const QTime &)),
                     this,                    SLOT   (timeChanged   (const QTime &)));

}

//------------------------------------------------------------------------------
//
QEStripChartDurationDialog::~QEStripChartDurationDialog ()
{
   delete ui;
}

//------------------------------------------------------------------------------
//
void QEStripChartDurationDialog::setDuration (int durationIn)
{
   int days;
   int secs;

   this->duration = MAX (1, durationIn);

   days = this->duration / secsPerDay;
   secs = this->duration % secsPerDay;
   QTime time (0, 0, 0, 0);
   time = time.addSecs (secs);

   this->ui->spinBox->setValue (days);
   this->ui->endTimeEdit->setTime (time);
}

//------------------------------------------------------------------------------
//
int QEStripChartDurationDialog::getDuration () const
{
   return this->duration;
}

//------------------------------------------------------------------------------
//
void QEStripChartDurationDialog::calcShowDuration ()
{   
   int days;
   int secs;

   days = this->ui->spinBox->value ();
   days = LIMIT (days, 0, 9999);
   secs = QTime ().secsTo (this->ui->endTimeEdit->time ());
   this->duration = secsPerDay * days + secs;

   this->duration = MAX (1, this->duration);

   this->ui->duration->setText (QEUtilities::intervalToString ((double) this->duration, 0, true));
   if (duration > 0) {
      this->ui->duration->setStyleSheet (valid);
   } else {
      this->ui->duration->setStyleSheet (invalid);
   }
}

//==============================================================================
// Slots.
//==============================================================================
//
void QEStripChartDurationDialog::daysChanged (const int)
{
   this->calcShowDuration ();
}

//------------------------------------------------------------------------------
//
void QEStripChartDurationDialog::timeChanged (const QTime &)
{
   this->calcShowDuration ();
}


//------------------------------------------------------------------------------
// User has pressed OK
//
void QEStripChartDurationDialog::on_buttonBox_accepted ()
{
   this->accept ();
}

//------------------------------------------------------------------------------
// User has pressed cancel
//
void QEStripChartDurationDialog::on_buttonBox_rejected ()
{
   this->close ();
}

// end
