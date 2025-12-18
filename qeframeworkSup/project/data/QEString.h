/*  QEString.h
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

// String wrapper for QCaObject variant data.

#ifndef QE_STRING_H
#define QE_STRING_H

#include <QMetaType>
#include <QVariant>
#include <QCaObject.h>
#include <QVector>
#include <QEStringFormatting.h>
#include <QEFrameworkLibraryGlobal.h>

// Structures used in signals to indicate connection and data updates.
//
struct QEStringValueUpdate {
   QString value;
   QCaAlarmInfo alarmInfo;
   QCaDateTime timeStamp;
   unsigned int variableIndex;
};
Q_DECLARE_METATYPE (QEStringValueUpdate)

struct QEStringArrayUpdate {
   QVector<QString> values;
   QCaAlarmInfo alarmInfo;
   QCaDateTime timeStamp;
   unsigned int variableIndex;
};
Q_DECLARE_METATYPE (QEStringArrayUpdate)

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEString :
      public qcaobject::QCaObject
{
   Q_OBJECT

public:
   QEString (QString pvName, QObject* eventObject,
             QEStringFormatting* stringFormattingIn,
             unsigned int variableIndexIn);

   QEString (QString pvName, QObject* eventObject,
             QEStringFormatting* stringFormattingIn,
             unsigned int variableIndexIn,
             UserMessage* userMessageIn);

   bool writeString (const QString &data, QString& message);
   bool writeStringElement (const QString &data, QString& message);
   bool writeString (const QVector<QString> &data, QString& message);

signals:
   // New style
   void valueUpdated (const QEStringValueUpdate& value);
   void arrayUpdated (const QEStringArrayUpdate& array);

   // Old style
   void stringChanged (const QString& value, QCaAlarmInfo& alarmInfo,
                       QCaDateTime& timeStamp, const unsigned int& variableIndex);

   void stringArrayChanged (const QVector<QString>& values,  QCaAlarmInfo& alarmInfo,
                            QCaDateTime& timeStamp, const unsigned int& variableIndex);

public slots:
   void writeString (const QString& data);
   void writeStringElement (const QString& data);
   void writeString (const QVector<QString>& data);

private:
   void initialise (QEStringFormatting* newStringFormat);
   QEStringFormatting* stringFormat;

private slots:
   void convertVariant (const QEVariantUpdate&);
};

#endif // QE_STRING_H
