/*  QEFloating.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2025 Australian Synchrotron
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
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

// Floating specific wrapper for QCaObject variant data.

#include "QEFloating.h"
#include <QDebug>
#include <QMetaType>
#include <QStringList>
#include <QEPlatform.h>
#include <QEVectorVariants.h>

#define DEBUG  qDebug () << "QEFloating" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QEFloating::QEFloating (QString pvName, QObject* eventObject,
                        QEFloatingFormatting* floatingFormattingIn,
                        unsigned int variableIndexIn) :
   QCaObject (pvName, eventObject, variableIndexIn)
{
   this->initialise (floatingFormattingIn);
}

//------------------------------------------------------------------------------
//
QEFloating::QEFloating (QString pvName, QObject* eventObject,
                        QEFloatingFormatting* floatingFormattingIn,
                        unsigned int variableIndexIn,
                        UserMessage* userMessageIn) :
   QCaObject (pvName, eventObject, variableIndexIn, userMessageIn)
{
   this->initialise (floatingFormattingIn);
}

//------------------------------------------------------------------------------
// Stream the QCaObject data through this class to generate floating data updates
//
void QEFloating::initialise (QEFloatingFormatting* floatingFormattingIn)
{
   this-> floatingFormat = floatingFormattingIn;

   QObject::connect (this, SIGNAL  (dataChanged (const QVariant&, QCaAlarmInfo&, QCaDateTime& , const unsigned int&)),
                     this, SLOT (convertVariant (const QVariant&, QCaAlarmInfo&, QCaDateTime& , const unsigned int&)));
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
// Slot to recieve data updates from the base QCaObject and generate floating updates.
//
void QEFloating::convertVariant (const QVariant &value, QCaAlarmInfo& alarmInfo,
                                 QCaDateTime& timeStamp, const unsigned int& variableIndex)
{
   const QMetaType::Type mtype = QEPlatform::metaType (value);

   // The expected varient type is one of:
   // a/ scalar
   // b/ QVariant::List
   // c/ QStringList
   // d/ one of the QEVectorVariants type.
   //
   const bool vlist = (mtype == QMetaType::QVariantList);
   const bool slist = (mtype == QMetaType::QStringList);
   const bool vector = QEVectorVariants::isVectorVariant (value);

   if (vlist || slist || vector) {
      // The value is some sort of array type.
      //
      emit floatingArrayChanged (this->floatingFormat->formatFloatingArray (value),
                                 alarmInfo, timeStamp, variableIndex);

      // Extract the scalar value,
      //
      const int ai = this->getArrayIndex ();
      const int count = vlist ? value.toList ().count () : (
                        slist ? value.toStringList().count () :
                        QEVectorVariants::vectorCount (value));

      if ((ai >= 0) && (ai < count)) {
         // Convert this array element as a scalar update.
         //
         const double item = floatingFormat->formatFloating (value, ai);
         emit floatingChanged (item, alarmInfo, timeStamp, variableIndex);
      }
   } else {
      // The value is a scalar type.
      //
      const double formatted = this->floatingFormat->formatFloating (value);
      emit floatingChanged (formatted, alarmInfo, timeStamp, variableIndex);

      // A scalar is also an array with one element.
      //
      QVector<double> array;
      array.append (formatted);
      emit floatingArrayChanged (array, alarmInfo, timeStamp, variableIndex);
   }
}

// end
