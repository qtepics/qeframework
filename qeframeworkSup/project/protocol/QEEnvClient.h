/*  QEEnvClient.h
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

#ifndef QE_ENV_CLIENT_H
#define QE_ENV_CLIENT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QCaAlarmInfo.h>
#include <QCaDateTime.h>
#include <QEBaseClient.h>
#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEEnvClient  : public QEBaseClient{
   Q_OBJECT
public:
   explicit QEEnvClient (const QString& pvName, QObject* parent);
   ~QEEnvClient ();

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

signals:
   void environmentVariableUpdate (const QString& pvName);

private:
   bool readVariable ();

   QVariant pvData;        // holds the value data
   QCaDateTime dateTime;
   int precision;
   bool envVarExists;      // exists and has a valid value - a bit of conflation

private slots:
   void doRefresh();
   void doConnectionUpdated();
   void doDataUpdated();
   void updateEnvironmentVariable (const QString& pvName);
};

//------------------------------------------------------------------------------
// This is essentially a singleton private class, but must be declared in the
// header file in order to use the meta object compiler (moc) to allow setup of
// the signals and slots. It uses the Meyer’s Singleton design pattern.
//
class QEEnvValueDispatcher : private QObject {
   Q_OBJECT
public:
signals:
   void environmentVariableUpdate (const QString& pvName);

private:
   // Private - this ensures there can only be one.
   //
   explicit QEEnvValueDispatcher ();
   ~QEEnvValueDispatcher ();

   // Initialise the QEEnvValueDispatcher singleton instance if needs be.
   // It is called each time a QEEnvClient is created.
   // This method aloo connect the its signal to the client slot.
   //
   static QEEnvValueDispatcher* initialise (QEEnvClient* client);

private slots:
   void updateEnvironmentVariable (const QString& pvName);

   friend class QEEnvClient;
};

#endif  // QE_ENV_CLIENT_H
