/*  QEStripChartTimeDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2022 Australian Synchrotron
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

#include "QEStripChartTimeDialog.h"
#include <QDebug>
#include <QECommon.h>
#include <ui_QEStripChartTimeDialog.h>

#define DEBUG qDebug () << "QEStripChartTimeDialog" << __LINE__ << __FUNCTION__ << "  "

static const QString valid   ("QWidget { background-color: #c0e0c0; }");
static const QString invalid ("QWidget { background-color: #e0e0a0; }");

//------------------------------------------------------------------------------
//
QEStripChartTimeDialog::QEStripChartTimeDialog (QWidget *parent) :
      QEDialog (parent),
      ui (new Ui::QEStripChartTimeDialog)
{
   this->ui->setupUi (this);

   this->ui->startTimeSlider->setTracking (true);
   this->ui->endTimeSlider->setTracking (true);

   QObject::connect (this->ui->startDate, SIGNAL (clicked          (const QDate &)),
                     this,                SLOT   (startDateClicked (const QDate &)));

   QObject::connect (this->ui->endDate,   SIGNAL (clicked          (const QDate &)),
                     this,                SLOT   (endDateClicked   (const QDate &)));

   QObject::connect (this->ui->startTimeEdit, SIGNAL (timeChanged      (const QTime &)),
                     this,                    SLOT   (startTimeChanged (const QTime &)));

   QObject::connect (this->ui->endTimeEdit,   SIGNAL (timeChanged      (const QTime &)),
                     this,                    SLOT   (endTimeChanged   (const QTime &)));

   QObject::connect (this->ui->startTimeSlider, SIGNAL (valueChanged            (int)),
                     this,                      SLOT   (startSliderValueChanged (int)));

   QObject::connect (this->ui->endTimeSlider,   SIGNAL (valueChanged            (int)),
                     this,                      SLOT   (endSliderValueChanged   (int)));
}

//------------------------------------------------------------------------------
//
QEStripChartTimeDialog::~QEStripChartTimeDialog ()
{
   delete ui;
}

//------------------------------------------------------------------------------
// static
QTime QEStripChartTimeDialog::timeZero ()
{
   return QTime (0, 0, 0, 0);
}

//------------------------------------------------------------------------------
//
void QEStripChartTimeDialog::setMaximumDateTime (QDateTime datetime)
{
   QDate date = datetime.date ();

   this->ui->startDate->setMaximumDate (date);
   this->ui->endDate->setMaximumDate   (date);
}

//------------------------------------------------------------------------------
//
void QEStripChartTimeDialog::setStartDateTime (QDateTime datetime)
{
   QDate date = datetime.date ();
   QTime time = datetime.time ();

   this->savedStartDateTime = datetime;

   // 240 steps - each 6 minutes.
   //
   int t = this->timeZero ().secsTo (time) / 360;

   this->ui->startDate->setSelectedDate (date);
   this->ui->startTimeSlider->setValue (t);
   this->ui->startTimeEdit->setTime (time);
   this->calcShowDuration ();
}

//------------------------------------------------------------------------------
//
QDateTime QEStripChartTimeDialog::getStartDateTime ()
{
   // By useing the saved time - we preserve the time info (UTC/Local).
   //
   this->savedStartDateTime.setDate (this->ui->startDate->selectedDate ());
   this->savedStartDateTime.setTime (this->ui->startTimeEdit->time ());
   return this->savedStartDateTime;
}

//------------------------------------------------------------------------------
//
void QEStripChartTimeDialog::setEndDateTime (QDateTime datetime)
{
   QDate date = datetime.date ();
   QTime time = datetime.time ();
   int t;

   this->savedEndDateTime = datetime;

   // 240 steps - each 6 minutes.
   t = this->timeZero ().secsTo (time) / 360;

   this->ui->endDate->setSelectedDate (date);
   this->ui->endTimeSlider->setValue (t);
   this->ui->endTimeEdit->setTime (time);
   this->calcShowDuration ();
}

//------------------------------------------------------------------------------
//
QDateTime QEStripChartTimeDialog::getEndDateTime ()
{
   this->savedEndDateTime.setDate (this->ui->endDate->selectedDate ());
   this->savedEndDateTime.setTime (this->ui->endTimeEdit->time ());
   return this->savedEndDateTime;
}

//------------------------------------------------------------------------------
//
QString QEStripChartTimeDialog::timeFormat (const int duration)
{
   QString image;   
   image = QEUtilities::intervalToString (duration, 0, true);
   return image;
}

//------------------------------------------------------------------------------
//
void QEStripChartTimeDialog::calcShowDuration ()
{
   int duration;

   duration = this->getStartDateTime ().secsTo (this->getEndDateTime ());
   this->ui->duration->setText (this->timeFormat (duration));
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
void QEStripChartTimeDialog::startDateClicked (const QDate &)
{
   this->calcShowDuration ();
}

//------------------------------------------------------------------------------
//
void QEStripChartTimeDialog::endDateClicked (const QDate &)
{
   this->calcShowDuration ();
}

//------------------------------------------------------------------------------
//
void QEStripChartTimeDialog::startTimeChanged (const QTime &time)
{
   const int t = this->timeZero ().secsTo (time) / 360;
   this->ui->startTimeSlider->setValue (t);
   this->calcShowDuration ();
}

//------------------------------------------------------------------------------
//
void QEStripChartTimeDialog::endTimeChanged (const QTime &time)
{
   const int t = this->timeZero ().secsTo (time) / 360;
   this->ui->endTimeSlider->setValue (t);
   this->calcShowDuration ();
}

//------------------------------------------------------------------------------
//
void QEStripChartTimeDialog::startSliderValueChanged (int value)
{
    const QTime time = this->timeZero ().addSecs (360 * value);
    this->ui->startTimeEdit->setTime (time);
    this->calcShowDuration ();
}

//------------------------------------------------------------------------------
//
void QEStripChartTimeDialog::endSliderValueChanged (int value)
{
   const QTime time = this->timeZero ().addSecs (360 * value);
   this->ui->endTimeEdit->setTime (time);
   this->calcShowDuration ();
}


//------------------------------------------------------------------------------
// User has pressed OK
//
void QEStripChartTimeDialog::on_buttonBox_accepted ()
{
   this->accept ();
}

//------------------------------------------------------------------------------
// User has pressed cancel
//
void QEStripChartTimeDialog::on_buttonBox_rejected ()
{
   this->close ();
}

// end
