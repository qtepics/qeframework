/*  QEArchiveInterfaceManager.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2021 Australian Synchrotron
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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

#include "QEArchiveInterfaceManager.h"

#include <QApplication>
#include <QDebug>
#include <QMutexLocker>
#include <QThread>

#include <QECommon.h>
#include <QEPvNameUri.h>
#include <QEArchiveManager.h>
#include <QEArchapplInterface.h>
#include <QEChannelArchiveInterface.h>

#define DEBUG  qDebug () << "QEArchiveInterfaceManager" << __LINE__ <<  __FUNCTION__  << "  "


//==============================================================================
// Conext objects used by the QEArchiveInterfaceManager
//==============================================================================
//
class QEArchiveInterfaceManager::NamesResponseContext : public QObject {
public:
   const QEArchiveInterfaceManager* archiveInterfaceManager;
   const int instance;

   explicit NamesResponseContext (const QEArchiveInterfaceManager* aim,
                                  const int instanceIn) :
      archiveInterfaceManager (aim),
      instance (instanceIn)
   { }
   ~NamesResponseContext () { }
};


//------------------------------------------------------------------------------
//
class QEArchiveInterfaceManager::ValuesResponseContext: public QObject {
public:
   const QEArchiveInterfaceManager* archiveInterfaceManager;
   const qint64 unique;

   explicit ValuesResponseContext (const QEArchiveInterfaceManager* aim,
                                   const qint64 uniqueIn) :
      archiveInterfaceManager (aim),
      unique (uniqueIn)
   { }
   ~ValuesResponseContext() { }
};


//==============================================================================
// QEArchiveInterfaceManager
//==============================================================================
//
QEArchiveInterfaceManager::QEArchiveInterfaceManager (const int instanceIn,
                                                      QEArchiveInterface* interfaceIn,
                                                      QEArchiveManager* ownerIn,
                                                      QObject* parent) :
   QTimer (parent),
   instance (instanceIn),
   archiveInterface (interfaceIn),
   owner (ownerIn)
{
   this->aimMutex = new QMutex();

   this->setSourceId (9001);
   this->state = QEArchiveInterface::Unknown;
   this->requestIndex = 0;
   this->archiveList.clear ();
   this->responseCount = 0;
   this->numberPVs = 0;
   this->unique = 0;

   QObject::connect (qApp, SIGNAL (aboutToQuit ()),
                     this, SLOT   (aboutToQuitHandler ()));

   QObject::connect (this, SIGNAL (timeout ()),
                     this, SLOT   (timeoutHandler ()));

   QObject::connect (this, SIGNAL (signalRequestArchives ()),
                     this, SLOT   (actionRequestArchives()));

   QObject::connect (this, SIGNAL (signalDataRequest (const QEArchiveAccess*,
                                                      const int,
                                                      const QEArchiveAccess::PVDataRequests&)),
                     this, SLOT   (actionDataRequest (const QEArchiveAccess*,
                                                      const int,
                                                      const QEArchiveAccess::PVDataRequests&)));

   // Signals from the archiveInterface
   #define ai this->archiveInterface

   QObject::connect (ai, SIGNAL (archivesResponse (const QObject*, const bool,
                                                   const QEArchiveInterface::ArchiveList&)),
                     this, SLOT (archivesResponse (const QObject*, const bool,
                                                   const QEArchiveInterface::ArchiveList&)));

   QObject::connect (ai, SIGNAL (pvNamesResponse (const QObject*, const bool,
                                                  const QEArchiveInterface::PVNameList&)),
                     this, SLOT (pvNamesResponse (const QObject*, const bool,
                                                  const QEArchiveInterface::PVNameList&)));

   QObject::connect (ai, SIGNAL (valuesResponse (const QObject*, const bool,
                                                 const QEArchiveInterface::ResponseValueList&)),
                     this, SLOT (valuesResponse (const QObject*, const bool,
                                                 const QEArchiveInterface::ResponseValueList&)));

   #undef ai
}

//------------------------------------------------------------------------------
//
QEArchiveInterfaceManager::~QEArchiveInterfaceManager ()
{
   delete this->aimMutex;
}

//------------------------------------------------------------------------------
// static
QEArchiveInterfaceManager* QEArchiveInterfaceManager::createInterfaceManager (
      const int instance,
      const QEArchiveAccess::ArchiverTypes archiverType,
      const QUrl& url,
      QEArchiveManager* owner)
{
   QEArchiveInterfaceManager* result = NULL;
   QEArchiveInterface* interface = NULL;
   QThread* thread = NULL;

   // Create the thread and the manager/worker.
   //
   switch (archiverType) {
      case QEArchiveAccess::CA:
         thread = new QThread (owner);
         interface = new QEChannelArchiveInterface (url, NULL);
         interface->moveToThread (thread);

         break;

      case QEArchiveAccess::ARCHAPPL:
         #ifdef QE_ARCHAPPL_SUPPORT
            thread = new QThread (owner);
            interface = new QEArchapplInterface (url, NULL);
         #else
            DEBUG << "Unexpected archiver type, no Archiver Appliance support";
            return NULL;
         #endif
         break;

      default:
         DEBUG << "Unexpected archiver type" << archiverType;
         return NULL;
   };

   // Object must be parent less to change thread affinity.
   //
   interface->moveToThread (thread);

   // Create the actual interface manager.
   //
   result = new QEArchiveInterfaceManager (instance, interface, owner, NULL);

   // Change the thread affinity for the interface manager object.
   //
   result->moveToThread (thread);

   // Lastly, connect the signal and start it.
   //
   QObject::connect (thread, SIGNAL (started ()),
                     result, SLOT   (started ()));

   thread->start ();
   return result;
}

//------------------------------------------------------------------------------
//
void QEArchiveInterfaceManager::getStatus (QEArchiveAccess::Status& status) const
{
   QUrl url = this->getUrl ();
   status.hostName = url.host ();
   status.portNumber = url.port();
   status.endPoint = url.path ();

   QMutexLocker locker (this->aimMutex);

   status.state = this->state;
   status.available = this->archiveList.count();
   status.read = this->responseCount;
   status.numberPVs = this->numberPVs;

   // Calculate the number of pending requwsts.
   //
   status.pending = 0;
   status.pending += this->archiveList.count() - this->responseCount;
   status.pending += this->requestQueue.count();
   status.pending += this->activeRequests.count();
}

//------------------------------------------------------------------------------
//
QUrl QEArchiveInterfaceManager::getUrl () const
{
   return this->archiveInterface->getUrl ();
}

//------------------------------------------------------------------------------
//
QString QEArchiveInterfaceManager::getName () const
{
   return this->archiveInterface->getName();
}


//------------------------------------------------------------------------------
// Archive requests
//------------------------------------------------------------------------------
//
void QEArchiveInterfaceManager::requestArchives ()
{
   emit this->signalRequestArchives();
   this->sendMessage (QString ("Requesting PV name info from ")
                      .append (this->getName ()),
                      message_types (MESSAGE_TYPE_INFO));
}

//------------------------------------------------------------------------------
// slot - from self
void QEArchiveInterfaceManager::actionRequestArchives ()
{
   this->state = QEArchiveInterface::Updating;
   this->numberPVs = 0;        // reset the count
   this->archiveInterface->archivesRequest (this);
}

//------------------------------------------------------------------------------
// slot - from archiveInterface
//
void QEArchiveInterfaceManager::archivesResponse (
      const QObject* userData,
      const bool isSuccess,
      const QEArchiveInterface::ArchiveList& archiveListIn)
{
   if (userData != this) {
      DEBUG  << "instance" << this->instance << "userData mis-match";
      return;
   }

   if (isSuccess) {
      QMutexLocker locker (this->aimMutex);

      this->archiveList = archiveListIn;
      this->requestIndex = 0;

      // We allow the 100mS timer to send staggered requests.
      // Only one for the archive applicance, but potentially many for the
      // Channel Access arhiver

   } else {
      QString message;
      message = QString ("request failure from %1").arg (this->getName ());
      this->sendMessage (message, message_types (MESSAGE_TYPE_ERROR));
      DEBUG << message;

      this->state = QEArchiveInterface::Error;
   }
}

//------------------------------------------------------------------------------
// Name requests
//------------------------------------------------------------------------------
// The is no requestNames function and associated signalRrequestNames signal
// as this is initiated internally.
//
void QEArchiveInterfaceManager::actionNamesRequest (const int index)
{
   // Sanity checks
   if (!this->archiveInterface) return;

   QEArchiveInterface::Archive archive;
   {
      QMutexLocker locker (this->aimMutex);
      if ((index < 0) || (index >= this->archiveList.count ())) return;
      archive = this->archiveList.value (index);
   }

   // Create the callback context.
   //
   NamesResponseContext* context = new NamesResponseContext (this, index);
   this->archiveInterface->namesRequest (context, archive.key, this->owner->getPattern());
}

//------------------------------------------------------------------------------
// slot - from archiveInterface
//
void QEArchiveInterfaceManager::pvNamesResponse (
      const QObject* userData,
      const bool isSuccess,
      const QEArchiveInterface::PVNameList& pvNameList)
{
   const NamesResponseContext* context =
         dynamic_cast <const NamesResponseContext*> (userData);

   if (!context || (context->archiveInterfaceManager != this)) {
      DEBUG  << "instance" << this->instance << "userData mis-match";
      // Should we delete userData in this situation?
      return;
   }

   QEArchiveInterface::Archive archive = this->archiveList.value (context->instance);
   if (isSuccess) {

      int number = pvNameList.count();
      this->numberPVs += number;

      // Hand off the the Archiver Manager.
      //
      emit this->aimPvNamesResponse (this, archive, pvNameList);

      this->responseCount++;
      if (this->responseCount == this->archiveList.count()) {
         this->state = QEArchiveInterface::Complete;
      } else {
         this->state = QEArchiveInterface::InComplete;
      }

      QString message;
      message = "PV name retrival from ";
      message.append (this->getName ());
      message.append (" complete");
      this->sendMessage (message);

   } else {
      this->sendMessage (QString ("PV names failure from ").
                         append (this->getName ()).
                         append (" for archive ").
                         append (archive.name),
                         message_types (MESSAGE_TYPE_ERROR));
   }

   if (context) delete context;
}


//------------------------------------------------------------------------------
// Data requests
//------------------------------------------------------------------------------
//
void QEArchiveInterfaceManager::dataRequest (const QEArchiveAccess* archiveAccess,
                                             const int key,
                                             const QEArchiveAccess::PVDataRequests& request)
{
   emit this->signalDataRequest (archiveAccess, key, request);
}

//------------------------------------------------------------------------------
// slot - from self
//
void QEArchiveInterfaceManager::actionDataRequest (
      const QEArchiveAccess* archiveAccess,
      const int key,
      const QEArchiveAccess::PVDataRequests& request)
{
   RequestInfo requestInfo;

   requestInfo.unique = -1;
   requestInfo.timeoutTime = QDateTime ();
   requestInfo.archiveAccess = archiveAccess;
   requestInfo.request = request;
   requestInfo.key = key;

   QMutexLocker locker (this->aimMutex);
   if (this->activeRequests.count() <= maxActiveQueueSize) {
      // activate the request immediately
      //
      this->activateDataRequest (requestInfo);
   } else {
      // place on queue for later activation
      //
      this->requestQueue.enqueue (requestInfo);
   }
}

//------------------------------------------------------------------------------
//
void QEArchiveInterfaceManager::activateDataRequest (RequestInfo& requestInfo)
{
   // No not calin mutex in this function.

   QDateTime timeNow = QDateTime::currentDateTime().toUTC();

   // Set unique identifer and timeout and add to the set of active requests.
   // Note: requests will timeout before unique is recycled unless the number
   // of requests exceeds ~70 million / second, so we are "safe".
   //
   this->unique++;
   requestInfo.unique = this->unique;
   requestInfo.timeoutTime = timeNow.addSecs (maxAllowedTime);
   this->activeRequests.insert (requestInfo.unique, requestInfo);

   // pass on to the inferface.

   QEArchiveAccess::PVDataRequests* request = & requestInfo.request;

   ValuesResponseContext* context =
         new ValuesResponseContext (this, requestInfo.unique);

   // Converts name to a list of one in order to satisfy the valuesRequest API.
   //
   QStringList pvNames;
   pvNames.append (request->pvName);

   this->archiveInterface->valuesRequest (
            context, request->startTime, request->endTime, request->count,
            request->how, pvNames,  request->key, request->element);
}

//------------------------------------------------------------------------------
// slot - from archiveInterface
//
void QEArchiveInterfaceManager::valuesResponse (
      const QObject* userData,
      const bool isSuccess,
      const QEArchiveInterface::ResponseValueList& valuesList)
{
   const ValuesResponseContext* context =
         dynamic_cast <const ValuesResponseContext*> (userData);

   if (!context || (context->archiveInterfaceManager != this)) {
      DEBUG  << "instance" << this->instance << "userData mis-match";
      // Should we delete userData in this situation
      return;
   }

   // Is this response expected, i.e. in the set of active requests?
   //
   if (!this->activeRequests.contains (context->unique)) {
      DEBUG  << "instance" << this->instance << "unique" << context->unique << "not active";
   }

   // Extract and remove from the active list.
   //
   RequestInfo requestInfo = this->activeRequests.take (context->unique);

   if (context) delete context;

   // Hand off the the Archiver Manager.
   //
   QEArchiveAccess::PVDataResponses response;

   response.userData = requestInfo.request.userData;
   response.isSuccess = ((isSuccess) && (valuesList.size () == 1));
   if (response.isSuccess) {
      response.pointsList = valuesList.front().dataPoints;
   }
   response.pvName = requestInfo.request.pvName;
   response.supplementary = response.isSuccess ? "okay" : "archiver response failure";

   // Hand off the the Archiver Manager.
   //
   emit this->aimDataResponse (requestInfo.archiveAccess, response);
}

//------------------------------------------------------------------------------
// slot
void QEArchiveInterfaceManager::started ()
{
   this->start (100);  // start own timer 100 mSec
}

//------------------------------------------------------------------------------
// slot
void QEArchiveInterfaceManager::aboutToQuitHandler ()
{
   QMutexLocker locker (this->aimMutex);
   this->stop();
   this->requestQueue.clear();
   this->activeRequests.clear();
}

//------------------------------------------------------------------------------
// slot
void QEArchiveInterfaceManager::timeoutHandler ()
{
   // Progress and time out any outstanding items

   // Are there any outstanding name requests?
   //
   if (this->requestIndex < this->archiveList.count()) {
      this->actionNamesRequest (this->requestIndex);
      this->requestIndex++;
   }

   // TODO: timeout any old request still awaiting a response

   // process active items if any.
   //
   QMutexLocker locker (this->aimMutex);
   while ((this->requestQueue.count() > 0) &&
          (this->activeRequests.count() <= maxActiveQueueSize))
   {
      RequestInfo info;
      info = this->requestQueue.dequeue();
      this->activateDataRequest (info);
   }
}

// end
