/*  QEEmitter.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2015-2024 Australian Synchrotron
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
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>
#include <QObject>
#include <QMetaObject>
#include <QVariant>

#include <QECommon.h>
#include <QCaObject.h>
#include <QEWidget.h>

#include <QEEmitter.h>

// Signal identifiers.
//
enum filterKinds {
   fkConnected = 0,
   fkUpdateEvent,   // no data per se
   fkInt, fkLong, fkLongLong, fkString, fkDouble, fkBool
};

//------------------------------------------------------------------------------
//
QEEmitter::QEEmitter (QEWidget* qewIn, QWidget* ownerIn)
{
   this->owner = ownerIn;
   this->qew = qewIn;

   for (int j = 0; j < ARRAY_LENGTH (this->filter); j++) {
      this->filter [j] = false;  // assume not allowed until we find out otherwise.
   }
   this->setupFilterComplete = false;
}

//------------------------------------------------------------------------------
//
QEEmitter::~QEEmitter ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
#define SIGNAL_EXISTS(member)   \
   (meta->indexOfSignal(QMetaObject::normalizedSignature (member)) >= 0)


void QEEmitter::setupFilter ()
{
   if (this->setupFilterComplete) return; // all done

   if (!this->owner) return;              // sainity check
   const QMetaObject* meta = this->owner->metaObject ();
   if (!meta) return;                     // sainity check

   // Find out which signal exists for this particular object.
   //
   this->filter [fkConnected]   = SIGNAL_EXISTS ("dbConnectionChanged (const bool&)");
   this->filter [fkUpdateEvent] = SIGNAL_EXISTS ("dbValueChanged ()");
   this->filter [fkInt]         = SIGNAL_EXISTS ("dbValueChanged (const int&)");
   this->filter [fkLong]        = SIGNAL_EXISTS ("dbValueChanged (const long&)");
   this->filter [fkLongLong]    = SIGNAL_EXISTS ("dbValueChanged (const qlonglong&)");
   this->filter [fkString]      = SIGNAL_EXISTS ("dbValueChanged (const QString&)");
   this->filter [fkDouble]      = SIGNAL_EXISTS ("dbValueChanged (const double&)");
   this->filter [fkBool]        = SIGNAL_EXISTS ("dbValueChanged (const bool&)");

   this->setupFilterComplete = true;
}

#undef SIGNAL_EXISTS

//------------------------------------------------------------------------------
//
void QEEmitter::emitDbConnectionChanged (const unsigned int variableIndex)
{
   if (!this->owner) return;
   const QMetaObject* meta = this->owner->metaObject ();
   if (!meta) return;

   if (!this->qew) return;
   qcaobject::QCaObject* qca = this->qew->getQcaItem (variableIndex);
   if (!qca) return;

   // Passed the sainity checks - let's start in earnest.
   //
   const char* member = "dbConnectionChanged";
   bool isConnected;

   this->setupFilter ();

   // Extract connection state.
   //
   if (this->filter [fkConnected]) {
      isConnected = qca->getChannelIsConnected ();
      const auto arg = Q_ARG (bool, isConnected);
      meta->invokeMethod (this->owner, member, Qt::DirectConnection, arg);
   }
}

//------------------------------------------------------------------------------
//
void QEEmitter::emitDbValueChangedPrivate (const bool useFormmattedText,
                                           const QString& formattedText,
                                           const unsigned int variableIndex)
{
   if (!this->owner) return;
   const QMetaObject* meta = this->owner->metaObject ();
   if (!meta) return;

   if (!this->qew) return;
   qcaobject::QCaObject* qca = this->qew->getQcaItem (variableIndex);
   if (!qca) return;

   // Passed the sainity checks - let's start in earnest.
   //
   const char* member = "dbValueChanged";

   bool isDefined;
   QVariant value;
   QCaAlarmInfo alarmInfo;
   QCaDateTime timeStamp;
   bool okay;

   this->setupFilter ();

   // Extract actual value.
   //
   qca->getLastData (isDefined, value, alarmInfo, timeStamp);

   if (!isDefined) return;    // don't send nonsense.

   // Is this a list?
   //
   if (value.type () == QVariant::List) {
      // Extract appropriate value.
      //
      int ai = qca->getArrayIndex ();
      if (ai < 0 || ai >= value.toList().count()) {
         // out of range
         return;
      }

      // Convert this array element as a scalar update.
      //
      value = value.toList().value (ai);
   }

   if (this->filter [fkUpdateEvent]) {
      // No argument - just a notification that an update has occured.
      //
      meta->invokeMethod (this->owner, member, Qt::DirectConnection);
   }

   const double dValue = value.toDouble (&okay);   // Extarct value as double.

   // Did we successfully extract a double value?
   // Is the signal not inhibited (yet)?
   //
   if (okay && this->filter [fkDouble]) {
      // Good to go - create required argument.
      //
      const auto arg = Q_ARG (double, dValue);
      meta->invokeMethod (this->owner, member, Qt::DirectConnection, arg);
   }

   // Use the double value to determine true/false.
   //
   if (okay && this->filter [fkBool]) {
      const bool bValue = (dValue != 0.0);
      const auto arg = Q_ARG (bool, bValue);
      meta->invokeMethod (this->owner, member, Qt::DirectConnection, arg);
   }

   const int iValue = value.toInt (&okay);
   if (okay && this->filter [fkInt]) {
      const auto arg = Q_ARG (int, iValue);
      meta->invokeMethod (this->owner, member, Qt::DirectConnection, arg);
   }

   if (okay && this->filter [fkLong]) {
      const long lValue = (long) iValue;
      const auto arg = Q_ARG (long, lValue);
      meta->invokeMethod (this->owner, member, Qt::DirectConnection, arg);
   }

   const qlonglong llValue = value.toLongLong (&okay);
   if (okay && this->filter [fkLongLong]) {
      const auto arg = Q_ARG (qlonglong, llValue);
      meta->invokeMethod (this->owner, member, Qt::DirectConnection, arg);
   }

   // If used has specificed a "nice" format, then use it else just let QVariant sort it out.
   //
   const QString sValue = useFormmattedText ? formattedText : value.toString ();
   if (this->filter [fkString]) {
      const auto arg = Q_ARG (QString, sValue);
      meta->invokeMethod (this->owner, member, Qt::DirectConnection, arg);
   }
}

//------------------------------------------------------------------------------
//
void QEEmitter::emitDbValueChanged (const unsigned int variableIndex)
{
   this->emitDbValueChangedPrivate (false, "", variableIndex);
}

//------------------------------------------------------------------------------
//
void QEEmitter::emitDbValueChanged (const QString& formatedText,
                                    const unsigned int variableIndex)

{
   this->emitDbValueChangedPrivate (true, formatedText, variableIndex);
}

// end
