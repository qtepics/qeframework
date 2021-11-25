/*  QEArchiveInterfaceCA.h
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
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_CHANNEL_ARCHIVE_INTERFACE_H
#define QE_CHANNEL_ARCHIVE_INTERFACE_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVector>
#include <QList>
#include <QStringList>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include <QCaDataPoint.h>
#include <QCaDateTime.h>
#include <QCaAlarmInfo.h>

#include <maiaXmlRpcClient.h>
#include <QEArchiveInterface.h>

/// This class provides a thin wrapper around the maiaXmlRpcClient specifically
/// for handling the EPICS Channel Access Archive XML RPC procedure calls.
/// While functionally thin, it does provide the conversion between the types
/// used by the XmlRpcClient (nested structure of QVarients) the types specific
/// archive EpicsQt, e.g. time to/from QCaDateTime and QCaDataPoint.
///
/// This class uses the libMaia client written by
/// Sebastian Wiedenroth <wiedi@frubar.net> and Karl Glatz.
///
class QEChannelArchiveInterface :
      public QEArchiveInterface
{
   Q_OBJECT

public:
   explicit QEChannelArchiveInterface (QUrl url, QObject* parent = 0);
   virtual ~QEChannelArchiveInterface ();

   // EPICS CA Archiver specific implementation of xxxRequest functions
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

   void setUrl (QUrl url);

private:
   friend class QEArchiveInterfaceAgent;

   typedef QMap<QString, QVariant> StringToVariantMaps;

   enum MetaType {
      mtEnumeration = 0,
      mtNumeric = 1
   };

   enum DataType {
      dtString = 0,
      dtEnumeration = 1,
      dtInteger = 2,
      dtDouble = 3
   };

   MaiaXmlRpcClient* client;

   void processInfo     (const QObject* userData, const QVariant& response);
   void processArchives (const QObject* userData, const QVariant& response);
   void processPvNames  (const QObject* userData, const QVariant& response);
   void processValues   (const QObject* userData, const QVariant& response,
                         const unsigned int requested_element);

   void processOnePoint (const DataType dtype,
                         const StringToVariantMaps& value,
                         const unsigned int requested_element,
                         QCaDataPoint & datum);

   void processOnePV (const StringToVariantMaps& map,
                      const unsigned int requested_element,
                      struct ResponseValues& item);

private slots:
   // Used by intermediary QEArchiveInterfaceAgent
   // Note need fully qualified QEArchiveInterface::Context in order to match signals.
   //
   void xmlRpcResponse (const QEArchiveInterface::Context& context, const QVariant & response);
   void xmlRpcFault    (const QEArchiveInterface::Context& context, int error, const QString & response);
};


//------------------------------------------------------------------------------
// Essentially a private class. It provides a means to add context (method and
// original user data) to the signal callbacks from the MaiaXmlRpcClient object.
//
// The MaiaXmlRpcClient seems to be asynchronous, thus this agent class can be
// passive. If we change underlying XML RPC library then the agent class can be
// modified to inherit from QThread to provide the asynchronicity if needs be.
//
class QEArchiveInterfaceAgent : public QObject {
   Q_OBJECT

private:
   friend class QEChannelArchiveInterface;

   explicit QEArchiveInterfaceAgent (MaiaXmlRpcClient* clientIn,
                                     QEChannelArchiveInterface* parent);
   ~QEArchiveInterfaceAgent ();

   QNetworkReply* call (QEArchiveInterface::Context& contextIn,
                        QString procedure,
                        QList<QVariant> args);

   MaiaXmlRpcClient* client;
   QEArchiveInterface::Context context;

signals:
   void xmlRpcResponse (const QEArchiveInterface::Context&, const QVariant &);
   void xmlRpcFault    (const QEArchiveInterface::Context&, int, const QString &);


private slots:
   // from maia xml_rpc client
   //
   void xmlRpcResponse (QVariant& response);
   void xmlRpcFault    (int error, const QString& response);
};

#endif // QE_CHANNEL_ARCHIVE_INTERFACE_H
