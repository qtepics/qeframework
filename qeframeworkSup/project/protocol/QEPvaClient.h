/*  QEPvaClient.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (C) 2018 Australian Synchrotron
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
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

#ifndef QE_PVA_CLIENT_H
#define QE_PVA_CLIENT_H

#include <stdint.h>
#include <QObject>
#include <QString>
#include <QFlags>
#include <QTimer>
#include <QVariant>
#include <QEPvaCheck.h>

#ifdef QE_INCLUDE_PV_ACCESS
#include <pv/pvAccess.h>
#endif

#include <QEBaseClient.h>
#include <Generic.h>
#include <QCaAlarmInfo.h>
#include <QCaDateTime.h>
#include <QEPvaData.h>
#include <QEFrameworkLibraryGlobal.h>

/// The QEPvaClient class
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPvaClient : public QEBaseClient
{
   Q_OBJECT
public:
   class Update;       // differed
   class UpdateQueue;  // differed

   explicit QEPvaClient (const QString& pvName,
                         qcaobject::QCaObject* parent);
   ~QEPvaClient ();

   // Open channel mode selection enumeration values and associated flags.
   //
   enum ChannelModes {
      None = 0x00,
      Read = 0x01,
      Monitor = 0x02,
      Write = 0x04,
   };

   Q_DECLARE_FLAGS (ChannelModesFlags, ChannelModes)

   bool openChannel (const ChannelModesFlags modes);
   void closeChannel ();

   generic::generic_types getDataType () const;
   QVariant getPvData () const;
   bool putPvData (const QVariant& value);

   bool getIsConnected () const;
   bool dataIsAvailable () const;

   QString getId () const;
   QString getRemoteAddress() const;

   QString getEgu () const;
   unsigned int getPrecision() const;
   unsigned int dataElementCount () const;
   double getDisplayLimitHigh () const;
   double getDisplayLimitLow () const;
   double getHighAlarmLimit () const;
   double getLowAlarmLimit () const;
   double getHighWarningLimit () const;
   double getLowWarningLimit () const;
   double getControlLimitHigh () const;
   double getControlLimitLow () const;
   double getMinStep () const;

   QStringList getEnumerations () const;
   QCaAlarmInfo getAlarmInfo () const;
   QCaDateTime  getTimeStamp () const;

private:
   void processUpdate (QEPvaClient::Update* update);

   // The framework does not use strong references to track QEPvaClient objects,
   // so we use a magic tag and unique identifier to detect stale references.
   //
   uint64_t magic;         // class type check
   uint64_t uniqueId;      // class instance check
   bool isConnected;       //
   bool firstUpdate;       //
   QString id;             // e.g.  "epics:nt/NTScalar:1.0"
   QVariant pvData;        // holds the value data

#ifdef QE_INCLUDE_PV_ACCESS
   // We need to keep strong references to these objects.
   //
   epics::pvAccess::ChannelRequester::shared_pointer channelRequester;
   epics::pvAccess::MonitorRequester::shared_pointer monitorRequester;
   epics::pvAccess::ChannelPutRequester::shared_pointer putRequester;
   epics::pvAccess::ChannelGetRequester::shared_pointer getRequester;
   epics::pvAccess::Channel::shared_pointer channel;
   epics::pvAccess::Monitor::shared_pointer monitor;
   epics::pvAccess::ChannelGet::shared_pointer getter;
   epics::pvAccess::ChannelPut::shared_pointer putter;

   QEPvaData::Enumerated enumeration;
   QEPvaData::Alarm alarm;
   QEPvaData::TimeStamp timeStamp;
   QEPvaData::Control control;
   QEPvaData::Display display;
   QEPvaData::ValueAlarm valueAlarm;
#endif

   friend class QEPvaClientReference;
   friend class QEPvaPutRequesterInterface;
};

Q_DECLARE_OPERATORS_FOR_FLAGS (QEPvaClient::ChannelModesFlags)

//------------------------------------------------------------------------------
// This is essentially a private class, but must be declared in the header
// file in order to use the meta object compiler (moc) to allow setup of the
// timeout/aboutToQuit slots.
//
class QEPvaClientManager : private QTimer {
   Q_OBJECT
private:
   explicit QEPvaClientManager ();
   ~QEPvaClientManager ();

   // Create the singleton QEPvaClientManager instance if needs be.
   // It is called each time a QCaObject.QEPvaClient is created.
   // This function is idempotent.
   //
   static void initialise ();

private slots:
   void timeoutHandler ();
   void aboutToQuitHandler ();

private:
   friend class QEPvaClient;
};

#endif // QE_PVA_CLIENT_H
