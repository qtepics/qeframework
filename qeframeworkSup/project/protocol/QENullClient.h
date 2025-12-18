/*  QENullClient.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2023-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_NULL_CLIENT_H
#define QE_NULL_CLIENT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QEBaseClient.h>
#include <QCaAlarmInfo.h>
#include <QCaDateTime.h>
#include <QEFrameworkLibraryGlobal.h>

/// Dummy/null client created when an invalid/unknown protocol is specified.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QENullClient : public QEBaseClient
{
   Q_OBJECT
public:
   explicit QENullClient (const QString& pvName, QObject* parent);
   ~QENullClient  ();

   // Override all virtual functions
   // Where appliable return null results.
   //
   bool openChannel (const ChannelModesFlags modes);
   void closeChannel ();

   QVariant getPvData () const;
   bool putPvData (const QVariant& value);

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

   QStringList getEnumerations() const;
   QCaAlarmInfo getAlarmInfo () const;
   QCaDateTime  getTimeStamp () const;
   QString getDescription () const;
   bool getReadAccess() const;
   bool getWriteAccess() const;

};

#endif   // QE_NULL_CLIENT_H
