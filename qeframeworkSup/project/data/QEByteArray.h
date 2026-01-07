/*  QEByteArray.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2009-2026 Australian Synchrotron
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
#include <QEChannel.h>
#include <QEFrameworkLibraryGlobal.h>

/// This class is essentiialy just a wrapper class for QEChannel.
/// It sets the QE_PRIORITY_LOW low, and the request signals to SIG_BYTEARRAY
/// It also provide a writeByteArray slot method.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEByteArray :
      public QEChannel
{
   Q_OBJECT

public:
   QEByteArray (const QString& pvName, QObject* eventObject, unsigned int variableIndexIn);
   QEByteArray (const QString& pvName, QObject* eventObject, unsigned int variableIndexIn,
                UserMessage* userMessageIn);

// Note: All signals are new provided by QEChannel.

public slots:
   void writeByteArray (const QByteArray& data);
};

#endif // QE_BYTE_ARRAY_H
