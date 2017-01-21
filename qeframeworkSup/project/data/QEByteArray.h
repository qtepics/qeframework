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

#ifndef QEBYTEARRAY_H
#define QEBYTEARRAY_H

#include <QtDebug>
#include <QVariant>
#include <QCaObject.h>
#include <QEPluginLibrary_global.h>

class QEPLUGINLIBRARYSHARED_EXPORT QEByteArray : public qcaobject::QCaObject {
    Q_OBJECT

  public:
    QEByteArray( QString recordName, QObject *eventObject, unsigned int variableIndexIn );
    QEByteArray( QString recordName, QObject *eventObject, unsigned int variableIndexIn, UserMessage* userMessageIn );

  signals:
    void byteArrayConnectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex );
    void byteArrayChanged( const QByteArray& value, unsigned long dataSize, QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp, const unsigned int& variableIndex );

  public slots:
    void writeByteArray( const QByteArray& data );

  private:
    void initialise();

  private slots:
    void forwardDataChanged( const QByteArray &value, unsigned long dataSize, QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp, const unsigned int& variableIndex );
    void forwardConnectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex);
};

#endif // QEBYTEARRAY_H
