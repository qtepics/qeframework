/*  QEArchiveManager.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andraz Pozar, Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_ARCHIVE_MANAGER_H
#define QE_ARCHIVE_MANAGER_H

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QUrl>

#include <QCaDateTime.h>
#include <QEArchiveAccess.h>
#include <QEArchiveInterface.h>
#include <UserMessage.h>

class QEArchiveInterfaceManager;        // differed

/// Archive Manager manages access to the archives, and provides a thick binding
/// around the Archive Interface class. It's main function is to provide a PV Name
/// to interface/archive mapping.
///
/// This is a singleton class - the single instance is declared in the .cpp file.
/// It is effectively a private class used execlusivly by QEArchiveAccess.
///
class QEArchiveManager :
      public QObject,
      public UserMessage
{
   Q_OBJECT

   friend class QEArchiveAccess;
   friend class QEArchiveInterfaceManager;

public:
   // Utility functions
   //
   // In the data structures, we store shared index as opposed to a shared QString
   // which still uses 24 bytes.
   //
   static int getArchiveNameIndex (const QString& archiveName);
   static QString getArchiveNameFromIndex (const int index);

   static int getPathIndex (const QString& pathName);
   static QString getPathFromIndex (const int index);

private:
   explicit QEArchiveManager (const QEArchiveAccess::ArchiverTypes archiverType);
   virtual ~QEArchiveManager ();

   // Declaring but not implementing copy constructor and assignemnt operator
   // to avoid having many copies of a singleton.
   //
   QEArchiveManager (QEArchiveManager const&);
   void operator= (QEArchiveManager const&);

   // This funtion will create the singleton QEArchiveManager object and all
   // the underlying infra structure if needs be or can be.
   //
   static QEArchiveManager* getInstance (QString& statusMessage);

   QEArchiveAccess::ArchiverTypes getArchiverType () const;

   int getInterfaceCount () const;
   int getNumberPVs () const;
   QString getPattern () const;
   QStringList getAllPvNames () const;   
   bool getArchivePvInformation (const QString& pvName,
                                 QString& effectivePvName,
                                 QEArchiveAccess::ArchiverPvInfoLists& data);
   void clear ();
   void resendStatus ();
   void processPending ();

   // Checks if the specified PV is archived. This is a smart check:
   // a) it removes any protocol qualifier (e.g. ca://); and
   // b) takes care of the {record name} and {record namer}.VAL ambiguity.
   // The actual archived name is returned in effectivePvName provided the
   // return value is true.
   //
   bool containsPvName (const QString& pvName,
                        QString& effectivePvName,
                        QEArchiveAccess::MetaRequests& meta);

   // Processes meta PV data from the archive interface managers.
   // This allows the QEArchiveManager to know if a PV is available
   // and if so, from which archive.
   //
   void processPvChannel (QEArchiveInterfaceManager* interfaceManager,
                          const QEArchiveInterface::Archive archive,
                          const QEArchiveInterface::PVName pvChannel);

   const QEArchiveAccess::ArchiverTypes archiverType;
   QString pattern;
   bool allowPendingRequests;
   uint32_t lastReadTime;    // only need seconds past epoch here
   QTimer* timer;

   typedef QList <QEArchiveInterfaceManager*> ArchiveInterfaceManagerLists;

   ArchiveInterfaceManagerLists archiveInterfaceManagerList;

   class PVNameToSourceSpecLookUp;
   PVNameToSourceSpecLookUp* pvNameToSourceLookUp;

   // Hold a set (list) of requests awaiting completion of the initial
   // data retrieval from the various archivers.
   //
   struct PendingRequest {
      const QEArchiveAccess* archiveAccess;
      QEArchiveAccess::PVDataRequests userRequest;
   };

   typedef QList<PendingRequest> PVDataRequestLists;
   PVDataRequestLists pendingRequests;

signals:
   // Signals to archiverAccess objects when the responses are ready
   //
   void archiveStatusResponse (const QEArchiveAccess::StatusList&);
   void readArchiveResponse (const QEArchiveAccess*,    // context
                             const QEArchiveAccess::PVDataResponses&);

private slots:
   // From archiveAccess instances.
   //
   void reInterogateArchives ();
   void archiveStatusRequest ();
   void readArchiveRequest (const QEArchiveAccess* archiveAccess,  // context
                            const QEArchiveAccess::PVDataRequests& request);


   // From the approprate archive interface manager
   // "aim" is acronym used in this class for Archive Interface Manager.
   //
   void aimPvNamesResponse  (QEArchiveInterfaceManager* interfaceManager,
                             const QEArchiveInterface::Archive archive,
                             const QEArchiveInterface::PVNameList& pvNameList);

   void aimDataResponse (const QEArchiveAccess* archiveAccess,
                         const QEArchiveAccess::PVDataResponses& response);

   // Internal slots
   //
   // This function connects the specified the archive(s). The format of the string is
   // space separated set of one or more hostname:port/endpoint triplets, e.g.
   //
   // "CR01ARC01:80/cgi-bin/ArchiveDataServer.cgi CR01ARC02:80/cgi-bin/ArchiveDataServer.cgi"  or
   // "cr01arc04:17665/mgmt/bpl/ sr02ir01arc01:17665/mgmt/bpl/ sr14id01arc01:17665/mgmt/bpl/"
   //
   // A leading http:// is not required.
   //
   // Once connected, it creates a map of PV names to host/archive key/available times.
   //
   // The pattern parameter can be used to restrict the set of extracted PVs. The same
   // pattern applies of all archives. The pattern is a regular expression.
   //
   void started ();

   // Clears any pending requests after initialisation
   //
   void clearPending ();

   void aboutToQuitHandler ();       // application is about to terminate
   void reInterogateTimeout ();      // daily auto archiver re-interogation
};

#endif  // QE_ARCHIVE_MANAGER_H
