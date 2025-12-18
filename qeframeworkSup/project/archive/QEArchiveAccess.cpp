/*  QEArchiveAccess.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2017-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andraz Pozar
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEArchiveAccess.h"
#include <QEArchiveManager.h>
#include <QECommon.h>

#define DEBUG qDebug () << "QEArchiveAccess" << __LINE__ << __FUNCTION__  << "  "

static bool archiverInitialised = false;
static QEArchiveManager* archiveManager = NULL;

//------------------------------------------------------------------------------
//
void QEArchiveAccess::initialiseArchiverType ()
{
   if (archiverInitialised) return;  // idempotent. Do we need a mutex?
   archiverInitialised = true;

   archiveManager = QEArchiveManager::getInstance (this->constructorMessage);
   if (!archiveManager) {
      this->constructorMessageType = message_types (MESSAGE_TYPE_ERROR);

      // Print the message after the construction has finished
      //
      QTimer::singleShot(0, this, SLOT (sendMessagePostConstruction()));
   }
}

//------------------------------------------------------------------------------
//
QEArchiveAccess::QEArchiveAccess (QObject * parent) : QObject (parent)
{
   this->initialiseArchiverType ();  // idempotent

   // Connect status request response signals.
   // Note: instance set up by initialiseArchiverType
   //
   if (!archiveManager) return;   // sanity check

   // Request that manager re-read the set of avialable PVs from the archiver.
   //
   QObject::connect (this,           SIGNAL (reInterogateArchives ()),
                     archiveManager, SLOT   (reInterogateArchives ()));

   // Connect archive status request and response signals.
   //
   QObject::connect (this,           SIGNAL (archiveStatusRequest ()),
                     archiveManager, SLOT   (archiveStatusRequest ()));

   QObject::connect (archiveManager, SIGNAL (archiveStatusResponse (const QEArchiveAccess::StatusList&)),
                     this,           SLOT   (archiveStatusResponse (const QEArchiveAccess::StatusList&)));


   // Connect data request and response signals.
   //
   QObject::connect (this,           SIGNAL (readArchiveRequest  (const QEArchiveAccess*,
                                                                  const QEArchiveAccess::PVDataRequests&)),
                     archiveManager, SLOT   (readArchiveRequest  (const QEArchiveAccess*,
                                                                  const QEArchiveAccess::PVDataRequests&)));

   // We send the archive data response to ourself, invoked by the archiveManager
   // calling the archiveResponse function. In this way, the response is only
   // sent to the acrive access object that requested it.
   //
   QObject::connect (this, SIGNAL (signalArchiveResponse (const QEArchiveAccess::PVDataResponses&)),
                     this, SLOT   (actionArchiveResponse (const QEArchiveAccess::PVDataResponses&)));
}

//------------------------------------------------------------------------------
//
QEArchiveAccess::~QEArchiveAccess () { }

//------------------------------------------------------------------------------
// Made non static to ensure a QEArchiveAccess instance exists before
// the function is used.
//
QEArchiveAccess::ArchiverTypes QEArchiveAccess::getArchiverType () const
{
   return archiveManager ? archiveManager->getArchiverType() : QEArchiveAccess::Error;
}

//------------------------------------------------------------------------------
//
void QEArchiveAccess::sendMessagePostConstruction()
{
   this->sendMessage (this->constructorMessage, this->constructorMessageType);
}

//------------------------------------------------------------------------------
//
unsigned int QEArchiveAccess::getMessageSourceId () const
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
   if (archiveManager) {
      emit this->archiveStatusRequest ();
   }
}

//------------------------------------------------------------------------------
//
void QEArchiveAccess::reReadAvailablePVs ()
{
   if (archiveManager) {
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

   emit this->readArchiveRequest (this, request);
}

//------------------------------------------------------------------------------
// Called by the QEArchiverManager in the QEArchiverManager's thread
// Sent to actionArchiveResponse slot processed in QEArchiveAccess's thread.
//
void QEArchiveAccess::archiveResponse (const QEArchiveAccess::PVDataResponses& response) const
{
    emit this->signalArchiveResponse (response);
}

//------------------------------------------------------------------------------
// slot
void QEArchiveAccess::actionArchiveResponse (const QEArchiveAccess::PVDataResponses& response)
{
   // Forward resonse on to the requestor.
   //
   emit this->setArchiveData (response.userData, response.isSuccess,
                              response.pointsList, response.pvName,
                              response.supplementary);
}

//------------------------------------------------------------------------------
// static functions
//------------------------------------------------------------------------------
//
//
bool QEArchiveAccess::isReady ()
{
   return archiveManager ? true : false;
}

//------------------------------------------------------------------------------
//
int QEArchiveAccess::getNumberInterfaces ()
{
   return archiveManager ? archiveManager->getInterfaceCount () : 0;
}

//------------------------------------------------------------------------------
//
QString QEArchiveAccess::getPattern ()
{
   return archiveManager ? archiveManager->getPattern() : "";
}

//------------------------------------------------------------------------------
//
int QEArchiveAccess::getNumberPVs ()
{
   return archiveManager ? archiveManager->getNumberPVs() : 0;
}

//------------------------------------------------------------------------------
//
QStringList QEArchiveAccess::getAllPvNames ()
{
   QStringList result;

   if (archiveManager) {
      result = archiveManager->getAllPvNames ();
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

   if (archiveManager && !pvName.isEmpty ()) {
      result = archiveManager->getArchivePvInformation (pvName, effectivePvName, data);
   }

   return result;
}

//------------------------------------------------------------------------------
// Register own meta types.
//
bool QEArchiveAccess::_registerMetaTypes()
{
   qRegisterMetaType<QEArchiveAccess::Status> ("QEArchiveAccess::Status");
   qRegisterMetaType<QEArchiveAccess::StatusList> ("QEArchiveAccess::StatusList");
   qRegisterMetaType<QEArchiveAccess::PVDataRequests> ("QEArchiveAccess::PVDataRequests");
   qRegisterMetaType<QEArchiveAccess::PVDataResponses> ("QEArchiveAccess::PVDataResponses");
   return true;
}

// Elaborate on start up.
//
static const bool _elaborate = QEArchiveAccess::_registerMetaTypes ();

// end
