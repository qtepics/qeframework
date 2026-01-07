/*  QEFloating.cpp
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

// Floating specific wrapper for QEChannel variant data.

#include "QEFloating.h"
#include <QDebug>
#include <QStringList>
#include <QEPlatform.h>
#include <QEVectorVariants.h>

#define DEBUG  qDebug () << "QEFloating" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QEFloating::QEFloating (const QString& pvName, QObject* eventObject,
                        QEFloatingFormatting* floatingFormattingIn,
                        unsigned int variableIndexIn) :
   QEChannel (pvName, eventObject, variableIndexIn)
{
   this->initialise (floatingFormattingIn);
}

//------------------------------------------------------------------------------
//
QEFloating::QEFloating (const QString& pvName, QObject* eventObject,
                        QEFloatingFormatting* floatingFormattingIn,
                        unsigned int variableIndexIn,
                        UserMessage* userMessageIn) :
   QEChannel (pvName, eventObject, variableIndexIn, userMessageIn)
{
   this->initialise (floatingFormattingIn);
}

//------------------------------------------------------------------------------
// Stream the QEChannel data through this class to generate floating data updates
//
void QEFloating::initialise (QEFloatingFormatting* floatingFormattingIn)
{
   this-> floatingFormat = floatingFormattingIn;

   QObject::connect (this, SIGNAL (valueUpdated (const QEVariantUpdate&)),
                     this, SLOT (convertVariant (const QEVariantUpdate&)));
}

//------------------------------------------------------------------------------
// Take a new floating value and write it to the database.
// The type of data formatted (text, floating, integer, etc) will be determined
// by the record data type,
// How the floating is parsed will be determined by the floating formatting.
//
void QEFloating::writeFloating (const double &data)
{
   this->writeData (floatingFormat->formatValue (data));
}

//------------------------------------------------------------------------------
// Take a new floating value, insert into array data updating the arrayIndex slot,
// and write whole array to the database.
// Formatting as per writeFloating.
//
void QEFloating::writeFloatingElement (const double &data)
{
   QVariant elementValue = floatingFormat->formatValue (data);
   this->writeDataElement (elementValue);
}

//------------------------------------------------------------------------------
// Take a new floating array and write it to the database.
// The type of data formatted (text, floating, integer, etc) will be determined
// by the record data type,
// How the floating is parsed will be determined by the floating formatting.
//
void QEFloating::writeFloating (const QVector<double> &data)
{
   this->writeData (floatingFormat->formatValue (data));
}

//------------------------------------------------------------------------------
// Slot to recieve data updates from the base QEChannel and generate floating updates.
//
void QEFloating::convertVariant (const QEVariantUpdate& update)
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
   QEFloatingValueUpdate valueUpdate;
   QEFloatingArrayUpdate arrayUpdate;

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
      const QVector<double> data = this->floatingFormat->formatFloatingArray (update.value);
      arrayUpdate.values = data;

      emit arrayUpdated (arrayUpdate);
      emit floatingArrayChanged (data, alarmInfo, timeStamp, update.variableIndex);

      // Extract the scalar value,
      //
      const int ai = this->getArrayIndex ();
      const int count = vlist ? update.value.toList ().count () : (
                        slist ? update.value.toStringList().count () :
                        QEVectorVariants::vectorCount (update.value));

      if ((ai >= 0) && (ai < count)) {
         // Convert this array element as a scalar update.
         //
         const double item = this->floatingFormat->formatFloating (update.value, ai);
         valueUpdate.value = item;

         emit valueUpdated (valueUpdate);
         emit floatingChanged (item, alarmInfo, timeStamp, update.variableIndex);
      }

   } else {
      // The value is a scalar type.
      //
      const double item = this->floatingFormat->formatFloating (update.value);
      valueUpdate.value = item;

      emit valueUpdated (valueUpdate);
      emit floatingChanged (item, alarmInfo, timeStamp, update.variableIndex);

      // A scalar is also an array with one element.
      //
      QVector<double> array;
      array.append (item);
      arrayUpdate.values = array;

      emit arrayUpdated (arrayUpdate);
      emit floatingArrayChanged (array, alarmInfo, timeStamp, update.variableIndex);
   }
}

//------------------------------------------------------------------------------
//
static bool registerMetaTypes()
{
   qRegisterMetaType<QEFloatingValueUpdate> ("QEFloatingValueUpdate");
   qRegisterMetaType<QEFloatingArrayUpdate> ("QEFloatingArrayUpdate");
   return true;
}

static const bool elaborate = registerMetaTypes();

// end
