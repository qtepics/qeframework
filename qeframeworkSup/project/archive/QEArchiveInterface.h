/*  QEArchiveInterface.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2021 Australian Synchrotron
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
 *  Contact details:
 *    andrews@ansto.gov.au
 */

#ifndef QE_ARCHIVE_INTERFACE_H
#define QE_ARCHIVE_INTERFACE_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QList>
#include <QStringList>
#include <QUrl>

#include <QCaDataPoint.h>
#include <QCaDateTime.h>
#include <QCaAlarmInfo.h>

/// This virtual class provides the functional interface to the achivers.
///
/// There are currently two sub classes:
///    QEChannelArchiveInterface (for the traditional EPICS CA archiver ); and
///    QEArchapplInterface (for the new Archive Appliance archiver)
///
/// Other archivers could be added.
///
class QEArchiveInterface :
      public QObject
{
   Q_OBJECT

public:
   // Extends the EPICS alarm severity with archive specials
   //
   enum archiveAlarmSeverity {
       // From Epics base alarm.h
       archSevNone       = 0,
       archSevMinor      = 1,
       archSevMajor      = 2,
       archSevInvalid    = 3,
       // From RawValue.h (arch-2.9.2)
       archSevEstRepeat  = 0x0f80,
       archSevDisconnect = 0x0f40,
       archSevStopped    = 0x0f20,    // aka Archive_Off
       archSevRepeat     = 0x0f10,
       archSevDisabled   = 0x0f08
   };
   Q_ENUMS (archiveAlarmSeverity)

   static QString alarmSeverityName (enum archiveAlarmSeverity severity);

   enum How {
      Raw = 0,
      SpreadSheet,
      Averaged,
      PlotBinning,
      Linear
   };
   Q_ENUMS (How)

   // Available archives for given host/port/end_point - key is most important.
   //
   struct Archive {
      int key;
      QString name;
      QString path;
   };

   // Available PV names (matching regular expression pattern) for
   // a given archive together with first/last available times.
   //
   struct PVName {
      QString pvName;
      QCaDateTime startTime;
      QCaDateTime endTime;
   };

   // Returned values for a single channel.
   //
   struct ResponseValues {
      // meta data
      QString pvName;
      double displayLow;
      double displayHigh;
      int precision;
      QString units;
      unsigned int elementCount;    // number of elements per the channel value.
      QCaDataPointList dataPoints;  // actual data
   };

   // Essentially private - needs to be public for meta data type declaration.
   //
   enum Methods {
      Information,
      Archives,
      Names,
      Values,
      Count
   };
   Q_ENUMS (Methods)

   enum States {
      Unknown,
      Updating,
      Complete,
      InComplete,
      No_Response,
      Error
   };
   Q_ENUMS (States)

   struct Context {
      QEArchiveInterface::Methods method;
      QObject* userData;
      unsigned int requested_element;
   };

   typedef QList <QEArchiveInterface::Archive> ArchiveList;
   typedef QList<QEArchiveInterface::PVName> PVNameList;
   typedef std::list<QEArchiveInterface::ResponseValues> ResponseValueList;

   //---------------------------------------------------------------------------
   //
   explicit QEArchiveInterface (QObject* parent = 0);
   virtual ~QEArchiveInterface ();

   virtual void setUrl (QUrl url);
   QUrl getUrl () const;

   // Returns string image of the url
   //
   QString getName () const;

   // Each of the xxxxRequest functions result in a xxxxResponse signal
   // being emitted. In each case the given userData is not used by the
   // QEArchiveInterface class per se other it is returned in the signal
   // to provide the caller with signal context.
   //
   virtual void infoRequest (QObject* userData) = 0;

   virtual void archivesRequest (QObject* userData) = 0;

   virtual void namesRequest (QObject* userData,
                              const int key,
                              QString pattern = ".*") = 0;

   /* The requested_element parameter specfies the (waveform) array element required.
    * This parameter applies to all the PVs requested. If different array elements
    * are needed for different PVs the separate calls to valuesRequest are required.
    * Note: element numbers start from 0. The default default value of 0 is suitable
    * for scalar PVs.
    */
   virtual void valuesRequest (QObject* userData,
                               const QCaDateTime startTime,
                               const QCaDateTime endTime,
                               const int count,
                               const How how,
                               const QStringList pvNames,
                               const int key = 0,
                               const unsigned int requested_element = 0) = 0;

   // Register these meta types.
   // Note: This function is public for conveniance only, and is invoked by the
   // module itself during program elaboration.
   //
   static bool registerMetaTypes ();

protected:
   static QCaDateTime convertArchiveToEpics (const int seconds, const int nanoSecs);
   static void convertEpicsToArchive (const QCaDateTime& datetime, int& seconds, int& nanoSecs);

signals:
   // The QObject* (1st) parameter is the userData supplied to the corresponding
   // xxxRequest functions.
   //
   // The boolean (2nd) parameter is a wasSuccessfull parameter, i.e. when true
   // this indicates a successfull response, and when false indicates a fault
   // condition. For the later case, the actual value parameters are undefined.
   //
   void pvNamesResponse  (const QObject*, const bool, const QEArchiveInterface::PVNameList&);
   void valuesResponse   (const QObject*, const bool, const QEArchiveInterface::ResponseValueList&);
   void infoResponse     (const QObject*, const bool, const int, const QString&);
   void archivesResponse (const QObject*, const bool, const QEArchiveInterface::ArchiveList&);
   void nextRequest      (const int requestIndex);

protected:
   QUrl mUrl;
};

Q_DECLARE_METATYPE (QEArchiveInterface::ArchiveList)
Q_DECLARE_METATYPE (QEArchiveInterface::PVNameList)
Q_DECLARE_METATYPE (QEArchiveInterface::ResponseValueList)
Q_DECLARE_METATYPE (QEArchiveInterface::Context)
Q_DECLARE_METATYPE (QEArchiveInterface::States)

#endif // QE_ARCHIVE_INTERFACE_H
