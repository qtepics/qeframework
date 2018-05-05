/*  QEArchiveAccess.cpp
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
 *  Copyright (c) 2017 Australian Synchrotron
 *
 *  Author:
 *    Andraz Pozar
 *  Contact details:
 *    andraz.pozar@synchrotron.org.au
 */

#include <QEArchiveAccess.h>
#include <QEArchiveManager.h>
#include <QECommon.h>

#include <QEAdaptationParameters.h>

#define DEBUG qDebug () << "QEArchiveAccess" << __LINE__ << __FUNCTION__  << "  "

static QEArchiveManager* instance = NULL;
static QMutex *archiveDataMutex = new QMutex ();
static bool archiveTypeErrorReported = false;

//==============================================================================
//
QEArchiveAccess::QEArchiveAccess (QObject * parent) : QObject (parent)
{
   // Are we using Archiver Appliance?
   //
   QEAdaptationParameters ap ("QE_");
   const QString archiveString = ap.getString ("archive_type", "CA").toUpper();

   bool conversionStatus = false;
   int archiverIntVal = QEUtilities::stringToEnum(*this, QString("ArchiverTypes"), archiveString, &conversionStatus);

   ArchiverTypes archiverType = QEArchiveAccess::Error;
   if (conversionStatus) {
      archiverType = (ArchiverTypes) archiverIntVal;
   } else {
      if (!archiveTypeErrorReported) {
         archiveTypeErrorReported = true;
         DEBUG << "QE_ARCHIVE_TYPE variable not correctly specified. Options are: CA or ARCHAPPL.";
         this->constructorMessage = "QE_ARCHIVE_TYPE variable not correctly specified. Options are: CA or ARCHAPPL.";
         this->constructorMessageType = message_types(MESSAGE_TYPE_ERROR);
         // Print the message after the construction has finished
         //
         QTimer::singleShot(0, this, SLOT (sendMessagePostConstruction()));
       }
      return;
   }

   // Construct and initialise singleton QEArchiveManager object if needs be.
   //
   switch (archiverType){
      case (QEArchiveAccess::CA):
         instance = &QEChannelArchiverManager::getInstance();
         break;
      case (QEArchiveAccess::ARCHAPPL):
         // Create ARCHAPPL manager instance only when built with ARCHAPPL support
         //
         #ifdef QE_ARCHAPPL_SUPPORT
            instance = &QEArchapplManager::getInstance();
         #else
            DEBUG << "QE_ARCHIVE_TYPE=ARCHAPPL but the QEFramework has not been built with Archiver Appliance support. Please consult the documentation.";
            this->constructorMessage = "QE_ARCHIVE_TYPE=ARCHAPPL but the QEFramework has not been built with Archiver Appliance support. Please consult the documentation.";
            this->constructorMessageType = message_types(MESSAGE_TYPE_ERROR);
            // Print the message after the construction has finished
            //
            QTimer::singleShot(0, this, SLOT (sendMessagePostConstruction()));
         #endif
         break;
      default:
         DEBUG << "Archiver type not supported";
         this->constructorMessage = "Archiver type not supported";
         this->constructorMessageType = message_types(MESSAGE_TYPE_ERROR);
         // Print the message after the construction has finished
         //
         QTimer::singleShot(0, this, SLOT (sendMessagePostConstruction()));

         return;
   }

   // Connect status request response signals.
   //
   if( instance )
   {
      // Request that manager re-read the set of avialble PVs from the archiver.
      //
      QObject::connect (this,     SIGNAL (reInterogateArchives ()),
                        instance, SLOT   (reInterogateArchives ()));

      // archive status request and response.
      //
      QObject::connect (this,     SIGNAL (archiveStatusRequest ()),
                        instance, SLOT   (archiveStatusRequest ()));

      QObject::connect (instance, SIGNAL (archiveStatusResponse (const QEArchiveAccess::StatusList&)),
                        this,     SLOT   (archiveStatusResponse (const QEArchiveAccess::StatusList&)));


      // Connect data request response signals.
      //
      QObject::connect (this,     SIGNAL (readArchiveRequest  (const QEArchiveAccess*, const QEArchiveAccess::PVDataRequests&)),
                        instance, SLOT   (readArchiveRequest  (const QEArchiveAccess*, const QEArchiveAccess::PVDataRequests&)));

      QObject::connect (instance, SIGNAL (readArchiveResponse (const QEArchiveAccess*, const QEArchiveAccess::PVDataResponses&)),
                        this,     SLOT   (readArchiveResponse (const QEArchiveAccess*, const QEArchiveAccess::PVDataResponses&)));
   }
}

//------------------------------------------------------------------------------
//
QEArchiveAccess::~QEArchiveAccess ()
{
}

void QEArchiveAccess::sendMessagePostConstruction()
{
   this->sendMessage(this->constructorMessage, this->constructorMessageType);
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
   if (instance) {
      emit this->archiveStatusRequest ();
   }
}

//------------------------------------------------------------------------------
//
void QEArchiveAccess::reReadAvailablePVs ()
{
   if (instance) {
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
//
QEArchiveAccess::ArchiverTypes QEArchiveAccess::getArchiverType ()
{
   return instance ? instance->archiverType : QEArchiveAccess::Error;
}

//------------------------------------------------------------------------------
// static functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Deprecated
void QEArchiveAccess::initialise (const QString&, const QString&)
{
   qDebug () << "\n This function is obsolete. Archives and pattern have to be defined using the appropriate environment variables.\n";
}

//------------------------------------------------------------------------------
// Deprecated
void QEArchiveAccess::initialise ()
{
   qDebug () << "\n This function is obsolete. Archives and pattern have to be defined using the appropriate environment variables.\n";
}

//------------------------------------------------------------------------------
//
bool QEArchiveAccess::isReady ()
{
   return instance ? instance->allArchivesRead : false;
}

//------------------------------------------------------------------------------
//
int QEArchiveAccess::getNumberInterfaces ()
{
   return instance ? instance->archiveInterfaceList.count () : 0;
}

//------------------------------------------------------------------------------
//
QString QEArchiveAccess::getPattern ()
{
   return instance ? instance->pattern : "";
}

//------------------------------------------------------------------------------
//
int QEArchiveAccess::getNumberPVs ()
{
   QMutexLocker locker (archiveDataMutex);
   return instance ? instance->pvNameToSourceLookUp.count () : 0;
}

//------------------------------------------------------------------------------
//
QStringList QEArchiveAccess::getAllPvNames ()
{
   QMutexLocker locker (archiveDataMutex);
   QStringList result;

   if (instance) {
      result = instance->pvNameToSourceLookUp.keys ();
   }

   return  result;
}

//------------------------------------------------------------------------------
//
bool QEArchiveAccess::getArchivePvInformation (const QString& pvName,
                                               QString& effectivePvName,
                                               ArchiverPvInfoLists& data)
{
   bool result = false;
   effectivePvName = pvName;
   data.clear ();

   if (instance && QEArchiveAccess::isReady () && !pvName.isEmpty ()) {
      result = instance->getArchivePvInformation(effectivePvName, data);
   }

   return result;
}

//------------------------------------------------------------------------------
// Register own meta types.
//
bool QEArchiveAccess::registerMetaTypes()
{
   qRegisterMetaType<QEArchiveAccess::Status> ("QEArchiveAccess::Status");
   qRegisterMetaType<QEArchiveAccess::StatusList> ("QEArchiveAccess::StatusList");
   qRegisterMetaType<QEArchiveAccess::PVDataRequests> ("QEArchiveAccess::PVDataRequests");
   qRegisterMetaType<QEArchiveAccess::PVDataResponses> ("QEArchiveAccess::PVDataResponses");
   return true;
}

// Elaborate on start up.
//
static const bool _elaborate = QEArchiveAccess::registerMetaTypes ();

// end
