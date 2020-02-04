/*  QEArchiveManager.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2020 Australian Synchrotron
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
 *  Authors:
 *    Andrew Starritt
 *    Andraz Pozar
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *    andraz.pozar@synchrotron.org.au
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
#include <QEArchiveAccess.h>
#include <UserMessage.h>
#include <QEFrameworkLibraryGlobal.h>


//------------------------------------------------------------------------------
// The archive manager can support many different interfaces.
//
typedef QList <QEArchiveInterface*> ArchiveInterfaceLists;

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
   QEArchiveInterface* interface;
   QHash <int, KeyTimeSpec> keyToTimeSpecLookUp;
};

//------------------------------------------------------------------------------
// Mapping by PV name to essentially archive source to key(s) and time range(s)
// that support the PV.
// NOTE: We use a map here as we want sorted keys.
//
typedef QMap <QString, SourceSpec> PVNameToSourceSpecLookUp;

//==============================================================================
// Private
// The following are private classes. They are only exposed in a header because
// the Qt SDK framework and/or meta object data compiler (moc) require that signals
// and slots are declared in header files. Clients should use the QEArchiveAccess
// specified above to initialise the archiver, request status or PV re-reads etc.
//
// This is a singleton class - the single instance is declared in the .cpp file.
//
class QEArchiveManager : public QObject, public UserMessage {
   Q_OBJECT

friend class QEArchiveAccess;

private:
   virtual bool getArchivePvInformation (QString& effectivePvName,
                                         QEArchiveAccess::ArchiverPvInfoLists& data) = 0;

protected:
   explicit QEArchiveManager ();

   // Declaring but not implementing copy constructor and assignemnt operator to avoid
   // having many copies of a singleton.
   //
   QEArchiveManager (QEArchiveManager const&);
   void operator= (QEArchiveManager const&);

   void clear ();
   void updateNumberArchivesRead ();  // technically includes not read due to failure
   void resendStatus ();

   // Checks if the specified PV is archived. This is a smart check:
   // a) it removes any protocol qualifier (e.g. ca://); and
   // b) takes care of the {record name} and {record namer}.VAL ambiguity.
   // The actual archived name is returned in effectivePvName provided the
   // return value is true.
   //
   bool containsPvName (const QString pvName, QString& effectivePvName);

   QString archives;
   QString pattern;
   QDateTime lastReadTime;
   QTimer* timer;

   ArchiveInterfaceLists archiveInterfaceList;
   PVNameToSourceSpecLookUp pvNameToSourceLookUp;

   bool allArchivesRead;
   int numberArchivesRead;
   bool environmentErrorReported;

   QEArchiveAccess::ArchiverTypes archiverType;

signals:
   // Signal to archiver access when the responses are ready
   //
   void archiveStatusResponse (const QEArchiveAccess::StatusList&);
   void readArchiveResponse (const QEArchiveAccess*, const QEArchiveAccess::PVDataResponses&);


protected slots:
   // From archive access
   //
   virtual void readArchiveRequest (const QEArchiveAccess* archiveAccess,
                                    const QEArchiveAccess::PVDataRequests& request) = 0;

   // From archive interface.
   //
   virtual void pvNamesResponse  (const QObject* userData, const bool isSuccess,
                                  const QEArchiveInterface::PVNameList& pvNameList) = 0;

private slots:
   // From owning thread
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
   void started ();

   // Internal slots
   //
   void aboutToQuitHandler ();       // application is about to terminate
   void timeout ();                  // auto archiver re-interogation
   void reInterogateArchives ();     // client requested archiver re-interogation

   // From archive access
   //
   void archiveStatusRequest ();

   // From archive interface
   //
   void nextRequest      (const int requestIndex);
   void archivesResponse (const QObject* userData, const bool isSuccess, const QEArchiveInterface::ArchiveList & archiveListIn);
   void valuesResponse   (const QObject* userData, const bool isSuccess, const QEArchiveInterface::ResponseValueList& valuesList);

};

//==============================================================================
// Implementation of archiver manager for EPICS Channel Archiver
//
class QEChannelArchiverManager : public QEArchiveManager {
   Q_OBJECT

friend class QEArchiveAccess;

private:
   QEChannelArchiverManager();

   static QEChannelArchiverManager& getInstance();
   bool getArchivePvInformation (QString& effectivePvName, QEArchiveAccess::ArchiverPvInfoLists& data);

   // Declaring but not implementing copy constructor and assignemnt operator to avoid
   // having many copies of a singleton.
   //
   QEChannelArchiverManager (QEChannelArchiverManager const&);
   void operator= (QEChannelArchiverManager const&);

private slots:
   void readArchiveRequest (const QEArchiveAccess* archiveAccess, const QEArchiveAccess::PVDataRequests& request);
   void pvNamesResponse  (const QObject* userData, const bool isSuccess, const QEArchiveInterface::PVNameList& pvNameList);

};

//==============================================================================
// Implementation of archiver manager for Archiver Appliance
//
class QEArchapplManager : public QEArchiveManager {
   Q_OBJECT

friend class QEArchiveAccess;

private:
   QEArchapplManager();

   static QEArchapplManager& getInstance();
   bool getArchivePvInformation (QString& effectivePvName, QEArchiveAccess::ArchiverPvInfoLists& data);

   // Declaring but not implementing copy constructor and assignemnt operator to avoid
   // having many copies of a singleton.
   //
   QEArchapplManager (QEArchapplManager const&);
   void operator= (QEArchapplManager const&);

private slots:
   void readArchiveRequest (const QEArchiveAccess* archiveAccess, const QEArchiveAccess::PVDataRequests& request);
   void pvNamesResponse  (const QObject* userData, const bool isSuccess, const QEArchiveInterface::PVNameList& pvNameList);

};

#endif  // QE_ARCHIVE_MANAGER_H
