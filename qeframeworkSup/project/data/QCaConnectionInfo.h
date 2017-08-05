/*
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
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
 *  Copyright (c) 2009, 2010 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

// Manage CA connection information

#ifndef QCACONNECTIONINFO_H
#define QCACONNECTIONINFO_H

#include <QEFrameworkLibraryGlobal.h>
#include <QString>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QCaConnectionInfo
{
public:
    QCaConnectionInfo();
    QCaConnectionInfo( unsigned short channelStateIn, unsigned short linkStateIn, QString recordName );

    bool isChannelConnected();   // Return true if the channel is connected
    bool isLinkUp();             // Return true if the link is up
    QString variable();          // Return the variable name. Usefull if same slot is used for reporting multiple QCa connections

private:
    unsigned short channelState;    // Channel status
    unsigned short linkState;       // Link status
    QString variableName;           // record name


};

#endif // QCACONNECTIONINFO_H
