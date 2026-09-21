/*  QELitClient.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_LIT_CLIENT_H
#define QE_LIT_CLIENT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QCaAlarmInfo.h>
#include <QCaDateTime.h>
#include <QEBaseClient.h>
#include <QEFrameworkLibraryGlobal.h>

/// This class provides a literal value channel type.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QELitClient : public QEBaseClient {
   Q_OBJECT
public:
   explicit QELitClient (const QString& pvName, QObject* parent);
   ~QELitClient ();

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

private:
   bool readVariable ();

   QVariant pvData;        // holds the value data
   QCaDateTime dateTime;
   int precision;

private slots:
   void doConnectionUpdated();
   void doDataUpdated();
};

#endif  // QE_LIT_CLIENT_H
