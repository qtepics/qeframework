/*  QEArchiveAccess.h
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

#ifndef QE_ARCHIVE_ACCESS_H
#define QE_ARCHIVE_ACCESS_H

#include <QList>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <QCaDateTime.h>
#include <QEArchiveInterface.h>

#include <UserMessage.h>
#include <QEFrameworkLibraryGlobal.h>


class QEArchiveManager;   // differed


/// This class provides the EPICS Qt Framework access to the archives and
/// indirect usage of the underlying QEArchiveManager.
///
/// Currently only handles scalar values but can/will be extended to
/// provide array data retrival.
///
/// NOTE: It is the creation of the first object of this class will cause the
/// QEArchiveManager to initialised if not already done so.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEArchiveAccess :
      public QObject,
      public UserMessage
{
   Q_OBJECT
public:
   enum ArchiverTypes {
      CA,          // Traditional EPIC Channel Access archiver
      ARCHAPPL,    // Archive Appliance archiver
      Error        // Malformed archiver specification
   };
   Q_ENUM (ArchiverTypes)

   explicit QEArchiveAccess (QObject* parent = 0);
   virtual ~QEArchiveAccess ();

   // Exposes archiver type to the client(s).
   //
   ArchiverTypes getArchiverType () const;

   // For use with the UserMessage mechanism.
   //
   unsigned int getMessageSourceId () const;
   void setMessageSourceId (unsigned int messageSourceId);

   // Is archiver communication ready.
   //
   static bool isReady ();

   // Number of archive interfaces.
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
   void readArchive (QObject* userData,      // provides call back signal context
                     const QString pvName,
                     const QCaDateTime startTime,
                     const QCaDateTime endTime,
                     const int count,
                     const QEArchiveInterface::How how,
                     const unsigned int element = 0);

   // Defines the nature of the archives found when the QEArchiveManager
   // interogated the available archives.
   //
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
      QString hostName;                      //
      int key;
      QString path;
      QCaDateTime startTime;
      QCaDateTime endTime;
   };
   typedef QList<ArchiverPvInfo>  ArchiverPvInfoLists;

   static bool getArchivePvInformation (const QString& pvName,
                                        QString& effectivePvName,
                                        ArchiverPvInfoLists& data);


   // --------------------------------------------------------------------------
   // These are essentially a private type, but must be public for
   // meta data registration.
   //
   enum MetaRequests {
      mrNone = 0,      // VALue request
      mrSeverity,      // SEVRity request
      mrStatus         // STATUS request
   };

   struct PVDataRequests {
      QObject* userData;
      int metaRequest;        // defined by MetaRequests
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
      int metaRequest;        // defined by MetaRequests
      bool isSuccess;
      QCaDataPointList pointsList;
      QString pvName;
      QString supplementary;  // error info when not successfull
   };

   // Register these meta types.
   // Note: This function is public for conveniance only, and is invoked by the
   // module itself during program elaboration.
   //
   static bool _registerMetaTypes ();

signals:
   // Signals back to users in response to above service requests.
   //
   void archiveStatus  (const QEArchiveAccess::StatusList& statusList);
   void setArchiveData (const QObject* userData,
                        const bool isOkay,
                        const QCaDataPointList& pointsList,
                        const QString& pvName,
                        const QString& supplementary);

private:
   void initialiseArchiverType ();

   friend class QEArchiveManager;
   void archiveResponse (const QEArchiveAccess::PVDataResponses& response) const;


   // Used to convey a message during the creation of the object.
   //
   QString constructorMessage;
   message_types constructorMessageType;

   // Requests responses to/from the Archive Manager.
   //
signals:
   void reInterogateArchives ();
   void archiveStatusRequest ();
   void readArchiveRequest (const QEArchiveAccess*,
                            const QEArchiveAccess::PVDataRequests&);

   // This is sent indirectly from the Archive Manager via emitArchiveResponse.
   //
   void signalArchiveResponse (const QEArchiveAccess::PVDataResponses& response) const;

private slots:
   // Note: The archiveStatusResponse sent to all ArchiveAccess objects.
   //
   void archiveStatusResponse (const QEArchiveAccess::StatusList&);

   void actionArchiveResponse (const QEArchiveAccess::PVDataResponses& response);

   void sendMessagePostConstruction ();
};

// Data of these types are distributed via the signal/slot mechanism. We must
// declare then as such (here) and register them (within implementation).
//
Q_DECLARE_METATYPE (QEArchiveAccess::Status)
Q_DECLARE_METATYPE (QEArchiveAccess::StatusList)
Q_DECLARE_METATYPE (QEArchiveAccess::PVDataRequests)
Q_DECLARE_METATYPE (QEArchiveAccess::PVDataResponses)

#endif // QE_ARCHIVE_ACCESS_H
