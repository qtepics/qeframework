/*  QEPvaClient_PVA.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2018-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_PVA_CLIENT_PVA_H
#define QE_PVA_CLIENT_PVA_H

#include <stdint.h>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariant>

#include <pv/pvAccess.h>
#include <QEPvaData.h>

#include <QEBaseClient.h>
#include <QCaAlarmInfo.h>
#include <QCaDateTime.h>
#include <QEFrameworkLibraryGlobal.h>

/// The QEPvaClient class - using PVA
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPvaClient : public QEBaseClient
{
   Q_OBJECT
public:
   class Update;       // differed
   class UpdateQueue;  // differed

   explicit QEPvaClient (const QString& pvName,
                         QObject* parent);
   ~QEPvaClient ();

   bool openChannel (const ChannelModesFlags modes);
   void closeChannel ();

   // Override QEBaseClient parent functions.
   //
   QVariant getPvData () const;
   bool putPvData (const QVariant& value);

   // Non-overriden functions
   //
   bool getIsConnected () const;
   bool dataIsAvailable () const;

   QString getId () const;
   QString getRemoteAddress() const;

   QString getEgu () const;
   int getPrecision() const;
   unsigned int hostElementCount () const;
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

   // More overrides.
   //
   QStringList getEnumerations () const;
   QCaAlarmInfo getAlarmInfo () const;
   QCaDateTime  getTimeStamp () const;
   QString getDescription () const;
   bool getReadAccess() const;
   bool getWriteAccess() const;

private:
   // The framework does not use strong references to track QEPvaClient objects,
   // so we use a magic tag and unique identifier to detect stale references.
   //
   uint64_t magic;           // class type check
   uint64_t uniqueId;        // class instance check
   bool isConnected;         //
   bool firstUpdate;         //
   QString id;               // e.g.  "epics:nt/NTScalar:1.0"
   QString pvType;           // e.g.  "double" when NTScalar or when NTArray
   QVariant pvData;          // holds the value data

   // Original PVA
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

   void processUpdate (QEPvaClient::Update* update);

   QEPvaData::Enumerated enumeration;
   QEPvaData::Alarm alarm;
   QEPvaData::TimeStamp timeStamp;
   QEPvaData::Control control;
   QEPvaData::Display display;
   QEPvaData::ValueAlarm valueAlarm;

   friend class QEPvaClientReference;
   friend class QEPvaPutRequesterInterface;
   friend class QEPvaClientManager;
};

//------------------------------------------------------------------------------
// This is essentially a singleton private class, but must be declared in the
// header file in order to use the meta object compiler (moc) to allow setup of
// the timeout slot. It uses the Meyer’s Singleton design pattern.
// The main purpose of this class to stimulate the underlying library on a
// regular basis in order to process PVA callbacks do a clean shutdown.
//
class QEPvaClientManager : private QObject {
   Q_OBJECT
public:
   // Initialse the singleton QEPvaClientManager instance if needs be.
   // It is called each time a QEPvaClient is created.
   // This function is idempotent.
   //
   static void initialise ();

private:
   // Private - this ensures there can only be one.
   //
   explicit QEPvaClientManager ();
   ~QEPvaClientManager ();

private slots:
   void timeoutHandler ();
};

#endif // QE_PVA_CLIENT_PVA_H
