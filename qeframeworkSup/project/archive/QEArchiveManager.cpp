/*  QEArchiveManager.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2021 Australian Synchrotron
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
 *    Andraz Pozar
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *    andraz.pozar@synchrotron.org.au
 */

#include "QEArchiveManager.h"
#include <stdlib.h>
#include <QApplication>
#include <QDebug>
#include <QHash>
#include <QMap>
#include <QList>
#include <QMutex>
#include <QRegExp>
#include <QThread>
#include <QUrl>

#include <QECommon.h>
#include <QEAdaptationParameters.h>
#include <QEPvNameUri.h>


#define DEBUG  qDebug () << "QEArchiveManager" << __LINE__ <<  __FUNCTION__  << "  "

//==============================================================================
//
class NamesResponseContext : public QObject {
public:
   QEArchiveInterface* interface;
   QEArchiveInterface::Archive archive;
   int instance;

   // constructor
   NamesResponseContext (QEArchiveInterface * interfaceIn,
                         QEArchiveInterface::Archive archiveIn,
                         int i)
   {
      this->interface = interfaceIn;
      this->archive = archiveIn;
      this->instance = i;   // used to determine if this last received, i.e complete. TODO maybe be received in order.
   }
};

//==============================================================================
//
class ValuesResponseContext : public QObject {
public:
   const QEArchiveAccess* archiveAccess;
   QString pvName;
   QObject* userData;

   // constructor
   ValuesResponseContext (const QEArchiveAccess* archiveAccessIn,
                          const QString& pvNameIn,
                          QObject* userDataIn)
   {
      this->archiveAccess = archiveAccessIn;
      this->pvName = pvNameIn;
      this->userData = userDataIn;
   }
};

//------------------------------------------------------------------------------
// This is just a regular thread
//
class QEArchiveThread : public QThread {
};


//==============================================================================
// Local Data
//==============================================================================
// We declare these items here as opposed as static members of the class because
// the latter made all the EPICS plugin widgets "go away" in designer.
// I think the are issues when QObjects declared in header files.
//

// Allows only one object to be effectively created. Second and subsequent object
// do nothing, except waste space.
//
static QMutex *singletonMutex = new QMutex ();
static QEArchiveThread *singletonThread = NULL;
static QMutex *archiveDataMutex = new QMutex ();


//==============================================================================
// QEArchiveManager Class Methods
//==============================================================================
//
//The singleton manager object is an orphan because we move it to singletonThread.
//
QEArchiveManager::QEArchiveManager() {

   QEAdaptationParameters ap ("QE_");

   QString archives = ap.getString ("archive_list", "");
   QString pattern  = ap.getString ("archive_pattern", ".*");

   this->allArchivesRead = false;
   this->numberArchivesRead = 0;
   this->environmentErrorReported = false;
   this->timer = new QTimer (this);
   this->setSourceId (9001);

   if (!archives.isEmpty ()) {
      this->archives = archives;
      this->pattern = pattern;
      this->lastReadTime = QDateTime::currentDateTime ().toUTC ().addSecs (-300);

   } else {
      this->archives = "";
      this->pattern = "";

      // Has this error already been reported??
      // Not strictly 100% thread safe but not strictly critical either.
      //
      if (!environmentErrorReported) {
         environmentErrorReported = true;
         qDebug() << "QE_ARCHIVE_LIST undefined. This is required to be defined in order to backfill QEStripChart widgets.";
         qDebug() << "Define as space delimited archiver URLs";
      }
   }
}

//------------------------------------------------------------------------------
//
QEArchiveManager::~QEArchiveManager() { }

//------------------------------------------------------------------------------
//
void QEArchiveManager::clear ()
{
   this->allArchivesRead = false;
   this->numberArchivesRead = 0;
   this->pvNameToSourceLookUp.clear ();
}

//------------------------------------------------------------------------------
// slot
void QEArchiveManager::started ()
{
   QStringList archiveList;
   QString item;
   QUrl url;
   QEArchiveInterface* interface = NULL;

   // Connect to the about to quit signal.
   // Note: qApp is defined in QApplication
   //
   QObject::connect (qApp, SIGNAL (aboutToQuit ()),
                     this, SLOT   (aboutToQuitHandler ()));

   // Split input string using space as delimiter.
   // Could extend to use regular expression and split on any white space character.
   //
   archiveList = archives.split (' ', QString::SkipEmptyParts);

   if (archiveList.count () == 0) {
      DEBUG << "no archives specified";
      this->sendMessage ("QEArchiveManager: no archives specified",
                         message_types (MESSAGE_TYPE_INFO));
      return;
   }

   this->clear ();

   this->sendMessage (QString ("pattern: ").append (pattern),
                      message_types (MESSAGE_TYPE_INFO));

   for (int j = 0; j < archiveList.count (); j++) {

      QString item = archiveList.value (j).indexOf("://") <= 0 ? "http://" : "";
      item.append(archiveList.value (j));

      url = QUrl (item);
      if (url.isValid()) {
         if (url.port() == -1) {
            url.setPort(80);
         }
      } else {
         DEBUG << "not a valid URL: " << archiveList.value (j);
         this->sendMessage (QString ("not a valid URL: ").append (archiveList.value (j)),
                            message_types (MESSAGE_TYPE_ERROR));
         return;
      }

      // Create and save a reference to each interface.
      //
      switch (this->archiverType){
         case (QEArchiveAccess::CA):
            interface = new QEChannelArchiveInterface (url, this);
            break;
         case (QEArchiveAccess::ARCHAPPL):
            if (!url.path().endsWith("/")) {
               url.setPath(url.path() + "/");
            }
            interface = new QEArchapplInterface (url, this);
            break;
         default:
            DEBUG << "Archiver type not supported ";
            this->sendMessage ("QEArchiveManager: Archiver type not supported",
                               message_types (MESSAGE_TYPE_WARNING));
            return;
      }

      this->archiveInterfaceList.append (interface);

      connect (interface, SIGNAL (archivesResponse (const QObject*, const bool, const QEArchiveInterface::ArchiveList &)),
               this,      SLOT   (archivesResponse (const QObject*, const bool, const QEArchiveInterface::ArchiveList &)));

      connect (interface, SIGNAL (pvNamesResponse  (const QObject*, const bool, const QEArchiveInterface::PVNameList &)),
               this,      SLOT   (pvNamesResponse  (const QObject*, const bool, const QEArchiveInterface::PVNameList &)));

      connect (interface, SIGNAL (valuesResponse   (const QObject*, const bool, const QEArchiveInterface::ResponseValueList &)),
               this,      SLOT   (valuesResponse   (const QObject*, const bool, const QEArchiveInterface::ResponseValueList &)));

      connect (interface, SIGNAL (nextRequest (const int )),
               this,      SLOT   (nextRequest (const int )));

      interface->archivesRequest (interface);
      interface->state = QEArchiveInterface::Updating;

      this->sendMessage (QString ("requesting PV name info from ").append (interface->getName ()),
                         message_types (MESSAGE_TYPE_INFO));
   }

   this->resendStatus ();

   // Lastly connect timer to re interogate the archiver automatically once a day.
   //
   connect (this->timer, SIGNAL (timeout ()),
            this,        SLOT   (timeout ()));

   this->timer->start (24*3600*1000);    // mSec
}

//------------------------------------------------------------------------------
// slot
void QEArchiveManager::aboutToQuitHandler ()
{
   this->timer->stop ();
}

//------------------------------------------------------------------------------
// slot
void QEArchiveManager::timeout ()
{
   this->reInterogateArchives ();
}

//------------------------------------------------------------------------------
//
void QEArchiveManager::archiveStatusRequest () {
   QMutexLocker locker (archiveDataMutex);

   this->resendStatus ();
}

//------------------------------------------------------------------------------
//
void QEArchiveManager::nextRequest (const int requestIndex)
{
   QEArchiveInterface* interface = dynamic_cast <QEArchiveInterface*> ( this->sender ());
   QEArchiveInterface::Archive archive;  // key (name and path)
   NamesResponseContext *context;

   // sainity checks
   if (!interface) return;
   if (requestIndex < 0 || requestIndex >= interface->archiveList.count ()) return;

   // Create the callback context.
   //
   archive = interface->archiveList.value (requestIndex);
   context = new NamesResponseContext (interface, archive, requestIndex + 1);

   // DEBUG <<  archive.key << pattern;
   interface->namesRequest (context, archive.key, pattern);
   this->resendStatus ();
}

//------------------------------------------------------------------------------
//
void QEArchiveManager::updateNumberArchivesRead ()
{
   this->numberArchivesRead++;

   // Have all arives been read - technically includes not read due to failure.
   //
   this->allArchivesRead = (this->numberArchivesRead == this->archiveInterfaceList.count ());

// DEBUG << "arch:" << this->numberArchivesRead
//       << "of" << this->archiveInterfaceList.count ()
//       << " ; ready " << this->allArchivesRead;

   if (this->allArchivesRead) {
      // All archives have now been read.
      this->lastReadTime = QDateTime::currentDateTime().toUTC ();
   }
}

//------------------------------------------------------------------------------
//
void QEArchiveManager::resendStatus ()
{
   QEArchiveAccess::StatusList statusList;
   int j;
   QUrl url;

   QEArchiveAccess::Status status;

   statusList.clear ();
   for (j = 0; j < this->archiveInterfaceList.count(); j++) {
      QEArchiveInterface* archiveInterface = this->archiveInterfaceList.value (j);

      url = archiveInterface->getUrl ();
      status.hostName = url.host ();
      status.portNumber = url.port();
      status.endPoint = url.path ();

      status.state = archiveInterface->state;
      status.available = archiveInterface->available;
      status.read = archiveInterface->read;
      status.numberPVs = archiveInterface->numberPVs;
      status.pending = archiveInterface->getNumberPending();

      statusList.append (status);
   }

   emit this->archiveStatusResponse (statusList);
}

//------------------------------------------------------------------------------
// slot
void QEArchiveManager::reInterogateArchives ()
{
   QDateTime timeNow = QDateTime::currentDateTime ().toUTC ();

   int timeSinceLastRead = this->lastReadTime.secsTo (timeNow);
   if (timeSinceLastRead >= 300) {

      // More than 5 minutes - re-start interogating the archiver.
      //
      this->clear ();
      for (int j = 0; j < this->archiveInterfaceList.count (); j++) {

         // Extract reference to each interface.
         //
         QEArchiveInterface* interface = this->archiveInterfaceList.value (j);

         interface->state = QEArchiveInterface::Updating;
         interface->available = 0;
         interface->read = 0;
         interface->numberPVs = 0;
         this->resendStatus ();

         interface->archivesRequest (interface);

         this->sendMessage (QString ("requesting PV name info from ").append (interface->getName ()),
                            message_types (MESSAGE_TYPE_INFO));
      }

   } else {
      this->sendMessage ("Re Archive PVs: less than 5 minutes since last update - request ignored.",
                         message_types (MESSAGE_TYPE_WARNING));
   }
}

//------------------------------------------------------------------------------
//
void QEArchiveManager::archivesResponse (const QObject * userData,
                                         const bool isSuccess,
                                         const QEArchiveInterface::ArchiveList& archiveListIn)
{
   QMutexLocker locker (archiveDataMutex);

   QEArchiveInterface *interface = (QEArchiveInterface *) userData;
   int count;

   if (isSuccess) {

      count = archiveListIn.count ();
      interface->available = count;
      interface->archiveList = archiveListIn;
      interface->requestIndex = 0;
      interface->timer->start ();
   } else {
      DEBUG << "request failure from " << interface->getName ();
      this->sendMessage (QString ("request failure from ").append (interface->getName ()),
                         message_types (MESSAGE_TYPE_ERROR));

      interface->state = QEArchiveInterface::Error;

      // must account for failures as well.
      //
      this->updateNumberArchivesRead();
   }

   this->resendStatus ();
}

//------------------------------------------------------------------------------
//
void QEArchiveManager::valuesResponse (const QObject* userData,
                                       const bool isSuccess,
                                       const QEArchiveInterface::ResponseValueList& valuesList)
{
   const ValuesResponseContext* context = dynamic_cast <ValuesResponseContext*> ((QObject*) userData);

   if (context) {
      QEArchiveAccess::PVDataResponses response;

      response.userData = context->userData;
      response.isSuccess = ((isSuccess) && (valuesList.size () == 1));
      if (response.isSuccess) {
         response.pointsList = valuesList.front().dataPoints;
      }
      response.pvName = context->pvName;
      response.supplementary = response.isSuccess ? "okay" : "archiver response failure";

      emit this->readArchiveResponse (context->archiveAccess, response);

      delete context;
   }
   this->resendStatus ();
}

//------------------------------------------------------------------------------
//
bool QEArchiveManager::containsPvName (const QString pvName,
                                       QString& effectivePvName)
{
   bool result;

   // Attempt to decode the given name into a protocol and an actual PV name.
   // If not specified, the 'ca://' Channel Access protocol is the default.
   //
   QEPvNameUri uri;
   result = uri.decodeUri (pvName, /* strict=> */ false);
   if (!result) {
      DEBUG << "PV protocol identification failed for:" << pvName;
      this->sendMessage (QString("PV protocol identification failed for: %1").arg (pvName),
                         message_types (MESSAGE_TYPE_WARNING));
      return false;
   }

   // Extract the PV name sans protocol qualifier.
   //
   effectivePvName = uri.getPvName ();

   QEPvNameUri::Protocol protocol = uri.getProtocol ();
   if (protocol != QEPvNameUri::ca) {
      DEBUG << "Only Channel Access protocol archiving is supported:" << pvName;
      this->sendMessage (QString("Only Channel Access protocol archiving is supported: %1").arg (pvName),
                         message_types (MESSAGE_TYPE_WARNING));
      return false;
   }

   // Is this PV currently being archived?
   //
   result = this->pvNameToSourceLookUp.contains (effectivePvName);
   if (!result) {
      // No - the PV 'as is' is not archived.
      // If user has requested XXXXXX.VAL, check if XXXXXX is archived.
      // Similarly, if user requested YYYYYY, check if YYYYYY.VAL is archived.
      //
      if (effectivePvName.right (4) == ".VAL") {
         // Remove the .VAL field and try again.
         //
         effectivePvName.chop (4);
      } else {
         // Add .VAL and try again.
         //
         effectivePvName.append (".VAL");
      }
      result = this->pvNameToSourceLookUp.contains (effectivePvName);
   }

   return result;
}

//==============================================================================
// QEChannelArchiverManager Class Methods
//==============================================================================
//
QEChannelArchiverManager& QEChannelArchiverManager::getInstance()
{
   static QEChannelArchiverManager* instance = NULL;
   {
      QMutexLocker locker (singletonMutex);
      if (!singletonThread) {
         // Create the thread and the manager.
         // Dynamically creating the QEChannelArchiverManager avoid shutdown warnings.
         //
         singletonThread = new QEArchiveThread ();
         instance = new QEChannelArchiverManager ();

         // Set instance to belong to thread, connect the signal and start it.
         //
         instance->moveToThread (singletonThread);

         // We're using EPICS Channel Archiver
         //
         instance->archiverType = QEArchiveAccess::CA;

         QObject::connect (singletonThread, SIGNAL (started ()), instance, SLOT  (started ()));

         singletonThread->start ();
      }
   }

   return *instance;
}

//------------------------------------------------------------------------------
//
QEChannelArchiverManager::QEChannelArchiverManager() : QEArchiveManager() { }

//------------------------------------------------------------------------------
//
QEChannelArchiverManager::~QEChannelArchiverManager() { }

//------------------------------------------------------------------------------
//
void QEChannelArchiverManager::pvNamesResponse (const QObject * userData,
                                                const bool isSuccess,
                                                const QEArchiveInterface::PVNameList &pvNameList)
{
   QMutexLocker locker (archiveDataMutex);

   NamesResponseContext *context = (NamesResponseContext *) userData;
   QEArchiveInterface *interface = context->interface;

   QString message;

   if (isSuccess) {
      interface->read++;

      for (int j = 0; j < pvNameList.count (); j++) {
         QEArchiveInterface::PVName pvChannel = pvNameList.value (j);
         KeyTimeSpec keyTimeSpec;
         SourceSpec sourceSpec;

         keyTimeSpec.key = context->archive.key;
         keyTimeSpec.name = context->archive.name;
         keyTimeSpec.path = context->archive.path;
         keyTimeSpec.startTime = pvChannel.startTime;
         keyTimeSpec.endTime = pvChannel.endTime;

         if (pvNameToSourceLookUp.contains (pvChannel.pvName) == false) {
            // First instance of this PV Name
            //
            interface->numberPVs++;
            sourceSpec.interface = context->interface;
            sourceSpec.keyToTimeSpecLookUp.insert (keyTimeSpec.key, keyTimeSpec);
            pvNameToSourceLookUp.insert (pvChannel.pvName, sourceSpec);

         } else {
            // Second or subsequent instance of this PV.
            //
            sourceSpec = pvNameToSourceLookUp.value (pvChannel.pvName);

            if (sourceSpec.interface == context->interface) {
               if (sourceSpec.keyToTimeSpecLookUp.contains (keyTimeSpec.key) == false) {

                  sourceSpec.keyToTimeSpecLookUp.insert (keyTimeSpec.key, keyTimeSpec);
                  // QHash: If there is already an item with the key, that item's value is replaced with value.
                  pvNameToSourceLookUp.insert (pvChannel.pvName, sourceSpec);

               } else {

                  message.sprintf ("PV %s has multiple instances of key %d",
                                   pvChannel.pvName.toLatin1().data (), keyTimeSpec.key ) ;
                  this->sendMessage (message, message_types (MESSAGE_TYPE_ERROR));

               }

            } else {

               message.sprintf ("PV %s hosted on multiple interfaces. Primary %s, Secondary %s",
                                pvChannel.pvName.toLatin1().data (),
                                sourceSpec.interface->getName ().toLatin1().data (),
                                context->interface->getName ().toLatin1().data ());
               this->sendMessage (message, message_types (MESSAGE_TYPE_ERROR));
            }
         }
      }

   } else {

      this->sendMessage (QString ("PV names failure from ").
                         append (context->interface->getName ()).
                         append (" for archive ").
                         append (context->archive.name),
                         message_types (MESSAGE_TYPE_ERROR));

   }

   if (context->instance == interface->available) {
      message = "PV name retrival from ";
      message.append (context->interface->getName ());
      message.append (" complete");
      this->sendMessage (message);

      if (interface->read == interface->available) {
         interface->state = QEArchiveInterface::Complete;

      } else {
         interface->state = QEArchiveInterface::InComplete;

      }

      this->updateNumberArchivesRead();
   }

   if (context) delete context;
   this->resendStatus ();
}

//------------------------------------------------------------------------------
//
void QEChannelArchiverManager::readArchiveRequest (const QEArchiveAccess* archiveAccess,
                                                   const QEArchiveAccess::PVDataRequests& request)
{
   QMutexLocker locker (archiveDataMutex);

   QString effectivePvName;
   bool isKnownPVName;
   int j;
   QStringList pvNames;
   SourceSpec sourceSpec;
   KeyTimeSpec keyTimeSpec;
   int key;
   int bestOverlap;
   QList < int >keys;
   QCaDateTime useStart;
   QCaDateTime useEnd;
   int overlap;
   QString message;

   QEArchiveAccess::PVDataResponses response;

   // Initialise failed response.
   //
   response.pvName = request.pvName;
   response.userData = request.userData;
   response.isSuccess = false;
   response.pointsList.clear ();
   response.supplementary = "fail";

   // Is this PV currently being archived?
   //
   isKnownPVName = this->containsPvName (request.pvName, effectivePvName);
   if (isKnownPVName) {
      sourceSpec = pvNameToSourceLookUp.value (effectivePvName);

      key = -1;
      bestOverlap = -864000;

      keys = sourceSpec.keyToTimeSpecLookUp.keys ();
      for (j = 0; j < keys.count (); j++) {

         keyTimeSpec = sourceSpec.keyToTimeSpecLookUp.value (keys.value (j));

         useStart = MAX (request.startTime.toUTC (), keyTimeSpec.startTime);
         useEnd = MIN (request.endTime.toUTC (), keyTimeSpec.endTime);

         // We don't worry about calculating the overlap to an accuracy
         // of any one than one second.
         //
         overlap = useStart.secsTo (useEnd);
         if (bestOverlap < overlap) {
            bestOverlap = overlap;
            key = keyTimeSpec.key;
         }
      }

      isKnownPVName = (key >= 0);
      if (isKnownPVName) {
         pvNames.append (effectivePvName);

         // Create a reasponse context.
         //
         ValuesResponseContext* context = new ValuesResponseContext (archiveAccess, request.pvName, request.userData);

         // The interface signals return data to the valuesResponse slot in the QEArchiveManager
         // object which (using supplied context) emits QEArchiveAccess setArchiveData signal on behalf
         // of this object.
         //
         sourceSpec.interface->valuesRequest (context,
                                              request.startTime, request.endTime,
                                              request.count, request.how,
                                              pvNames, key, request.element);
         this->resendStatus ();

      } else {
         message = "Archive Manager: PV ";
         message.append (request.pvName);
         message.append (" has no matching time overlaps.");
         this->sendMessage (message, message_types (MESSAGE_TYPE_WARNING));

         response.supplementary = message;
         emit this->readArchiveResponse (archiveAccess, response);
      }

   } else {
      message = "Archive Manager: PV ";
      message.append (request.pvName);
      message.append (" not found in archive.");
      this->sendMessage (message, message_types (MESSAGE_TYPE_WARNING));

      response.supplementary = message;
      emit this->readArchiveResponse (archiveAccess, response);
   }
}

//------------------------------------------------------------------------------
//
bool QEChannelArchiverManager::getArchivePvInformation (QString& effectivePvName,
                                                        QEArchiveAccess::ArchiverPvInfoLists& data)
{
   bool result = false;

   const QString tempName = effectivePvName;
   bool isKnownPVName = this->containsPvName (tempName, effectivePvName);
   if (isKnownPVName) {
      SourceSpec sourceSpec = pvNameToSourceLookUp.value (effectivePvName);
      QList<int> keys;

      keys = sourceSpec.keyToTimeSpecLookUp.keys ();
      for (int j = 0; j < keys.count (); j++) {
         int key = keys.value (j, -1);
         if (key < 0) continue;
         KeyTimeSpec keyTimeSpec = sourceSpec.keyToTimeSpecLookUp.value (key);

         QEArchiveAccess::ArchiverPvInfo item;

         item.key = key;
         item.path = keyTimeSpec.path;
         item.startTime = keyTimeSpec.startTime;
         item.endTime = keyTimeSpec.endTime;

         data.append (item);
         result = true;
      }
   }

   return result;
}

//==============================================================================
// QEArchapplManager Class Methods
//==============================================================================
//
QEArchapplManager& QEArchapplManager::getInstance()
{
   static QEArchapplManager* instance = NULL;
   {
      QMutexLocker locker (singletonMutex);
      if (!singletonThread) {
         // Create the thread and the manager.
         // Dynamically creating the QEChannelArchiverManager avoid shutdown warnings.
         //
         singletonThread = new QEArchiveThread ();
         instance = new QEArchapplManager();

         // Set instance to belong to thread, connect the signal and start it.
         //
         instance->moveToThread (singletonThread);

         // We're using Archiver Appliance
         //
         instance->archiverType = QEArchiveAccess::ARCHAPPL;

         QObject::connect (singletonThread, SIGNAL (started ()), instance, SLOT  (started ()));

         singletonThread->start ();
      }
   }

   return *instance;
}

//------------------------------------------------------------------------------
//
QEArchapplManager::QEArchapplManager() : QEArchiveManager() { }

//------------------------------------------------------------------------------
//
QEArchapplManager::~QEArchapplManager() { }

//------------------------------------------------------------------------------
//
void QEArchapplManager::readArchiveRequest (const QEArchiveAccess* archiveAccess,
                                            const QEArchiveAccess::PVDataRequests& request)
{

   QMutexLocker locker (archiveDataMutex);

   QString effectivePvName;
   bool isKnownPVName;
   QStringList pvNames;
   QString message;
   SourceSpec sourceSpec;
   QEArchiveAccess::PVDataResponses response;

   // Initialise failed response.
   //
   response.pvName = request.pvName;
   response.userData = request.userData;
   response.isSuccess = false;
   response.pointsList.clear ();
   response.supplementary = "fail";

   // Is this PV currently being archived?
   //
   isKnownPVName = this->containsPvName (request.pvName, effectivePvName);
   if (isKnownPVName) {
      sourceSpec = pvNameToSourceLookUp.value (effectivePvName);

      pvNames.append (effectivePvName);

      // Create a reasponse context.
      //
      ValuesResponseContext* context = new  ValuesResponseContext (archiveAccess, request.pvName, request.userData);

      // The interface signals return data to the valuesResponse slot in the QEArchiveManager
      // object which (using supplied context) emits QEArchiveAccess setArchiveData signal on behalf
      // of this object.
      //
      sourceSpec.interface->valuesRequest (context,
                                           request.startTime, request.endTime,
                                           request.count, request.how,
                                           pvNames, 0, request.element);
      this->resendStatus ();

   } else {
      message = "Archive Manager: PV ";
      message.append (request.pvName);
      message.append (" not found in archive.");
      this->sendMessage (message, message_types (MESSAGE_TYPE_WARNING));

      response.supplementary = message;
      emit this->readArchiveResponse (archiveAccess, response);
   }
}

//------------------------------------------------------------------------------
//
void QEArchapplManager::pvNamesResponse  (const QObject* userData, const bool isSuccess,
                                          const QEArchiveInterface::PVNameList& pvNameList)
{
   QMutexLocker locker (archiveDataMutex);

   NamesResponseContext *context = (NamesResponseContext *) userData;
   QEArchiveInterface *interface = context->interface;

   QString message;

   SourceSpec sourceSpec;
   sourceSpec.interface = (QEArchapplInterface*) context->interface;

   if (isSuccess) {
      interface->read++;

      for (int j = 0; j < pvNameList.count (); j++) {
         QEArchiveInterface::PVName pvChannel = pvNameList.value (j);

         if (pvNameToSourceLookUp.contains (pvChannel.pvName) == false) {
            // First instance of this PV Name
            //
            KeyTimeSpec keyTimeSpec;
            keyTimeSpec.key = 0;
            keyTimeSpec.name = QString("Archiver Appliance");
            keyTimeSpec.path = QString("");
            keyTimeSpec.startTime = pvChannel.startTime;
            keyTimeSpec.endTime = pvChannel.endTime;

            interface->numberPVs++;
            sourceSpec.keyToTimeSpecLookUp.insert (keyTimeSpec.key, keyTimeSpec);
            pvNameToSourceLookUp.insert (pvChannel.pvName, sourceSpec);
         }
      }

   } else {

      this->sendMessage (QString ("PV names failure from ").
                         append (context->interface->getName ()).
                         append (" for archive ").
                         append (context->archive.name),
                         message_types (MESSAGE_TYPE_ERROR));

   }

   if (context->instance == interface->available) {
      message = "PV name retrival from ";
      message.append (context->interface->getName ());
      message.append (" complete");
      this->sendMessage (message);

      if (interface->read == interface->available) {
         interface->state = QEArchiveInterface::Complete;

      } else {
         interface->state = QEArchiveInterface::InComplete;

      }

      this->updateNumberArchivesRead();
   }

   if (context) delete context;
   this->resendStatus ();
}

//------------------------------------------------------------------------------
//
bool QEArchapplManager::getArchivePvInformation (QString& effectivePvName,
                                                 QEArchiveAccess::ArchiverPvInfoLists& data)
{
   bool result = false;

   const QString tempName = effectivePvName;
   bool isKnownPVName = this->containsPvName (tempName, effectivePvName);
   if (isKnownPVName) {
      SourceSpec sourceSpec = pvNameToSourceLookUp.value (effectivePvName);

      // As this is an AA, which doesn't have the concept of keys, we know
      // that there is only one key for every PV and that is 0.
      //
      const int key = 0;
      KeyTimeSpec keyTimeSpec = sourceSpec.keyToTimeSpecLookUp.value (key);

      QEArchiveAccess::ArchiverPvInfo item;

      item.key = key;
      item.path = keyTimeSpec.path;
      item.startTime = keyTimeSpec.startTime;
      item.endTime = keyTimeSpec.endTime;

      data.append (item);
      result = true;
   }

   return result;
}

// end
