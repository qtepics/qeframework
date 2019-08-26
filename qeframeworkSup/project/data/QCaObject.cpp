/*  QCaObject.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2019 Australian Synchrotron
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
 *    Anthony Owen
 *  Contact details:
 *    anthony.owen@gmail.com
 */

#include "QCaObject.h"

#include <QtCore>
#include <QApplication>
#include <QDebug>
#include <QByteArray>
#include <QECommon.h>
#include <QEPvNameUri.h>
#include <QEBaseClient.h>
#include <QECaClient.h>
#include <QEPvaClient.h>
#include <QEIntegerFormatting.h>
#include <QEFloatingFormatting.h>

#define DEBUG qDebug () << "QCaObject" << __LINE__ << __FUNCTION__ << "  "

using namespace qcaobject;

//==============================================================================
// qcaobject::QCaObject proper
//==============================================================================
//
int QCaObject::totalChannelCount = 0;
int QCaObject::disconnectedCount = 0;
int QCaObject::connectedCount = 0;

// Each created QCaObject is given a unique idtentity.
//
QCaObject::ObjectIdentity QCaObject::nextObjectIdentity = 0;

//------------------------------------------------------------------------------
// static
int* QCaObject::getDisconnectedCountRef()
{
   return &QCaObject::disconnectedCount;
}

//------------------------------------------------------------------------------
// static
int* QCaObject::getConnectedCountRef()
{
   return &QCaObject::connectedCount;
}

//------------------------------------------------------------------------------
// The event object can be any Qt object with an event queue.
// A filter will be inserted (and removed) by this class to catch
// events from this class and pass them back to this class for processing.
// In other words, the event object does not need to be set up in any way.
// It just need to have a suitable event loop running.
//
QCaObject::QCaObject( const QString& newRecordName,
                      QObject* parent,
                      const unsigned int variableIndexIn,
                      SignalsToSendFlags signalsToSendIn,
                      priorities priorityIn ) : QObject (parent)
{
   this->initialise (newRecordName, variableIndexIn, NULL,
                     signalsToSendIn, priorityIn);
}

//------------------------------------------------------------------------------
//
QCaObject::QCaObject( const QString& newRecordName,
                      QObject* parent,
                      const unsigned int variableIndexIn,
                      UserMessage* userMessageIn,
                      SignalsToSendFlags signalsToSendIn,
                      priorities priorityIn ) : QObject (parent)
{
   this->initialise (newRecordName, variableIndexIn, userMessageIn,
                     signalsToSendIn, priorityIn);
}

//------------------------------------------------------------------------------
//
void QCaObject::initialise( const QString& newRecordName,
                            const unsigned int variableIndexIn,
                            UserMessage* userMessageIn,
                            SignalsToSendFlags signalsToSendIn,
                            priorities priorityIn )
{
   // Allocate a new object identity for this QCaObject.
   // We do not worry about wrap arround.
   //
   this->objectIdentity = ++QCaObject::nextObjectIdentity;

   // Ensure client object pointers are null.
   //
   this->caClient = NULL;
   this->pvaClient = NULL;
   this->arrayIndex = 0;

   // Note the record required name and associated index.
   //
   this->recordName = newRecordName;
   this->variableIndex = variableIndexIn;
   this->userMessage = userMessageIn;
   this->signalsToSend = signalsToSendIn;

   // Attempt to decode the given name into a protocol and an actual PV name.
   // If not specified, the 'ca://' Channel Access protocol is the default.
   //
   QEPvNameUri uri;
   const bool decodeOkay = uri.decodeUri (newRecordName, /* strict=> */ false);
   if (!decodeOkay) {
      DEBUG << "PV protocol identification failed for:" << newRecordName;
      return;
   }

   const QEPvNameUri::Protocol protocol = uri.getProtocol ();
   const QString pvName = uri.getPvName ();

   switch (protocol) {

      case QEPvNameUri::ca:
         this->caClient = new QECaClient (pvName, this);
         this->caClient->setPriority (int (priorityIn));
         QObject::connect (this->caClient, SIGNAL (connectionUpdated (const bool)),
                           this,           SLOT   (connectionUpdate  (const bool)));
         QObject::connect (this->caClient, SIGNAL (dataUpdated (const bool)),
                           this,           SLOT   (dataUpdate  (const bool)));
         QObject::connect (this->caClient, SIGNAL (putCallbackComplete    (const bool)),
                           this,           SLOT   (putCallbackNotifcation (const bool)));
         break;

      case QEPvNameUri::pva:
         this->pvaClient = new QEPvaClient (pvName, this);
         QObject::connect (this->pvaClient, SIGNAL (connectionUpdated (const bool)),
                           this,            SLOT   (connectionUpdate  (const bool)));
         QObject::connect (this->pvaClient, SIGNAL (dataUpdated (const bool)),
                           this,            SLOT   (dataUpdate  (const bool)));
         break;

      default:
         DEBUG << "Unknown protocol" << protocol << int (protocol);
         return;
   }

   // Setup any the mechanism to handle messages to the user, if supplied
   this->setUserMessage( userMessageIn );

   // Update counters. Ensure consistant
   //
   QCaObject::totalChannelCount++;
   QCaObject::connectedCount = LIMIT (QCaObject::connectedCount, 0, QCaObject::totalChannelCount);
   QCaObject::disconnectedCount = QCaObject::totalChannelCount - QCaObject::connectedCount;
}

//------------------------------------------------------------------------------
// Destructor.
//
QCaObject::~QCaObject()
{
   // NOTE: we call closeChannel before the client destructor so that the overriden
   // connectionUpdate still gets invoked.
   if (this->caClient) this->caClient->closeChannel ();
   if (this->pvaClient) this->pvaClient->closeChannel ();

   QCaObject::totalChannelCount--;
   QCaObject::connectedCount = LIMIT (QCaObject::connectedCount, 0, QCaObject::totalChannelCount);
   QCaObject::disconnectedCount = QCaObject::totalChannelCount - QCaObject::connectedCount;
}

//------------------------------------------------------------------------------
//
bool QCaObject::isCaChannel () const
{
    return (this->caClient != NULL);
}

//------------------------------------------------------------------------------
//
bool QCaObject::isPvaChannel () const
{
    return (this->pvaClient != NULL);
}

//------------------------------------------------------------------------------
//
void QCaObject::setSignalsToSend (const SignalsToSendFlags signalsToSendIn)
{
   this->signalsToSend = signalsToSendIn;
}

//------------------------------------------------------------------------------
//
QCaObject::SignalsToSendFlags QCaObject::getSignalsToSend () const
{
   return this->signalsToSend;
}


//------------------------------------------------------------------------------
// Simple function call selection with optional arguments.
//
#define RESULT_SELECT(cafunc, pvafunc, ...) {                                  \
   if (this->caClient) {                                                       \
      result = this->caClient->cafunc (__VA_ARGS__);                           \
   } else if (this->pvaClient)  {                                              \
      result = this->pvaClient->pvafunc (__VA_ARGS__);                         \
   }                                                                           \
}


//------------------------------------------------------------------------------
// Clear channel connection state - and signal "initial" change of state
// Note: was original in setChannelExpired
//
void QCaObject::clearConnectionState()
{
   // setChannelExpired
   // Signal a connection change.
   // (This is done with some licence. There isn't really a connection change.
   //  The connection has gone from 'no connection' to 'not connectet yet')
   //
   QCaConnectionInfo connectionInfo( QCaConnectionInfo::NEVER_CONNECTED,
                                     QCaConnectionInfo::LINK_DOWN,
                                     this->getRecordName() );

   emit connectionChanged( connectionInfo, variableIndex );
   emit connectionChanged( connectionInfo );
}

//------------------------------------------------------------------------------
//
bool QCaObject::subscribe()
{
   bool result = false;
   if (this->caClient) {
      this->clearConnectionState();
      this->caClient->setReadMode (ACAI::Subscribe);
      result = this->caClient->openChannel ();
   } else if (this->pvaClient) {
      result = this->pvaClient->openChannel (QEPvaClient::Monitor | QEPvaClient::Write);
   }
   return result;
}

//------------------------------------------------------------------------------
//
bool QCaObject::singleShotRead()
{
   bool result = false;
   if (this->caClient) {
      this->clearConnectionState();
      this->caClient->setReadMode (ACAI::SingleRead);
      result = this->caClient->openChannel ();
   } else if (this->pvaClient) {
      result = this->pvaClient->openChannel (QEPvaClient::Read | QEPvaClient::Write);
   }
   return result;
}

//------------------------------------------------------------------------------
//
bool QCaObject::connectChannel()
{
   bool result = false;
   if (this->caClient) {
      this->clearConnectionState();
      this->caClient->setReadMode (ACAI::NoRead);
      result = this->caClient->openChannel ();
   } else if (this->pvaClient) {
      result = this->pvaClient->openChannel (QEPvaClient::Write);
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QCaObject::closeChannel()
{
   if (this->caClient) {
      this->caClient->closeChannel();
   } else if (this->pvaClient) {
      this->pvaClient->closeChannel();
   }
}

//------------------------------------------------------------------------------
// Returns true if the type of data being read or to be written is known.
// The data type will be unknown until a connection is established.
//
bool QCaObject::dataTypeKnown() const
{
   bool result = false;
   RESULT_SELECT (isConnected, getIsConnected);
   return result;
}

//------------------------------------------------------------------------------
// Returns the assigned variable index.
//
unsigned int QCaObject::getVariableIndex () const
{
   return this->variableIndex;
}

//------------------------------------------------------------------------------
// Returns the type of data being read or to be written.
// This will be Type::GENERIC_UNKNOWN if a connection has never been established.
//
generic::generic_types QCaObject::getDataType() const
{
   using namespace generic;
   generic::generic_types result = GENERIC_UNKNOWN;
   RESULT_SELECT (getDataType, getDataType);
   return result;
}

//------------------------------------------------------------------------------
// Setup the user message mechanism.
// After calling this method the QCaObject can report errors to the user.
//
void QCaObject::setUserMessage( UserMessage* userMessageIn )
{
   this->userMessage = userMessageIn;
   if (this->caClient) {
      this->caClient->setUserMessage (userMessageIn);
   } else if (this->pvaClient) {
      this->pvaClient->setUserMessage (userMessageIn);
   }
}

//------------------------------------------------------------------------------
// Setup the number of elements required.
// This can be called before a subscription, or during a subscription, in which
// case the connection will be renewed with the new element count, if changed.
//
void  QCaObject::setRequestedElementCount( unsigned int elementCount )
{
   if (this->caClient) {
      this->caClient->setRequestCount (elementCount);
   }
}

//------------------------------------------------------------------------------
// Determine if the channel is currently connected
//
bool QCaObject::isChannelConnected() const
{
   bool result = false;
   RESULT_SELECT (isConnected, getIsConnected);
   return result;
}

//------------------------------------------------------------------------------
// Extract last emmited connection info: indicates if channel is connected.
//
bool  QCaObject::getChannelIsConnected () const
{
   bool result = false;
   RESULT_SELECT (isConnected, getIsConnected);
   return result;
}

//------------------------------------------------------------------------------
// Extract last emmited connection info: indicates if channel link is up.
//
bool  QCaObject::getIsLinkUp () const
{
   bool result = false;
   RESULT_SELECT (isConnected, getIsConnected);
   return result;
}

//------------------------------------------------------------------------------
// Indicates if last data is defined/meaningful.
//
bool  QCaObject::getDataIsAvailable() const
{
   bool result = false;
   RESULT_SELECT (dataIsAvailable, dataIsAvailable);
   return result;
}

//------------------------------------------------------------------------------
// Return the current value as string
//
QString  QCaObject::getStringValue() const
{
   QString result = "";
   if (this->caClient) {
      result =  QString::fromStdString (this->caClient->getString (this->getArrayIndex()));
   }
   return result;
}

//------------------------------------------------------------------------------
// Return the current value as integer
//
long  QCaObject::getIntegerValue() const
{
   QEIntegerFormatting formatter;
   long result;
   result = formatter.formatInteger (this->getVariant(), this->arrayIndex);
   return result;
}

//------------------------------------------------------------------------------
// Return the current value as floating
//
double  QCaObject::getFloatingValue() const
{
   QEFloatingFormatting formatter;
   double result;
   result = formatter.formatFloating (this->getVariant(), this->arrayIndex);
   return result;
}

//------------------------------------------------------------------------------
// Return the current value as integer array
//
QVector<long> QCaObject::getIntegerArray () const
{
   QEIntegerFormatting formatter;
   QVector<long> result;
   result = formatter.formatIntegerArray (this->getVariant());
   return result;
}

//------------------------------------------------------------------------------
// Return the current value as floating array
//
QVector<double> QCaObject::getFloatingArray () const
{
   QEFloatingFormatting formatter;
   QVector<double> result;
   result = formatter.formatFloatingArray (this->getVariant());
   return result;
}

//------------------------------------------------------------------------------
// static
QCaObject::ObjectIdentity QCaObject::nullObjectIdentity ()
{
   return 0;
}

//------------------------------------------------------------------------------
//
QCaObject::ObjectIdentity QCaObject::getObjectIdentity () const
{
   return this->objectIdentity;
}

//------------------------------------------------------------------------------
// Return the record name (technically the process variable name).
//
QString  QCaObject::getRecordName() const
{
   return this->recordName;
}

//------------------------------------------------------------------------------
// Return the engineering units, if any
//
QString  QCaObject::getEgu() const
{
   QString result = "";
   if (this->caClient) {
      result = QString::fromStdString (this->caClient->units());
   } else if (this->pvaClient) {
      result =  this->pvaClient->getEgu ();
   }
   return result;
}

//------------------------------------------------------------------------------
// Return the PV hostname/remote address
//
QString  QCaObject::getHostName() const
{
   QString result = "";
   if (this->caClient) {
      result = QString::fromStdString (this->caClient->hostName());
   } else if (this->pvaClient) {
      result =  this->pvaClient->getRemoteAddress ();
   }
   return result;
}

//------------------------------------------------------------------------------
// Get the read access of the current connection.
//
bool  QCaObject::getReadAccess() const
{
   if (this->caClient)
      return this->caClient->readAccess();
   return false;
}

//------------------------------------------------------------------------------
// Get the write access of the current connection.
//
bool  QCaObject::getWriteAccess() const
{
   if (this->caClient)
      return this->caClient->writeAccess();
   return false;
}

//------------------------------------------------------------------------------
// Return the PV native field type
//
QString QCaObject::getFieldType() const
{
   QString result = "";
   if (this->caClient) {
      result = QString::fromStdString (ACAI::clientFieldTypeImage (this->caClient->hostFieldType()));
   } else if (this->pvaClient) {
      result = this->pvaClient->getId ();
   }
   return result;
}

//------------------------------------------------------------------------------
// Return the PV native element count
//
unsigned long QCaObject::getElementCount() const
{
   unsigned long result = 0;
   RESULT_SELECT (dataElementCount, dataElementCount);
   return result;
}

//------------------------------------------------------------------------------
// Set nominated array index used to extract a scalar from an array data set.
// Defaults to zero, i.e. first element.
//
void QCaObject::setArrayIndex( const int indexIn )
{
   this->arrayIndex = MAX( 0, indexIn );
}

//------------------------------------------------------------------------------
// Return nominated array index used to extract a scalar from an array data set.
//
int QCaObject::getArrayIndex() const
{
   return this->arrayIndex;
}

//------------------------------------------------------------------------------
// Return the enumerations strings, if any
//
QStringList QCaObject::getEnumerations() const
{
   QStringList result;
   RESULT_SELECT (getEnumerations, getEnumerations);
   return result;
}

//------------------------------------------------------------------------------
// Return the precision, if any
//
unsigned int  QCaObject::getPrecision() const
{
   unsigned int result = 0;
   RESULT_SELECT (precision, getPrecision);
   return result;
}

//------------------------------------------------------------------------------
// Return the display upper limit, if any
//
double  QCaObject::getDisplayLimitUpper() const
{
   double result = 0.0;
   RESULT_SELECT (upperDisplayLimit, getDisplayLimitHigh);
   return result;
}

//------------------------------------------------------------------------------
// Return the display lower limit, if any
//
double  QCaObject::getDisplayLimitLower() const
{
   double result = 0.0;
   RESULT_SELECT (lowerDisplayLimit, getDisplayLimitLow);
   return result;
}

//------------------------------------------------------------------------------
// Return the alarm upper limit, if any
//
double  QCaObject::getAlarmLimitUpper() const
{
   double result = 0.0;
   RESULT_SELECT (upperAlarmLimit, getHighAlarmLimit);
   return result;
}

//------------------------------------------------------------------------------
// Return the alarm lower limit, if any
//
double  QCaObject::getAlarmLimitLower() const
{
   double result = 0.0;
   RESULT_SELECT (lowerAlarmLimit, getLowAlarmLimit);
   return result;
}

//------------------------------------------------------------------------------
// Return the warning upper limit, if any
//
double  QCaObject::getWarningLimitUpper() const
{
   double result = 0.0;
   RESULT_SELECT (upperWarningLimit, getHighWarningLimit);
   return result;
}

//------------------------------------------------------------------------------
// Return the warning lower limit, if any
//
double  QCaObject::getWarningLimitLower() const
{
   double result = 0.0;
   RESULT_SELECT (lowerWarningLimit, getLowWarningLimit);
   return result;
}

//------------------------------------------------------------------------------
// Return the control upper limit, if any
//
double  QCaObject::getControlLimitUpper() const
{
   double result = 0.0;
   RESULT_SELECT (upperControlLimit, getControlLimitHigh);
   return result;
}

//------------------------------------------------------------------------------
// Return the control lower limit, if any
//
double  QCaObject::getControlLimitLower() const
{
   double result = 0.0;
   RESULT_SELECT (lowerControlLimit, getControlLimitLow);
   return result;
}

//------------------------------------------------------------------------------
// Set if callbacks are required on write completion. (default is write with no callback)
// Note, this is not just for better write status, if affects the behaviour of the write as follows:
// When using write with callback, then record will finish processing before accepting next write.
// Writing with callback may be required when writing code that is tightly integrated with record
// processing and code needs to know processing has completed.
// Writing with no callback is more desirable when a detachement from record processing is required, for
// example in a GUI after issuing a motor record move a motor stop command will take effect immedietly
// if writing without callback, but will only take affect after the move has finished if writing with callback.
//
void QCaObject::enableWriteCallbacks( bool enable )
{
   if (this->caClient)
      this->caClient->setUsePutCallback( enable );
}

//------------------------------------------------------------------------------
// Determine if callbacks are required on write completion.
//
bool QCaObject::isWriteCallbacksEnabled() const
{
   if (this->caClient)
      return this->caClient->usePutCallback();

   return false;
}

//------------------------------------------------------------------------------
// Return alarm state associated with last update
//
QCaAlarmInfo QCaObject::getAlarmInfo() const
{
   QCaAlarmInfo result;
   RESULT_SELECT (getAlarmInfo, getAlarmInfo);
   return result;
}

//------------------------------------------------------------------------------
// Return date time associated with last update
//
QCaDateTime  QCaObject::getDateTime () const
{
   QCaDateTime result;
   RESULT_SELECT (getTimeStamp, getTimeStamp);
   return result;
}

//------------------------------------------------------------------------------
// Handle connection status change - emit to awaiting objects.
//
void QCaObject::connectionUpdate (const bool isConnected)
{
   QCaConnectionInfo connectionInfo;

   if (isConnected) {
      connectionInfo = QCaConnectionInfo( QCaConnectionInfo::CONNECTED, QCaConnectionInfo::LINK_UP, this->recordName );
      QCaObject::connectedCount++;
   } else {
      connectionInfo = QCaConnectionInfo( QCaConnectionInfo::CLOSED, QCaConnectionInfo::LINK_DOWN, this->recordName );
      QCaObject::connectedCount--;
   }

   QCaObject::connectedCount = LIMIT (QCaObject::connectedCount, 0, QCaObject::totalChannelCount);
   QCaObject::disconnectedCount = QCaObject::totalChannelCount - QCaObject::connectedCount;

   emit connectionChanged( connectionInfo, this->variableIndex );
   emit connectionChanged( connectionInfo );  // deprecated
}

//------------------------------------------------------------------------------
// New data available - emit to awaiting objects.
//
void QCaObject::dataUpdate (const bool firstUpdateIn)
{
   static const char* varSignal =
         SIGNAL (dataChanged (const QVariant&, QCaAlarmInfo&, QCaDateTime&,
                              const unsigned int&));

   static const char* byteSignal =
         SIGNAL (dataChanged (const QByteArray&, unsigned long, QCaAlarmInfo&,
                              QCaDateTime&, const unsigned int&));

   QCaAlarmInfo alarmInfo;
   QCaDateTime timeStamp;

   if (this->caClient) {
      alarmInfo = this->caClient->getAlarmInfo ();
      timeStamp = this->caClient->getTimeStamp ();
   } else if (this->pvaClient) {
      alarmInfo = this->pvaClient->getAlarmInfo ();
      timeStamp = this->pvaClient->getTimeStamp ();
   } else {
       return;
   }

   this->firstUpdate = firstUpdateIn;

   if (this->signalsToSend & SIG_VARIANT) {
      // Only form variant and emit signal if at least one receiver.
      //
      int number = this->receivers (varSignal);
      if (number > 0) {
         QVariant variantValue = this->getVariant ();
         emit dataChanged (variantValue, alarmInfo, timeStamp, this->variableIndex);
      }
   }

   if (this->signalsToSend & SIG_BYTEARRAY) {
      // Only form byte array and emit signal if at least one receiver.
      //
      int number = this->receivers (byteSignal);
      if (number > 0) {
         QByteArray byteArrayValue = this->getByteArray ();
         unsigned long dataSize = 0;
         if (this->caClient) {
            dataSize = this->caClient->dataElementSize ();
         }
         emit dataChanged (byteArrayValue, dataSize, alarmInfo, timeStamp, this->variableIndex);
      }
   }
}

//------------------------------------------------------------------------------
// Putcallback notification.
//
void QCaObject::putCallbackNotifcation( const bool isSuccessful )
{
   qDebug () << __FUNCTION__ << this->getRecordName() << isSuccessful;
}

//------------------------------------------------------------------------------
// Converts EPICS CA data into a variant.
//
QVariant QCaObject::getVariant () const
{
   QVariant result = QVariant (QVariant::Invalid);  // default
   RESULT_SELECT (getPvData, getPvData);
   return result;
}

//------------------------------------------------------------------------------
//
QByteArray QCaObject::getByteArray() const
{
   QByteArray result;

   if (this->caClient) {
      size_t count = 0;
      const char* rawData = (const char *) this->caClient->rawDataPointer(count);
      result.append (rawData, (int) count);
   } else if (this->pvaClient) {
      // Doesn't apply to pvdata yet
   }
   return  result;
}

//------------------------------------------------------------------------------
// Extract last emmited data
//
void QCaObject::getLastData( bool& isDefinedOut, QVariant& valueOut,
                             QCaAlarmInfo& alarmInfoOut, QCaDateTime& timeStampOut ) const
{
   isDefinedOut = this->getDataIsAvailable ();
   valueOut = this->getVariant();

   if (this->caClient) {
      alarmInfoOut = this->caClient->getAlarmInfo ();
      timeStampOut = this->caClient->getTimeStamp ();
   } else if (this->pvaClient) {
      alarmInfoOut = this->pvaClient->getAlarmInfo ();
      timeStampOut = this->pvaClient->getTimeStamp ();
   }
}

//------------------------------------------------------------------------------
// Write a data out to channel
//
bool QCaObject::writeData( const QVariant& value )
{
   bool result = false;
   if (this->caClient) {
      result = this->caClient->putPvData (value);
   } else if (this->pvaClient) {
      result = this->pvaClient->putPvData (value);
   }

   return result;
}

//------------------------------------------------------------------------------
// Update current data [arrayIndex] with new element value and write to channel.
// Returns false if the array index is out of range.
//
bool QCaObject::writeDataElement( const QVariant& elementValue )
{
   QVariant lastVariantValue = this->getVariant ();

   bool result;

   if( lastVariantValue.type() == QVariant::List ) {
      QVariantList valueList = lastVariantValue.toList ();
      if( ( this->arrayIndex >= 0 ) && ( this->arrayIndex < valueList.size() ) )
      {
         valueList.replace( this->arrayIndex, elementValue );  // replace with new value
         result = writeData( valueList );
      } else {
         result = false;
      }
   } else {
      if( this->arrayIndex == 0 ) {
         result = writeData( elementValue );       // not an array - write as scalar
      } else {
         result = false;
      }
   }
   return result;
}

//------------------------------------------------------------------------------
// Re-emit the last data emited, if any.
// This can be used after a property of a widget using this QCaObject has changed to
// force an update of the data and a re-presentation of the data in the widget to
// reflect the new property
//
void QCaObject::resendLastData()
{
   if( this->getDataIsAvailable() ){
      this->dataUpdate( false );
   }
}

// end
