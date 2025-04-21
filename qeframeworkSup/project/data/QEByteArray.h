/*  QEByteArray.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2024 Australian Synchrotron
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
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QE_BYTE_ARRAY_H
#define QE_BYTE_ARRAY_H

#include <QtDebug>
#include <QVariant>
#include <QCaObject.h>
#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEByteArray : public qcaobject::QCaObject {
   Q_OBJECT

public:
   QEByteArray (QString pvName, QObject* eventObject, unsigned int variableIndexIn);
   QEByteArray (QString pvName, QObject* eventObject, unsigned int variableIndexIn,
                UserMessage* userMessageIn);

signals:
   void byteArrayChanged (const QByteArray& value, unsigned long dataSize,
                          QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp,
                          const unsigned int& variableIndex);

public slots:
   void writeByteArray (const QByteArray& data);

private:
   void initialise();

private slots:
   void forwardDataChanged (const QByteArray &value, unsigned long dataSize,
                            QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp,
                            const unsigned int& variableIndex);
};

#endif // QE_BYTE_ARRAY_H
