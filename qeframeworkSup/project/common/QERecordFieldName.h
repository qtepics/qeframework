/*  QERecordFieldName.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2016-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
