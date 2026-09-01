/*  QEPvLoadSaveDelayEditDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEPvLoadSaveDelayEditDialog.h"
#include <QDebug>
#include <QString>
#include <QECommon.h>
#include <ui_QEPvLoadSaveDelayEditDialog.h>

static const double maxDelay = 3600.0;   // 1 hour

//------------------------------------------------------------------------------
//
QEPvLoadSaveDelayEditDialog::QEPvLoadSaveDelayEditDialog (QWidget *parent) :
      QEDialog (parent),
      ui (new Ui::QEPvLoadSaveDelayEditDialog)
{
   this->ui->setupUi (this);
   this->delay = 5.0;   // default
}

//------------------------------------------------------------------------------
//
QEPvLoadSaveDelayEditDialog::~QEPvLoadSaveDelayEditDialog ()
{
   delete ui;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveDelayEditDialog::setDelay (const double delayIn)
{
   this->delay = LIMIT (delayIn, 0.0, maxDelay);
   this->outputText ();
}

//------------------------------------------------------------------------------
//
double QEPvLoadSaveDelayEditDialog::getDelay () const
{
   return this->delay;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveDelayEditDialog::outputText ()
{
   const QString text = QString::number (this->delay, 'f', 3);
   this->ui->delayEdit->setText (text);
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveDelayEditDialog::captureText ()
{
   const QString text = this->ui->delayEdit->text ();
   bool okay;
   double v = text.toDouble(&okay);
   if (okay) {
      this->delay = LIMIT (v, 0.0, maxDelay);
      this->outputText ();
   }
}

//------------------------------------------------------------------------------
// User has pressed OK
//
void QEPvLoadSaveDelayEditDialog::on_buttonBox_accepted ()
{
   this->captureText ();  //ensure delayList is up-to-date
   this->accept ();
}

//------------------------------------------------------------------------------
// User has pressed Cancel
//
void QEPvLoadSaveDelayEditDialog::on_buttonBox_rejected ()
{
   this->close ();
}

// end
