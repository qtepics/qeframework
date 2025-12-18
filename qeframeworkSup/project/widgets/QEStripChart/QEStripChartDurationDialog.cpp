/*  QEStripChartDurationDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEStripChartDurationDialog.h"
#include <QDebug>
#include <QECommon.h>
#include <ui_QEStripChartDurationDialog.h>


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
   secs = QTime (0, 0, 0, 0).secsTo (this->ui->endTimeEdit->time ());
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
