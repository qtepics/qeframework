/*  QERecordFieldName.h
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
 *  Copyright (c) 2016,2018 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

/* The QERecordFieldName class refectored out of QEPvPropertiesUtilities
 */

#ifndef QE_RECORD_FIELD_NAME_H
#define QE_RECORD_FIELD_NAME_H

#include <QString>
#include <QEFrameworkLibraryGlobal.h>

//------------------------------------------------------------------------------
// All functions are static.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QERecordFieldName {
public:
   // Converts PV name to record name, e.g.:
   //
   // SR11BCM01:CURRENT_MONITOR.PREC => SR11BCM01:CURRENT_MONITOR
   // SR11BCM01:CURRENT_MONITOR.VAL  => SR11BCM01:CURRENT_MONITOR
   // SR11BCM01:CURRENT_MONITOR      => SR11BCM01:CURRENT_MONITOR
   //
   static QString recordName (const QString & pvName);

   // Converts PV name to field name, e.g.:
   //
   // SR11BCM01:CURRENT_MONITOR.PREC => PREC
   // SR11BCM01:CURRENT_MONITOR.VAL  => VAL
   // SR11BCM01:CURRENT_MONITOR      => VAL (it's the default)
   //
   static QString fieldName (const QString & pvName);

   // From field PV name
   // (SR11BCM01:CURRENT_MONITOR.PREC, EGU) => SR11BCM01:CURRENT_MONITOR.EGU
   // (SR11BCM01:CURRENT_MONITOR,      EGU) => SR11BCM01:CURRENT_MONITOR.EGU
   //
   static QString fieldPvName (const QString & pvName, const QString & field);

   // Form pseudo field record type PV name.
   // SR11BCM01:CURRENT_MONITOR.PREC => SR11BCM01:CURRENT_MONITOR.RTYP
   //
   static QString rtypePvName (const QString & pvName);

   // This function indicates if the given string is a valid PV name,
   // not ony in the pureset EPICS sense, but that it also conforms to
   // the naming standard.
   //
   static bool pvNameIsValid (const QString & pvName);

   // Remove any qualifiers (e.g. " CPP", " MSI") prior to validating the PV
   // name. The modified PV name is returned in the pvName parameter if valid,
   // otherwise the pvName parameter is cleared.
   //
   static bool extractPvName (const QString & item, QString & pvName);
};

# endif  // QE_RECORD_FIELD_NAME_H
