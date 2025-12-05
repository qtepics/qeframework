/*  QEInteger.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2025 Australian Synchrotron
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License.
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

// Integer specific wrapper for QCaObject.

#ifndef QE_INTEGER_H
#define QE_INTEGER_H

#include <QMetaType>
#include <QVariant>
#include <QCaObject.h>
#include <QEIntegerFormatting.h>
#include <QEFrameworkLibraryGlobal.h>

// Structures used in signals to indicate connection and data updates.
//
struct QEIntegerValueUpdate {
   long value;
   QCaAlarmInfo alarmInfo;
   QCaDateTime timeStamp;
   unsigned int variableIndex;
};
Q_DECLARE_METATYPE (QEIntegerValueUpdate)

struct QEIntegerArrayUpdate {
   QVector<long> values;
   QCaAlarmInfo alarmInfo;
   QCaDateTime timeStamp;
   unsigned int variableIndex;
};
Q_DECLARE_METATYPE (QEIntegerArrayUpdate)

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEInteger :
      public qcaobject::QCaObject
{
   Q_OBJECT

public:
   QEInteger (QString pvName, QObject* eventObject,
              QEIntegerFormatting* integerFormattingIn,
              unsigned int variableIndexIn);

   QEInteger (QString pvName, QObject* eventObject,
              QEIntegerFormatting* integerFormattingIn,
              unsigned int variableIndexIn,
              UserMessage* userMessageIn);

signals:
   // New style
   void valueUpdated (const QEIntegerValueUpdate& value);
   void arrayUpdated (const QEIntegerArrayUpdate& array);

   // Old style
   void integerChanged (const long& value, QCaAlarmInfo& alarmInfo,
                        QCaDateTime& timeStamp, const unsigned int& variableIndex);
   void integerArrayChanged (const QVector<long>& values, QCaAlarmInfo& alarmInfo,
                             QCaDateTime& timeStamp, const unsigned int& variableIndex);

public slots:
   void writeInteger (const long &data);
   void writeIntegerElement (const long &data);
   void writeInteger (const QVector<long> &data);

private:
   void initialise (QEIntegerFormatting* integerFormattingIn);
   QEIntegerFormatting* integerFormat;

private slots:
   void convertVariant (const QEVariantUpdate&);
};

#endif // QE_INTEGER_H
