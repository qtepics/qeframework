/*  QCaObject.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2009-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Anthony Owen
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
#include <QEVectorVariants.h>

#define DEBUG qDebug () << "QCaObject" << __LINE__ << __FUNCTION__ << "  "

using namespace qcaobject;

//==============================================================================
// qcaobject::QCaObject proper
//==============================================================================
//
int QCaObject::totalChannelCount = 0;
int QCaObject::disconnectedCount = 0;
int QCaObject::connectedCount = 0;

// Each created QCaObject is given a unique identity.
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
QCaObject::QCaObject (const QString& newPvName,
                      QObject* parent,
                      const unsigned int variableIndexIn,
                      SignalsToSendFlags signalsToSendIn,
                      priorities priorityIn) : QObject (parent)
{
   this->initialise (newPvName, variableIndexIn, NULL,
                     signalsToSendIn, priorityIn);
}

//------------------------------------------------------------------------------
//
QCaObject::QCaObject (const QString& newPvName,
                      QObject* parent,
                      const unsigned int variableIndexIn,
                      UserMessage* userMessageIn,
                      SignalsToSendFlags signalsToSendIn,
                      priorities priorityIn) : QObject (parent)
{
   this->initialise (newPvName, variableIndexIn, userMessageIn,
                     signalsToSendIn, priorityIn);
}

//------------------------------------------------------------------------------
//
void QCaObject::initialise (const QString& newPvName,
                            const unsigned int variableIndexIn,
                            UserMessage* userMessageIn,
                            SignalsToSendFlags signalsToSendIn,
                            priorities priorityIn)
{
   // Ensure client object pointer is null.
   //
   this->client = NULL;

   // Allocate a new object identity for this QCaObject.
   // We do not worry about wrap arround (it has ~1E19 values).
   //
   this->objectIdentity = ++QCaObject::nextObjectIdentity;

   this->arrayIndex = 0;

   // Note the record required name and associated index.
   //
   this->processVariableName = newPvName;
   this->variableIndex = variableIndexIn;
   this->userMessage = userMessageIn;
   this->signalsToSend = signalsToSendIn;

   // Attempt to decode the given name into a protocol and an actual PV name.
   // If not specified, the 'ca://' Channel Access protocol is the default.
   //
   QEPvNameUri uri;
   const bool decodeOkay = uri.decodeUri (newPvName, /* strict=> */ false);
   if (!decodeOkay) {
      DEBUG << "PV protocol identification failed for:" << newPvName;
      // See comment below
      this->client = new QENullClient (newPvName, this);
      return;
   }

   const QEPvNameUri::Protocol protocol = uri.getProtocol ();
   const QString pvName = uri.getPvName ();

   QECaClient* caClient = NULL;

   switch (protocol) {

      case QEPvNameUri::ca:
         this->client = caClient = new QECaClient (pvName, this);
         caClient->setPriority (int (priorityIn));
         break;

      case QEPvNameUri::pva:
         this->client = new QEPvaClient (pvName, this);
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

   // Do the plumbing.
   //
   QObject::connect (this->client, SIGNAL (connectionUpdated (const bool)),
                     this,         SLOT   (connectionUpdate  (const bool)));
   QObject::connect (this->client, SIGNAL (dataUpdated (const bool)),
                     this,         SLOT   (dataUpdate  (const bool)));
   QObject::connect (this->client, SIGNAL (putCallbackComplete    (const bool)),
                     this,         SLOT   (putCallbackNotifcation (const bool)));

   // Setup any the mechanism to handle messages to the user, if supplied
   //
   this->setUserMessage (userMessageIn);

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
   // NOTE: Sometimes explicitly calling closeChannel() here causes error:
   //   corrupted double-linked list
   //   Aborted (core dumped)
   //
   // We avoid the corruption by first disconnecting any signal/slot connections.
   //
   if (this->client) {
      QObject::disconnect (this->client, SIGNAL (connectionUpdated (const bool)),
                           this,         SLOT   (connectionUpdate  (const bool)));
      QObject::disconnect (this->client, SIGNAL (dataUpdated (const bool)),
                           this,         SLOT   (dataUpdate  (const bool)));
      QObject::disconnect (this->client, SIGNAL (putCallbackComplete    (const bool)),
                           this,         SLOT   (putCallbackNotifcation (const bool)));

      this->client->closeChannel ();
   }

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
   QCaConnectionInfo connectionInfo (QCaConnectionInfo::NEVER_CONNECTED,
                                     this->getPvName());

   QEConnectionUpdate connection;
   connection.connectionInfo = connectionInfo;
   connection.variableIndex = this->variableIndex;

   emit connectionUpdated (connection);
   emit connectionChanged( connectionInfo, this->variableIndex);
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
unsigned int QCaObject::getVariableIndex() const
{
   return this->variableIndex;
}

//------------------------------------------------------------------------------
// Setup the user message mechanism.
// After calling this method the QCaObject can report errors to the user.
//
void QCaObject::setUserMessage (UserMessage* userMessageIn)
{
   this->userMessage = userMessageIn;
   this->client->setUserMessage (userMessageIn);
}

//------------------------------------------------------------------------------
// Setup the number of elements required.
// This can be called before a subscription, or during a subscription, in which
// case the connection will be renewed with the new element count, if changed.
//
void  QCaObject::setRequestedElementCount (unsigned int elementCount)
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
bool QCaObject::getDataIsAvailable() const
{
   return this->client->dataIsAvailable();
}

//------------------------------------------------------------------------------
// Indicates if last update was a meta data update.
//
bool QCaObject::getIsMetaDataUpdate () const
{
   return this->isFirstMetaUpdate;
}

//------------------------------------------------------------------------------
// Return the current value as string
//
QString  QCaObject::getStringValue (const QE::ArrayActions arrayAction) const
{
   QEStringFormatting formatter;
   formatter.setArrayAction (arrayAction);
   QVariant data = this->getVariant();
   QString result = formatter.formatString (data, this->arrayIndex);
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
// Return the process variable name.
//
QString QCaObject::getPvName() const
{
   return this->processVariableName;
}

//------------------------------------------------------------------------------
// Return the record name (technically the process variable name).
//
QString QCaObject::getRecordName() const
{
   return this->processVariableName;
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
void QCaObject::enableWriteCallbacks (bool enable)
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
      connectionInfo = QCaConnectionInfo (QCaConnectionInfo::CONNECTED,
                                          this->processVariableName);
      QCaObject::connectedCount++;
   } else {
      connectionInfo = QCaConnectionInfo (QCaConnectionInfo::CLOSED,
                                          this->processVariableName);
      QCaObject::connectedCount--;
   }

   QCaObject::connectedCount = LIMIT (QCaObject::connectedCount, 0, QCaObject::totalChannelCount);
   QCaObject::disconnectedCount = QCaObject::totalChannelCount - QCaObject::connectedCount;

   QEConnectionUpdate connection;
   connection.connectionInfo = connectionInfo;
   connection.variableIndex = this->variableIndex;

   emit connectionUpdated (connection);
   emit connectionChanged (connectionInfo, this->variableIndex);
}

//------------------------------------------------------------------------------
// New data available - emit to awaiting objects.
//
void QCaObject::dataUpdate (const bool isMetaUpdateIn)
{
   if (!this->client) return;   // sanity check

   // We need non-const copies, at least for now, for old style signals.
   //
   QCaAlarmInfo alarmInfo = this->client->getAlarmInfo ();
   QCaDateTime timeStamp = this->client->getTimeStamp ();

   this->isFirstMetaUpdate = isMetaUpdateIn;

   if (this->signalsToSend & SIG_VARIANT) {
      // Only form variant and emit signal if a varient has been requested.
      const QVariant variantValue = this->getVariant ();

      QEVariantUpdate valueUpdate;
      valueUpdate.value = variantValue;
      valueUpdate.alarmInfo = alarmInfo;
      valueUpdate.timeStamp = timeStamp;
      valueUpdate.variableIndex = variableIndex;
      valueUpdate.isMetaUpdate = isMetaUpdateIn;

      emit valueUpdated (valueUpdate);
      emit dataChanged (variantValue, alarmInfo, timeStamp, this->variableIndex);
   }

   if (this->signalsToSend & SIG_BYTEARRAY) {
      // Only form byte array and emit signal if byte array has been requested.
      //
      const QByteArray byteArrayValue = this->getByteArray ();
      const unsigned dataSize = this->getDataElementSize ();

      // Did we manage to actually extract a byte array?
      //
      if ((byteArrayValue.size() > 0) && (dataSize > 0)) {

         QEByteArrayUpdate arrayUpdate;
         arrayUpdate.array = byteArrayValue;
         arrayUpdate.dataElementSize = dataSize;
         arrayUpdate.alarmInfo = alarmInfo;
         arrayUpdate.timeStamp = timeStamp;
         arrayUpdate.variableIndex = variableIndex;
         arrayUpdate.isMetaUpdate = isMetaUpdateIn;

         emit byteArrayUpdated (arrayUpdate);
         emit byteArrayChanged (byteArrayValue, dataSize, alarmInfo, timeStamp, this->variableIndex);
      }
   }
}

//------------------------------------------------------------------------------
// Putcallback notification.
//
void QCaObject::putCallbackNotifcation( const bool isSuccessful )
{
   qDebug () << __FUNCTION__ << this->getPvName() << isSuccessful;
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
   const QVariant value = this->getVariant ();
   const bool isVector = QEVectorVariants::isVectorVariant (value);

   QByteArray result;

   // We expect this to be one of the vector variant.
   // If not return an empty array.
   //
   if (isVector) {
      bool okay;
      result = QEVectorVariants::getAsByteArray (value, okay);
   }

   return result;
}

//------------------------------------------------------------------------------
//
unsigned QCaObject::getDataElementSize () const
{
   const QVariant value = this->getVariant ();
   const bool isVector = QEVectorVariants::isVectorVariant (value);

   unsigned result = 0;

   // We expect this to be one of the vector variant.
   // If not return 0.
   //
   if (isVector) {
      result = QEVectorVariants::getElementSize (value);
   }

   return result;
}

//------------------------------------------------------------------------------
// Extract last emmited data
//
void QCaObject::getLastData (bool& isDefinedOut, QVariant& valueOut,
                             QCaAlarmInfo& alarmInfoOut, QCaDateTime& timeStampOut) const
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
bool QCaObject::writeData (const QVariant& value)
{
   if (!this->client) return false;   // sanity check
   if (!this->writeEnabled()) return false;
   return this->client->putPvData (value);
}

//------------------------------------------------------------------------------
// Update current data [arrayIndex] with new element value and write to channel.
// Returns false if the array index is out of range.
//
bool QCaObject::writeDataElement (const QVariant& elementValue)
{
   const QVariant lastValue = this->getVariant ();
   const QMetaType::Type ltype = QEPlatform::metaType (lastValue);
   const bool isVectorVariant = QEVectorVariants::isVectorVariant (lastValue);
   const bool isVariantList = (ltype == QMetaType::QVariantList);
   const bool isStringList = (ltype == QMetaType::QStringList);

   bool result = false;

   if (isVectorVariant) {
      const int count = QEVectorVariants::vectorCount (lastValue);
      if ((this->arrayIndex >= 0) && (this->arrayIndex < count)) {
         QVariant vector = lastValue;
         result = QEVectorVariants::replaceValue (vector, this->arrayIndex, elementValue);
         if (result) {
            result = this->writeData (vector);
         } else {
            DEBUG << "update of [" << this->arrayIndex << "] with value:"
                  << elementValue << "failed (of range)";
         }
      }

   } else if (isStringList) {
      QStringList stringList = lastValue.toStringList();
      const int count = stringList.size();
      if ((this->arrayIndex >= 0) && (this->arrayIndex < count)) {
         stringList.replace (this->arrayIndex, elementValue.toString());
         QVariant value = QVariant (stringList);
         result = this->writeData (value);
      }

   } else if (isVariantList) {
      QVariantList valueList = lastValue.toList ();
      const int count = valueList.size();
      if ((this->arrayIndex >= 0) && (this->arrayIndex < count)) {
         valueList.replace (this->arrayIndex, elementValue);  // replace with new value
         result = this->writeData (valueList);
      }

   } else  {
      // The value is a scalar type.
      //
      if (this->arrayIndex == 0) {
         result = this->writeData (elementValue);   // not an array - write as scalar
      }
   }
   return result;
}

//------------------------------------------------------------------------------
// Re-emit the last data emited, if any.
// This can be used after a property of a widget using this QCaObject has changed to
// force an update of the data and a re-presentation of the data in the widget to
// reflect the new property.
//
void QCaObject::resendLastData()
{
   if (this->getDataIsAvailable()) {
      this->dataUpdate (false);
   }
}

//------------------------------------------------------------------------------
//
static bool registerMetaTypes()
{
   qRegisterMetaType<QEConnectionUpdate> ("QEConnectionUpdate");
   qRegisterMetaType<QEVariantUpdate> ("QEVariantUpdate");
   qRegisterMetaType<QEByteArrayUpdate> ("QEByteArrayUpdate");
   return true;
}

static const bool elaborate = registerMetaTypes();

// end
