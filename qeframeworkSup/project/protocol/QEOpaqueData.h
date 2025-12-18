/*  QEOpaqueData.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2019-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_OPAQUE_DATA_H
#define QE_OPAQUE_DATA_H

#include <QDebug>
#include <QList>
#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QEFrameworkLibraryGlobal.h>
#include <QEPvaCheck.h>

#ifdef QE_INCLUDE_PV_ACCESS
#include <pv/pvData.h>
#endif

/// Defines a data class type, specifically to support the unknown PV Access
/// data types such an none normative types.
/// This type is registered as a QVariant type, and can be set/get like this:
///
///   QEOpaqueData opaque;
///   QVariant var;
///
///   var.setValue (opaque);       or
///   var = opaque.toVariant();          -- defined in this module
///
///   opaque = var.value<QEOpaqueData>();
///   opaque = qvariant_cast<QEOpaqueData>(var);  or
///   opaque.assignFromVariant (var);    -- defined in this module
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEOpaqueData {
public:
   // explicit is a no-no here.
   QEOpaqueData ();
   QEOpaqueData (const QEOpaqueData& other);
   ~QEOpaqueData ();

   QEOpaqueData& operator=(const QEOpaqueData& other);

#ifdef QE_INCLUDE_PV_ACCESS
   // Note: we can read any type for now.
   //
   bool assignFrom (const epics::pvData::StructureConstPtr ptr);
#endif

   // Clear all opaque data - added for completeness
   //
   void clear ();

   // Converstion to QVariant
   //
   QVariant toVariant () const;

   // Returns true if item is a QEOpaqueData variant, i.e. can be used
   // as a parameter to the assignFromVariant function.
   //
   static bool isAssignableVariant (const QVariant& item);

   // Returns true if the assignment from item is successful.
   //
   bool assignFromVariant (const QVariant& item);

   // Register the QEOpaqueData meta type.
   // Note: This function is public for conveniance only, and is invoked by
   // the module itself during program elaboration.
   //
   static bool registerMetaType ();

private:
   // no data per se.
};

// allows qDebug() << QEOpaqueData object.
//
QDebug QE_FRAMEWORK_LIBRARY_SHARED_EXPORT
operator<<(QDebug dbg, const QEOpaqueData& opaque);

Q_DECLARE_METATYPE (QEOpaqueData)

#endif  // QE_OPAQUE_DATA_H
