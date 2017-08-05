/*  QEInteger.h
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
 *  Copyright (c) 2009, 2010, 2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

// Integer specific wrapper for QCaObject.

#ifndef QEINTEGER_H
#define QEINTEGER_H

#include <QtDebug>
#include <QVariant>
#include <QCaObject.h>
#include <QEIntegerFormatting.h>
#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEInteger : public qcaobject::QCaObject {
    Q_OBJECT

  public:
    QEInteger( QString recordName, QObject *eventObject, QEIntegerFormatting *integerFormattingIn, unsigned int variableIndexIn );
    QEInteger( QString recordName, QObject *eventObject, QEIntegerFormatting *integerFormattingIn, unsigned int variableIndexIn, UserMessage* userMessageIn );

  signals:
    void integerConnectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex );
    void integerChanged( const long& value, QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp, const unsigned int& variableIndex );
    void integerArrayChanged( const QVector<long>& values, QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp, const unsigned int& variableIndex );

  public slots:
    void writeInteger( const long &data );
    void writeIntegerElement( const long &data );
    void writeInteger( const QVector<long> &data );

  private:
    void initialise( QEIntegerFormatting *integerFormattingIn );
    QEIntegerFormatting *integerFormat;

  private slots:
    void convertVariant( const QVariant &value, QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp, const unsigned int& variableIndex );
    void forwardConnectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex);
};

#endif // QEINTEGER_H
