/* QENullClient.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (C) 2023 Australian Synchrotron
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

};

#endif   // QE_NULL_CLIENT_H
