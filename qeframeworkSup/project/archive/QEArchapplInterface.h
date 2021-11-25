/*  QEArchapplInterface.h
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
 *    Andraz Pozar
 *  Contact details:
 *    andrazp@ansto.gov.au
 */

#ifndef QE_ARCH_APPL_INTERFACE_H
#define QE_ARCH_APPL_INTERFACE_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVector>
#include <QList>
#include <QStringList>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#include <QEArchiveInterface.h>
#include <QCaDataPoint.h>
#include <QCaDateTime.h>
#include <QCaAlarmInfo.h>

class QEArchapplNetworkManager;  // differed

/// Interface to EPICS Archiver Appliance.
///
class QEArchapplInterface :
      public QEArchiveInterface
{
   Q_OBJECT

public:
   explicit QEArchapplInterface (QUrl url, QObject* parent = 0);
   virtual ~QEArchapplInterface ();

   // Archiver Appliance specific implementation of xxxRequest function
   // Override QEArchiveInterface virtual methods.
   //
   void namesRequest (QObject* userData,
                      const int key,
                      QString pattern = ".*");

   void valuesRequest (QObject* userData,
                       const QCaDateTime startTime,
                       const QCaDateTime endTime,
                       const int count,
                       const How how,
                       const QStringList pvNames,
                       const int key = 0,
                       const unsigned int requested_element = 0);

   void infoRequest (QObject* userData);

   void archivesRequest (QObject* userData);

public slots:
   // Triggered by signals coming from network manager
   //
   void networkManagerResponse(const QEArchiveInterface::Context & context,
                               QNetworkReply* reply);
   void networkManagerFault(const QEArchiveInterface::Context& context,
                            const QNetworkReply::NetworkError error);

private:
   QEArchapplNetworkManager* networkManager;

   void processInfo     (const QObject* userData, QNetworkReply* reply);
   void processArchives (const QObject* userData);
   void processPvNames  (const QObject* userData, QNetworkReply* reply);
   void processValues   (const QObject* userData, QNetworkReply* reply,
                         const unsigned int requested_element);
};


//------------------------------------------------------------------------------
// This is a private class - network manager for Archiver Appliance. It's used
// for managing connection and actual data retrival.
//
// When the data is received it emits a signal so that QEArchapplInterface can
// start with data processing.
//
class QEArchapplNetworkManager : public QObject {
   Q_OBJECT

private:
   explicit QEArchapplNetworkManager(const QUrl& url, QObject* parent = 0);
   ~QEArchapplNetworkManager();

   // Values request struct contraining all needed information
   // to retrieve values from Archiver Appliance
   //
   typedef struct ArchapplValuesRequest {
      QStringList names;
      QEArchiveInterface::How how;
      QString startTime;
      QString endTime;
      int count;
   } ValuesRequest;


   // Archiver appliance uses two separate URLs. One is specialized in
   // data retrieval and the other one provides all other status information
   // about PVs and appliance itself.
   //
   QUrl bplURL, dataURL;
   QNetworkAccessManager* networkManager;

   void getPVs(const QEArchiveInterface::Context& context, const QString& pattern);
   void getApplianceInfo(const QEArchiveInterface::Context& context);
   void executeRequest(const QUrl url, const QEArchiveInterface::Context& context);
   void getValues(const QEArchiveInterface::Context& context, const ValuesRequest& request, const unsigned int binSize);

signals:
   // Signals that a response from the Archiver Appliance is ready. The type of reponse
   // is set in the cotext
   //
   void networkManagerResponse(const QEArchiveInterface::Context& context, QNetworkReply* reply);

   // Signals that the request has failed
   //
   void networkManagerFault (const QEArchiveInterface::Context& context,
                             const QNetworkReply::NetworkError error);

private slots:
   // Triggered from networ manager when the reply is finished and data us ready
   //
   void replyFinished();

   // We are very popular
   //
   friend class QEArchapplInterface;
};

#endif // QE_ARCH_APPL_INTERFACE_H
