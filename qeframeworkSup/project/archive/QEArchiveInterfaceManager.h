/*  QEArchiveInterfaceManager.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2021-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_ARCHIVE_INTERFACE_MANAGER_H
#define QE_ARCHIVE_INTERFACE_MANAGER_H

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QString>
#include <QTimer>
#include <QUrl>

#include <UserMessage.h>
#include <QEArchiveAccess.h>
#include <QEArchiveInterface.h>

class QEArchiveManager;   // differed

//------------------------------------------------------------------------------
// This is a private class used by the QEArchiveManager.
// There is one instance per interface as defined in the QE_ARCHIVE_LIST
// envirionment variable. Each manager instance runs in it's own thread.
//
class QEArchiveInterfaceManager : public QTimer, public UserMessage {
   Q_OBJECT

   friend class QEArchiveManager;
private:
   explicit QEArchiveInterfaceManager (const int instance,
                                       QEArchiveInterface* interface,
                                       QEArchiveManager* owner,
                                       QObject* parent);
   virtual ~QEArchiveInterfaceManager ();

   // Returns a QEArchiveInterfaceManager ref or NULL if there is a problem.
   //
   static QEArchiveInterfaceManager* createInterfaceManager (
         const int instance,
         const QEArchiveAccess::ArchiverTypes archiverType,
         const QUrl& url,
         QEArchiveManager* owner);


   void getStatus (QEArchiveAccess::Status& status) const;

   QUrl getUrl () const;
   QString getName () const;       // Returns string image of the url

   // Called by QEArchiveManager in QEArchiveManager's thread. These functions
   // send signals to self to run in the QEArchiveInterfaceManager own thread.
   // We go indirect like this as only our instance of signalDataRequest
   // to connected to our instance of actionDataRequest.
   //
   void requestArchives ();
   void dataRequest (const QEArchiveAccess* archiveAccess,
                     const int key,
                     const QEArchiveAccess::PVDataRequests& request);

signals:
   // Signals to self
   //
   void signalRequestArchives ();
   void signalDataRequest (const QEArchiveAccess*,
                           const int,
                           const QEArchiveAccess::PVDataRequests&);

   // Signals to the Archive Manager when the responses are available.
   //
   void aimPvNamesResponse (QEArchiveInterfaceManager*,
                            const QEArchiveInterface::Archive,
                            const QEArchiveInterface::PVNameList&);

   void aimDataResponse (const QEArchiveAccess*,    // context
                         const QEArchiveAccess::PVDataResponses&);

private slots:
   // From the Archive Manager via self
   //
   void actionRequestArchives ();
   void actionDataRequest (const QEArchiveAccess* archiveAccess,  // context
                           const int key,
                           const QEArchiveAccess::PVDataRequests& request);

   // From the archive interface
   //
   void archivesResponse (const QObject* userData, const bool isSuccess,
                          const QEArchiveInterface::ArchiveList & archiveListIn);

   void pvNamesResponse  (const QObject* userData, const bool isSuccess,
                          const QEArchiveInterface::PVNameList& pvNameList);

   void valuesResponse   (const QObject* userData, const bool isSuccess,
                          const QEArchiveInterface::ResponseValueList& valuesList);

private:
   enum Constants {
      maxActiveQueueSize = 200,   // maxiumum number of outstanding requests allowed.
      maxAllowedTime = 60         // allowd time before timeout (in seconds).
   };

   struct RequestInfo {
      int unique;
      QDateTime timeoutTime;
      const QEArchiveAccess* archiveAccess;
      QEArchiveAccess::PVDataRequests request;
      int key;
   };

   void actionNamesRequest (const int index);
   void activateDataRequest (RequestInfo& info);
   void dump() const;             // diagnostic debug output only.

   const int instance;
   QEArchiveInterface* archiveInterface;
   QEArchiveManager* owner;

   QMutex* aimMutex;

   QEArchiveInterface::States state;
   QEArchiveInterface::ArchiveList archiveList;
   int requestIndex;
   int responseCount;
   volatile int numberPVs;

   typedef QQueue <RequestInfo> RequestQueues;
   RequestQueues requestQueue;

   int unique;
   typedef QHash <int, RequestInfo>  RequestLists;
   RequestLists activeRequests;

   class NamesResponseContext;
   class ValuesResponseContext;

private slots:
   void started ();              // From owning thread
   void aboutToQuitHandler ();   // From the application
   void timeoutHandler ();       // From self
};


#endif  // QE_ARCHIVE_INTERFACE_MANAGER_H
