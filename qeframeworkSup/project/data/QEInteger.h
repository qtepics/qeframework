/*  QEInteger.h
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
