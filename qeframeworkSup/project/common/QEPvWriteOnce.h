/*  QEPvWriteOnce.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2022-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_PV_WRITE_ONCE_H
#define QE_PV_WRITE_ONCE_H

#include <QObject>
#include <QCaObject.h>
#include <QEEnums.h>
#include <QEFrameworkLibraryGlobal.h>

/// This classs provides a basic write and forget class.
/// It has been refactored out of QEMenuButton so that it can
/// by used by windowCustomisation.
/// NOTE: The object deletes itself once the write is complete or times out.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPvWriteOnce : public QObject {
   Q_OBJECT
public:
   explicit QEPvWriteOnce (const QString& pvName,
                           const QString& value,
                           const QE::Formats format = QE::Default,
                           QObject* parent = 0);
   ~QEPvWriteOnce();

   // This causes the object to be deleted once the write is complete.
   // Do NOT save references to QEPvWriteOnce objects after calling this method.
   // The macro substitutions apply to both the pvName and the written value.
   //
   bool writeNow (const QString& macroSubstitutions);

   QString pvName;
   QString value;                       // value to write to the variable
   QE::Formats format;                  //

private:
   QString substitutedValue;            // value post substitution
   qcaobject::QCaObject* qca;

   void writeToVariable (qcaobject::QCaObject* qca);

private slots:
   void connectionChanged (QCaConnectionInfo& connectionInfo,
                           const unsigned int& variableIndex);
   void connectionTimeout();
};

#endif // QE_PV_WRITE_ONCE_H
