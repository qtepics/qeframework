/*  QEBaseClient.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (C) 2018-2023 Australian Synchrotron
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

#ifndef QE_BASE_CLIENT_H
#define QE_BASE_CLIENT_H

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
   enum Type { CAType,        // Channel Access
               PVAType };     // PV Access

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
   virtual QVariant getPvData () const = 0;
   virtual bool putPvData (const QVariant& value) = 0;
   virtual QStringList getEnumerations() const = 0;
   virtual QCaAlarmInfo getAlarmInfo () const = 0;
   virtual QCaDateTime  getTimeStamp () const = 0;
   virtual QString getDescription () const = 0;

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

#endif // QE_BASE_CLIENT_H
