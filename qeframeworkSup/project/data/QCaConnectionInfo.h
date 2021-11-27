/*  QCaConnectionInfo.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2021 Australian Synchrotron
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
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

// Manage CA connection information

#ifndef QCA_CONNECTION_INFO_H
#define QCA_CONNECTION_INFO_H

#include <QEFrameworkLibraryGlobal.h>
#include <QString>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QCaConnectionInfo
{
public:
    // These were previously declared in CaConnection.h
    //
    enum link_states { LINK_UP, LINK_DOWN, LINK_UNKNOWN };

    enum channel_states { NEVER_CONNECTED, PREVIOUSLY_CONNECTED,
                          CONNECTED, CLOSED, CHANNEL_UNKNOWN };

    explicit QCaConnectionInfo();
    explicit QCaConnectionInfo( const channel_states channelStateIn,
                                const link_states linkStateIn,
                                const QString& pvName );
    ~QCaConnectionInfo();

    bool isChannelConnected() const;  // Return true if the channel is connected

    QString variable() const;         // Return the variable name. Usefull if same slot is used for reporting multiple QCa connections

private:
    channel_states channelState;      // Channel status
    link_states linkState;            // Link status
    QString variableName;             // record name
};

#endif // QCA_CONNECTION_INFO_H
