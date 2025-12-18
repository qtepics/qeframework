/*  QEBaseClient.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2018-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEBaseClient.h"
#include <QDebug>

#define DEBUG qDebug () << "QEBaseClient" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QEBaseClient::QEBaseClient (const Type type, const QString& pvName,
                            QObject* parent) :
   QObject(parent),
   clientType(type),
   clientPvName(pvName)
{
   this->userMessage = NULL;
}

//------------------------------------------------------------------------------
//
QEBaseClient::~QEBaseClient () {}

//------------------------------------------------------------------------------
//
void QEBaseClient::setUserMessage (UserMessage* userMessageIn)
{
   this->userMessage = userMessageIn;
}

//------------------------------------------------------------------------------
//
UserMessage* QEBaseClient::getUserMessage () const
{
   return this->userMessage;
}

//------------------------------------------------------------------------------
//
QEBaseClient::Type QEBaseClient::getType () const
{
   return this->clientType;
}

//------------------------------------------------------------------------------
//
QString QEBaseClient::getPvName () const
{
   return this->clientPvName;
}

// end
