/*  QEBaseClient.h
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

#ifndef QE_BASE_CLIENT_H
#define QE_BASE_CLIENT_H

#include <QFlags>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QEFrameworkLibraryGlobal.h>
#include <QCaAlarmInfo.h>
#include <QCaDateTime.h>
#include <UserMessage.h>

/// This class serves a common base class to both CA abd PVA client class types.
/// While its main function is to be a QObject, i.e. has a parent and therefore
/// is auto deleted when paranet is deleted and also support signals.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEBaseClient : public QObject
{
   Q_OBJECT
public:
   enum Type {
      NullType,      // Unknown/Invalid
      CAType,        // Channel Access
      PVAType        // PV Access
   };

   // Open channel mode selection enumeration values and associated flags.
   //
   enum ChannelModes {
      None = 0x00,
      Read = 0x01,
      Monitor = 0x02,
      Write = 0x04,
   };

   Q_DECLARE_FLAGS (ChannelModesFlags, ChannelModes)

   explicit QEBaseClient (const Type type,
                          const QString& pvName,
                          QObject* parent);
   ~QEBaseClient ();

   // Setup the user message mechanism.
   // After calling this method the client can report errors to the user.
   //
   void setUserMessage (UserMessage* userMessage);
   UserMessage* getUserMessage () const;

   Type    getType () const;         // Returns the client type.
   QString getPvName () const;       // Returns the associated PV name

   // Sub classes must provide these.
   //
   virtual bool openChannel (const ChannelModesFlags modes) = 0;
   virtual void closeChannel () = 0;

   virtual QVariant getPvData () const = 0;
   virtual bool putPvData (const QVariant& value) = 0;

   virtual bool getIsConnected () const = 0;
   virtual bool dataIsAvailable () const = 0;

   virtual QString getId () const = 0;
   virtual QString getRemoteAddress() const = 0;

   virtual QString getEgu () const = 0;
   virtual int getPrecision() const = 0;
   virtual unsigned int hostElementCount () const = 0;
   virtual unsigned int dataElementCount () const = 0;
   virtual double getDisplayLimitHigh () const = 0;
   virtual double getDisplayLimitLow () const = 0;
   virtual double getHighAlarmLimit () const = 0;
   virtual double getLowAlarmLimit () const = 0;
   virtual double getHighWarningLimit () const = 0;
   virtual double getLowWarningLimit () const = 0;
   virtual double getControlLimitHigh () const = 0;
   virtual double getControlLimitLow () const = 0;
   virtual double getMinStep () const = 0;

   virtual QStringList getEnumerations() const = 0;
   virtual QCaAlarmInfo getAlarmInfo () const = 0;
   virtual QCaDateTime  getTimeStamp () const = 0;
   virtual QString getDescription () const = 0;
   virtual bool getReadAccess() const = 0;    // true indicates readable
   virtual bool getWriteAccess() const = 0;   // true indicates writeable

signals:
   // Sub classes may emit these signals.
   //
   void connectionUpdated (const bool isConnected);
   void dataUpdated (const bool firstUpdate);
   void putCallbackComplete (const bool isSuccessful);

private:
   const Type clientType;
   const QString clientPvName;
   UserMessage* userMessage;
};

Q_DECLARE_OPERATORS_FOR_FLAGS (QEBaseClient::ChannelModesFlags)

#endif // QE_BASE_CLIENT_H
