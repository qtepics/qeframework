/*  QEPvLoadSaveTimeDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2016-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include <QDebug>
#include <QEPvLoadSaveTimeDialog.h>
#include <QECommon.h>

#include <ui_QEPvLoadSaveTimeDialog.h>


//------------------------------------------------------------------------------
//
QEPvLoadSaveTimeDialog::QEPvLoadSaveTimeDialog (QWidget *parent) :
      QEDialog (parent),
      ui (new Ui::QEPvLoadSaveTimeDialog)
{
   this->ui->setupUi (this);

   this->ui->timeSlider->setTracking (true);

   QObject::connect (this->ui->dateCalendar, SIGNAL (clicked     (const QDate &)),
                     this,                   SLOT   (dateClicked (const QDate &)));

   QObject::connect (this->ui->timeEdit, SIGNAL (timeChanged (const QTime &)),
                     this,               SLOT   (timeChanged (const QTime &)));

   QObject::connect (this->ui->timeSlider, SIGNAL (valueChanged       (int)),
                     this,                 SLOT   (sliderValueChanged (int)));
}

//------------------------------------------------------------------------------
//
QEPvLoadSaveTimeDialog::~QEPvLoadSaveTimeDialog ()
{
   delete ui;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveTimeDialog::setMaximumDateTime (const QDateTime& datetime)
{
   QDate date = datetime.date ();

   this->ui->dateCalendar->setMaximumDate (date);
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveTimeDialog::setDateTime (const QDateTime& datetime)
{
   QDate date = datetime.date ();
   QTime time = datetime.time ();
   int t;

   this->savedDateTime = datetime;

   // 240 steps - each 6 minutes.
   t = QTime ().secsTo (time) / 360;

   this->ui->dateCalendar->setSelectedDate (date);
   this->ui->timeSlider->setValue (t);
   this->ui->timeEdit->setTime (time);
}

//------------------------------------------------------------------------------
//
QDateTime QEPvLoadSaveTimeDialog::getDateTime ()
{
   // By useing the saved time - we preserve the time info (UTC/Local).
   //
   this->savedDateTime.setDate (this->ui->dateCalendar->selectedDate ());
   this->savedDateTime.setTime (this->ui->timeEdit->time ());
   return this->savedDateTime;
}

//==============================================================================
// Slots.
//==============================================================================
//
void QEPvLoadSaveTimeDialog::dateClicked (const QDate &)
{
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveTimeDialog::timeChanged (const QTime &time)
{
   int t;

   t = QTime ().secsTo (time) / 360;
   this->ui->timeSlider->setValue (t);
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveTimeDialog::sliderValueChanged (int value)
{
    QTime time;

    time = QTime ().addSecs (360 * value);
    this->ui->timeEdit->setTime (time);
}

//------------------------------------------------------------------------------
// User has pressed OK
//
void QEPvLoadSaveTimeDialog::on_buttonBox_accepted ()
{
   this->accept ();
}

//------------------------------------------------------------------------------
// User has pressed cancel
//
void QEPvLoadSaveTimeDialog::on_buttonBox_rejected ()
{
   this->close ();
}

// end
