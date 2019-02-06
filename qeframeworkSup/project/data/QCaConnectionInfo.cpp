/*  CaConnection.cpp
 *
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
 *  Copyright (c) 2009,2010,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

// CA connection info manager

#include <QCaConnectionInfo.h>

/*
  Construct an empty instance.
  By default the channel and link states are unknown
 */
QCaConnectionInfo::QCaConnectionInfo()
{
    channelState = CHANNEL_UNKNOWN;
    linkState = LINK_UNKNOWN;
}

/*
  Construct an instance given a channel and link state
 */
QCaConnectionInfo::QCaConnectionInfo( const channel_states channelStateIn,
                                      const link_states linkStateIn,
                                      const QString& variableNameIn )
{
    channelState = channelStateIn;
    linkState = linkStateIn;
    variableName = variableNameIn;
}

/*
   Destruct - place holder
 */
QCaConnectionInfo::~QCaConnectionInfo() { }

/*
  Return true if the channel is connected
 */
bool QCaConnectionInfo::isChannelConnected() const
{
    return( channelState == CONNECTED );
}

/*
  Return true if the link is up - deprecated
 */
bool QCaConnectionInfo::isLinkUp() const
{
    return( linkState == LINK_UP );
}

/*
 Return the variable name
 Usefull if same slot is used for reporting multiple QCa connections
 */
QString QCaConnectionInfo::variable() const
{
    return variableName;
}

// end
