/*  QEPvaClient_PVXS.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */


/// QE_INCLUDE_PV_ACCESS is defined and QE_PVXS_SUPPORT is defined

#include "QEPvaClient_PVXS.h"
#include <stdint.h>
#include <pvxs/data.h>
#include <QDebug>
#include <QMetaType>
#include <QTimer>

#include <QEPlatform.h>
#include <QEThreadSafeQueue.h>
#include <QEVectorVariants.h>

#define DEBUG qDebug() << "QEPvaClient_PVXS" << __LINE__ << __FUNCTION__ << "  "

// Used for sanity checks.
// Where did this number come from? Try: echo pvxs | md5sum
//
static const uint64_t clientMagicNumber = 0xabead5745dec27a9;
static const QVariant nullVariant;

static pvxs::client::Context pvxsContext;

//==============================================================================
// QEPvaClient
//==============================================================================
//
QEPvaClient::QEPvaClient (const QString& pvName,
                          QObject* parent) :
   QEBaseClient (QEBaseClient::PVAType, pvName, parent)
{
   static bool contextIsSetup = false;

   if (!contextIsSetup) {
      // First time - create the context.
      //
      contextIsSetup = true;
      pvxsContext = pvxs::client::Context::fromEnv();
   }

   QEPvaClientManager::initialise ();  // idempotent - do first.

   // Allocate this object a unique id, being 64 bits we won't run out.
   //
   static uint64_t nextUniqueId = 0;
   nextUniqueId++;
   this->clientId = nextUniqueId;

   this->subscriptionId = 0;
   this->remoteAddress = "";
   this->id = "";
   this->pvData = nullVariant;
   this->firstUpdate = false;
   this->isConnected = false;

   this->subscription.reset();   // implicit when created

   this->magic = clientMagicNumber;
}

//------------------------------------------------------------------------------
//
QEPvaClient::~QEPvaClient ()
{
   this->allowSignalEmission = false;
   this->closeChannel ();
   this->magic = 0;
   this->clientId = 0;
   this->subscriptionId = 0;
   this->subscription.reset();   // implicit when deleted
}

//------------------------------------------------------------------------------
//
bool QEPvaClient::openChannel (const ChannelModesFlags)
{
   this->closeChannel();
   const std::string pvName = this->getPvName().toStdString();

   // Allocate this a unique subscription id, being 64 bits we won't run out.
   //
   static uint64_t nextUniqueId = 0;
   nextUniqueId++;
   this->subscriptionId = nextUniqueId;

   // As well as the object itself, save unique ids in case memory is recycled.
   //
   EventContext context;
   context.client = this;
   context.clientId = this->clientId;
   context.subscriptionId = this->subscriptionId;

   this->subscription =
         pvxsContext
         .monitor (pvName)
         .pvRequest ("")
         .maskConnected (false)
         .maskDisconnected (false)
         .event([context](pvxs::client::Subscription& subscription) { // lamda syntax
             QEPvaClient::eventCallback (context, subscription);
          })
         .exec();

   return (this->subscription != nullptr);
}

//------------------------------------------------------------------------------
//
void QEPvaClient::closeChannel ()
{
   if (this->subscription) {
      this->subscription->cancel();
      this->subscription.reset();   // clears smart/shared pointer
      this->isConnected = false;
      this->remoteAddress = "";
      this->id = "";
   }
}

//------------------------------------------------------------------------------
//
bool QEPvaClient::getIsConnected () const
{
   return this->isConnected;
}

//------------------------------------------------------------------------------
//
bool QEPvaClient::dataIsAvailable () const
{
   const QMetaType::Type mtype = QEPlatform::metaType (this->pvData);
   return mtype != QMetaType::UnknownType;
}

//------------------------------------------------------------------------------
//
QVariant QEPvaClient::getPvData () const
{
   return this->pvData;
}

//------------------------------------------------------------------------------
//
bool QEPvaClient::putPvData (const QVariant& value)
{
   if (!this->isConnected) {
      return false;
   }

   const std::string pvName = this->getPvName().toStdString();

   this->putOp =
         pvxsContext
         .put(pvName)
         .build([this, value](pvxs::Value&& protoValue) -> pvxs::Value {
             // protoValue is like the pvValue with all values stripped out.
             return QEPvaClient::assignValue (this, value, protoValue);
          })
         .result([this, value](pvxs::client::Result&& protoResult) {
             const QString typeName = value.typeName();
             try {
                // always returns empty Value on success
                protoResult();  // otherwise raises an exception
//              this->processPutStatus (true, "okay", typeName);
             } catch (std::exception& e){
                this->processPutStatus (false, e.what(), typeName);
             }
         })
         .exec();

   return true;
}

//------------------------------------------------------------------------------
//
QString QEPvaClient::getId () const
{
   QString suffix = "";
   if (!this->pvType.isEmpty()) {
      suffix = QString (" (%2)").arg (this->pvType);
   }
   return QString ("%1%2").arg (this->id).arg (suffix);
}

//------------------------------------------------------------------------------
//
QString QEPvaClient::getRemoteAddress() const
{
   return this->remoteAddress;
}

//------------------------------------------------------------------------------
//
QString QEPvaClient::getEgu() const
{
   return this->display.units;
}

//------------------------------------------------------------------------------
//
int QEPvaClient::getPrecision() const
{
   return this->display.precision;
}

//------------------------------------------------------------------------------
//
unsigned int QEPvaClient::hostElementCount () const
{
   // These are treated the same for PV Acccess.
   //
   return this->dataElementCount();
}

//------------------------------------------------------------------------------
//
unsigned int QEPvaClient::dataElementCount () const
{
   const QMetaType::Type mtype = QEPlatform::metaType (this->pvData);

   int result = 0;

   if (mtype == QMetaType::QVariantList) {
      result = this->pvData.toList().count();

   } else if (QEVectorVariants::isVectorVariant (this->pvData)) {
      result = QEVectorVariants::vectorCount (this->pvData);

   } else if (mtype == QMetaType::QStringList) {
      result = this->pvData.toStringList().count();

   } else if (mtype != QMetaType::UnknownType) {
      // Assume singular value.
      result = 1;
   }

   return result;
}

//------------------------------------------------------------------------------
//
double QEPvaClient::getDisplayLimitHigh() const
{
   return this->display.limitHigh;
}

//------------------------------------------------------------------------------
//
double QEPvaClient::getDisplayLimitLow() const
{
   return this->display.limitLow;
}

//------------------------------------------------------------------------------
//
double QEPvaClient::getHighAlarmLimit() const
{
   return this->valueAlarm.highAlarmLimit;
}

//------------------------------------------------------------------------------
//
double QEPvaClient::getLowAlarmLimit() const
{
   return this->valueAlarm.lowAlarmLimit;
}

//------------------------------------------------------------------------------
//
double QEPvaClient::getHighWarningLimit() const
{
   return this->valueAlarm.highWarningLimit;
}

//------------------------------------------------------------------------------
//
double QEPvaClient::getLowWarningLimit() const
{
   return this->valueAlarm.lowWarningLimit;
}

//------------------------------------------------------------------------------
//
double QEPvaClient::getControlLimitHigh() const
{
   return this->control.limitHigh;
}

//------------------------------------------------------------------------------
//
double QEPvaClient::getControlLimitLow() const
{
   return this->control.limitLow;
}

//------------------------------------------------------------------------------
//
double QEPvaClient::getMinStep() const
{
   return this->control.minStep;
}

//------------------------------------------------------------------------------
//
QStringList QEPvaClient::getEnumerations () const
{
   return this->enumeration.choices;
}

//------------------------------------------------------------------------------
//
QCaAlarmInfo QEPvaClient::getAlarmInfo () const
{
   const QCaAlarmInfo::Status   status   = static_cast <QCaAlarmInfo::Status> (this->alarm.status);
   const QCaAlarmInfo::Severity severity = static_cast <QCaAlarmInfo::Severity> (this->alarm.severity);

   return QCaAlarmInfo (QEPvNameUri::pva, this->getPvName(), status, severity, this->alarm.message);
}

//------------------------------------------------------------------------------
//
QCaDateTime  QEPvaClient::getTimeStamp () const
{
   // PV Access and Channel Access use a different epoch times.
   // Must convert back to the CA reference to create a QCaDateTime.
   //
   return QCaDateTime (this->timeStamp.secondsPastEpoch - POSIX_TIME_AT_EPICS_EPOCH,
                       this->timeStamp.nanoseconds,
                       this->timeStamp.userTag);
}

//------------------------------------------------------------------------------
//
QString QEPvaClient::getDescription () const
{
   return this->display.description;
}

//------------------------------------------------------------------------------
//
bool QEPvaClient::getReadAccess() const
{
   return true;
}

//------------------------------------------------------------------------------
//
bool QEPvaClient::getWriteAccess() const
{
   return true;
}

//------------------------------------------------------------------------------
//
void QEPvaClient::processConnect (const pvxs::client::Connected& conn)
{
   this->remoteAddress = QString::fromStdString (conn.peerName);
   this->isConnected = true;
   this->firstUpdate = true;  // as in the next update will be the first (for this sunscription)

   // connection emit event.
   //
   if (this->allowSignalEmission) {
      emit this->connectionUpdated (true);
   }
}

//------------------------------------------------------------------------------
//
void QEPvaClient::processDisconnect (const pvxs::client::Disconnect& /* disconn */)
{
   this->remoteAddress = "";
   this->id = "";
   this->pvData = nullVariant;
   this->enumeration.isDefined = false;
   this->alarm.isDefined = false;
   this->timeStamp.isDefined = false;
   this->control.isDefined = false;
   this->display.isDefined = false;
   this->valueAlarm.isDefined = false;
   this->isConnected = false;

   // connection emit event.
   //
   if (this->allowSignalEmission) {
      emit this->connectionUpdated (false);
   }
}

//------------------------------------------------------------------------------
//
void QEPvaClient::processUpdate (const pvxs::Value& pvValue)
{
   // type identity e.g. epics:nt/NTScalar:1.0
   //
   this->id = QString::fromStdString (pvValue.id());

   // We extract and convert pvValue => pvData once.
   // (as opposed to on-the-fly each time getPvData is called).
   //
   bool status = QEPvaData::extractValue (pvValue, this->pvData, this->pvType);
   if (!status) {
      if (this->firstUpdate) {  // report only once.
         DEBUG << this->getPvName() << "cannot extract value from" << this->id;
      }
      this->firstUpdate = false;
      return;
   }

   const bool isEnum = pvValue.idStartsWith ("epics:nt/NTEnum:");
   bool isMetaUpdate = false;   // hypothesize not a meta data update.

   // Extract amd assihgn associated meta data.
   // Note: alarm and timeStamp data does not require is meta data
   // update considerations.
   //
   if (isEnum) {
      QEPvaData::Enumerated xenumeration;
      xenumeration.extract (pvValue);     // i.e. the choices
      this->enumeration.assign (xenumeration, isMetaUpdate);
   }

   this->alarm.extract (pvValue);
   this->timeStamp.extract (pvValue);

   QEPvaData::Display xdisplay;
   xdisplay.extract (pvValue);
   this->display.assign (xdisplay, isMetaUpdate);

   QEPvaData::Control xcontrol;
   xcontrol.extract (pvValue);
   this->control.assign (xcontrol, isMetaUpdate);

   QEPvaData::ValueAlarm xvalueAlarm;
   xvalueAlarm.extract (pvValue);
   this->valueAlarm.assign (xvalueAlarm, isMetaUpdate);

   // The first post connection update is always considered
   // a meta data update.
   //
   if (this->allowSignalEmission) {
      emit dataUpdated (this->firstUpdate || isMetaUpdate);
   }
   this->firstUpdate = false;
}

//------------------------------------------------------------------------------
//
static QEThreadSafeQueue <QEPvaClient::EventContext> pvaEventQueue;

#define ASSERT(condition, message) {                       \
   if(!(condition)) {                                      \
      DEBUG << message;                                    \
      return;                                              \
   }                                                       \
}

// Some assert checks occur and are semi-expected, e.g. when reconnecting
// all PV on a form. This is a no debug message assert macro.
//
#define AFFIRM(condition) {                                \
   if(!(condition)) {                                      \
      return;                                              \
   }                                                       \
}

//------------------------------------------------------------------------------
// eventCallback and timeoutHandler only do some basic checks.
// processEvents has to do the full check because by the time the
// item is pull off the pvaClientUpdateQueue, the client object may have
// been deleted andrepurposed.
//------------------------------------------------------------------------------
//
void QEPvaClient::processEvents (const EventContext& context)
{
   // Perform sanity checks.
   //
   ASSERT (this->magic == clientMagicNumber, "magic number mis-match");
   ASSERT (context.client, "null client");
   ASSERT (context.client == this, "specified client is not this client");
   ASSERT (context.clientId == this->clientId, "client id mis-match");
   ASSERT (context.subscriptionId == this->subscriptionId, "subscription id mis-match");
   ASSERT (this->subscription, "null subscription");

   // Should we limit number processed at any one time??
   //
   while (true) {
      try {
         // Does the subscription still exist?
         //
         while (this->subscription) {
            const pvxs::Value update = this->subscription->pop();
            if (!update) break;  // Update available?
            this->processUpdate (update);
         }
         break;

      } catch (pvxs::client::Connected& conn) {
         this->processConnect (conn);

      } catch (pvxs::client::Disconnect& conn) {
         this->processDisconnect (conn);

      } catch (std::exception& err) {
         DEBUG << this->getPvName() << " Error "
                << typeid (err).name() << " : " << err.what() <<"\n";
      }
   }
}

//------------------------------------------------------------------------------
// Puts a ref to the client on a queue of clients with "stuff-do-to"
// static
void QEPvaClient::eventCallback (const EventContext& context,
                                 pvxs::client::Subscription& sub)
{
   QEPvaClient* self = context.client;

   // Apply basic checks.
   //
   ASSERT (self, "null object");
   ASSERT (self->magic == clientMagicNumber, "magic number mis-match");
   ASSERT (context.clientId == self->clientId, "client id mis-match");
   ASSERT (context.subscriptionId == self->subscriptionId, "subscription id mis-match");

   const std::string pvName = self->getPvName().toStdString();
   ASSERT (pvName == sub.name(), "PV name mis-match");

   // We could of had an epicsEvent or semaphore per QEPvaClient, however
   // then we would need to:
   // a/ keep track of all of them; and
   // b/ check each one in turn to see if any events pending.
   //
   // So we just put a QEPvaClient object ref into a queue/list
   // which is examined by the poll function.
   //
   pvaEventQueue.enqueue (context);
}


//------------------------------------------------------------------------------
// static
pvxs::Value QEPvaClient::assignValue (QEPvaClient* self,
                                      const QVariant& value,
                                      const pvxs::Value& protoValue)
{
   pvxs::Value result = protoValue.cloneEmpty();
   if (!self) return result;

   // We assume it has a value field, i.e. is an epics:nt/NT.... type
   //
   bool status = QEPvaData::infuseValue (result, value);
   if (!status) {
      DEBUG << "assignValue: '" << self->getPvName() << "' failed "
            << value.typeName() << " data type\n";
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEPvaClient::processPutStatus (const bool status,
                                    const QString& message,
                                    const QString& additonal)
{
   DEBUG << "put '" << this->getPvName() << "' "
         << (status ? "success" : "failure")
         << ": " << message << " (" << additonal << ")";
}


//==============================================================================
// Helper class: QEPvaClientManager
//==============================================================================
//
QEPvaClientManager::QEPvaClientManager () : QObject (nullptr)
{
   // Schedule first poll event.
   //
   QTimer::singleShot (1, this, SLOT (timeoutHandler ()));
}

//------------------------------------------------------------------------------
//
QEPvaClientManager::~QEPvaClientManager ()
{
   pvaEventQueue.clear();
}

//------------------------------------------------------------------------------
//
void QEPvaClientManager::initialise ()
{
   // The singleton object created and seup when first needed, and not before.
   // Just declaring as a regular static QObject doesn't work as that would
   // require it to initialise before the construction of the QApplication.
   //
   static QEPvaClientManager singleton;
}

//------------------------------------------------------------------------------
//
void QEPvaClientManager::timeoutHandler ()
{
   while (true) {
      QEPvaClient::EventContext context = { nullptr, 0, 0 };
      const bool anyItem = pvaEventQueue.dequeue (context);

      if (!anyItem) break;  // all done for now

      // Onl do the not null refernce  check here, all the additioal sanity
      // checkes are done by processEvents.
      //
      QEPvaClient* self = context.client;
      if (self) {
         self->processEvents (context);
      } else {
         DEBUG << "null object\n";
      }
   }

   // Schedule another poll event - 16 mS approx 60Hz.
   // Note: the delay is relative to the end of processing the poll function.
   //
   QTimer::singleShot (16, this, SLOT (timeoutHandler ()));
}

// end
