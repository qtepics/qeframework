/*  QEArchiveManager.h
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
 *  Copyright (c) 2012,2016,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

/*
 * Archive Manager manages access to the archives, and provides a thick binding
 * around the Archive Interface class. It's main function is to provide a PV Name
 * to interface/archive mapping.
 */

#ifndef QE_ARCHIVE_MANAGER_H
#define QE_ARCHIVE_MANAGER_H

#include <QList>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QTimer>

#include <QCaDateTime.h>
#include <QEArchiveInterface.h>
#include <UserMessage.h>
#include <QEPluginLibrary_global.h>

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
class QEPLUGINLIBRARYSHARED_EXPORT QEArchiveAccess : public QObject, UserMessage {
   Q_OBJECT
public:
   explicit QEArchiveAccess (QObject* parent = 0);
   virtual ~QEArchiveAccess ();

   unsigned int getMessageSourceId ();
   void setMessageSourceId (unsigned int messageSourceId);

   static void initialise (const QString& archives, const QString& pattern);
   static void initialise ();
   static bool isReady ();
   static int getNumberInterfaces ();
   static QString getPattern ();
   static int getNumberPVs ();

   static QStringList getAllPvNames ();

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
   void readArchive (QObject* userData,        // provides call back signal context
                     const QString pvName,
                     const QCaDateTime startTime,
                     const QCaDateTime endTime,
                     const int count,
                     const QEArchiveInterface::How how,
                     const unsigned int element = 0);

   enum States {
      Unknown,
      Updating,
      Complete,
      InComplete,
      No_Response,
      Error
   };
   Q_ENUMS (States)

   struct Status {
      QString hostName;  //
      int portNumber;    //
      QString endPoint;  //
      States state;      //
      int available;     // number of archives
      int read;          // number of archives suiccessfully read
      int numberPVs;     //
      int pending;       // number of outstanding request/responces
   };

   typedef QList<Status> StatusList;


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

   friend class QEArchiveManager;
};

// These type are distributed via the signal/slot mechanism. Must
// declare then as such (here) and register them (within implementation).
//
Q_DECLARE_METATYPE (QEArchiveAccess::States)
Q_DECLARE_METATYPE (QEArchiveAccess::Status)
Q_DECLARE_METATYPE (QEArchiveAccess::StatusList)
Q_DECLARE_METATYPE (QEArchiveAccess::PVDataRequests)
Q_DECLARE_METATYPE (QEArchiveAccess::PVDataResponses)


//==============================================================================
// Private
// The following are private classes. They are only exposed in a header because
// the Qt SDK framework and/or meta object data compiler (moc) require that signals
// and slots are declared in header files. Clients should use the QEArchiveAccess
// specified above to initialise the archiver, request status or PV re-reads etc.
//
// This is a singleton class - the single instance is declared in the .cpp file.
//
class QEArchiveManager : public QObject, UserMessage {
   Q_OBJECT
private:
   QEArchiveManager (const QString& archives, const QString& pattern);

   // This function connects the specified the archive(s). The format of the string is
   // space separated set of one or more hostname:port/endpoint triplets, e.g.
   //
   // "CR01ARC01:80/cgi-bin/ArchiveDataServer.cgi CR01ARC02:80/cgi-bin/ArchiveDataServer.cgi"
   //
   // A leading http:// is neither required nor permitted.
   //
   // Once connected, it creates a map pf PV names to host/archive key/available times.
   //
   // The pattern parameter can be used to restrict the set of extracted PVs. The same
   // pattern applies of all archives. The pattern is a regular expression.
   //
   void setup ();

   // Idempotent and thread safe initialise functions.
   // The second overloaded form uses the environment variables QE_ARCHIVE_LIST and
   // QE_ARCHIVE_PATTERN. If QE_ARCHIVE_PATTERN is undefined then ".*" is used.
   //
   static void initialise (const QString& archives, const QString& pattern);
   static void initialise ();

   void clear ();
   void resendStatus ();

   QString archives;
   QString pattern;
   QDateTime lastReadTime;
   QTimer* timer;

   friend class QEArchiveAccess;

   // Status request/response from/to archive interface objects.
   //
private slots:
   void archiveStatusRequest ();                                     // from archive interface
signals:
   void archiveStatusResponse (const QEArchiveAccess::StatusList&);  // to archive interface


   // Data request/response from/to archive interface objects.
   //
private slots:
   void timeout ();                  // auto archiver re-interogation
   void reInterogateArchives ();     // client requested archiver re-interogation
   void readArchiveRequest (const QEArchiveAccess* archiveAccess,
                            const QEArchiveAccess::PVDataRequests& request);

signals:
   void readArchiveResponse (const QEArchiveAccess*,
                             const QEArchiveAccess::PVDataResponses&);

private slots:
   // From owning thread
   void started ();

   // From archive interface.
   //
   void archivesResponse (const QObject* userData, const bool isSuccess, const QEArchiveInterface::ArchiveList & archiveList);
   void pvNamesResponse  (const QObject* userData, const bool isSuccess, const QEArchiveInterface::PVNameList& pvNameList);
   void valuesResponse   (const QObject* userData, const bool isSuccess, const QEArchiveInterface::ResponseValueList& valuesList);

   // From ArchiveInterfacePlus
   //
   void nextRequest      (const int requestIndex);
};


// This class essentially justs extends QEArchiveInterface by adding some additional
// status data and a timer.
//
class ArchiveInterfacePlus : public QEArchiveInterface {
   Q_OBJECT
private:
   explicit ArchiveInterfacePlus (QUrl url, QObject* parent = 0);

   QEArchiveInterface::ArchiveList archiveList;
   QEArchiveAccess::States state;
   int available;
   int read;
   int numberPVs;

   int requestIndex;

   QTimer* timer;

   friend class QEArchiveManager;

signals:
   void nextRequest (const int requestIndex);

private slots:
   void timeout ();
};

#endif  // QE_ARCHIVE_MANAGER_H
