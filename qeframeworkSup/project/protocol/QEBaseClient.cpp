/*  QEBaseClient.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (C) 2018-2024 Australian Synchrotron
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
