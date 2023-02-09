/*  QECaClient.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (C) 2018-2023 Australian Synchrotron
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
#include <QTimer>
#include <acai_version.h>
#include <QEPvNameUri.h>
#include <QERecordFieldName.h>

#define DEBUG qDebug () << "QECaClient" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QECaClient::QECaClient (const QString& pvNameIn,
                        QObject* parent) :
   QEBaseClient (QEBaseClient::CAType, pvNameIn, parent),
   ACAI::Client (pvNameIn.toStdString())
{
   this->descriptionClient = NULL;
   QECaClientManager::initialise ();   // idempotent
}

//------------------------------------------------------------------------------
//
QECaClient::~QECaClient ()
{
   this->closeChannel ();
}

//------------------------------------------------------------------------------
// Form DESCription PV name and request data.
// Note: the assumption here is that the PV is a the name of a record or a
// record field hosted on an IOC. However if this is a PV hosted on a Portable
// Channel Access Server (PCAS) such as a gateway generated PV or a pycas PV,
// the <name>.DESC PV may not exist.
//
void QECaClient::requestDescription ()
{
   if (!this->descriptionClient) {
      QString pvName = this->getPvName();

      if (pvName.endsWith (".DESC")) {
         // This client is already looking at a description field.
         //
         this->descriptionClient = this;
      } else {
         QString descPvName = QERecordFieldName::fieldPvName (pvName, "DESC");
         this->descriptionClient = new QECaClient (descPvName, this);
         this->descriptionClient->openChannel();
      }
   }
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
// Write a data out to channel.
// We convert to the hold field type here - if we can.
//
bool QECaClient::putPvData (const QVariant& value)
{
   const QVariant::Type vtype = value.type ();
   const ACAI::ClientFieldType fieldType = this->hostFieldType ();
   const QString fieldName = clientFieldTypeImage (fieldType).c_str();

   // Note: min and max field value functions require ACAI 1.6.4 or later.
   //
   #if (ACAI_VERSION < ACAI_INT_VERSION(1, 6, 4))
   #error The EPICS Qt framework required ACAI 1.6.4 or later.
   #endif

   const double min = this->minFieldValue();
   const double max = this->maxFieldValue();

   bool result = true;        // hypothesize success
   bool knownType = true;     // hypothesize we can handle the variant type.
   bool valueOkay = true;     // hypothesize value coverts to numeric type
   bool valueInRange = true;  // hypothesize value in field type range
   QString extra = "";

// DEBUG << this->getPvName() << fieldName << min  << max;

   if (vtype != QVariant::List) {
      // Process as scaler
      //
      ACAI::ClientInteger i;
      ACAI::ClientFloating f;
      QByteArray bytes;

      switch (fieldType) {
         case ACAI::ClientFieldSTRING:
            result = this->putString (value.toString ().toStdString ());
            break;

         case ACAI::ClientFieldENUM:
         case ACAI::ClientFieldCHAR:
         case ACAI::ClientFieldSHORT:
         case ACAI::ClientFieldLONG:
            f = value.toDouble (&valueOkay);
            if (!valueOkay) {
               result = false;
               break;
            }

            if ((f < min) || (f > max)) {
               valueInRange = false;
               result = false;
               break;
            }

            i = value.toInt (&valueOkay);
            if (!valueOkay) {
               result = false;
               break;
            }

            result = this->putInteger (i);
            break;

         case ACAI::ClientFieldFLOAT:
         case ACAI::ClientFieldDOUBLE:
            f = value.toDouble (&valueOkay);
            if (!valueOkay) {
               result = false;
               break;
            }

            if ((f < min) || (f > max)) {
               valueInRange = false;
               result = false;
               break;
            }

            result = this->putFloating (f);
            break;

         default:
            result = false;
            knownType = false;
            break;
      }

      extra = QString(", source type %1.").arg (value.typeName ());

   } else {
      // Process as array.
      //
      const QVariantList valueArray = value.toList ();
      const int number = valueArray.count ();
      const QVariant firstValue = valueArray.value (0);

      ACAI::ClientFloatingArray fltArray;
      ACAI::ClientIntegerArray  intArray;
      ACAI::ClientStringArray   strArray;

      switch (fieldType) {
         case ACAI::ClientFieldSTRING:
            for (int j = 0; j < number; j++) {
               strArray.push_back (valueArray.value (j).toString ().toStdString ());
            }
            result = this->putStringArray (strArray);
            break;

         case ACAI::ClientFieldENUM:
         case ACAI::ClientFieldCHAR:
         case ACAI::ClientFieldSHORT:
         case ACAI::ClientFieldLONG:
            for (int j = 0; j < number; j++) {
               ACAI::ClientFloating f;
               f = valueArray.value (j).toDouble (&valueOkay);
               if (!valueOkay) {
                  result = false;
                  break;
               }

               if ((f < min) || (f > max)) {
                  valueInRange = false;
                  result = false;
                  break;
               }

               ACAI::ClientInteger i;
               i = valueArray.value (j).toInt (&valueOkay);
               if (!valueOkay) {
                  result = false;
                  break;
               }

               intArray.push_back (i);
            }
            if (result) {
               result = this->putIntegerArray (intArray);
            }
            break;

         case ACAI::ClientFieldFLOAT:
         case ACAI::ClientFieldDOUBLE:
            for (int j = 0; j < number; j++) {
               ACAI::ClientFloating f;
               f = valueArray.value (j).toDouble (&valueOkay);
               if (!valueOkay) {
                  result = false;
                  break;
               }

               if ((f < min) || (f > max)) {
                  valueInRange = false;
                  result = false;
                  break;
               }

               fltArray.push_back (f);
            }
            if (result) {
               result = this->putFloatingArray (fltArray);
            }
            break;

         default:
            result = false;
            knownType = false;
            break;
      }

      extra = QString(" source list of %1.").arg (firstValue.typeName ());
   }

   // Report error - use the UserMessage system if we can otherwise use qDebug.
   //
   if (!result) {
      QString msg (this->getPvName());
      msg.append (" Put channel failed: ");

      QString data = value.toString();
      if (data.length() > 40) {
         data = data.left (18) + "..." + data.right (18);
      }
      msg.append (data);
      msg.append (extra);

      if (!valueOkay) {
         msg.append (" Value is not numeric.");
      }
      else if (!valueInRange) {
         msg.append (" Value out of range for ");
         msg.append (fieldName);
         msg.append (" field type.");
      }
      else if (!knownType) {
         msg.append (" Unhandled field type ");
         msg.append (fieldName);
         msg.append (".");
      }
      else if (!this->isConnected()) {
         msg.append (" Channel disconnected.");

      } else if (!this->writeAccess()) {
         msg.append (" Channel has no write access.");

      } else {
         msg.append (" Unknown error.");
      }

      UserMessage* user_msg = this->getUserMessage();
      if (user_msg) {
         user_msg->sendMessage (msg, "QECaClient::putPvData()",
                                message_types (MESSAGE_TYPE_ERROR));
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

   return QCaAlarmInfo (QEPvNameUri::ca, this->getPvName(), status, severity, "");
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
QString QECaClient::getDescription () const
{
   QString result;
   if (this->descriptionClient) {
      result = QString::fromStdString (this->descriptionClient->getString());
   } else {
      // We use a slot, mainly to overcome the const qualifier error.
      //
      QTimer::singleShot (0, this, SLOT (requestDescription ()));
   }
   return result;
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
   if (!singleton) {   // Mutex needed ??
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
QECaClientManager::QECaClientManager () : QObject (NULL)
{
   this->stillRunning = true;
   ACAI::Client::initialise ();
   ACAI::Client::setNotificationHandler (QECaClientManager::notificationHandlers);

   // Connect to the about to quit signal.
   // Note: qApp is defined in QApplication
   //
   QObject::connect (qApp, SIGNAL (aboutToQuit ()),
                     this, SLOT   (aboutToQuitHandler ()));

   // Schedule first poll event.
   //
   QTimer::singleShot (1, this, SLOT (timeoutHandler ()));
}

//------------------------------------------------------------------------------
// destructor - place holder
//
QECaClientManager::~QECaClientManager () { }

//------------------------------------------------------------------------------
//
void QECaClientManager::timeoutHandler ()
{
   if (!this->stillRunning) return;

   // The ACAI package requires a regular poll.
   // Catch any exceptions here.
   //
   try {
      ACAI::Client::poll ();
   }
   catch (...) {
      DEBUG << ": poll exception.";
   }

   // Schedule another poll event - 16 mS approx 60Hz.
   // Note: the delay is relative to the end of processing the poll function.
   //
   QTimer::singleShot (16, this, SLOT (timeoutHandler ()));
}

//------------------------------------------------------------------------------
//
void QECaClientManager::aboutToQuitHandler ()
{
   this->stillRunning = false;
   ACAI::Client::finalise ();
}

// end
