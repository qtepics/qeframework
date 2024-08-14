/*  QEPvWriteOnce.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2022 Australian Synchrotron
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
 *    Andrew Starritt
 *  Contact details:
 *    andrews@ansto.gov.au
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
