/*  QENullClient.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2023-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
bool QENullClient::getReadAccess() const { return false; }
bool QENullClient::getWriteAccess() const { return false; }

// end
