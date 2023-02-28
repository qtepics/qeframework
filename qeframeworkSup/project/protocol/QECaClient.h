/*  QECaClient.h
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

#ifndef QE_CA_CLIENT_H
#define QE_CA_CLIENT_H

#include <acai_client_types.h>
#include <acai_client.h>

#include <QEBaseClient.h>
#include <QCaAlarmInfo.h>
#include <QCaDateTime.h>
#include <QEFrameworkLibraryGlobal.h>

/// The main purpose of this class is to convert regular call backs from
/// the ACAI client class into Qt signals (specified in QEBaseClient).
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QECaClient :
      public QEBaseClient, public ACAI::Client
{
   Q_OBJECT
public:
   explicit QECaClient (const QString& pvName,
                        QObject* parent);
   ~QECaClient ();

   // Override QEBaseClient parent functions.
   //
   QVariant getPvData () const;
   bool putPvData (const QVariant& value);

   QStringList getEnumerations() const;
   QCaAlarmInfo getAlarmInfo () const;
   QCaDateTime  getTimeStamp () const;
   QString getDescription () const;

protected:
   // Override ACAI::Client parent class functions.
   //
   void connectionUpdate (const bool isConnected);
   void dataUpdate (const bool firstUpdate);
   void putCallbackNotifcation (const bool isSuccessful);

private:
   // Local conveniance functions.
   //
   bool varientToFloat (const QVariant& qValue, ACAI::ClientFloating& fValue, bool& valueInRange);
   bool varientToInteger (const QVariant& qValue, ACAI::ClientInteger& iValue, bool& valueInRange);
   bool varientToEnumIndex (const QVariant& qValue, ACAI::ClientInteger& index, bool& valueInRange);

   QECaClient* descriptionClient;  // connects to the .DESC field

private slots:
   void requestDescription ();
};

//------------------------------------------------------------------------------
// This is essentially a private class, but must be declared in the header
// file in order to use the meta object compiler (moc) to allow setup of the
// timeout/aboutToQuit slots.
// The main purpose of this class to stimulate the underlying library on a
// regular basis in order to process CA callbacks. It also receives the
// aboutToQuit signal in order to do a clean shutdown.
//
class QECaClientManager : private QObject {
   Q_OBJECT
private:
   explicit QECaClientManager ();
   ~QECaClientManager ();

   // Create the singleton QECaClientManager instance if needs be.
   // It is called each time a QCaObject.QECaClient is created.
   // This function is idempotent.
   //
   static void initialise ();

private:
   // Not used directly, set pass as a parameter.
   static void notificationHandlers (const char* notification);

   bool stillRunning;
private slots:
   void timeoutHandler ();
   void aboutToQuitHandler ();

   friend class QECaClient;
};

#endif // QE_CA_CLIENT_H
