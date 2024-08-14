/*  QEArchiveManager.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2024 Australian Synchrotron
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

#include <QApplication>
#include <QDebug>
#include <QHash>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>

#include <QECommon.h>
#include <QEPvNameUri.h>
#include <QEAdaptationParameters.h>
#include <QEArchiveInterfaceManager.h>
#include <QEArchiveAccess.h>
#include <QCaDataPoint.h>

#define DEBUG  qDebug () << "QEArchiveManager" << __LINE__ <<  __FUNCTION__  << "  "


//==============================================================================
// PVNameToSourceSpecLookUp types
//==============================================================================
// Archive class type provides key (and name and path - these not used as such
// but may prove to be usefull).
// For a particualar PV, we also retrieve and store start and stop times.
//
class KeyTimeSpec : public QEArchiveInterface::Archive {
public:
   uint32_t startTime;  // only need seconds past epoch here
   uint32_t endTime;
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
   QEArchiveInterfaceManager* interfaceManager;
   QHash <int, KeyTimeSpec> keyToTimeSpecLookUp;
};

//------------------------------------------------------------------------------
// Mapping by PV name to essentially archive source to key(s) and time range(s)
// that support the PV.
// NOTE: We use a map here as we want sorted keys.
//
typedef QMap <QString, SourceSpec> PVNameToSourceSpecMap;

// This is essentially just a renaming exercise.
//
class QEArchiveManager::PVNameToSourceSpecLookUp : public PVNameToSourceSpecMap
{
public:
   explicit PVNameToSourceSpecLookUp() : PVNameToSourceSpecMap () { }
   ~PVNameToSourceSpecLookUp() { }
};


//==============================================================================
// Local Data
//==============================================================================
// We declare these items here as opposed as static members of the class because
// the latter made all the EPICS plugin widgets "go away" in designer.
// I think the are issues when QObjects declared in header files.
//
// Allows only one QEArchiveManager and thread object to be created.
//
static QMutex* singletonMutex = new QMutex ();
static QEArchiveManager* singletonManager = NULL;
static QThread* singletonThread = NULL;


// Protects pvNameToSourceLookUp
//
static QMutex* archiveDataMutex = new QMutex ();

// This list holds archive names and paths - essentially only applicable to 
// the traditional Channel Access archiver. We hold an indices (2 bytes each)
// as opposed to a QStrings (24 bytes each) in the SourceSpec type.
//
static QStringList archiveNameList;
static QStringList pathNameList;

//==============================================================================
// QEArchiveManager
//==============================================================================
//
// The singleton manager object is an orphan because we move it to singletonThread.
//
QEArchiveManager::QEArchiveManager (const QEArchiveAccess::ArchiverTypes archiverTypeIn) :
   archiverType (archiverTypeIn)
{
   this->setSourceId (9001);
   this->allowPendingRequests = true;

   this->pvNameToSourceLookUp = new PVNameToSourceSpecLookUp ();
   this->timer = new QTimer (this);

   // The started function does all the initialisation.
   //
   QObject::connect (singletonThread,  SIGNAL (started ()),
                     this,             SLOT   (started ()));

   // Connect to the about to quit signal.
   // Note: qApp is defined in QApplication
   //
   QObject::connect (qApp, SIGNAL (aboutToQuit ()),
                     this, SLOT   (aboutToQuitHandler ()));

   // Connect timer to timeout slot
   //
   connect (this->timer, SIGNAL (timeout ()),
            this,        SLOT   (reInterogateTimeout ()));
}

//------------------------------------------------------------------------------
//
void QEArchiveManager::started ()
{
   QEAdaptationParameters ap ("QE_");
   const QString archives = ap.getString ("archive_list", "");
   this->pattern = ap.getString ("archive_pattern", ".*");

   // Normally a 5 minute wait to re-interogaye the archives, but allow first
   // re-request to be done after 3 minutes.
   //
   const QCaDateTime caDateTimeNow = QCaDateTime::currentDateTime ();
   const uint32_t timeNow = caDateTimeNow.getSeconds();
   this->lastReadTime = timeNow - 120;
   this->archiveInterfaceManagerList.clear ();
   this->clear();

   this->sendMessage (QString ("pattern: ").append (this->pattern),
                      message_types (MESSAGE_TYPE_INFO));

   // Split input string using space as delimiter.
   // Could extend to use regular expression and split on any white space character.
   //
   QStringList archiveList = QEUtilities::split (archives);

   int count = 0;
   for (int j = 0; j < archiveList.count (); j++) {

      const QString item = archiveList.value (j);
      const QString prefix = item.indexOf ("://") <= 0 ? "http://" : "";

      QUrl url (prefix + item);
      if (!url.isValid()) {
         const QString message = QString ("not a valid URL: %1").arg (item);
         DEBUG << message;
         this->sendMessage (message, message_types (MESSAGE_TYPE_ERROR));
         continue;
      }

      // If no port defined, go with port 80 by default.
      //
      if (url.port() == -1) {
         url.setPort (80);
      }

      // Create and save a reference to each archive interface manager.
      //
      QEArchiveInterfaceManager* aim;
      aim = QEArchiveInterfaceManager::createInterfaceManager
               (count, this->archiverType, url, this);

      if (aim == NULL) {
         // Could not create this archive interface manager - skip and continue.
         continue;
      }

      this->archiveInterfaceManagerList.append (aim);
      count++;

      // connect archive interface managers signals to our slots.
      //
      QObject::connect (aim,
                        SIGNAL (aimPvNamesResponse (QEArchiveInterfaceManager*,
                                                    const QEArchiveInterface::Archive,
                                                    const QEArchiveInterface::PVNameList&)),
                        this,
                        SLOT   (aimPvNamesResponse (QEArchiveInterfaceManager*,
                                                    const QEArchiveInterface::Archive,
                                                    const QEArchiveInterface::PVNameList&)));

      QObject::connect (aim,
                        SIGNAL (aimDataResponse (const QEArchiveAccess*,
                                                 const QEArchiveAccess::PVDataResponses&)),
                        this,
                        SLOT   (aimDataResponse (const QEArchiveAccess*,
                                                 const QEArchiveAccess::PVDataResponses&)));

      // Lastly prod the archive interface manager to start interogating the
      // archive to provide info re which PVs are archived and over which time
      // period.
      //
      aim->requestArchives();
   }

   // Allow 60 seconds for all archives to respond before clearing out
   // any pending requests.
   // Empircally, the rate is approx 5000 PV / sec.
   //
   QTimer::singleShot (60*1000, this, SLOT (clearPending ()));

   this->resendStatus ();

   // Any valid archives specified?
   //
   if (count == 0) {
      qDebug() << "QE_ARCHIVE_LIST environment variable is undefined/empty/invalid";
      qDebug() << "This is required to be defined in order to backfill QEStripChart widgets.";
      qDebug() << "Define as space delimited archiver URLs";

      this->sendMessage ("QEArchiveManager: no valid archives specified",
                         message_types (MESSAGE_TYPE_INFO));
   }

   // Lastly start timer to re interogate the archiver automatically once a day.
   //
   this->timer->start (24*3600*1000);    // mSec
}

//------------------------------------------------------------------------------
//
QEArchiveManager::~QEArchiveManager() { }

//------------------------------------------------------------------------------
// static
QEArchiveManager* QEArchiveManager::getInstance (QString& statusMessage)
{
   QMutexLocker locker (singletonMutex);

   statusMessage = "";   // only set if there is a problem.

   if (singletonManager) {
      return singletonManager;   // already created.
   }

   QEAdaptationParameters ap ("QE_");
   const QString archiveString = ap.getString ("archive_type", "CA").toUpper();

   bool conversionStatus = false;
   int archiverIntVal = QEUtilities::stringToEnum (
            QEArchiveAccess::staticMetaObject,
            QString ("ArchiverTypes"),
            archiveString, &conversionStatus);

   if (!conversionStatus) {
      // Note: caller reports errors, we are static and can't use sendMessage
      //
      statusMessage = QString ("QE_ARCHIVE_TYPE variable '%1' not correctly specified. "
                               "Options are: CA or ARCHAPPL.").arg(archiveString);
      DEBUG << statusMessage;
      return singletonManager;  // It is still NULL
   }

   QEArchiveAccess::ArchiverTypes archiverType;
   archiverType = static_cast<QEArchiveAccess::ArchiverTypes> (archiverIntVal);

   // Question: Is there any need for a separeate thread for the QEArchiveManager
   // object itself as each archiveInterface is running in its own thread??
   // The manager creates a bunch of threads, one for each archiver.
   //
   // Dynamically creating the manager thread avoids shutdown warnings.
   //
   switch (archiverType) {
      case (QEArchiveAccess::CA):
         singletonThread = new QThread (NULL);
         singletonManager = new QEArchiveManager (archiverType);
         break;

      case (QEArchiveAccess::ARCHAPPL):
         // Create ARCHAPPL manager instance only when built with ARCHAPPL support
         //
         #ifdef QE_ARCHAPPL_SUPPORT
            singletonThread = new QThread (NULL);
            singletonManager = new QEArchiveManager (archiverType);
         #else
            statusMessage =
                  "QE_ARCHIVE_TYPE=ARCHAPPL but the QEFramework has not been built "
                  "with Archiver Appliance support, i.e. QE_ARCHAPPL_SUPPORT=YES. "
                  "Please consult the documentation.";
            DEBUG << statusMessage;
         #endif
         break;

      default:
         statusMessage =
               QString ("Archiver type '%1' not supported").arg (archiveString);
         DEBUG << statusMessage;
         break;
   }

   // Check we created the manager and it's thread.
   //
   if (singletonManager && singletonThread) {
      // Set the manager to belong to its own thread, and start it.
      //
      singletonManager->moveToThread (singletonThread);
      singletonThread->start ();
   }

   return singletonManager;
}

//------------------------------------------------------------------------------
// static
int QEArchiveManager::getArchiveNameIndex (const QString& archiveName)
{
   // Locker?
   int result;

   if (archiveNameList.contains (archiveName)) {
      result = archiveNameList.indexOf (archiveName);

   } else {
      result = archiveNameList.count();
      archiveNameList.append (archiveName);
   }
   return result;
}

//------------------------------------------------------------------------------
// static
QString QEArchiveManager::getArchiveNameFromIndex (const int index)
{
   return archiveNameList.value (index, "");
}

//------------------------------------------------------------------------------
// static
int QEArchiveManager::getPathIndex (const QString& pathName)
{
   // Locker?
   int result;

   if (pathNameList.contains (pathName)) {
      result = pathNameList.indexOf (pathName);

   } else {
      result = pathNameList.count();
      pathNameList.append (pathName);
   }
   return result;
}

//------------------------------------------------------------------------------
// static
QString QEArchiveManager::getPathFromIndex (const int index)
{
   return pathNameList.value (index, "");
}

//------------------------------------------------------------------------------
//
QEArchiveAccess::ArchiverTypes
QEArchiveManager::getArchiverType () const
{
   return this->archiverType;
}

//------------------------------------------------------------------------------
//
int QEArchiveManager::getInterfaceCount () const
{
   return this->archiveInterfaceManagerList.count();
}

//------------------------------------------------------------------------------
//
int QEArchiveManager::getNumberPVs () const
{
   QMutexLocker locker (archiveDataMutex);

   return this->pvNameToSourceLookUp->count();
}

//------------------------------------------------------------------------------
//
QString QEArchiveManager::getPattern () const
{
   return this->pattern;
}

//------------------------------------------------------------------------------
//
QStringList QEArchiveManager::getAllPvNames () const
{
   QMutexLocker locker (archiveDataMutex);

   QStringList result;
   result = this->pvNameToSourceLookUp->keys ();
   return result;
}

//------------------------------------------------------------------------------
//
bool QEArchiveManager::getArchivePvInformation (
      const QString& pvName,
      QString& effectivePvName,
      QEArchiveAccess::ArchiverPvInfoLists& data)
{
   QMutexLocker locker (archiveDataMutex);

   bool result = false;
   QEArchiveAccess::MetaRequests meta;

   bool isKnownPVName = this->containsPvName (pvName, effectivePvName, meta);
   if (isKnownPVName) {
      SourceSpec sourceSpec = this->pvNameToSourceLookUp->value (effectivePvName);
      QList<int> keys;

      QString hostName = "";
      if (sourceSpec.interfaceManager) {
        QUrl url = sourceSpec.interfaceManager->getUrl();
        hostName = url.host();
      }

      keys = sourceSpec.keyToTimeSpecLookUp.keys ();
      for (int j = 0; j < keys.count (); j++) {
         int key = keys.value (j, -1);
         if (key < 0) continue;
         KeyTimeSpec keyTimeSpec = sourceSpec.keyToTimeSpecLookUp.value (key);

         QEArchiveAccess::ArchiverPvInfo item;

         item.hostName = hostName;
         item.key = key;
         item.path = QEArchiveManager::getPathFromIndex (keyTimeSpec.pathIndex);
         item.startTime = QCaDateTime (keyTimeSpec.startTime, 0, 0);
         item.endTime = QCaDateTime (keyTimeSpec.endTime, 0, 0);

//         if (item.endTime < item.startTime) {
//            DEBUG << pvName << item.startTime << item.endTime;
//         }

         data.append (item);
         result = true;
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEArchiveManager::clear ()
{
   QMutexLocker locker (archiveDataMutex);
   this->pvNameToSourceLookUp->clear ();
   this->allowPendingRequests = true;
}

//------------------------------------------------------------------------------
//
void QEArchiveManager::resendStatus ()
{
   QEArchiveAccess::StatusList statusList;

   statusList.clear ();
   for (int j = 0; j < this->archiveInterfaceManagerList.count(); j++) {
      QEArchiveInterfaceManager* aim = this->archiveInterfaceManagerList.value (j);
      QEArchiveAccess::Status status;
      aim->getStatus (status);
      statusList.append (status);
   }

   emit this->archiveStatusResponse (statusList);
}

//------------------------------------------------------------------------------
//
bool QEArchiveManager::containsPvName (const QString& pvName,
                                       QString& effectivePvName,
                                       QEArchiveAccess::MetaRequests& meta)
{
   // NOTE: no lock here

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

   meta = QEArchiveAccess::mrNone;   // default

   // Extract the PV name excluding protocol qualifier.
   //
   effectivePvName = uri.getPvName ();

   QEPvNameUri::Protocol protocol = uri.getProtocol ();
   if (protocol != QEPvNameUri::ca) {
      DEBUG << "Only Channel Access protocol archiving is supported:" << pvName;
      this->sendMessage (QString ("Only Channel Access protocol archiving is supported: %1").arg (pvName),
                         message_types (MESSAGE_TYPE_WARNING));
      return false;
   }

   // Is this PV currently being archived?
   //
   result = this->pvNameToSourceLookUp->contains (effectivePvName);
   if (!result) {
      // No - the PV 'as is' is not archived.
      // If user has requested XXXXXX.VAL, check if XXXXXX is archived.
      // Similarly, if user just requested YYYYYY, check if YYYYYY.VAL is archived.
      //
      if (effectivePvName.right (4) == ".VAL") {
         // Remove the .VAL field and try again.
         //
         effectivePvName.chop (4);

      } else if (effectivePvName.right (5) == ".SEVR") {
         // Remove the .VAL field and try again.
         //
         effectivePvName.chop (5);
         meta = QEArchiveAccess::mrSeverity;

      } else if (effectivePvName.right (5) == ".STAT") {
         // Remove the .VAL field and try again.
         //
         effectivePvName.chop (5);
         meta = QEArchiveAccess::mrStatus;

      } else {
         // Add .VAL and try again.
         // This might now be name.FIELD.VAL but it won't exist
         //
         effectivePvName.append (".VAL");
      }
      result = this->pvNameToSourceLookUp->contains (effectivePvName);
   }

   return result;
}

//------------------------------------------------------------------------------
// slot
void QEArchiveManager::aboutToQuitHandler ()
{
   this->timer->stop ();
}

//------------------------------------------------------------------------------
// slot
void QEArchiveManager::reInterogateTimeout ()
{
   this->reInterogateArchives ();
}

//------------------------------------------------------------------------------
// slot
void QEArchiveManager::reInterogateArchives ()
{
   const QCaDateTime caDateTimeNow = QCaDateTime::currentDateTime ();
   const uint32_t timeNow = caDateTimeNow.getSeconds();
   const uint32_t timeSinceLastRead = timeNow - this->lastReadTime;
   if (timeSinceLastRead >= 300) {
      this->lastReadTime = timeNow;

      // More than 5 minutes - re-start interogating the archiver.
      //
      this->clear ();

      for (int j = 0; j < this->archiveInterfaceManagerList.count (); j++) {

         QEArchiveInterfaceManager* aim = this->archiveInterfaceManagerList.value (j);

         // Extract reference to each interface.
         //
         aim->requestArchives ();
      }

      // Allow 60 seconds for all archives to respond before clearing out
      // any pending requests.
      //
      QTimer::singleShot (60*1000, this, SLOT (clearPending ()));

      this->resendStatus ();

   } else {
      this->sendMessage ("Re Archive PVs: less than 5 minutes since last update - request ignored.",
                         message_types (MESSAGE_TYPE_WARNING));
   }
}

//------------------------------------------------------------------------------
// slot
void QEArchiveManager::archiveStatusRequest ()
{
   this->resendStatus ();
}

//------------------------------------------------------------------------------
// slot
void QEArchiveManager::readArchiveRequest (const QEArchiveAccess* archiveAccess,
                                           const QEArchiveAccess::PVDataRequests& request)
{
   QMutexLocker locker (archiveDataMutex);

   QString effectivePvName;
   QEArchiveAccess::MetaRequests meta;
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
   bool isKnownPVName = this->containsPvName (request.pvName, effectivePvName, meta);
   if (isKnownPVName) {

      const SourceSpec sourceSpec = this->pvNameToSourceLookUp->value (effectivePvName);

      int key = -1;
      int bestOverlap = -864000;  // we allow 10 days grace.

      // Check times here - really only applicable to the EPICS CA archiver
      // which supported both a long term and a short term sub-archives
      // for various catagories of data types.
      //
      QList <int> keys = sourceSpec.keyToTimeSpecLookUp.keys ();
      for (int j = 0; j < keys.count (); j++) {

         const KeyTimeSpec keyTimeSpec = sourceSpec.keyToTimeSpecLookUp.value (keys.value (j));

         // Can't use const here
         QCaDateTime startTime = QCaDateTime (keyTimeSpec.startTime, 0, 0);
         QCaDateTime endTime = QCaDateTime (keyTimeSpec.endTime, 0, 0);

         const QCaDateTime useStart = MAX (request.startTime.toUTC (), startTime);
         const QCaDateTime useEnd = MIN (request.endTime.toUTC (), endTime);

         // We don't worry about calculating the overlap to an accuracy
         // of any more one than one second.
         //
         int overlap = useStart.secsTo (useEnd);
         if (overlap > bestOverlap) {
            bestOverlap = overlap;
            key = keyTimeSpec.key;
         }
      }

      if (key >= 0) {
         // All looks good - re-route to the appropriate interface manager
         //
         QEArchiveAccess::PVDataRequests modifiedRequest;
         modifiedRequest = request;
         modifiedRequest.pvName = effectivePvName;
         modifiedRequest.metaRequest = meta;
         sourceSpec.interfaceManager->dataRequest (archiveAccess, key, modifiedRequest);
         this->resendStatus ();

      } else {
         message = "Archive Manager: PV ";
         message.append (request.pvName);
         message.append (" has no matching time overlaps.");
         this->sendMessage (message, message_types (MESSAGE_TYPE_WARNING));

         response.supplementary = message;
         if (archiveAccess)   // sanity check
            archiveAccess->archiveResponse (response);
      }
   }

   else if (this->allowPendingRequests) {
      // Put on pending queue if still initialising.
      //
      PendingRequest pendingRequest;
      pendingRequest.archiveAccess = archiveAccess;
      pendingRequest.userRequest = request;
      this->pendingRequests.prepend (pendingRequest);

   } else {
      message = "Archive Manager: PV ";
      message.append (request.pvName);
      message.append (" not found in archive.");
      this->sendMessage (message, message_types (MESSAGE_TYPE_WARNING));

      response.supplementary = message;
      if (archiveAccess)   // sanity check
         archiveAccess->archiveResponse (response);
   }
}

//------------------------------------------------------------------------------
//
void QEArchiveManager::processPending ()
{
   // NOTE: We iterate backwards because we sometimes remove items from the list.
   //
   const int last = this->pendingRequests.count() - 1;
   for (int j = last; j >= 0;  j--) {

      PendingRequest pendingRequest = this->pendingRequests.value (j);
      QString effectivePvName;
      QEArchiveAccess::MetaRequests meta;

      bool isKnownPVName = this->containsPvName (pendingRequest.userRequest.pvName,
                                                 effectivePvName, meta);

      if (isKnownPVName) {
         // readArchiveRequest will not add back to the list because containsPvName true.
         //
         this->readArchiveRequest (pendingRequest.archiveAccess, pendingRequest.userRequest);
         this->pendingRequests.removeAt (j);
      }
   }
}

//------------------------------------------------------------------------------
// slot - called at 60 seconds after start
//
void QEArchiveManager::clearPending ()
{
   this->allowPendingRequests = false;

   const int last = this->pendingRequests.count() - 1;
   for (int j = last; j >= 0; j--) {

      PendingRequest pendingRequest = this->pendingRequests.value (j);
      QEArchiveAccess::PVDataResponses response;
      QString message;

      // Set failed response.
      //
      response.pvName = pendingRequest.userRequest.pvName;
      response.userData = pendingRequest.userRequest.userData;
      response.isSuccess = false;
      response.pointsList.clear ();
      response.supplementary = "fail";

      message = "Archive Manager: PV ";
      message.append (pendingRequest.userRequest.pvName);
      message.append (" not found in archive.");
      this->sendMessage (message, message_types (MESSAGE_TYPE_WARNING));
      response.supplementary = message;

      if (pendingRequest.archiveAccess)   // sanity check
         pendingRequest.archiveAccess->archiveResponse (response);
   }

   this->pendingRequests.clear ();
   this->resendStatus ();
}

//------------------------------------------------------------------------------
//
void QEArchiveManager::processPvChannel (QEArchiveInterfaceManager* interfaceManager,
                                         const QEArchiveInterface::Archive archive,
                                         const QEArchiveInterface::PVName pvChannel)
{
   QMutexLocker locker (archiveDataMutex);

   QString message;
   KeyTimeSpec keyTimeSpec;
   SourceSpec sourceSpec;

   keyTimeSpec.key = archive.key;
   keyTimeSpec.nameIndex = archive.nameIndex;
   keyTimeSpec.pathIndex = archive.pathIndex;
   keyTimeSpec.startTime = pvChannel.startTime.getSeconds();
   keyTimeSpec.endTime = pvChannel.endTime.getSeconds();

   // Is the end time invalid?
   //
   if (keyTimeSpec.endTime < keyTimeSpec.startTime) {
//    DEBUG << pvChannel.pvName
//          << "  start: " << keyTimeSpec.startTime
//          << "  end: " << keyTimeSpec.endTime
//          << "  => " << this->lastReadTime;

      // The end time cannot sensibly be less that start time, so
      // set to last read time (which is essentially the current time).
      //
      keyTimeSpec.endTime = this->lastReadTime;
   }

   if (!this->pvNameToSourceLookUp->contains (pvChannel.pvName)) {
      // First instance of this PV Name
      //
      sourceSpec.interfaceManager = interfaceManager;
      sourceSpec.keyToTimeSpecLookUp.insert (keyTimeSpec.key, keyTimeSpec);
      this->pvNameToSourceLookUp->insert (pvChannel.pvName, sourceSpec);
      return;
   }

   // Second or subsequent instance of this PV name.
   // To be acceptable, this must be from the same archive host, i.e. the same
   // archive interface, i.e. same archive interface manager.
   //
   sourceSpec = this->pvNameToSourceLookUp->value (pvChannel.pvName);
   if (interfaceManager != sourceSpec.interfaceManager) {
      message = QString ("PV %1 hosted on multiple interfaces. Primary %2, Secondary %3")
            .arg (pvChannel.pvName)
            .arg (sourceSpec.interfaceManager->getName())
            .arg (interfaceManager->getName());
      this->sendMessage (message, message_types (MESSAGE_TYPE_ERROR));
      return;
   }

   // Second or subsequent instance of this PV must have a differnt key
   // (corresponding to short/long term archive).
   //
   if (sourceSpec.keyToTimeSpecLookUp.contains (keyTimeSpec.key)) {
      message = QString ("PV %1 has multiple instances of key %2")
            .arg (pvChannel.pvName)
            .arg ( keyTimeSpec.key);
      this->sendMessage (message, message_types (MESSAGE_TYPE_ERROR));
      return;
   }

   // All good to go with subsequent entry.
   //
   sourceSpec.keyToTimeSpecLookUp.insert (keyTimeSpec.key, keyTimeSpec);

   // QHash: If there is already an item with the key, that item's
   // value is replaced with value.
   //
   this->pvNameToSourceLookUp->insert (pvChannel.pvName, sourceSpec);
}

//------------------------------------------------------------------------------
// slot - from archive interface manager
//
void QEArchiveManager::aimPvNamesResponse  (
      QEArchiveInterfaceManager* interfaceManager,
      const QEArchiveInterface::Archive archive,
      const QEArchiveInterface::PVNameList& pvNameList)
{   
   for (int j = 0; j < pvNameList.count (); j++) {
      const QEArchiveInterface::PVName pvChannel = pvNameList.value (j);
      this->processPvChannel (interfaceManager, archive, pvChannel);
   }

   // We have had an updaye, process any pending requests.
   //
   this->processPending ();

   this->resendStatus();
}

//------------------------------------------------------------------------------
// slot - from archive interface manager
//
void QEArchiveManager::aimDataResponse (
      const QEArchiveAccess* archiveAccess,
      const QEArchiveAccess::PVDataResponses& response)
{
   // We just take the response and pass it back to the requestor.
   //
   if (archiveAccess)  { // sanity check

      const bool isSeverity = response.metaRequest == QEArchiveAccess::mrSeverity;
      const bool isStatus   = response.metaRequest == QEArchiveAccess::mrStatus;

      // Was this a meta data request?
      //
      if (isSeverity || isStatus) {
         // In the data points lits, replace the VALue with the severity or status as requested.
         //
         QCaDataPointList metaPointsList;
         const int n = response.pointsList.count();
         metaPointsList.reserve (n);
         for (int j = 0; j < n; j++) {
            QCaDataPoint point = response.pointsList.value (j);
            point.value = isSeverity ? point.alarm.getSeverity() : point.alarm.getStatus();
            point.alarm = QCaAlarmInfo ();   // clear the alrm info so that always displayable.
            metaPointsList.append (point);
         }

         QEArchiveAccess::PVDataResponses metaResponse;
         metaResponse = response;
         metaResponse.pointsList = metaPointsList;
         archiveAccess->archiveResponse (metaResponse);
      } else {
         // Just return as is.
         archiveAccess->archiveResponse (response);
      }
   }

   this->resendStatus ();
}

// end
