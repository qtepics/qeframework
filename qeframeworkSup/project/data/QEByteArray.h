/*  QEByteArray.h
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
