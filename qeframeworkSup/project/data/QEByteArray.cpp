/*  QEByteArray.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/* Description:
 *
 * This class is similar to QEInteger, QEFloating and QEString classes in that it will always
 * emit signals containing QByteArray data no mater what the type of the underlying PV.
 * The other classes mentioned achieve this by converting the data retrieved as appropriate.
 * This class always simply returns the raw data as a byte array
 *
 */

#include "QEByteArray.h"
#include <QDebug>

#define DEBUG  qDebug () << "QEByteArray" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// QE Byte array creation.
//
// Note, the QCaObject is created with low priorityfor the following scenario:
// Several large rapidly updating images being displayed. Network bandwidth is
// far less than would support the image update rate.
// When scalar values are requested, the request times out before the scalar updates.
//
QEByteArray::QEByteArray (QString pvName,
                          QObject* eventObject,
                          unsigned int variableIndexIn) :
   QCaObject (pvName, eventObject, variableIndexIn,
              SIG_BYTEARRAY, QE_PRIORITY_LOW)
{
   this->initialise();
}

//------------------------------------------------------------------------------
//
QEByteArray::QEByteArray (QString pvName,
                          QObject* eventObject,
                          unsigned int variableIndexIn,
                          UserMessage* userMessageIn) :
   QCaObject (pvName, eventObject, variableIndexIn,
              userMessageIn, SIG_BYTEARRAY, QE_PRIORITY_LOW)
{
   this->initialise();
}

//------------------------------------------------------------------------------
// Stream the QCaObject data through this class to generate byte array data updates
//
void QEByteArray::initialise()
{
   QObject::connect (this, SIGNAL      (dataChanged (const QByteArray&, unsigned long, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                     this, SLOT (forwardDataChanged (const QByteArray&, unsigned long, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));
}

//------------------------------------------------------------------------------
// Take a new byte array value and write it to the database.
//
void QEByteArray::writeByteArray (const QByteArray &data)
{
   this->writeData (QVariant (data));
}

//------------------------------------------------------------------------------
// Slot to recieve data updates from the base QCaObject and generate byte array updates.
//
void QEByteArray::forwardDataChanged (const QByteArray &value,
                                      unsigned long dataSize,
                                      QCaAlarmInfo& alarmInfo,
                                      QCaDateTime& timeStamp,
                                      const unsigned int& variableIndex)
{
   emit byteArrayChanged (value, dataSize, alarmInfo, timeStamp, variableIndex);
}

// end
