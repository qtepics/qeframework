/*  QECaClient.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (C) 2018-2020 Australian Synchrotron
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

#include "QECaClient.h"
#include <QApplication>
#include <QDebug>
#include <QCaObject.h>

#define DEBUG qDebug () << "QECaClient" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QECaClient::QECaClient (const QString& pvNameIn,
                        qcaobject::QCaObject* parent) :
   QEBaseClient (QEBaseClient::CAType, pvNameIn, parent),
   ACAI::Client (pvNameIn.toStdString())
{
   QECaClientManager::initialise ();   // idempotent
}

//------------------------------------------------------------------------------
//
QECaClient::~QECaClient ()
{
   this->closeChannel ();
}

//------------------------------------------------------------------------------
//
generic::generic_types QECaClient::getDataType() const
{
   using namespace generic;
   generic::generic_types result = GENERIC_UNKNOWN;

   const ACAI::ClientFieldType fieldType = this->hostFieldType ();
   switch (fieldType) {
      case ACAI::ClientFieldSTRING:  result = GENERIC_STRING; break;
      case ACAI::ClientFieldSHORT:   result = GENERIC_SHORT;  break;
      case ACAI::ClientFieldFLOAT:   result = GENERIC_FLOAT;  break;
      case ACAI::ClientFieldENUM:    result = GENERIC_SHORT;  break;
      case ACAI::ClientFieldCHAR:    result = GENERIC_UNSIGNED_CHAR; break;
      case ACAI::ClientFieldLONG:    result = GENERIC_LONG;   break;
      case ACAI::ClientFieldDOUBLE:  result = GENERIC_DOUBLE; break;
      default: break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
QVariant QECaClient::getPvData () const
{
   QVariant result = QVariant (QVariant::Invalid);  // default

   if (!this->dataIsAvailable ()) return result;

   const ACAI::ClientFieldType fieldType = this->dataFieldType ();
   const unsigned int number = this->dataElementCount ();

   // Specials.
   //
   if (this->processingAsLongString()) {
      result = QVariant (QString::fromStdString (this->getString (0)));
      return result;
   }

   if (number == 1) {
      // Treat as a scaler
      //
      switch (fieldType) {

         case ACAI::ClientFieldSTRING:
            result = QVariant (QString::fromStdString (this->getString (0)));
            break;

         case ACAI::ClientFieldCHAR:
            result = QVariant (qlonglong (this->getInteger (0)));
            break;

            // We treat enums as integer here - QEStringFormatting does the converion,
            // maybe using a local enumeration lookup.
            //
         case ACAI::ClientFieldENUM:
         case ACAI::ClientFieldSHORT:
         case ACAI::ClientFieldLONG:
            result = QVariant (qlonglong (this->getInteger (0)));
            break;

         case ACAI::ClientFieldFLOAT:
         case ACAI::ClientFieldDOUBLE:
            result = QVariant (this->getFloating (0));
            break;

         default:
            break;
      }

   } else {
      // Treat as an array
      //
      QVariantList list;

      switch (fieldType) {

         case ACAI::ClientFieldSTRING:
            for (unsigned int j = 0; j < number; j++) {
               list.append (QVariant (QString::fromStdString (this->getString (j))));
            }
            break;

         case ACAI::ClientFieldCHAR:
            for (unsigned int j = 0; j < number; j++) {
               list.append (QVariant (qlonglong (this->getInteger (j))));
            }
            break;

         case ACAI::ClientFieldENUM:
         case ACAI::ClientFieldSHORT:
         case ACAI::ClientFieldLONG:
            for (unsigned int j = 0; j < number; j++) {
               list.append (QVariant (qlonglong (this->getInteger (j))));
            }
            break;

         case ACAI::ClientFieldFLOAT:
         case ACAI::ClientFieldDOUBLE:
            for (unsigned int j = 0; j < number; j++) {
               list.append (QVariant (this->getFloating (j)));
            }
            break;

         default:
            break;
      }
      result = list;
   }

   return result;
}

//------------------------------------------------------------------------------
// Write a data out to channel
//
bool QECaClient::putPvData (const QVariant& value)
{
   const QVariant::Type vtype = value.type ();

   bool result = false;
   bool knownType = true;  // hypothesize we can handle the variant type.
   bool okay;
   QString extra = "";

   if (vtype != QVariant::List) {
      // Process as scaler
      //      
      ACAI::ClientInteger i;
      ACAI::ClientFloating f;
      QByteArray bytes;

      switch (vtype) {
         case QVariant::Char:
         case QVariant::Bool:
         case QVariant::Int:
            i = value.toInt (&okay);
            if (okay) {
               result = this->putInteger (i);
            }
            break;

         case QVariant::UInt:
         case QVariant::LongLong:
         case QVariant::ULongLong:
            f = value.toDouble (&okay);
            if (okay) {
               if ((f >= -2147483648.0) && (f < 2147483647.0)) {
                  i = value.toInt (&okay);
                  if (okay) {
                     result = this->putInteger (i);
                  }
               } else {
                  // Too big for int - use double.
                  result = this->putFloating (f);
               }
            }
            break;

         case QVariant::Double:
            f = value.toDouble (&okay);
            if (okay) {
               result = this->putFloating (f);
            }
            break;

         case QVariant::String:
            result = this->putString (value.toString ().toStdString ());
            break;

         case QVariant::ByteArray:
            bytes = value.toByteArray ();
            // NOTE: requires acai 1-5-8 orlater.
            result = this->putByteArray ((void*) bytes.constData (), bytes.size());
            break;

         default:
            result = false;
            knownType = false;
            break;
      }
      extra = QString(", type %1.").arg (value.typeName ());

   } else {
      // Process as array.
      //
      QVariantList valueArray = value.toList ();
      const int number = valueArray.count ();

      ACAI::ClientFloatingArray fltArray;
      ACAI::ClientIntegerArray  intArray;
      ACAI::ClientStringArray   strArray;

      // Use type of first element to determine type.
      // We only cater of lists of basic types.
      //
      QVariant firstValue = valueArray.value (0);
      switch (firstValue.type ()) {
         case QVariant::Char:
         case QVariant::Bool:
         case QVariant::Int:
            for (int j = 0; j < number; j++) {
               intArray.push_back (valueArray.value (j).toInt ());
            }
            result = this->putIntegerArray (intArray);
            break;

         case QVariant::UInt:
         case QVariant::LongLong:
         case QVariant::ULongLong:
         case QVariant::Double:
            for (int j = 0; j < number; j++) {
               fltArray.push_back (valueArray.value (j).toDouble ());
            }
            result = this->putFloatingArray (fltArray);
            break;

         case QVariant::String:
            for (int j = 0; j < number; j++) {
               strArray.push_back (valueArray.value (j).toString ().toStdString ());
            }
            result = this->putStringArray (strArray);
            break;

         default:
            result = false;
            knownType = false;
            break;
      }
      extra = QString(" list of %1.").arg (firstValue.typeName ());
   }

   // Report error - if we can.
   //
   if (!result) {
      QString msg( this->cPvName() );
      msg.append( " Put channel failed: " );

      QString data = value.toString();
      if( data.length() > 40 ){
         data = data.left (18) + "..." + data.right( 18 );
      }
      msg.append (data);
      msg.append (extra);

      if( !knownType ){
         msg.append (" Unhandled varient type.");
      }
      else if (!this->isConnected()) {
         msg.append (" Channel disconnected.");

      } else if (!this->writeAccess()) {
         msg.append (" Channel has no write access.");

      } else {
         msg.append (" Unknown error");
      }

      UserMessage* user_msg = this->getUserMessage();
      if (user_msg) {
         user_msg->sendMessage (msg, "QCaObject::writeData()",
                                message_types ( MESSAGE_TYPE_ERROR ));
      } else {
         DEBUG << msg;
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
QStringList QECaClient::getEnumerations() const
{
   QStringList enumerations;

   const int n = this->enumerationStatesCount ();
   for (int j = 0; j < n; j++) {
      enumerations.append (QString::fromStdString (this->getEnumeration (j)));
   }

   return enumerations;
}

//------------------------------------------------------------------------------
//
QCaAlarmInfo QECaClient::getAlarmInfo () const
{
   const QCaAlarmInfo::Status   status   = (QCaAlarmInfo::Status) this->alarmStatus ();
   const QCaAlarmInfo::Severity severity = (QCaAlarmInfo::Severity) this->alarmSeverity ();

   // TODO: Prefix with "ca://" ??
   //
   return QCaAlarmInfo (this->getPvName(), status, severity, "");
}

//------------------------------------------------------------------------------
//
QCaDateTime QECaClient::getTimeStamp () const
{
   const ACAI::ClientTimeStamp updateTime = this->timeStamp ();
   return QCaDateTime (updateTime.secPastEpoch, updateTime.nsec, 0);
}

//------------------------------------------------------------------------------
//
void QECaClient::connectionUpdate (const bool isConnected)
{
   emit this->connectionUpdated (isConnected);
}

//------------------------------------------------------------------------------
//
void QECaClient::dataUpdate (const bool firstUpdate)
{
    emit this->dataUpdated (firstUpdate);
}

//------------------------------------------------------------------------------
//
void QECaClient::putCallbackNotifcation (const bool isSuccessful)
{
    emit this->putCallbackComplete (isSuccessful);
}


//==============================================================================
// Helper class: QECaClientTimer
//==============================================================================
//
// This object created when first needed, and not before.
// Just declaring as a regular object doesn't work.
//
static QECaClientManager* singleton = NULL;

//------------------------------------------------------------------------------
// static
void QECaClientManager::initialise ()
{
   if (!singleton) {     // Mutex ??
      singleton = new QECaClientManager ();
   }
}

//------------------------------------------------------------------------------
// static
void QECaClientManager::notificationHandlers (const char* notification)
{
   UserMessage userMessage;
   userMessage.sendMessage (notification, message_types (MESSAGE_TYPE_ERROR));
}

//------------------------------------------------------------------------------
// constructor
//
QECaClientManager::QECaClientManager () : QTimer (NULL)
{
   ACAI::Client::initialise ();
   ACAI::Client::setNotificationHandler (QECaClientManager::notificationHandlers);

   // Connect to the about to quit signal.
   // Note: qApp is defined in QApplication
   //
   QObject::connect (qApp, SIGNAL (aboutToQuit ()),
                     this, SLOT   (aboutToQuitHandler ()));

   // Connect and start regular timed event.
   //
   QObject::connect (this, SIGNAL (timeout ()),
                     this, SLOT   (timeoutHandler ()));

   this->start (16);   // 16 mSec ~ 60 Hz.
}

//------------------------------------------------------------------------------
// destructor - place holder
//
QECaClientManager::~QECaClientManager () { }

//------------------------------------------------------------------------------
//
void QECaClientManager::timeoutHandler ()
{
   // The ACAI package requires a regular poll.
   //
   ACAI::Client::poll ();
}

//------------------------------------------------------------------------------
//
void QECaClientManager::aboutToQuitHandler ()
{
   this->stop ();  // Stop the timer.
   ACAI::Client::finalise ();
}

// end
