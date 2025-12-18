/*  QCaConnectionInfo.cpp
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
