/*  QEPvaClient.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at
 *  the Australian Synchrotron.
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

#include "QEPvaClient.h"

#ifdef QE_INCLUDE_PV_ACCESS

#include <QApplication>
#include <QDebug>
#include <QQueue>
#include <QMutex>

#include <epicsTime.h>
#include <QEPvNameUri.h>
#include <pv/monitor.h>
#include <pv/pvAccess.h>
#include <pv/pvData.h>
#include <pv/clientFactory.h>
#include <QEVectorVariants.h>

#define DEBUG qDebug () << "QEPvaClient" << __LINE__ << __FUNCTION__ << "  "

#define MAGIC_VALUE  0x3243F6A8885A308D

static const QVariant nullVariant;

//==============================================================================
// QEPvaClientReference
//==============================================================================
// Replace with a weak pointer when I understand them better ??
//
class QEPvaClientReference {
public:
   explicit QEPvaClientReference (const QEPvaClient* clientIn,
                                  const uint64_t uniqueIdIn)
      : theClient (clientIn), theUniqueId (uniqueIdIn)
   {}

   ~QEPvaClientReference() {}

   QEPvaClient* getReference () const;

   inline const QEPvaClient* client () const { return this->theClient;   }
   inline uint64_t uniqueId () const         { return this->theUniqueId; }

private:
   const QEPvaClient* theClient;
   const uint64_t theUniqueId;
};

//------------------------------------------------------------------------------
//
QEPvaClient* QEPvaClientReference::getReference () const
{
   // TODO use a weak pointer to shared pointer conversion??
   //
   QEPvaClient* result = NULL;
   if (this->theClient &&
       (this->theClient->magic == MAGIC_VALUE) &&
       (this->theClient->uniqueId == this->theUniqueId)) {
      result = (QEPvaClient*) this->theClient;
   }
   return result;
}


//==============================================================================
// QEPvaClientUpdate
//==============================================================================
//
// Hold the data associated with a connection event or monitor event.
// Value data is held as a variant. Other support data, e.g. timestamp is
// held as a basic QEPvaData::TimeStamp class.
//
// This data is placed on an queue by the PVA callback threads (via overloaded
// virtual functions). And taken off the queue by the QEPvaClient running in the
// main application thread (as this thread only is allowed to update the widgets).
//
// Non Qt threads can't send signals. Potentially we could have placed the update
// on the event queue for the main thread.
//
class QEPvaClient::Update {
public:
   enum UpdateKind {
      ukConnection,
      ukData
   };

   explicit Update (const QEPvaClientReference& clientReference,
                    const QString id,
                    const UpdateKind kind,
                    const QVariant& pvData,
                    const bool isConnected);
   ~Update ();

   // process this update - intended to be called within the main Qt thread.
   //
   void process ();

   inline QEPvaClientReference getClientReference () const { return this->clientReference; }
   inline QString getId () const                { return this->id; }
   inline UpdateKind getKind () const           { return this->kind; }

   // Only one of the following makes sense depending on the update kind.
   //
   inline QVariant getPvData () const           { return this->pvData; }
   inline bool getIsConnected () const          { return this->isConnected; }

   QEPvaData::Enumerated enumeration;
   QEPvaData::Alarm alarm;
   QEPvaData::TimeStamp timeStamp;
   QEPvaData::Control control;
   QEPvaData::Display display;
   QEPvaData::ValueAlarm valueAlarm;

private:
   const QEPvaClientReference clientReference;
   const QString id;
   const UpdateKind kind;
   QVariant pvData;
   bool isConnected;
};

//------------------------------------------------------------------------------
//
QEPvaClient::Update::Update (const QEPvaClientReference& clientReferenceIn,
                             const QString idIn,
                             const UpdateKind kindIn,
                             const QVariant& pvDataIn,
                             const bool isConnectedIn) :
   clientReference (clientReferenceIn),
   id (idIn),
   kind (kindIn)
{
   this->pvData = pvDataIn;
   this->isConnected = isConnectedIn;
}

//------------------------------------------------------------------------------
//
QEPvaClient::Update::~Update () { }

//------------------------------------------------------------------------------
//
void QEPvaClient::Update::process()
{
   // Convert to valid reference if we can.
   //
   QEPvaClient* validClient = this->clientReference.getReference();
   if (validClient) {
      // The client does the actual processing.
      //
      validClient->processUpdate (this);
   }
}

//==============================================================================
// Wrapper around a QQueue with mutex.
//==============================================================================
//
class QEPvaClient::UpdateQueue {
public:
   explicit UpdateQueue();
   ~UpdateQueue();

   // Thread safe enqueue
   void enqueue (QEPvaClient::Update* update);

   // Thread safe dequeue - returns NULL if/when the queue is empty.
   QEPvaClient::Update* dequeue();

private:
   mutable QMutex mutex;

   typedef QQueue <QEPvaClient::Update*> PvaLoadQueue;
   PvaLoadQueue actualQueue;
};

//------------------------------------------------------------------------------
//
static QEPvaClient::UpdateQueue* pvaClientUpdateQueue = NULL;

//------------------------------------------------------------------------------
//
QEPvaClient::UpdateQueue::UpdateQueue () { }

//------------------------------------------------------------------------------
//
QEPvaClient::UpdateQueue::~UpdateQueue ()
{
   QMutexLocker locker (&this->mutex);
   while (!this->actualQueue.isEmpty()) {
      QEPvaClient::Update* item = this->actualQueue.dequeue();
      delete item;
   }
}

//------------------------------------------------------------------------------
//
void QEPvaClient::UpdateQueue::enqueue (QEPvaClient::Update* update)
{
   QMutexLocker locker (&this->mutex);
   this->actualQueue.enqueue (update);
}

//------------------------------------------------------------------------------
//
QEPvaClient::Update* QEPvaClient::UpdateQueue::dequeue()
{
   QMutexLocker locker (&this->mutex);
   QEPvaClient::Update* result = NULL;

   if (!this->actualQueue.isEmpty()) {
      result = this->actualQueue.dequeue();
   }
   return result;
}

//==============================================================================
// Channel Requester Get, Monitor and Put implementation interface classes
//==============================================================================
//
namespace pva = epics::pvAccess;
namespace pvd = epics::pvData;

//==============================================================================
//
// All interface classes also inherit from this class.
// It provides client reference, PV name and message handling.
//
class QEPvaRequesterCommon
{
public:
   explicit QEPvaRequesterCommon (const QEPvaClientReference& clientReferenceIn) :
      clientReference (clientReferenceIn),
      pvName (clientReferenceIn.client()->getPvName ()) { }

   ~QEPvaRequesterCommon () { }
   inline uint64_t uniqueId () const  { return this->clientReference.uniqueId (); }

protected:
   const QEPvaClientReference clientReference;
   const QString pvName;

   void handleMessage (std::string const & message, pvd::MessageType messageType)
   {
      DEBUG << this->pvName
            << pvd::getMessageTypeName (messageType).c_str()
            << message.c_str();
   }
};

//==============================================================================
// QEPvaChannelRequesterInterface
//==============================================================================
// Listener for connect state changes.
//
class QEPvaChannelRequesterInterface :
      public pva::ChannelRequester,
      public QEPvaRequesterCommon
{
public:
   explicit QEPvaChannelRequesterInterface (const QEPvaClientReference& clientReference) :
      QEPvaRequesterCommon (clientReference) {}
   ~QEPvaChannelRequesterInterface () {}

   // overide parent virtual functions.
   //
   std::string getRequesterName () { return "QEPvaClient.ChannelRequester"; }

   void message (std::string const & message, pvd::MessageType messageType)
   { this->handleMessage (message, messageType); }

   void channelCreated (const pvd::Status& status,
                        pva::Channel::shared_pointer const & channel);

   void channelStateChange (pva::Channel::shared_pointer const & channel,
                            pva::Channel::ConnectionState connectionState);
};

//------------------------------------------------------------------------------
//
void QEPvaChannelRequesterInterface::channelCreated (const pvd::Status& status,
                                                     pva::Channel::shared_pointer const& )
{
   if (!status.isSuccess()) {
      DEBUG << this->pvName << "isSuccess" << status.isSuccess();
   }
}

//------------------------------------------------------------------------------
//
void QEPvaChannelRequesterInterface::channelStateChange (pva::Channel::shared_pointer const &,
                                                         pva::Channel::ConnectionState connectionState)
{
// DEBUG << this->pvName << Channel::ConnectionStateNames[connectionState];

   QEPvaClient::Update* item = NULL;

   switch (connectionState) {

      case pva::Channel::NEVER_CONNECTED:
         break;

      case pva::Channel::CONNECTED:
         item = new QEPvaClient::Update (this->clientReference, "",
                                         QEPvaClient::Update::ukConnection,
                                         nullVariant,
                                         true);
         pvaClientUpdateQueue->enqueue (item);
         break;

      case pva::Channel::DISCONNECTED:
         item = new QEPvaClient::Update (this->clientReference, "",
                                         QEPvaClient::Update::ukConnection,
                                         nullVariant,
                                         false);
         pvaClientUpdateQueue->enqueue (item);
         break;

      case pva::Channel::DESTROYED:
         break;
   }
}

//==============================================================================
// QEPvaMonitorRequesterInterface
//==============================================================================
//
class QEPvaMonitorRequesterInterface :
      public pvd::MonitorRequester,
      public QEPvaRequesterCommon
{
public:
   explicit QEPvaMonitorRequesterInterface (const QEPvaClientReference& clientReference) :
      QEPvaRequesterCommon (clientReference) {}
   ~QEPvaMonitorRequesterInterface () {}

   // overide parent virtual functions.
   //
   std::string getRequesterName () { return "QEPvaClient.Monitor"; }

   void message (std::string const & message, pvd::MessageType messageType)
   { this->handleMessage (message, messageType); }

   void monitorConnect (pvd::Status const & status,
                        pva::MonitorPtr const & monitor,
                        pvd::StructureConstPtr const & structure);

   void channelDisconnect (bool destroy);

   void monitorEvent (pva::MonitorPtr const & monitor);

   void unlisten (pva::MonitorPtr const & monitor);

   void stopMonitor ();

private:
   void processElement (pva::MonitorElement::const_shared_pointer element);
};

//------------------------------------------------------------------------------
// Duplicated connection reported by channelStateChange
//
void QEPvaMonitorRequesterInterface::monitorConnect (pvd::Status const & status,
                                                     pva::MonitorPtr const & monitor,
                                                     pvd::StructureConstPtr const &)
{
   if (status.isSuccess()) {
      pvd::Status startStatus = monitor->start ();
      if (!startStatus.isSuccess())  {
          DEBUG << this->pvName <<  "channel monitor start" << startStatus.getMessage().c_str();
      }

   } else {
       DEBUG << this->pvName << "monitorConnect" << status.getMessage().c_str();
   }
}

//------------------------------------------------------------------------------
//
void QEPvaMonitorRequesterInterface::channelDisconnect (bool destroy)
{
   if (destroy) {
      DEBUG << ">>>>" << this->pvName << this->uniqueId () << "  destroy" << destroy;
   }
}

//------------------------------------------------------------------------------
//
void QEPvaMonitorRequesterInterface::monitorEvent (pva::MonitorPtr const & monitor)
{
   // OPTIONS...
   // Poll all, but only process last element ?
   // Put the element on the queue and process in main thread ?
   // Put the element on a queue and process in worker thread and then place
   // processed data on 2nd queue (or use a signal/slot) for main thread ?
   //
   while (true) {
      pva::MonitorElement::shared_pointer element = monitor->poll ();
      if (!element) break;
      this->processElement (element);
      monitor->release (element);
   }
}

//------------------------------------------------------------------------------
//
void QEPvaMonitorRequesterInterface::processElement (pva::MonitorElement::const_shared_pointer element)
{
// pvd::PVStructure::const_shared_pointer pv = element->pvStructurePtr; - a lot easier with not a const shared ptr
// Make a bunch of NT utility functions more usefull

   pvd::PVStructure::shared_pointer pv = element->pvStructurePtr;
   pvd::StructureConstPtr ptr = pv->getStructure();
   if (!ptr) {
      DEBUG << this->pvName << this->uniqueId() << "no pv structure";
      return;
   }

   const QString pvIdentity = QString::fromStdString (ptr->getID ());

   // The extracted value is a basic variant, a QE vector varient or one
   // of the specialised variants: QENTTableData, QENTImageData.
   //
   QVariant value;
   const bool status = QEPvaData::extractValue (pv, value);
   if(!status) {
      DEBUG << this->pvName << this->uniqueId() << "cannot extract value from" << pvIdentity;
      return;
   }

   // Create the update item
   //
   QEPvaClient::Update* item =
         new QEPvaClient::Update (this->clientReference, pvIdentity,
                                  QEPvaClient::Update::ukData, value, false);

   // Extract associated meta data
   //
   item->enumeration.extract (pv);     // i.e. the choices
   item->timeStamp.extract (pv);
   item->alarm.extract (pv);
   item->control.extract (pv);
   item->display.extract (pv );
   item->valueAlarm.extract (pv);

   // We have copied all the element data.
   //
   pvaClientUpdateQueue->enqueue (item);
}

//------------------------------------------------------------------------------
//
void QEPvaMonitorRequesterInterface::unlisten (pva::MonitorPtr const & monitor)
{
   DEBUG << this->pvName;
   monitor->stop ();
}

//==============================================================================
// Get - place holder
//==============================================================================
//
class QEPvaGetRequesterInterface :
      public pva::ChannelGetRequester,
      public QEPvaRequesterCommon
{
public:
   explicit QEPvaGetRequesterInterface (const QEPvaClientReference& clientReference) :
      QEPvaRequesterCommon (clientReference) {}
   ~QEPvaGetRequesterInterface () {}

   // overide parent virtual functions.
   //
   std::string getRequesterName () { return "QEPvaClient.GetRequester"; }

   void message (std::string const & message, pvd::MessageType messageType)
   { this->handleMessage (message, messageType); }


   void channelGetConnect (const pvd::Status& /* status */,
                           pva::ChannelGet::shared_pointer const & /* channelGet */,
                           pvd::Structure::const_shared_pointer const & /* structure */) {}

   void channelDisconnect(bool /* destroy */) {}

   void getDone (const pvd::Status& /* status */,
                 pva::ChannelGet::shared_pointer const & /* channelGet */,
                 pvd::PVStructure::shared_pointer const & /* pvStructure */,
                 pvd::BitSet::shared_pointer const & /* bitSet */) {}
};


//==============================================================================
// Put - place holder
//==============================================================================
//
class QEPvaPutRequesterInterface :
      public pva::ChannelPutRequester,
      public QEPvaRequesterCommon
{
public:
   explicit QEPvaPutRequesterInterface (const QEPvaClientReference& clientReference) :
      QEPvaRequesterCommon (clientReference)
   {
      this->putIsConnected = false;
   }
   ~QEPvaPutRequesterInterface () {}

   // overide parent virtual functions.
   //
   std::string getRequesterName () { return "QEPvaClient.PutRequester"; }

   void message (std::string const & message, pvd::MessageType messageType)
   { this->handleMessage (message, messageType); }

   void channelPutConnect(
       const pvd::Status& status,
       pva::ChannelPut::shared_pointer const & channelPut,
       pvd::Structure::const_shared_pointer const & structure);

   void putDone(
       const pvd::Status& status,
       pva::ChannelPut::shared_pointer const & channelPut);

   void getDone(
       const pvd::Status& status,
       pva::ChannelPut::shared_pointer const & channelPut,
       pvd::PVStructure::shared_pointer const & pvStructure,
       pvd::BitSet::shared_pointer const & bitSet);

   pvd::StructureConstPtr getStructure() const { return this->structure; }
   bool putPvData (QEPvaClient* client, const QVariant& value);

private:
   bool putIsConnected;
   pvd::StructureConstPtr structure;
   QVariant pvData;
};

//------------------------------------------------------------------------------
//
void QEPvaPutRequesterInterface::channelPutConnect(
    const pvd::Status& status,
    pva::ChannelPut::shared_pointer const & /* channelPut */,
    pvd::Structure::const_shared_pointer const & structureIn)
{
   this->putIsConnected = status.isSuccess();
   if (this->putIsConnected) {
      this->structure = structureIn;
   } else {
      this->structure.reset();
   }
}

//------------------------------------------------------------------------------
//
void QEPvaPutRequesterInterface::putDone(
    const pvd::Status& status,
    pva::ChannelPut::shared_pointer const & /* channelPut */)
{
   if (!status.isSuccess()) {
      DEBUG << this->pvName << this->uniqueId() << "failure";
   }
}

//------------------------------------------------------------------------------
//
void QEPvaPutRequesterInterface::getDone(
    const pvd::Status& status,
    pva::ChannelPut::shared_pointer const & /* channelPut */,
    pvd::PVStructure::shared_pointer const & /* pvStructure */,
    pvd::BitSet::shared_pointer const & /* bitSet */)
{
   DEBUG << this->pvName<< this->uniqueId()  << status.isSuccess();
}

//------------------------------------------------------------------------------
//
bool QEPvaPutRequesterInterface::putPvData (QEPvaClient* client, const QVariant& value)
{
   if (!this->putIsConnected) return false;

   epics::pvData::PVStructurePtr result;
   result = pvd::getPVDataCreate()->createPVStructure (structure);

   bool okay = QEPvaData::infuseValue (result, value);
   if (!okay) {
      DEBUG << this->pvName << this->uniqueId() << "infuse value fail" << value;
      return false;
   }

   pvd::BitSet* bits = new pvd::BitSet();
   pvd::BitSet::shared_pointer putBitSet (bits);
   bits->clear();
   bits->set (0);  // i.e. the first or value bit

   client->putter->put (result, putBitSet);
   return true;
}


//==============================================================================
// QEPvaClient
//==============================================================================

static pva::ChannelProvider::shared_pointer pvaProvider = NULL;

//------------------------------------------------------------------------------
//
QEPvaClient::QEPvaClient (const QString& pvName,
                          QObject* parent) :
   QEBaseClient (QEBaseClient::PVAType, pvName, parent)
{
   QEPvaClientManager::initialise ();  // idempotent - do first.

   // Allocate this object a unique id, being 64 bits we won't run out.
   //
   static uint64_t nextUniqueId = 0;
   nextUniqueId++;
   this->uniqueId = nextUniqueId;

   this->getter = NULL;
   this->putter = NULL;
   this->monitor = NULL;
   this->channel = NULL;

   this->id = "";
   this->pvData = nullVariant;
   this->firstUpdate = false;

   // Create the channel, monitor, put and get requestor and convert to saved shared pointers
   //
   QEPvaClientReference clientReference (this, this->uniqueId);

   QEPvaChannelRequesterInterface* ri = new QEPvaChannelRequesterInterface (clientReference);
   this->channelRequester = pva::ChannelRequester::shared_pointer (ri);

   QEPvaMonitorRequesterInterface* mi = new QEPvaMonitorRequesterInterface (clientReference);
   this->monitorRequester = pva::MonitorRequester::shared_pointer (mi);

   QEPvaPutRequesterInterface* pi = new QEPvaPutRequesterInterface (clientReference);
   this->putRequester = pva::ChannelPutRequester::shared_pointer (pi);

   QEPvaGetRequesterInterface* gi = new QEPvaGetRequesterInterface (clientReference);
   this->getRequester = pva::ChannelGetRequester::shared_pointer (gi);

   // Lastly set the class type magic value.
   //
   this->magic = MAGIC_VALUE;
}

//------------------------------------------------------------------------------
//
QEPvaClient::~QEPvaClient()
{
   this->closeChannel ();
   this->magic = 0;
   this->uniqueId = 0;
   this->getRequester.reset();
   this->putRequester.reset();
   this->monitorRequester.reset ();
   this->channelRequester.reset ();
}

//------------------------------------------------------------------------------
//
bool QEPvaClient::openChannel (const ChannelModesFlags modes)
{
   static const std::string monitorRequest ("field()");   // the lot - all fields
   static const std::string putRequest ("field(value)");  // just the value

   bool result = false;
   pvd::PVStructure::shared_pointer pvRequest;

   if (modes == ChannelModes::None) {
      return false;
   }

   // We need to hold a reference to the channel to keep it "alive"
   // The channel keeps the requestor and the monitor "alive".
   //
   this->channel = pvaProvider->createChannel (this->getPvName().toStdString(),
                                               this->channelRequester, 10);

   // Now configure the channel.
   // User has requested read/get mode.
   //
   if (modes & ChannelModes::Read) {
      pvRequest = pvd::CreateRequest::create()->createRequest (monitorRequest);
      if (pvRequest.get()) {
         this->getter = this->channel->createChannelGet (this->getRequester, pvRequest);
         result = true;
      } else {
         DEBUG << "failed to parse get request string";
      }
   }

   // User has requested monitor/subscription mode.
   //
   if (modes & ChannelModes::Monitor) {
      // Create the monitor
      pvRequest = pvd::CreateRequest::create()->createRequest (monitorRequest);
      if (pvRequest.get()) {
         this->monitor = this->channel->createMonitor (this->monitorRequester, pvRequest);
         result = true;
      } else {
         DEBUG << "failed to parse monitor request string";
      }
   }

   // User has requested write/put mode.
   //
   if (modes & ChannelModes::Write) {
      pvRequest = pvd::CreateRequest::create()->createRequest (putRequest);
      if (pvRequest.get()) {
         this->putter = this->channel->createChannelPut (this->putRequester, pvRequest);
         result = true;
      } else {
          DEBUG << "failed to parse put request string";
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEPvaClient::closeChannel ()
{
   if (this->getter) {
      this->getter->destroy();
   }
   if (this->putter) {
      this->putter->destroy();
   }
   if (this->monitor) {
      this->monitor->stop ();
      this->monitor->destroy ();
   }
   if (this->channel) {
      this->channel->destroy ();
   }

   this->getter.reset ();
   this->putter.reset ();
   this->monitor.reset ();
   this->channel.reset ();
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
   // sainity checks
   if (this->putter.get() == NULL || this->putRequester.get() == NULL) {
      DEBUG << "missing objects";
      return false;
   }

   QEPvaPutRequesterInterface* interface =
         dynamic_cast <QEPvaPutRequesterInterface*> (this->putRequester.get ());
   if (!interface) {
      DEBUG << "dynamic cast failed. objects";
      return false;
   }

   return interface->putPvData (this, value);
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
   return this->pvData.type() != QVariant::Invalid;
}

//------------------------------------------------------------------------------
//
QString QEPvaClient::getEgu () const
{
   return this->display.units;
}

//------------------------------------------------------------------------------
//
QString QEPvaClient::getId () const
{
   return this->id;
}

//------------------------------------------------------------------------------
//
QStringList QEPvaClient::getEnumerations () const
{
   return this->enumeration.choices;
}

//------------------------------------------------------------------------------
//
int QEPvaClient::getPrecision () const
{
   return this->display.precision;
}

//------------------------------------------------------------------------------
//
unsigned int QEPvaClient::dataElementCount () const
{
   const int type = this->pvData.type();

   int result = 0;

   if (type == QVariant::List) {
      result = this->pvData.toList().count();

   } else if (QEVectorVariants::isVectorVariant (this->pvData)) {
      result = QEVectorVariants::vectorCount (this->pvData);

   } else if (type == QVariant::StringList) {
      result = this->pvData.toStringList().count();

   } else if (type != QVariant::Invalid) {
      // Assume singular value.
      result = 1;
   }

   return result;
}

//------------------------------------------------------------------------------
//
double QEPvaClient::getDisplayLimitHigh () const
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
// More or less equivilent to getHostName
//
QString QEPvaClient::getRemoteAddress() const
{
   QString result;
   if (this->channel && this->channel->isConnected()) {
      result = QString::fromStdString (this->channel->getRemoteAddress());
   }
   return result;
}

//------------------------------------------------------------------------------
//
QCaAlarmInfo QEPvaClient::getAlarmInfo () const
{
   const QCaAlarmInfo::Status   status   = (QCaAlarmInfo::Status) this->alarm.status;
   const QCaAlarmInfo::Severity severity = (QCaAlarmInfo::Severity) this->alarm.severity;

   return QCaAlarmInfo (QEPvNameUri::pva, this->getPvName(), status, severity, this->alarm.message);
}

//------------------------------------------------------------------------------
//
QCaDateTime QEPvaClient::getTimeStamp () const
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
void QEPvaClient::processUpdate (QEPvaClient::Update* update)
{
   if (!update) return;  // sanity check

   const QEPvaClient::Update::UpdateKind kind = update->getKind();

   switch (kind) {
      case QEPvaClient::Update::ukConnection:
         this->isConnected = update->getIsConnected();
         if (!this->isConnected) {
            this->id = "";
            this->pvData = nullVariant;
            this->enumeration.isDefined = false;
            this->alarm.isDefined = false;
            this->timeStamp.isDefined = false;
            this->control.isDefined = false;
            this->display.isDefined = false;
            this->valueAlarm.isDefined = false;
         }
         emit connectionUpdated (this->isConnected);
         this->firstUpdate = true;
         break;

      case QEPvaClient::Update::ukData:
         this->id = update->getId ();
         this->pvData = update->getPvData ();

         // Assign other items.
         //
         this->alarm.assign (update->alarm);
         this->timeStamp.assign (update->timeStamp);
         this->display.assign (update->display);
         this->control.assign (update->control);
         this->valueAlarm.assign (update->valueAlarm);
         this->enumeration.assign (update->enumeration);

         emit dataUpdated (this->firstUpdate);
         this->firstUpdate = false;
         break;

      default:
         DEBUG << "Unexpected update kind" << int (kind);
         break;
   }
}


//==============================================================================
// QEPvaClientTimer and queue
//==============================================================================
//
QEPvaClientManager* singleton = NULL;

//------------------------------------------------------------------------------
// static
void QEPvaClientManager::initialise ()
{
   if (!singleton) {
      singleton = new QEPvaClientManager ();
   }
}

//------------------------------------------------------------------------------
//
QEPvaClientManager::QEPvaClientManager () : QTimer (NULL)
{
   // Create the update queue.
   //
   pvaClientUpdateQueue = new QEPvaClient::UpdateQueue ();

   // Connect to the about to quit signal.
   // Note: qApp is defined in QApplication.
   //
   QObject::connect (qApp, SIGNAL (aboutToQuit ()),
                     this, SLOT   (aboutToQuitHandler ()));

   // Connect and start regular timed event.
   //
   QObject::connect (this, SIGNAL (timeout ()),
                     this, SLOT   (timeoutHandler ()));

   pva::ClientFactory::start();

   pva::ChannelProviderRegistry::shared_pointer providerRegistry = pva::ChannelProviderRegistry::clients();
   pvaProvider = providerRegistry->getProvider("pva");

   this->start (16);   // 16 mSec ~ 60 Hz.
}

//------------------------------------------------------------------------------
//
QEPvaClientManager::~QEPvaClientManager ()
{
   pva::ClientFactory::stop();
   delete pvaClientUpdateQueue;
   pvaClientUpdateQueue = NULL;
}

//------------------------------------------------------------------------------
//
//slots
void QEPvaClientManager::timeoutHandler ()
{
   while (pvaClientUpdateQueue) {
      QEPvaClient::Update* item;
      item = pvaClientUpdateQueue->dequeue ();
      if (!item) break;  // all done
      item->process();
      delete item;
   }
}

//------------------------------------------------------------------------------
//
void QEPvaClientManager::aboutToQuitHandler ()
{
   this->stop ();  // Stop the timer.
}

#else

// QE_INCLUDE_PV_ACCESS not defined - just provide stubb functions.

QEPvaClient::QEPvaClient (const QString& pvName,
                          QObject* parent) :
   QEBaseClient (QEBaseClient::PVAType, pvName, parent) { }
QEPvaClient::~QEPvaClient () { }
bool QEPvaClient::openChannel (const ChannelModesFlags) { return false; }
void QEPvaClient::closeChannel () { }
bool QEPvaClient::getIsConnected () const { return false; }
bool QEPvaClient::dataIsAvailable () const { return false; }
QString QEPvaClient::getId () const { return ""; }
QString QEPvaClient::getRemoteAddress() const { return ""; }
QString QEPvaClient::getEgu() const { return ""; }
int QEPvaClient::getPrecision() const { return 0; }
unsigned int QEPvaClient::dataElementCount () const { return 0; }
double QEPvaClient::getDisplayLimitHigh() const { return 0.0; }
double QEPvaClient::getDisplayLimitLow() const { return 0.0; }
double QEPvaClient::getHighAlarmLimit() const { return 0.0; }
double QEPvaClient::getLowAlarmLimit() const { return 0.0; }
double QEPvaClient::getHighWarningLimit() const { return 0.0; }
double QEPvaClient::getLowWarningLimit() const { return 0.0; }
double QEPvaClient::getControlLimitHigh() const { return 0.0; }
double QEPvaClient::getControlLimitLow() const { return 0.0; }
double QEPvaClient::getMinStep() const { return 0.0; }
QVariant QEPvaClient::getPvData () const { QVariant d; return d; }
bool QEPvaClient::putPvData (const QVariant&) { return false; }
QStringList QEPvaClient::getEnumerations () const { QStringList d; return d; }
QCaAlarmInfo QEPvaClient::getAlarmInfo () const { QCaAlarmInfo d; return d; }
QCaDateTime  QEPvaClient::getTimeStamp () const { QCaDateTime d; return d; }
QString QEPvaClient::getDescription () const { return ""; }
void QEPvaClient::processUpdate (QEPvaClient::Update*) { }

QEPvaClientManager::QEPvaClientManager () { }
QEPvaClientManager::~QEPvaClientManager () { }
void QEPvaClientManager::initialise () { }
void QEPvaClientManager::timeoutHandler () { }
void QEPvaClientManager::aboutToQuitHandler () { }

#endif

// end
