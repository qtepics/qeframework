/*  CaConnection.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2025 Australian Synchrotron
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  The EPICS QT Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

// CA connection info manager

#include "QCaConnectionInfo.h"

/*
  Construct an empty instance.
  By default the channel and link states are unknown
 */
QCaConnectionInfo::QCaConnectionInfo()
{
   this->channelState = CHANNEL_UNKNOWN;
   this->variableName = "";
}

/*
  Construct an instance given a channel and link state
 */
QCaConnectionInfo::QCaConnectionInfo( const channel_states channelStateIn,
                                      const QString& variableNameIn )
{
   this->channelState = channelStateIn;
   this->variableName = variableNameIn;
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
   return( this->channelState == CONNECTED );
}

/*
 Return the variable name
 Usefull if same slot is used for reporting multiple QCa connections
 */
QString QCaConnectionInfo::variable() const
{
   return this->variableName;
}

//------------------------------------------------------------------------------
//
static bool registerMetaTypes()
{
   qRegisterMetaType<QCaConnectionInfo> ("QCaConnectionInfo");
   return true;
}

static const bool elaborate = registerMetaTypes();

// end
