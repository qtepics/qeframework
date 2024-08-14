/*  QECaClient.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (C) 2018-2024 Australian Synchrotron
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
#include <QEVectorVariants.h>

#define DEBUG qDebug () << "QECaClient" << __LINE__ << __FUNCTION__ << "  "

//==============================================================================
// QE_ACAI_Client
//==============================================================================
//
// Its sole purpose is to override connectionUpdate, dataUpdate and
// putCallbackNotifcation.
//
class QE_ACAI_Client : public ACAI::Client {
public:
  explicit  QE_ACAI_Client (const QString& pvName, QECaClient* owner);
   ~QE_ACAI_Client ();

protected:
   // Override ACAI::Client parent class functions.
   //
   void connectionUpdate (const bool isConnected);
   void dataUpdate (const bool firstUpdate);
   void putCallbackNotifcation (const bool isSuccessful);
private:
   QECaClient* const owner;   // ptr is const, not the object itself.
};

//------------------------------------------------------------------------------
//
QE_ACAI_Client::QE_ACAI_Client (const QString& pvName,
                                QECaClient* ownerIn) :
   ACAI::Client (pvName.toStdString()),
   owner(ownerIn)
{ }

//------------------------------------------------------------------------------
//
QE_ACAI_Client::~QE_ACAI_Client () { }

//------------------------------------------------------------------------------
// Override ACAI::Client parent class functions.
//
void QE_ACAI_Client::connectionUpdate (const bool isConnected)
{
   if (this->owner) this->owner->connectionUpdate (isConnected);
}

//------------------------------------------------------------------------------
//
void QE_ACAI_Client::dataUpdate (const bool firstUpdate)
{
   if (this->owner) this->owner->dataUpdate (firstUpdate);
}

//------------------------------------------------------------------------------
//
void QE_ACAI_Client::putCallbackNotifcation (const bool isSuccessful)
{
   if (this->owner) this->owner->putCallbackNotifcation (isSuccessful);
}


//==============================================================================
// QECaClient
//==============================================================================
//
QECaClient::QECaClient (const QString& pvNameIn,
                        QObject* parent) :
   QEBaseClient (QEBaseClient::CAType, pvNameIn, parent),
   mainClient (new QE_ACAI_Client (pvNameIn, this))
{
   this->descClient = NULL;     // we don't create unless requested.
   QECaClientManager::initialise ();   // idempotent
}

//------------------------------------------------------------------------------
//
QECaClient::~QECaClient ()
{
   // valueClient and descriptionClient have an owner(this) but not a parent,
   // so we must explicitly delete these QE_ACAI_Client objects.
   //
   this->mainClient->closeChannel ();

   if (this->descClient && (this->descClient != this->mainClient)) {
      // This is a separate client
      this->descClient->closeChannel();
      delete this->descClient;
      this->descClient = NULL;
   }

   delete this->mainClient;
}

//------------------------------------------------------------------------------
// Form DESCription PV name and request data.
// Note: the assumption here is that the PV is a the name of a record or a
// record field hosted on an IOC. However if this is a PV hosted on a Portable
// Channel Access Server (PCAS) such as a gateway generated PV or a pycas PV,
// the <name>.DESC PV may not exist, or if it does it may not actually be
// a description.
//
void QECaClient::requestDescription ()
{
   if (!this->descClient) {
      QString pvName = this->getPvName();

      if (pvName.endsWith (".DESC")) {
         // This client is already looking at a description field.
         //
         this->descClient = this->mainClient;
      } else {
         QString descPvName = QERecordFieldName::fieldPvName (pvName, "DESC");
         this->descClient = new QE_ACAI_Client (descPvName, this);
         this->descClient->openChannel();
      }
   }
}

//------------------------------------------------------------------------------
//
bool QECaClient::openChannel (const ChannelModesFlags modes)
{
   ACAI::ReadModes readMode;

   if (modes == QEBaseClient::None) {
      return false;
   }

   if (modes & QEBaseClient::Write) {
      // just connects, however allows writes
      readMode = ACAI::NoRead;
   }

   if (modes & QEBaseClient::Read) {
      // single one-off read only and allows writes
      readMode = ACAI::SingleRead;
   }

   if (modes & QEBaseClient::Monitor) {
      // read plus subscription and allows writes
      readMode = ACAI::Subscribe;
   }

   this->mainClient->setReadMode (readMode);
   return this->mainClient->openChannel ();
}

//------------------------------------------------------------------------------
//
void QECaClient::closeChannel ()
{
   this->mainClient->closeChannel ();
}

//------------------------------------------------------------------------------
//
bool QECaClient::getIsConnected () const
{
   return this->mainClient->isConnected ();
}

//------------------------------------------------------------------------------
//
bool QECaClient::dataIsAvailable () const
{
   return this->mainClient->dataIsAvailable();
}

//------------------------------------------------------------------------------
//
QString QECaClient::getId () const
{
   ACAI::ClientFieldType ft = this->mainClient->hostFieldType();
   return QString::fromStdString (ACAI::clientFieldTypeImage (ft));
}

//------------------------------------------------------------------------------
//
const void* QECaClient::getRawDataPointer (size_t& count,
                                           const size_t offset) const
{
   return this->mainClient->rawDataPointer (count, offset);
}

//------------------------------------------------------------------------------
//
QVariant QECaClient::getPvData () const
{
   QVariant result = QVariant (QVariant::Invalid);  // default

   if (!this->dataIsAvailable ()) return result;

   const ACAI::ClientFieldType fieldType = this->mainClient->dataFieldType ();
   const unsigned int number = this->dataElementCount ();

   // Specials.
   //
   if (this->mainClient->processingAsLongString()) {
      result = QVariant (QString::fromStdString (this->mainClient->getString (0)));
      return result;
   }

   if (number == 1) {
      // Treat as a scaler
      //
      switch (fieldType) {

         case ACAI::ClientFieldSTRING:
            result = QVariant (QString::fromStdString (this->mainClient->getString (0)));
            break;

         case ACAI::ClientFieldCHAR:
            result = QVariant (qlonglong (this->mainClient->getInteger (0)));
            break;

            // We treat enums as integer here - QEStringFormatting does the converion,
            // maybe using a local enumeration lookup.
            //
         case ACAI::ClientFieldENUM:
         case ACAI::ClientFieldSHORT:
         case ACAI::ClientFieldLONG:
            result = QVariant (qlonglong (this->mainClient->getInteger (0)));
            break;

         case ACAI::ClientFieldFLOAT:
         case ACAI::ClientFieldDOUBLE:
            result = QVariant (this->mainClient->getFloating (0));
            break;

         default:
            break;
      }

   } else {
      // Treat as an array
      // Consider using own QEVectorVariants
      //
      QVariantList list;

      switch (fieldType) {

         case ACAI::ClientFieldSTRING:
            for (unsigned int j = 0; j < number; j++) {
               list.append (QVariant (QString::fromStdString (this->mainClient->getString (j))));
            }
            break;

         case ACAI::ClientFieldCHAR:
            for (unsigned int j = 0; j < number; j++) {
               list.append (QVariant (qlonglong (this->mainClient->getInteger (j))));
            }
            break;

         case ACAI::ClientFieldENUM:
         case ACAI::ClientFieldSHORT:
         case ACAI::ClientFieldLONG:
            for (unsigned int j = 0; j < number; j++) {
               list.append (QVariant (qlonglong (this->mainClient->getInteger (j))));
            }
            break;

         case ACAI::ClientFieldFLOAT:
         case ACAI::ClientFieldDOUBLE:
            for (unsigned int j = 0; j < number; j++) {
               list.append (QVariant (this->mainClient->getFloating (j)));
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
   const ACAI::ClientFieldType fieldType = this->mainClient->hostFieldType ();
   const QString fieldName = clientFieldTypeImage (fieldType).c_str();

   // Note: min and max field value functions require ACAI 1.6.4 or later.
   //
   #if (ACAI_VERSION < ACAI_INT_VERSION(1, 6, 4))
   #error The EPICS Qt framework requires ACAI 1.6.4 or later.
   #endif

   bool result = true;        // hypothesize success
   bool knownType = true;     // hypothesize we can handle the variant type.
   bool valueOkay = true;     // hypothesize value converts to numeric type
   bool valueInRange = true;  // hypothesize value in field type range
   QString extra = "";

   // Do special for ByteArray type (see GUI-216)
   //
   if (vtype == QVariant::ByteArray) {
      QByteArray bytes = value.toByteArray ();
      // NOTE: requires acai 1-5-8 orlater.
      result = this->mainClient->putByteArray ((void*) bytes.constData (), bytes.size());
   }
   else if (vtype != QVariant::List) {
      // Process as scaler
      //
      ACAI::ClientInteger i;
      ACAI::ClientFloating f;
      QByteArray bytes;

      switch (fieldType) {
         case ACAI::ClientFieldSTRING:
            result = this->mainClient->putString (value.toString ().toStdString ());
            break;

         case ACAI::ClientFieldENUM:
            result = this->varientToEnumIndex (value, i, valueInRange);
            if (!result) break;
            result = this->mainClient->putInteger (i);
            break;

         case ACAI::ClientFieldCHAR:
            if (vtype == QVariant::String) {
               // Treat as long string.
               QString str = value.toString ();
               const int len = str.length();
               const std::string keepInScope = str.toStdString();
               const char* text = keepInScope.c_str();
               result = this->mainClient->putByteArray (text, len + 1); // include the zero
            } else {
               // Treat as numeric.
               result = this->varientToInteger (value, i, valueInRange);
               if (!result) break;
               result = this->mainClient->putInteger (i);
            }
            break;

         case ACAI::ClientFieldSHORT:
         case ACAI::ClientFieldLONG:
            result = this->varientToInteger (value, i, valueInRange);
            if (!result) break;
            result = this->mainClient->putInteger (i);
            break;

         case ACAI::ClientFieldFLOAT:
         case ACAI::ClientFieldDOUBLE:
            result = this->varientToFloat (value, f, valueInRange);
            if (!result) break;
            result = this->mainClient->putFloating (f);
            break;

         default:
            result = false;
            knownType = false;
            break;
      }

      extra = QString(", source type %1.").arg (value.typeName ());

   } else {
      // Process as array.
      // Consider accepting own QEVectorVariants
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
            result = this->mainClient->putStringArray (strArray);
            break;

         case ACAI::ClientFieldENUM:
            // Do we get arrays of enums?
            //
            for (int j = 0; j < number; j++) {
               ACAI::ClientInteger i;
               result = this->varientToEnumIndex (valueArray.value (j), i, valueInRange);
               if (!result) break;
               intArray.push_back (i);
            }
            if (result) {
               result = this->mainClient->putIntegerArray (intArray);
            }
            break;

         case ACAI::ClientFieldCHAR:
         case ACAI::ClientFieldSHORT:
         case ACAI::ClientFieldLONG:
            for (int j = 0; j < number; j++) {
               ACAI::ClientInteger i;
               result = this->varientToInteger (valueArray.value (j), i, valueInRange);
               if (!result) break;
               intArray.push_back (i);
            }
            if (result) {
               result = this->mainClient->putIntegerArray (intArray);
            }
            break;

         case ACAI::ClientFieldFLOAT:
         case ACAI::ClientFieldDOUBLE:
            for (int j = 0; j < number; j++) {
               ACAI::ClientFloating f;
               result = this->varientToFloat (valueArray.value (j), f, valueInRange);
               if (!result) break;
               fltArray.push_back (f);
            }
            if (result) {
               result = this->mainClient->putFloatingArray (fltArray);
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
      else if (!this->mainClient->isConnected()) {
         msg.append (" Channel disconnected.");

      } else if (!this->mainClient->writeAccess()) {
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
bool QECaClient::varientToFloat (const QVariant& qValue,
                                 ACAI::ClientFloating& fltValue,
                                 bool& valueInRange)
{
   bool result;
   valueInRange = true;   // hypothesize good input

   fltValue = qValue.toDouble (&result);
   if (result) {
      const double min = this->mainClient->minFieldValue();
      const double max = this->mainClient->maxFieldValue();

      if ((fltValue < min) || (fltValue > max)) {
         valueInRange = false;
         result = false;
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
bool QECaClient::varientToInteger (const QVariant& qValue,
                                   ACAI::ClientInteger& intValue,
                                   bool& valueInRange)
{
   bool result;
   valueInRange = true;   // hypothesize good input

   // Alas toInt does a really rubbish job with respect to out of range float
   // values; e.g. QVariant(double, 4.44e+18) can be "successfully" converted
   // to an int by toInt() and return an "okay" result. So we to toDouble first
   // and check the range.
   //
   double f = qValue.toDouble (&result);
   if (result) {
      const double min = this->mainClient->minFieldValue();
      const double max = this->mainClient->maxFieldValue();

      if ((f < min) || (f > max)) {
         valueInRange = false;
         result = false;
      } else {
         // We could call toInt here but the conversion might be expensive
         // and we already have a numeric value.
         //
         intValue = static_cast<ACAI::ClientInteger> (f);
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
bool QECaClient::varientToEnumIndex (const QVariant& qValue,
                                     ACAI::ClientInteger& index,
                                     bool& valueInRange)
{
   const QVariant::Type vtype = qValue.type ();
   bool result;
   valueInRange = true;   // hypothesize good input
   result = true;

   // value van be string or not-string.
   //
   if (vtype == QVariant::String) {
      // Decode the "string" value
      // NOTE: This checks the known enumeration values.
      // If the IOC has been patched and there has been no channel re-connection,
      // then the user/client will continue to use the original enum values.
      //
      ACAI::ClientString enumText = qValue.toString ().toStdString ();
      index = this->mainClient->getEnumerationIndex (enumText);
      if (index < 0) {
         valueInRange = false;
         result = false;
      }
   }

   if ((vtype != QVariant::String) || !result) {
      // Either the varient is a not string type or the string did not match,
      // however we may have a string like "3" which is a perfectly good integer.
      // Decode the "integer" value.
      //
      result = this->varientToInteger (qValue, index, valueInRange);
   }

   return result;
}

//------------------------------------------------------------------------------
// As-is call throughs.
//
QString QECaClient::getRemoteAddress() const       { return QString::fromStdString (this->mainClient->hostName()); }
QString QECaClient::getEgu () const                { return QString::fromStdString (this->mainClient->units()); }
int QECaClient::getPrecision() const               { return this->mainClient->precision (); }
unsigned int QECaClient::hostElementCount () const { return this->mainClient->hostElementCount(); }
unsigned int QECaClient::dataElementCount () const { return this->mainClient->dataElementCount(); }
double QECaClient::getDisplayLimitHigh () const    { return this->mainClient->upperDisplayLimit(); }
double QECaClient::getDisplayLimitLow () const     { return this->mainClient->lowerDisplayLimit(); }
double QECaClient::getHighAlarmLimit () const      { return this->mainClient->upperAlarmLimit(); }
double QECaClient::getLowAlarmLimit () const       { return this->mainClient->lowerAlarmLimit(); }
double QECaClient::getHighWarningLimit () const    { return this->mainClient->upperWarningLimit(); }
double QECaClient::getLowWarningLimit () const     { return this->mainClient->lowerWarningLimit(); }
double QECaClient::getControlLimitHigh () const    { return this->mainClient->upperControlLimit(); }
double QECaClient::getControlLimitLow () const     { return this->mainClient->lowerControlLimit(); }
double QECaClient::getMinStep () const             { return 0.0; }  // not applicable to CA

//------------------------------------------------------------------------------
//
QStringList QECaClient::getEnumerations() const
{
   QStringList enumerations;

   const int n = this->mainClient->enumerationStatesCount ();
   for (int j = 0; j < n; j++) {
      enumerations.append (QString::fromStdString (this->mainClient->getEnumeration (j)));
   }

   return enumerations;
}

//------------------------------------------------------------------------------
//
QCaAlarmInfo QECaClient::getAlarmInfo () const
{
   const QCaAlarmInfo::Status   status   = (QCaAlarmInfo::Status) this->mainClient->alarmStatus ();
   const QCaAlarmInfo::Severity severity = (QCaAlarmInfo::Severity) this->mainClient->alarmSeverity ();

   return QCaAlarmInfo (QEPvNameUri::ca, this->getPvName(), status, severity, "");
}

//------------------------------------------------------------------------------
//
QCaDateTime QECaClient::getTimeStamp () const
{
   const ACAI::ClientTimeStamp updateTime = this->mainClient->timeStamp ();
   return QCaDateTime (updateTime.secPastEpoch, updateTime.nsec, 0);
}

//------------------------------------------------------------------------------
//
QString QECaClient::getDescription () const
{
   QString result;
   if (this->descClient) {
      result = QString::fromStdString (this->descClient->getString());
   } else {
      // We use a slot, mainly to overcome the const qualifier error.
      //
      QTimer::singleShot (0, this, SLOT (requestDescription ()));
   }
   return result;
}

//------------------------------------------------------------------------------
// As-is call throughs.
void QECaClient::setPriority (const unsigned int priority)   { this->mainClient->setPriority (priority); }
void QECaClient::setRequestCount (const unsigned int number) { this->mainClient->setRequestCount (number); }
bool QECaClient::getReadAccess() const                       { return this->mainClient->readAccess(); }
bool QECaClient::getWriteAccess() const                      { return this->mainClient->writeAccess(); }
void QECaClient::setUsePutCallback (const bool enable)       { this->mainClient->setUsePutCallback (enable); }
bool QECaClient::getUsePutCallback() const                   { return this->mainClient->usePutCallback(); }
unsigned QECaClient::getDataElementSize() const              { return this->mainClient->dataElementSize(); }

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
