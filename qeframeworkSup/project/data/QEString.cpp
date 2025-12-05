/*  QEString.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2025 Australian Synchrotron
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License.
 *
 *  The EPICS QT Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

// String specifc wrapper for QCaObject variant data.

#include "QEString.h"
#include <QDebug>
#include <QStringList>
#include <QEPlatform.h>
#include <QEVectorVariants.h>

#define DEBUG qDebug() << "QEString" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QEString::QEString (QString pvName, QObject* eventObject,
                    QEStringFormatting* stringFormattingIn,
                    unsigned int variableIndexIn) :
   QCaObject (pvName, eventObject, variableIndexIn)
{
   this->initialise (stringFormattingIn);
}

//------------------------------------------------------------------------------
//
QEString::QEString (QString pvName, QObject* eventObject,
                    QEStringFormatting* stringFormattingIn,
                    unsigned int variableIndexIn,
                    UserMessage* userMessageIn) :
   QCaObject (pvName, eventObject, variableIndexIn, userMessageIn)
{
   this->initialise (stringFormattingIn);
}

//------------------------------------------------------------------------------
// Stream the QCaObject data through this class to generate textual data
// updates.
//
void QEString::initialise (QEStringFormatting* stringFormattingIn)
{
   this->stringFormat = stringFormattingIn;

   QObject::connect (this, SIGNAL (valueUpdated (const QEVariantUpdate&)),
                     this, SLOT (convertVariant (const QEVariantUpdate&)));
}

//------------------------------------------------------------------------------
// Take a new text value and write it to the database.
// The type of data formatted (text, floating, integer, etc) will be determined by the record data type,
// How the text is parsed will be determined by the string formatting.
// For example, assume the record is a floating point record and the text is formatted as an integer.
// The string is parsed as in integer (123 is OK, 123.456 would fail), then converted to a floating point number.
// The above example is pedantic
// if the string formatting
//
bool QEString::writeString (const QString &data, QString& message)
{
   bool ok = false;
   QVariant formattedData = stringFormat->formatValue (data, ok);
   if (ok) {
      this->writeData (formattedData);
   } else {
      message = QString ("Write failed, unabled to format: '").append (data).append ("'.");
   }
   return ok;
}

//------------------------------------------------------------------------------
//
void QEString::writeString (const QString &data)
{
   QString message;
   bool ok = this->writeString (data, message);
   if (!ok) {
      qDebug() << message;
   }
}

//------------------------------------------------------------------------------
// Take a new string value, insert into array data updating the arrayIndex slot,
// and write whole array to the database.
// Formatting as per writeString.
//
bool QEString::writeStringElement (const QString &data, QString& message)
{
   bool ok = false;
   QVariant elementValue = this->stringFormat->formatValue (data, ok);
   if (ok) {
      this->writeDataElement (elementValue);
   } else {
      message = QString ("Write element failed, unabled to format:'").append (data).append ("'.");
   }
   return ok;
}

//------------------------------------------------------------------------------
//
void QEString::writeStringElement (const QString& data)
{
   QString message;
   bool ok = this->writeStringElement (data, message);
   if (!ok) {
      qDebug() << message;
   }
}

//------------------------------------------------------------------------------
// Take a new string array and write it to the database.
//
bool QEString::writeString (const QVector<QString> &data, QString& message)
{
   bool ok = false;
   QVariant arrayValue = this->stringFormat->formatValue (data, ok);
   if (ok) {
      writeData (arrayValue);
   } else {
      message = QString ("Write element failed, unabled to format string array.");
   }

   return ok;
}

//------------------------------------------------------------------------------
//
void QEString::writeString (const QVector<QString>& data)
{
   QString message;
   bool ok = this->writeString (data, message);
   if (!ok) {
      qDebug() << message;
   }
}

//------------------------------------------------------------------------------
// Take a new value from the database and emit a string,formatted
// as directed by the set of formatting information held by this class
//
void QEString::convertVariant (const QEVariantUpdate& update)
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

   // Set up variable details used by some formatting options.
   //
   this->stringFormat->setDbEgu (this->getEgu());
   this->stringFormat->setDbEnumerations (this->getEnumerations());
   this->stringFormat->setDbPrecision (this->getPrecision());

   // Signaled data structures.
   //
   QEStringValueUpdate valueUpdate;
   QEStringArrayUpdate arrayUpdate;

   // Set up associated meta data members.
   //
   valueUpdate.alarmInfo = arrayUpdate.alarmInfo = update.alarmInfo;
   valueUpdate.timeStamp = arrayUpdate.timeStamp = update.timeStamp;
   valueUpdate.variableIndex = arrayUpdate.variableIndex = update.variableIndex;

   // We need non-const copies as (old) signal parameters are not all const.
   //
   QCaAlarmInfo alarmInfo = update.alarmInfo;
   QCaDateTime  timeStamp = update.timeStamp;

   if (vlist || slist || vector) {
      // The value is some sort of array type.
      //
      const QVector<QString> data = this->stringFormat->formatStringArray (update.value);
      arrayUpdate.values = data;

      emit arrayUpdated (arrayUpdate);
      emit stringArrayChanged (data, alarmInfo, timeStamp, update.variableIndex);

      // Extract the scalar value,
      //
      const int ai = this->getArrayIndex ();
      const int count = vlist ? update.value.toList ().count () : (
                        slist ? update.value.toStringList().count () :
                        QEVectorVariants::vectorCount (update.value));

      if ((ai >= 0) && (ai < count)) {
         // Convert this array element as a scalar update.
         //
         const QString item = this->stringFormat->formatString (update.value, ai);
         valueUpdate.value = item;

         emit valueUpdated (valueUpdate);
         emit stringChanged (item, alarmInfo, timeStamp, update.variableIndex);
      }

   } else {
      // The value is a scalar type.
      //
      const QString item = this->stringFormat->formatString (update.value, 0);
      valueUpdate.value = item;

      emit valueUpdated (valueUpdate);
      emit stringChanged (item, alarmInfo, timeStamp, update.variableIndex);

      // A scalar is also an array with one element.
      //
      QVector<QString> array;
      array.append (item);
      arrayUpdate.values = array;

      emit arrayUpdated (arrayUpdate);
      emit stringArrayChanged (array, alarmInfo, timeStamp, update.variableIndex);
   }
}

//------------------------------------------------------------------------------
//
static bool registerMetaTypes()
{
   qRegisterMetaType<QEStringValueUpdate> ("QEStringValueUpdate");
   qRegisterMetaType<QEStringArrayUpdate> ("QEStringArrayUpdate");
   return true;
}

static const bool elaborate = registerMetaTypes();

// end
