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

// String wrapper for QCaObject variant data.

#ifndef QESTRING_H
#define QESTRING_H

#include <QtDebug>
#include <QVariant>
#include <QCaObject.h>
#include <QVector>
#include <QEStringFormatting.h>
#include <QEPluginLibrary_global.h>

class QEPLUGINLIBRARYSHARED_EXPORT QEString : public qcaobject::QCaObject {
    Q_OBJECT

  public:
    QEString( QString recordName, QObject *eventObject, QEStringFormatting *stringFormattingIn, unsigned int variableIndexIn );
    QEString( QString recordName, QObject *eventObject, QEStringFormatting *stringFormattingIn, unsigned int variableIndexIn, UserMessage* userMessageIn );
    bool writeString( const QString &data, QString& message );
    bool writeStringElement( const QString &data, QString& message );
    bool writeString( const QVector<QString> &data, QString& message );

  signals:
    void stringConnectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int &variableIndex );
    void stringChanged( const QString& value, QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp, const unsigned int& variableIndex );
    void stringArrayChanged( const QVector<QString>& values, QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp, const unsigned int& variableIndex );

  public slots:
    void writeString( const QString& data );
    void writeStringElement( const QString& data );
    void writeString( const QVector<QString>& data );

  private:
    void initialise( QEStringFormatting* newStringFormat );
    QEStringFormatting *stringFormat;

  private slots:
    void convertVariant( const QVariant& value, QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp, const unsigned int& variableIndex );
    void forwardConnectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex);
};

#endif // QESTRING_H
