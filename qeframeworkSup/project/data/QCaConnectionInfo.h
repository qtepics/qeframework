/*  QCaConnectionInfo.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2009-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

// Manage CA/PVA connection information

#ifndef QCA_CONNECTION_INFO_H
#define QCA_CONNECTION_INFO_H

#include <QEFrameworkLibraryGlobal.h>
#include <QString>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QCaConnectionInfo
{
public:
   enum channel_states {
      NEVER_CONNECTED,
      CONNECTED,
      CLOSED,
      CHANNEL_UNKNOWN
   };

   explicit QCaConnectionInfo();
   explicit QCaConnectionInfo( const channel_states channelStateIn,
                               const QString& pvName );
   ~QCaConnectionInfo();

   bool isChannelConnected() const;  // Return true if the channel is connected

   QString variable() const;         // Return the variable name. Usefull if same slot is used for reporting multiple connections

private:
   channel_states channelState;      // Channel status
   QString variableName;             // Process variable name
};

#endif // QCA_CONNECTION_INFO_H
