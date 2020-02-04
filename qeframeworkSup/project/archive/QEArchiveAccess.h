/*  QEArchiveAccess.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2017-2020 Australian Synchrotron
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
 *    Andraz Pozar
 *  Contact details:
 *    andraz.pozar@synchrotron.org.au
 */

#ifndef QE_ARCHIVE_ACCESS_H
#define QE_ARCHIVE_ACCESS_H

#include <QList>
#include <QMetaType>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QTimer>

#include <QCaDateTime.h>
#include <QEArchiveInterface.h>

#include <UserMessage.h>
#include <QEFrameworkLibraryGlobal.h>

// This class provides user access to the archives and indirect usage of the
// underlying QEArchiveManager.
//
// Currently only handles scalar values but can/will be extended to
// provide array data retrival.
//
// NOTE: It is the creation of the first object of this class will cause the
// QEArchiveManager to initialised if not already done so. The QEArchiveManager
// may also be explicitly initialised prior to that by invoking one of the
// static initialise functions.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEArchiveAccess : public QObject, public UserMessage {
   Q_OBJECT
public:
   explicit QEArchiveAccess (QObject* parent = 0);
   virtual ~QEArchiveAccess ();

   unsigned int getMessageSourceId ();
   void setMessageSourceId (unsigned int messageSourceId);

   // This function no longer does anything. Archives and pattern have to
   // be defined using the appropriate environment variables.
   // Please consult the documentation
   //
   Q_DECL_DEPRECATED
   static void initialise (const QString& archives, const QString& pattern);

   // This function no longer does anything. Archives and pattern have to
   // be defined using the appropriate environment variables.
   // Please consult the documentation
   //
   Q_DECL_DEPRECATED
   static void initialise ();

   // Is archiver communication ready
   //
   static bool isReady ();

   // Number of archiving interfaces. This should be 1 in case of Archiver Appliance
   //
   static int getNumberInterfaces ();

   // Pattern used to poll for all PVs from the archiver
   //
   static QString getPattern ();

   // Number of PVs available
   //
   static int getNumberPVs ();

   static QStringList getAllPvNames ();

   // Requests re-transmission of archive status.
   // Returned status is via archiveStatus signal.
   // This info re-emitted on change, but this allows an (initial) status quo update.
   //
   void resendStatus ();

   // Requests a re-read of the available PVs.
   //
   void reReadAvailablePVs ();

   // Simple archive request - single scaler PV, or one arbitary element from
   // a single array PV.  No extended meta data, just values + timestamp + alarm info.
   // The data, if any, is sent via the setArchiveData signal.
   //
   // Returned data is via setArchiveData signal.
   //
   void readArchive (QObject* userData,                     // provides call back signal context
                     const QString pvName,
                     const QCaDateTime startTime,
                     const QCaDateTime endTime,
                     const int count,
                     const QEArchiveInterface::How how,
                     const unsigned int element = 0);

   struct Status {
      QString hostName;                      //
      int portNumber;                        //
      QString endPoint;                      //
      QEArchiveInterface::States state;      //
      int available;                         // number of archives
      int read;                              // number of archives successfully read
      int numberPVs;                         //
      int pending;                           // number of outstanding request/responses
   };
   typedef QList<Status> StatusList;

   // Supports getArchivePvInformation
   //
   struct ArchiverPvInfo {
     int key;
     QString path;
     QCaDateTime startTime;
     QCaDateTime endTime;
   };
   typedef QList<ArchiverPvInfo>  ArchiverPvInfoLists;

   static bool getArchivePvInformation (const QString& pvName,
                                        QString& effectivePvName,
                                        ArchiverPvInfoLists& data);

   enum ArchiverTypes {
      CA,
      ARCHAPPL,
      Error
   };
   Q_ENUMS (ArchiverTypes)

   // Exposes archiver type to the client
   //
   static ArchiverTypes getArchiverType ();

   // These are essentially a private type, but must be public for metat data registration.
   //
   struct PVDataRequests {
      QObject* userData;
      int key;
      QString pvName;
      QCaDateTime startTime;
      QCaDateTime endTime;
      int count;
      QEArchiveInterface::How how;
      unsigned int element;
   };

   struct PVDataResponses {
      QObject* userData;
      bool isSuccess;
      QCaDataPointList pointsList;
      QString pvName;
      QString supplementary;  // error info when not successfull
   };

   // Register these meta types.
   // Note: This function is public for conveniance only, and is invoked by the
   // module itself during program elaboration.
   //
   static bool registerMetaTypes ();

signals:
   // Signals back to users in response to above service requests.
   //
   void archiveStatus  (const QEArchiveAccess::StatusList& statusList);
   void setArchiveData (const QObject* userData, const bool isOkay,
                        const QCaDataPointList& pointsList,
                        const QString& pvName, const QString& supplementary);

   // Depricated
   void setArchiveData (const QObject*, const bool, const QCaDataPointList&);

   // Requests responses to/from the Archive Manager.
   // NOTE: response goes to all archive access instances.
signals:
   void reInterogateArchives ();
   void archiveStatusRequest ();
   void readArchiveRequest (const QEArchiveAccess*,
                            const QEArchiveAccess::PVDataRequests&);

private slots:
   void archiveStatusResponse (const QEArchiveAccess::StatusList&);

   void readArchiveResponse (const QEArchiveAccess* archiveAccess,
                             const QEArchiveAccess::PVDataResponses& response);

   void sendMessagePostConstruction ();

   void retryTimeout ();

private:
   // Used to convey a message during the creation of the object.
   //
   QString constructorMessage;
   message_types constructorMessageType;

   // Hold a list (queue) of requesyt awaiting completion of intial
   // data retrieval from the archivers.
   //
   typedef QList<PVDataRequests> PVDataRequestLists;
   PVDataRequestLists pendingRequests;
   QMutex pendingRequestMutex;
};

// These type are distributed via the signal/slot mechanism. Must
// declare then as such (here) and register them (within implementation).
//
Q_DECLARE_METATYPE (QEArchiveAccess::Status)
Q_DECLARE_METATYPE (QEArchiveAccess::StatusList)
Q_DECLARE_METATYPE (QEArchiveAccess::PVDataRequests)
Q_DECLARE_METATYPE (QEArchiveAccess::PVDataResponses)

#endif // QE_ARCHIVE_ACCESS_H
