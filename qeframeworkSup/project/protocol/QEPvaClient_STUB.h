/*  QEPvaClient_STUB.h
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

#ifndef QE_PVA_CLIENT_STUB_H
#define QE_PVA_CLIENT_STUB_H

#include <stdint.h>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariant>

#include <QEBaseClient.h>
#include <QCaAlarmInfo.h>
#include <QCaDateTime.h>
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
};

#endif // QE_PVA_CLIENT_H
