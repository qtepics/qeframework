/*  QENullClient.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (C) 2023-2024 Australian Synchrotron
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

#include "QENullClient.h"
#include <QDebug>

#define DEBUG qDebug () << "QEPvaClient" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QENullClient::QENullClient (const QString& pvNameIn,
                            QObject* parent) :
   QEBaseClient (QEBaseClient::NullType, pvNameIn, parent)
{ }

//------------------------------------------------------------------------------
//
QENullClient::~QENullClient  ()  { }

//------------------------------------------------------------------------------
//
bool QENullClient::openChannel (const ChannelModesFlags) { return false; }
void QENullClient::closeChannel ()  { }
QVariant QENullClient::getPvData () const { QVariant d; return d; }
bool QENullClient::putPvData (const QVariant&)  { return false; }
bool QENullClient::getIsConnected () const { return false; }
bool QENullClient::dataIsAvailable () const { return false; }
QString QENullClient::getId () const { return ""; }
QString QENullClient::getRemoteAddress() const { return ""; }
QString QENullClient::getEgu () const { return ""; }
int QENullClient::getPrecision() const { return 0; }
unsigned int QENullClient::hostElementCount () const { return 0; }
unsigned int QENullClient::dataElementCount () const { return 0; }
double QENullClient::getDisplayLimitHigh () const { return 0.0; }
double QENullClient::getDisplayLimitLow () const { return 0.0; }
double QENullClient::getHighAlarmLimit () const { return 0.0; }
double QENullClient::getLowAlarmLimit () const { return 0.0; }
double QENullClient::getHighWarningLimit () const { return 0.0; }
double QENullClient::getLowWarningLimit () const { return 0.0; }
double QENullClient::getControlLimitHigh () const { return 0.0; }
double QENullClient::getControlLimitLow () const { return 0.0; }
double QENullClient::getMinStep () const { return 0.0; }
QStringList QENullClient::getEnumerations() const { QStringList d; return d; }
QCaAlarmInfo QENullClient::getAlarmInfo () const { QCaAlarmInfo d; return d; }
QCaDateTime QENullClient::getTimeStamp () const { QCaDateTime d; return d; }
QString QENullClient::getDescription () const { return ""; }

// end
