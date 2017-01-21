/*  QEArchiveManager.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2012,2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <stdlib.h>

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

#include <QEArchiveManager.h>

#define DEBUG  qDebug () << "QEArchiveManager" << __LINE__ <<  __FUNCTION__  << "  "


//------------------------------------------------------------------------------
//
ArchiveInterfacePlus::ArchiveInterfacePlus (QUrl url, QObject* parent) : QEArchiveInterface (url, parent)
{
   this->state = QEArchiveAccess::Unknown;
   this->available = 0;
   this->read = 0;
   this->numberPVs = 0;

   this->timer = new QTimer (this);
   this->timer->setInterval (100);   // Allow 100 mS between requests.

   this->requestIndex = 0;

   connect (this->timer, SIGNAL (timeout ()),
            this,        SLOT   (timeout ()));
}

//------------------------------------------------------------------------------
//
void ArchiveInterfacePlus::timeout ()
{
   if (this->requestIndex < this->available) {
      emit this->nextRequest (this->requestIndex);
      this->requestIndex++;
   } else {
      // All requests send - we can stop now.
      //
      this->timer->stop ();
   }
}

//------------------------------------------------------------------------------
// The archive manager can support many different interfaces.
//
typedef QList <ArchiveInterfacePlus*> ArchiveInterfaceLists;


//------------------------------------------------------------------------------
// Archive class type provides key (and name and path - these not used as such
// but may prove to be usefull).
// For a particualar PV, we also retrieve a start and stop time.
//
class KeyTimeSpec : public QEArchiveInterface::Archive {
public:
   QCaDateTime startTime;
   QCaDateTime endTime;
};

//------------------------------------------------------------------------------
// Each PV may have one or more archives available on the same
// host, e.g. a short term archive and a long term archive.
// However we expect all archives for a particlar PV to be co-hosted.
//
// This type provides a mapping from key (sparce numbers) to KeyTimeSpec
// which contain the key itself, together with the available start/stop
// times. This allows use to choose the key that best fits the request
// time frame.
//
// Note: QHash provides faster lookups than QMap.
//       When iterating over a QMap, the items are always sorted by key.
//       With QHash, the items are arbitrarily ordered.
//
class SourceSpec {
public:
   ArchiveInterfacePlus* interface;
   QHash <int, KeyTimeSpec> keyToTimeSpecLookUp;
};

//------------------------------------------------------------------------------
// Mapping by PV name to essentially archive source to key(s) and time range(s)
// that support the PV.
// NOTE: We use a map here as we want sorted keys.
//
typedef QMap <QString, SourceSpec> PVNameToSourceSpecLookUp;


//------------------------------------------------------------------------------
//
class QEArchiveThread : public QThread {
public:
   void run () { exec (); }
};

//==============================================================================
// Local Data
//==============================================================================
// We declare these items here as opposed as static members of the class because
// the later made all the EPICS plugin widgets "go away" in designer.
// I think the are issues when QObjects declared in header files.
//

// Allows only one object to be effectively created. Second and subsequent object
// do nothing, except waste space.
//
static QMutex *singletonMutex = new QMutex ();
static QEArchiveManager *singletonManager = NULL;
static QEArchiveThread *singletonThread = NULL;

static QMutex *archiveDataMutex = new QMutex ();
static ArchiveInterfaceLists archiveInterfaceList;
static PVNameToSourceSpecLookUp pvNameToSourceLookUp;

static bool allArchivesRead = false;
static int numberArchivesRead = 0;
static bool environmentErrorReported = false;


//==============================================================================
// QEArchiveManager Class Methods
//==============================================================================
//
//The singleton manager object is an orphan because we move it to singletonThread.
//
QEArchiveManager::QEArchiveManager (const QString& archivesIn,
                                    const QString& patternIn) : QObject (NULL)
{
   this->archives = archivesIn;
   this->pattern = patternIn;

   this->timer = new QTimer (this);
   this->lastReadTime = QDateTime::currentDateTime ().toUTC ().addSecs (-300);

   // Hard-coded message Id.
   //
   this->setSourceId (9001);

   // Register status message types.
   //
   qRegisterMetaType<QEArchiveAccess::States> ("QEArchiveAccess::States");
   qRegisterMetaType<QEArchiveAccess::Status> ("QEArchiveAccess::Status");
   qRegisterMetaType<QEArchiveAccess::StatusList> ("QEArchiveAccess::StatusList");

   qRegisterMetaType<QEArchiveAccess::PVDataRequests> ("QEArchiveAccess::PVDataRequests");
   qRegisterMetaType<QEArchiveAccess::PVDataResponses> ("QEArchiveAccess::PVDataResponses");
}

//------------------------------------------------------------------------------
//
void QEArchiveManager::clear ()
{
   allArchivesRead = false;
   numberArchivesRead = 0;
   pvNameToSourceLookUp.clear ();
}

//------------------------------------------------------------------------------
// called from start (which is a thread slot).
//
void QEArchiveManager::setup ()
{
   QStringList archiveList;
   QString item;
   QUrl url;
   ArchiveInterfacePlus* interface;

   // First check we are the one and only ....
   // Belts 'n' braces sanity check.
   //
   if (this != singletonManager) {
      // This is NOT the singleton object.
      //
      this->sendMessage ("QEArchiveManager::initialise - attempt to use non-singleton object",
                         message_types (MESSAGE_TYPE_ERROR));
      return;
   }

   // Split input string using space as delimiter.
   // Could extend to use regular expression and split on any white space character.
   //
   archiveList = archives.split (' ', QString::SkipEmptyParts);

   if (archiveList.count () == 0) {
      this->sendMessage ("QEArchiveManager: no archives specified",
                         message_types (MESSAGE_TYPE_INFO));
      return;
   }

   this->clear ();

   this->sendMessage (QString ("pattern: ").append (pattern),
                      message_types (MESSAGE_TYPE_INFO));

   for (int j = 0; j < archiveList.count (); j++) {

      item = "http://";
      item.append (archiveList.value (j));
      url = QUrl (item);

      // Create and save a reference to each interface.
      //
      interface = new ArchiveInterfacePlus (url, this);
      archiveInterfaceList.append (interface);

      connect (interface, SIGNAL (archivesResponse (const QObject*, const bool, const QEArchiveInterface::ArchiveList &)),
               this,      SLOT   (archivesResponse (const QObject*, const bool, const QEArchiveInterface::ArchiveList &)));

      connect (interface, SIGNAL (pvNamesResponse  (const QObject*, const bool, const QEArchiveInterface::PVNameList &)),
               this,      SLOT   (pvNamesResponse  (const QObject*, const bool, const QEArchiveInterface::PVNameList &)));

      connect (interface, SIGNAL (valuesResponse   (const QObject*, const bool, const QEArchiveInterface::ResponseValueList &)),
               this,      SLOT   (valuesResponse   (const QObject*, const bool, const QEArchiveInterface::ResponseValueList &)));

      connect (interface, SIGNAL (nextRequest (const int )),
               this,      SLOT   (nextRequest (const int )));

      interface->archivesRequest (interface);
      interface->state = QEArchiveAccess::Updating;

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
void QEArchiveManager::started ()
{
   this->setup ();
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
      for (int j = 0; j < archiveInterfaceList.count (); j++) {

         // Extract reference to each interface.
         //
         ArchiveInterfacePlus* interface = archiveInterfaceList.value (j);

         interface->archivesRequest (interface);
         interface->state = QEArchiveAccess::Updating;
         interface->available = 0;
         interface->read = 0;
         interface->numberPVs = 0;

         this->sendMessage (QString ("requesting PV name info from ").append (interface->getName ()),
                            message_types (MESSAGE_TYPE_INFO));
      }

      this->resendStatus ();

   } else {
      this->sendMessage ("Re Archive PVs: less than 5 minutes since last update - request ignored.",
                         message_types (MESSAGE_TYPE_WARNING));
   }
}

//------------------------------------------------------------------------------
// slot
void QEArchiveManager::timeout ()
{
   this->reInterogateArchives ();
}


//------------------------------------------------------------------------------
// static
void QEArchiveManager::initialise (const QString& archivesIn, const QString& patternIn)
{
   if (archivesIn.isEmpty ()) {
      qDebug() << "QEArchiveManager: Archive list is undefined.";
   }

   QMutexLocker locker (singletonMutex);

   if (!singletonManager) {
      singletonManager = new QEArchiveManager (archivesIn, patternIn);
      singletonThread = new QEArchiveThread ();

      // Remome the singletonManager to belong to thread, connect the signal and start it.
      //
      singletonManager->moveToThread (singletonThread);

      QObject::connect (singletonThread, SIGNAL (started ()),
                        singletonManager, SLOT  (started ()));

      singletonThread->start ();
   }
}

//------------------------------------------------------------------------------
// static
void QEArchiveManager::initialise ()
{
   {
      QMutexLocker locker (singletonMutex);
      if (singletonManager) return; // Already defined.
   }

   QEAdaptationParameters ap ("QE_");

   QString archives = ap.getString ("archive_list", "");
   QString pattern  = ap.getString ("archive_pattern", ".*");

   if (!archives.isEmpty ()) {
      // Something, albeit maybe invalid, has been defined - proceed with initialisation.
      //
      QEArchiveManager::initialise (archives, pattern);
   } else {
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
void QEArchiveManager::resendStatus ()
{
   QEArchiveAccess::StatusList statusList;
   int j;
   QUrl url;

   QEArchiveAccess::Status status;

   statusList.clear ();
   for (j = 0; j < archiveInterfaceList.count(); j++) {
      ArchiveInterfacePlus* archiveInterface = archiveInterfaceList.value (j);

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
//
void QEArchiveManager::archiveStatusRequest () {
   QMutexLocker locker (archiveDataMutex);

   this->resendStatus ();
}

//==============================================================================
//
class NamesResponseContext:public QObject {
public:
   ArchiveInterfacePlus* interface;
   QEArchiveInterface::Archive archive;
   int instance;

   // constructor
   NamesResponseContext (ArchiveInterfacePlus * interfaceIn,
                         ArchiveInterfacePlus::Archive archiveIn,
                         int i)
   {
      this->interface = interfaceIn;
      this->archive = archiveIn;
      this->instance = i;   // used to determine if this last received, i.e complete. TODO maybe be received in order.
   }
};


//------------------------------------------------------------------------------
//
void QEArchiveManager::archivesResponse (const QObject * userData,
                                         const bool isSuccess,
                                         const QEArchiveInterface::ArchiveList& archiveListIn)
{
   QMutexLocker locker (archiveDataMutex);

   ArchiveInterfacePlus *interface = (ArchiveInterfacePlus *) userData;
   int count;

   if (isSuccess) {

      count = archiveListIn.count ();
      interface->available = count;
      interface->archiveList = archiveListIn;
      interface->requestIndex = 0;
      interface->timer->start ();

      //      for (j = 0; j < count; j++) {
      //         QEArchiveInterface::Archive archive;  // key (name and path)
      //         NamesResponseContext *context;

      //         // Create the callback context.
      //         //
      //         archive = archiveListIn.value (j);
      //         context = new NamesResponseContext (interface, archive, j + 1);

      //         DEBUG <<  archive.key << pattern;
      //         interface->namesRequest (context, archive.key, pattern);
      //      }

   } else {
      this->sendMessage (QString ("request failure from ").append (interface->getName ()),
                         message_types (MESSAGE_TYPE_ERROR));

      interface->state = QEArchiveAccess::Error;

   }

   this->resendStatus ();
}

//------------------------------------------------------------------------------
//
void QEArchiveManager::nextRequest (const int requestIndex)
{
   ArchiveInterfacePlus* interface = dynamic_cast <ArchiveInterfacePlus *> ( this->sender ());
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
void QEArchiveManager::pvNamesResponse (const QObject * userData,
                                        const bool isSuccess,
                                        const QEArchiveInterface::PVNameList &pvNameList)
{
   QMutexLocker locker (archiveDataMutex);

   NamesResponseContext *context = (NamesResponseContext *) userData;
   ArchiveInterfacePlus *interface = context->interface;

   QString message;
   int j;

   if (isSuccess) {
      interface->read++;

      //    DEBUG << interface->read << pvNameList.count ();

      for (j = 0; j < pvNameList.count (); j++) {
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
         interface->state = QEArchiveAccess::Complete;

      } else {
         interface->state = QEArchiveAccess::InComplete;

      }

      numberArchivesRead++;
      allArchivesRead = (numberArchivesRead = archiveInterfaceList.count ());

      if (allArchivesRead) {
      // All archives have now been read
         this->lastReadTime = QDateTime::currentDateTime().toUTC ();
      }
   }

   delete context;
   this->resendStatus ();
}


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
//
void QEArchiveManager::readArchiveRequest (const QEArchiveAccess* archiveAccess,
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
   effectivePvName = request.pvName;

   isKnownPVName = pvNameToSourceLookUp.contains (effectivePvName);
   if (!isKnownPVName) {
      // No - the PV 'as is' is not archived.
      // If user has requested XXXXXX.VAL, check if XXXXXX is archived.
      // Similarly, if user requested YYYYYY, check if YYYYYY.VAL archived.
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
      isKnownPVName = pvNameToSourceLookUp.contains (effectivePvName);
   }

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
         ValuesResponseContext* context = new  ValuesResponseContext (archiveAccess, request.pvName, request.userData);

         // The interface signals return data to the valuesResponse slot in the QEArchiveManager
         // object which (using supplied context) emits QEArchiveAccess setArchiveData signal on behalf
         // of this object.
         //
         sourceSpec.interface->valuesRequest (context, key,
                                              request.startTime, request.endTime,
                                              request.count, request.how,
                                              pvNames, request.element);
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
void QEArchiveManager::valuesResponse (const QObject* userData,
                                       const bool isSuccess,
                                       const QEArchiveInterface::ResponseValueList& valuesList)
{
   const ValuesResponseContext* context = dynamic_cast <ValuesResponseContext*> ((QObject*) userData);

   if (context) {
      QEArchiveAccess::PVDataResponses response;

      response.userData = context->userData;
      response.isSuccess = ((isSuccess) && (valuesList.count () == 1));
      if (response.isSuccess) {
         response.pointsList = valuesList.value (0).dataPoints;
      }
      response.pvName = context->pvName;
      response.supplementary = response.isSuccess ? "okay" : "archiver response failure";

      emit this->readArchiveResponse (context->archiveAccess, response);
	  
      delete context;
   }
   this->resendStatus ();
}

//==============================================================================
//
QEArchiveAccess::QEArchiveAccess (QObject * parent) : QObject (parent)
{
   // Construct and initialise singleton QEArchiveManager object if needs be.
   //
   QEArchiveManager::initialise ();

   // Connect status request response signals.
   //
   if( singletonManager )
   {
      // Request that manager re-read the set of avialble PVs from the archiver.
      //
      QObject::connect (this,             SIGNAL (reInterogateArchives ()),
                        singletonManager, SLOT   (reInterogateArchives ()));

      // archive status request and response.
      //
      QObject::connect (this,             SIGNAL (archiveStatusRequest ()),
                        singletonManager, SLOT   (archiveStatusRequest ()));

      QObject::connect (singletonManager, SIGNAL (archiveStatusResponse (const QEArchiveAccess::StatusList&)),
                        this,             SLOT   (archiveStatusResponse (const QEArchiveAccess::StatusList&)));


      // Connect data request response signals.
      //
      QObject::connect (this,             SIGNAL (readArchiveRequest  (const QEArchiveAccess*, const QEArchiveAccess::PVDataRequests&)),
                        singletonManager, SLOT   (readArchiveRequest  (const QEArchiveAccess*, const QEArchiveAccess::PVDataRequests&)));

      QObject::connect (singletonManager, SIGNAL (readArchiveResponse (const QEArchiveAccess*, const QEArchiveAccess::PVDataResponses&)),
                        this,             SLOT   (readArchiveResponse (const QEArchiveAccess*, const QEArchiveAccess::PVDataResponses&)));
   }
}

//------------------------------------------------------------------------------
//
QEArchiveAccess::~QEArchiveAccess ()
{
}

//------------------------------------------------------------------------------
//
unsigned int QEArchiveAccess::getMessageSourceId ()
{
   return this->getSourceId ();
}

//------------------------------------------------------------------------------
//
void QEArchiveAccess::setMessageSourceId (unsigned int messageSourceIdIn)
{
   this->setSourceId (messageSourceIdIn);
}

//------------------------------------------------------------------------------
//
void QEArchiveAccess::resendStatus ()
{
   if (singletonManager) {
      emit this->archiveStatusRequest ();
   }
}

//------------------------------------------------------------------------------
//
void QEArchiveAccess::reReadAvailablePVs ()
{
   if (singletonManager) {
      emit this->reInterogateArchives ();
   }
}

//------------------------------------------------------------------------------
//
void QEArchiveAccess::archiveStatusResponse (const QEArchiveAccess::StatusList& stringList)
{
   // Just re-broadcast status signal - no filtering.
   //
   emit this->archiveStatus (stringList);
}

//------------------------------------------------------------------------------
//
void QEArchiveAccess::readArchive (QObject* userData,
                                   const QString pvName,
                                   const QCaDateTime startTime,
                                   const QCaDateTime endTime,
                                   const int count,
                                   const QEArchiveInterface::How how,
                                   const unsigned int element)
{
   QEArchiveAccess::PVDataRequests request;

   // Set up request - aggragate parameters
   //
   request.userData = userData;
   request.pvName = pvName;
   request.startTime = startTime;
   request.endTime = endTime;
   request.count = count;
   request.how = how;
   request.element = element;

   // and hand-ball off to archiver manager thread.
   //
   emit this->readArchiveRequest (this, request);
}

//------------------------------------------------------------------------------
//
void QEArchiveAccess::readArchiveResponse (const QEArchiveAccess* archiveAccess,
                                           const QEArchiveAccess::PVDataResponses& response)
{
   // Filter and re-broadcast status signal.
   //
   if (archiveAccess == this) {
      emit this->setArchiveData (response.userData, response.isSuccess,
                                 response.pointsList, response.pvName,
                                 response.supplementary);

      // Depricated form.
      emit this->setArchiveData (response.userData, response.isSuccess,
                                 response.pointsList);
   }
}


//------------------------------------------------------------------------------
// static functions
//------------------------------------------------------------------------------
//
void QEArchiveAccess::initialise (const QString& archives, const QString& pattern)
{
   // Construct and initialise singleton QEArchiveManager object if needs be.
   //
   QEArchiveManager::initialise (archives, pattern);
}

//------------------------------------------------------------------------------
//
void QEArchiveAccess::initialise ()
{
   // Construct and initialise singleton QEArchiveManager object if needs be.
   //
   QEArchiveManager::initialise ();
}

//------------------------------------------------------------------------------
//
bool QEArchiveAccess::isReady ()
{
   return allArchivesRead;
}

//------------------------------------------------------------------------------
//
int QEArchiveAccess::getNumberInterfaces ()
{
   return archiveInterfaceList.count ();
}

//------------------------------------------------------------------------------
//
QString QEArchiveAccess::getPattern ()
{
   QMutexLocker locker (singletonMutex);

   return singletonManager ? singletonManager->pattern : "";
}

//------------------------------------------------------------------------------
//
int QEArchiveAccess::getNumberPVs ()
{
   QMutexLocker locker (archiveDataMutex);

   return pvNameToSourceLookUp.count ();
}

//------------------------------------------------------------------------------
//
QStringList QEArchiveAccess::getAllPvNames ()
{
   QMutexLocker locker (archiveDataMutex);
   QStringList result;

   result = pvNameToSourceLookUp.keys ();
   return  result;
}

// end
