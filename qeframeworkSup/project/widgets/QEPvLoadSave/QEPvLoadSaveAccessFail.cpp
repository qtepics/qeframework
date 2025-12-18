/*  QEPvLoadSaveAccessFail.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2017-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEPvLoadSaveAccessFail.h"
#include <QDebug>
#include <ui_QEPvLoadSaveAccessFail.h>

#define DEBUG  qDebug () << "QEPvLoadSaveAccessFail" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QEPvLoadSaveAccessFail::QEPvLoadSaveAccessFail (QEPvLoadSave* ownerIn,
                                                QWidget* parent) :
   QWidget (parent),
   owner (ownerIn),
   ui (new Ui::QEPvLoadSaveAccessFail)
{
   this->ui->setupUi (this);
}

//------------------------------------------------------------------------------
//
QEPvLoadSaveAccessFail::~QEPvLoadSaveAccessFail ()
{
   delete this->ui;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveAccessFail::clear ()
{
   this->ui->textEdit->clear ();
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveAccessFail::addPVName (const QString& pvName)
{
   if (!pvName.isEmpty ()) {
      this->ui->textEdit->append (pvName);
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveAccessFail::addPVNames (const QStringList& pvNameList)
{
   const int n = pvNameList.count ();
   for (int j = 0; j < n; j++) {
      QString pvName = pvNameList.value (j, "");
      this->addPVName (pvName);
   }
}

// end
