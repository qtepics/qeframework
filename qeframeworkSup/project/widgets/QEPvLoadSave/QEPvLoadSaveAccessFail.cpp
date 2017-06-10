/*  QEPvLoadSaveAccessFail.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
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
 *  Copyright (c) 2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
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
