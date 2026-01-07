/*  QEInteger.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2009-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

// Integer specific wrapper for QEChannel variant data.

#include "QEInteger.h"
#include <QDebug>
#include <QStringList>
#include <QEPlatform.h>
#include <QEVectorVariants.h>

#define DEBUG  qDebug () << "QEInteger" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QEInteger::QEInteger (const QString& pvName, QObject* eventObject,
                      QEIntegerFormatting* integerFormattingIn,
                      unsigned int variableIndexIn) :
   QEChannel (pvName, eventObject, variableIndexIn)
{
   this->initialise (integerFormattingIn);
}

//------------------------------------------------------------------------------
//
QEInteger::QEInteger (const QString& pvName, QObject* eventObject,
                      QEIntegerFormatting* integerFormattingIn,
                      unsigned int variableIndexIn,
                      UserMessage* userMessageIn) :
   QEChannel (pvName, eventObject, variableIndexIn, userMessageIn)
{
   this->initialise (integerFormattingIn);
}

//------------------------------------------------------------------------------
// Stream the QEChannel data through this class to generate integer data updates
//
void QEInteger::initialise (QEIntegerFormatting* integerFormattingIn)
{
   this->integerFormat = integerFormattingIn;

   QObject::connect (this, SIGNAL (valueUpdated (const QEVariantUpdate&)),
                     this, SLOT (convertVariant (const QEVariantUpdate&)));
}

//------------------------------------------------------------------------------
// Take a new integer value and write it to the database.
// The type of data formatted (text, floating, integer, etc) will be determined
// by the record data type,
// How the integer is parsed will be determined by the integer formatting.
//
void QEInteger::writeInteger (const long &data)
{
   this->writeData (integerFormat->formatValue (data));
}

//------------------------------------------------------------------------------
// Take a new integer value, insert into array data updating the arrayIndex slot,
// and write whole array to the database.
// Formatting as per writeInteger.
//
void QEInteger::writeIntegerElement (const long &data)
{
   QVariant elementValue = integerFormat->formatValue (data);
   this->writeDataElement (elementValue);
}

//------------------------------------------------------------------------------
// Take a new integer array and write it to the database.
// The type of data formatted (text, floating, integer, etc) will be determined
// by the record data type,
// How the integer is parsed will be determined by the integer formatting.
//
void QEInteger::writeInteger (const QVector < long >&data)
{
   this->writeData (integerFormat->formatValue (data));
}

//------------------------------------------------------------------------------
// Slot to recieve data updates from the base QEChannel and generate integer updates.
//
void QEInteger::convertVariant (const QEVariantUpdate& update)
{
   const QMetaType::Type mtype = QEPlatform::metaType (update.value);

   // The expected varient type is one of:
   // a/ scalar
   // b/ QVariant::List
   // c/ QStringList
   // d/ one of the QEVectorVariants type.
   //
   const bool vlist = (mtype == QMetaType::QVariantList);
   const bool slist = (mtype == QMetaType::QStringList);
   const bool vector = QEVectorVariants::isVectorVariant (update.value);

   // Signaled data structures.
   //
   QEIntegerValueUpdate valueUpdate;
   QEIntegerArrayUpdate arrayUpdate;

   // Set up associated meta data members.
   //
   valueUpdate.alarmInfo = arrayUpdate.alarmInfo = update.alarmInfo;
   valueUpdate.timeStamp = arrayUpdate.timeStamp = update.timeStamp;
   valueUpdate.variableIndex = arrayUpdate.variableIndex = update.variableIndex;
   valueUpdate.isMetaUpdate = arrayUpdate.isMetaUpdate = update.isMetaUpdate;

   // We need non-const copies as (old) signal parameters are not all const.
   //
   QCaAlarmInfo alarmInfo = update.alarmInfo;
   QCaDateTime  timeStamp = update.timeStamp;

   if (vlist || slist || vector) {
      // The value is some sort of array type.
      //
      const QVector<long> data = this->integerFormat->formatIntegerArray (update.value);
      arrayUpdate.values = data;

      emit arrayUpdated (arrayUpdate);
      emit integerArrayChanged (data, alarmInfo, timeStamp, update.variableIndex);

      // Extract the scalar value,
      //
      const int ai = this->getArrayIndex ();
      const int count = vlist ? update.value.toList ().count () : (
                        slist ? update.value.toStringList().count () :
                        QEVectorVariants::vectorCount (update.value));

      if ((ai >= 0) && (ai < count)) {
         // Convert this array element as a scalar update.
         //
         const long item = integerFormat->formatInteger (update.value, ai);
         valueUpdate.value = item;

         emit valueUpdated (valueUpdate);
         emit integerChanged (item, alarmInfo, timeStamp, update.variableIndex);
      }

   } else {
      // The value is a scalar type.
      //
      const long item = this->integerFormat->formatInteger (update.value);
      valueUpdate.value = item;

      emit valueUpdated (valueUpdate);
      emit integerChanged (item, alarmInfo, timeStamp, update.variableIndex);

      // A scalar is also an array with one element.
      //
      QVector<long> array;
      array.append (item);
      arrayUpdate.values = array;

      emit arrayUpdated (arrayUpdate);
      emit integerArrayChanged (array, alarmInfo, timeStamp, update.variableIndex);
   }
}

//------------------------------------------------------------------------------
//
static bool registerMetaTypes()
{
   qRegisterMetaType<QEIntegerValueUpdate> ("QEIntegerValueUpdate");
   qRegisterMetaType<QEIntegerArrayUpdate> ("QEIntegerArrayUpdate");
   return true;
}

static const bool elaborate = registerMetaTypes();

// end
