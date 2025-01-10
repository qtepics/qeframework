/*  QCaObject.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2024 Australian Synchrotron
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
#include <QMetaType>
#include <QECommon.h>
#include <QEAdaptationParameters.h>
#include <QEPlatform.h>
#include <QEPvNameUri.h>
#include <QENullClient.h>
#include <QECaClient.h>
#include <QEPvaClient.h>
#include <QEStringFormatting.h>
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
   // Ensure client object pointer is null.
   //
   this->client = NULL;

   // Allocate a new object identity for this QCaObject.
   // We do not worry about wrap arround.
   //
   this->objectIdentity = ++QCaObject::nextObjectIdentity;

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
      // See comment below
      this->client = new QENullClient (newRecordName, this);
      return;
   }

   const QEPvNameUri::Protocol protocol = uri.getProtocol ();
   const QString pvName = uri.getPvName ();

   QECaClient* caClient;

   switch (protocol) {

      case QEPvNameUri::ca:
         this->client = caClient = new QECaClient (pvName, this);
         caClient->setPriority (int (priorityIn));
         QObject::connect (this->client, SIGNAL (connectionUpdated (const bool)),
                           this,         SLOT   (connectionUpdate  (const bool)));
         QObject::connect (this->client, SIGNAL (dataUpdated (const bool)),
                           this,         SLOT   (dataUpdate  (const bool)));
         QObject::connect (this->client, SIGNAL (putCallbackComplete    (const bool)),
                           this,         SLOT   (putCallbackNotifcation (const bool)));
         break;

      case QEPvNameUri::pva:
         this->client = new QEPvaClient (pvName, this);
         QObject::connect (this->client, SIGNAL (connectionUpdated (const bool)),
                           this,         SLOT   (connectionUpdate  (const bool)));
         QObject::connect (this->client, SIGNAL (dataUpdated (const bool)),
                           this,         SLOT   (dataUpdate  (const bool)));
         QObject::connect (this->client, SIGNAL (putCallbackComplete    (const bool)),
                           this,         SLOT   (putCallbackNotifcation (const bool)));
         break;

      default:
         DEBUG << "Unknown protocol" << protocol << int (protocol);
         // By having a null client, it saves the need to have code like, e.g.:
         //
         //   if (this->client) result = this->client->getEgu();
         //
         // every where. We can always do:
         //
         //   result = this->client->getEgu();
         //
         this->client = new QENullClient (pvName, this);
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
   // Note: closeChannel and openChannel are now dispatching
   //
   if (this->client) this->client->closeChannel ();

   QCaObject::totalChannelCount--;
   QCaObject::connectedCount = LIMIT (QCaObject::connectedCount, 0, QCaObject::totalChannelCount);
   QCaObject::disconnectedCount = QCaObject::totalChannelCount - QCaObject::connectedCount;
}

//------------------------------------------------------------------------------
//
QECaClient* QCaObject::asCaClient () const
{
   return qobject_cast <QECaClient*>(this->client);
}

//------------------------------------------------------------------------------
//
QEPvaClient* QCaObject::asPvaClient () const
{
   return qobject_cast <QEPvaClient*>(this->client);
}

//------------------------------------------------------------------------------
//
bool QCaObject::isCaChannel () const
{
   return (this->client->getType() == QEBaseClient::CAType);
}

//------------------------------------------------------------------------------
//
bool QCaObject::isPvaChannel () const
{
   return (this->client->getType() == QEBaseClient::PVAType);
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
                                     this->getRecordName() );

   emit connectionChanged( connectionInfo, variableIndex );
}

//------------------------------------------------------------------------------
//
bool QCaObject::subscribe()
{
   this->clearConnectionState();
   return this->client->openChannel (QEBaseClient::Monitor | QEBaseClient::Write);
}

//------------------------------------------------------------------------------
//
bool QCaObject::singleShotRead()
{
   this->clearConnectionState();
   return this->client->openChannel (QEBaseClient::Read | QEBaseClient::Write);
}

//------------------------------------------------------------------------------
//
bool QCaObject::connectChannel()
{
   this->clearConnectionState();
   return this->client->openChannel (QEBaseClient::Write);
}

//------------------------------------------------------------------------------
//
void QCaObject::closeChannel()
{
   this->client->closeChannel();
}

//------------------------------------------------------------------------------
// Returns true if the type of data being read or to be written is known.
// The data type will be unknown until a connection is established.
//
bool QCaObject::dataTypeKnown() const
{
   return this->client->getIsConnected();
}

//------------------------------------------------------------------------------
// Returns the assigned variable index.
//
unsigned int QCaObject::getVariableIndex () const
{
   return this->variableIndex;
}

//------------------------------------------------------------------------------
// Setup the user message mechanism.
// After calling this method the QCaObject can report errors to the user.
//
void QCaObject::setUserMessage( UserMessage* userMessageIn )
{
   this->userMessage = userMessageIn;
   this->client->setUserMessage (userMessageIn);
}

//------------------------------------------------------------------------------
// Setup the number of elements required.
// This can be called before a subscription, or during a subscription, in which
// case the connection will be renewed with the new element count, if changed.
//
void  QCaObject::setRequestedElementCount( unsigned int elementCount )
{
   QECaClient* caClient = this->asCaClient();
   if (caClient) {
      caClient->setRequestCount (elementCount);
   }
}

//------------------------------------------------------------------------------
// Extract last emmited connection info: indicates if channel is connected.
//
bool  QCaObject::getChannelIsConnected () const
{
   return this->client->getIsConnected();
}

//------------------------------------------------------------------------------
// Indicates if last data is defined/meaningful.
//
bool  QCaObject::getDataIsAvailable() const
{
   return this->client->dataIsAvailable();
}

//------------------------------------------------------------------------------
// Return the current value as string
//
QString  QCaObject::getStringValue() const
{
   QEStringFormatting formatter;
   QString result;
   result = formatter.formatString (this->getVariant(), this->arrayIndex);
   return result;
}

//------------------------------------------------------------------------------
// Return the current value as boolean
//
bool QCaObject::getBooleanValue() const
{
   QEFloatingFormatting formatter;
   double temp;
   temp = formatter.formatFloating (this->getVariant(), this->arrayIndex);
   return (temp != 0.0);
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
double QCaObject::getFloatingValue() const
{
   QEFloatingFormatting formatter;
   double result;
   result = formatter.formatFloating (this->getVariant(), this->arrayIndex);
   return result;
}

//------------------------------------------------------------------------------
// Return the current value as boolean array
//
QVector<bool> QCaObject::getBooleanArray () const
{
   QEFloatingFormatting formatter;
   QVector<bool> result;
   QVector<double> temp;
   temp = formatter.formatFloatingArray (this->getVariant());
   const int n = temp.count();
   for (int j = 0; j < n; j++) {
      const bool b = (temp.value(j, 0.0) != 0.0);
      result.append (b);
   }
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
//
void QCaObject::writeStringValue (const QString& value)
{
   QVariant varData;
   varData.setValue (value);
   this->writeDataElement (varData);
}

//------------------------------------------------------------------------------
//
void QCaObject::writeBooleanValue (const bool value)
{
   QVariant varData;
   varData.setValue (value ? 1 : 0);  // bool to integer
   this->writeDataElement (varData);
}

//------------------------------------------------------------------------------
//
void QCaObject::writeIntegerValue (const long value)
{
   QVariant varData;
   varData.setValue (value);
   this->writeDataElement (varData);
}

//------------------------------------------------------------------------------
//
void QCaObject::writeFloatingValue (const double value)
{
   QVariant varData;
   varData.setValue (value);
   this->writeDataElement (varData);
}

//------------------------------------------------------------------------------
//
void QCaObject::writeBooleanArray (const QVector<bool>& array)
{
   QVariantList varData;
   for (int j = 0; j < array.count(); j++) {
      QVariant varElement;
      varElement.setValue (array.value (j) ? 1 : 0);  // bool to integer
      varData.append (varElement);
   }
   this->writeData (varData);
}

//------------------------------------------------------------------------------
//
void QCaObject::writeIntegerArray (const QVector<long>& array)
{
   QVariantList varData;
   for (int j = 0; j < array.count(); j++) {
      QVariant varElement;
      varElement.setValue (array.value (j));
      varData.append (varElement);
   }
   this->writeData (varData);
}

//------------------------------------------------------------------------------
//
void QCaObject::writeFloatingArray (const QVector<double>& array)
{
   QVariantList varData;
   for (int j = 0; j < array.count(); j++) {
      QVariant varElement;
      varElement.setValue (array.value (j));
      varData.append (varElement);
   }
   this->writeData (varData);
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
QString QCaObject::getRecordName() const
{
   return this->recordName;
}

//------------------------------------------------------------------------------
// Return the engineering units, if any
//
QString QCaObject::getEgu() const
{
   return this->client->getEgu ();
}

//------------------------------------------------------------------------------
// Return the PV hostname/remote address
//
QString  QCaObject::getHostName() const
{
   return this->client->getRemoteAddress ();
}

//------------------------------------------------------------------------------
// Get the read access of the current connection.
//
bool QCaObject::getReadAccess() const
{
   return this->client->getReadAccess();
}

//------------------------------------------------------------------------------
// Get the write access of the current connection.
// This is deternmined by the clinet and anp specified adaptation parameter.
//
bool QCaObject::getWriteAccess() const
{
   return this->client->getWriteAccess() && this->writeEnabled();
}

//------------------------------------------------------------------------------
// Return the PV native field type
//
QString QCaObject::getFieldType() const
{
   return this->client->getId ();
}

//------------------------------------------------------------------------------
// Return the PV native/host element count
//
unsigned long QCaObject::getHostElementCount() const
{
   return this->client->hostElementCount ();
}

//------------------------------------------------------------------------------
// Return the element count of number elements requested.
//
unsigned long QCaObject::getDataElementCount() const
{
   return this->client->dataElementCount ();
}

//------------------------------------------------------------------------------
// Obsolete
unsigned long QCaObject::getElementCount() const
{
   return this->getHostElementCount();
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
   return this->client->getEnumerations ();
}

//------------------------------------------------------------------------------
// Return the precision, if any
// TODO: consider changing getPrecision to return int
//
unsigned int QCaObject::getPrecision() const
{
   int result = this->client->getPrecision();
   if (result < 0) result = 0;
   return unsigned (result);
}

//------------------------------------------------------------------------------
// Return the display upper limit, if any
//
double  QCaObject::getDisplayLimitUpper() const
{
   return this->client->getDisplayLimitHigh();
}

//------------------------------------------------------------------------------
// Return the display lower limit, if any
//
double  QCaObject::getDisplayLimitLower() const
{
   return this->client->getDisplayLimitLow();
}

//------------------------------------------------------------------------------
// Return the alarm upper limit, if any
//
double  QCaObject::getAlarmLimitUpper() const
{
   return this->client->getHighAlarmLimit();
}

//------------------------------------------------------------------------------
// Return the alarm lower limit, if any
//
double  QCaObject::getAlarmLimitLower() const
{
   return this->client->getLowAlarmLimit();
}

//------------------------------------------------------------------------------
// Return the warning upper limit, if any
//
double  QCaObject::getWarningLimitUpper() const
{
   return this->client->getHighWarningLimit();
}

//------------------------------------------------------------------------------
// Return the warning lower limit, if any
//
double  QCaObject::getWarningLimitLower() const
{
   return this->client->getLowWarningLimit();
}

//------------------------------------------------------------------------------
// Return the control upper limit, if any
//
double  QCaObject::getControlLimitUpper() const
{
   return this->client->getControlLimitHigh();
}

//------------------------------------------------------------------------------
// Return the control lower limit, if any
//
double  QCaObject::getControlLimitLower() const
{
   return this->client->getControlLimitLow();
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
   QECaClient* caClient = this->asCaClient();
   if (caClient)
      caClient->setUsePutCallback( enable );
}

//------------------------------------------------------------------------------
// Determine if callbacks are required on write completion.
//
bool QCaObject::isWriteCallbacksEnabled() const
{
   QECaClient* caClient = this->asCaClient();
   if (caClient)
      return caClient->getUsePutCallback();

   return false;
}

//------------------------------------------------------------------------------
// Return alarm state associated with last update
//
QCaAlarmInfo QCaObject::getAlarmInfo() const
{
   return this->client->getAlarmInfo ();
}

//------------------------------------------------------------------------------
// Return date time associated with last update
//
QCaDateTime  QCaObject::getDateTime () const
{
   return this->client->getTimeStamp ();
}

//------------------------------------------------------------------------------
// Return the DESCription associated with this PV.
//
QString QCaObject::getDescription () const
{
   return this->client->getDescription ();
}

//------------------------------------------------------------------------------
// Handle connection status change - emit to awaiting objects.
//
void QCaObject::connectionUpdate (const bool isConnected)
{
   QCaConnectionInfo connectionInfo;

   if (isConnected) {
      connectionInfo = QCaConnectionInfo( QCaConnectionInfo::CONNECTED,
                                          this->recordName );
      QCaObject::connectedCount++;
   } else {
      connectionInfo = QCaConnectionInfo( QCaConnectionInfo::CLOSED,
                                          this->recordName );
      QCaObject::connectedCount--;
   }

   QCaObject::connectedCount = LIMIT (QCaObject::connectedCount, 0, QCaObject::totalChannelCount);
   QCaObject::disconnectedCount = QCaObject::totalChannelCount - QCaObject::connectedCount;

   emit connectionChanged( connectionInfo, this->variableIndex );
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

   if (!this->client) return;   // sanity check

   alarmInfo = this->client->getAlarmInfo ();
   timeStamp = this->client->getTimeStamp ();

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
         QECaClient* caClient = this->asCaClient();
         if (caClient) {
            dataSize = caClient->getDataElementSize ();
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
   return this->client->getPvData ();
}

//------------------------------------------------------------------------------
//
QByteArray QCaObject::getByteArray() const
{
   QByteArray result;

   // Doesn't apply to pva data yet - return an empty array.
   //
   QECaClient* caClient = this->asCaClient();
   if (caClient) {
      size_t count = 0;
      const char* rawData = (const char *) caClient->getRawDataPointer(count);
      result.append (rawData, (int) count);
   }
   return result;
}

//------------------------------------------------------------------------------
// Extract last emmited data
//
void QCaObject::getLastData( bool& isDefinedOut, QVariant& valueOut,
                             QCaAlarmInfo& alarmInfoOut, QCaDateTime& timeStampOut ) const
{
   isDefinedOut = this->getDataIsAvailable ();
   valueOut = this->getVariant();
   alarmInfoOut = this->client->getAlarmInfo ();
   timeStampOut = this->client->getTimeStamp ();
}

//------------------------------------------------------------------------------
//
bool QCaObject::writeEnabled() const
{
   QEAdaptationParameters ap ("QE_");

   const bool read_only = ap.getBool ("read_only");  // default is false
   return !read_only;
}

//------------------------------------------------------------------------------
// Write a data out to channel
//
bool QCaObject::writeData( const QVariant& value )
{
   if (!this->client) return false;   // sanity check
   if (!this->writeEnabled()) return false;
   return this->client->putPvData (value);
}

//------------------------------------------------------------------------------
// Update current data [arrayIndex] with new element value and write to channel.
// Returns false if the array index is out of range.
//
bool QCaObject::writeDataElement( const QVariant& elementValue )
{
   QVariant lastVariantValue = this->getVariant ();

   bool result;

   const QMetaType::Type ltype = QEPlatform::metaType (lastVariantValue);
   if( ltype ==  QMetaType::QVariantList ) {
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
