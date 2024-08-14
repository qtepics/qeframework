/*  QENTTableData.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (C) 2018-2024 Australian Synchrotron
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
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_NT_TABLE_DATA_H
#define QE_NT_TABLE_DATA_H

#include <QDebug>
#include <QList>
#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QEFrameworkLibraryGlobal.h>
#include <QEPvaCheck.h>

#ifdef QE_INCLUDE_PV_ACCESS
#include <pv/nttable.h>
#include <pv/pvData.h>
#endif

/// Defines a table data class type, specifically to support the epics:nt/NTTable type.
/// This type is registered as a QVariant type, and can be set/get like this:
///
///   QETableData tab;
///   QVariant var;
///
///   var.setValue <QENTTableData> (tab);       or
///   var = tab.toVariant();
///
///   tab = var.value<QENTTableData>();
///   tab = qvariant_cast<QENTTableData>(var);  or
///   tab.assignFromVariant (var);
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QENTTableData {
public:
   // explicit is a no-no here.
   QENTTableData ();
   QENTTableData (const QENTTableData& other);
   ~QENTTableData ();

   QENTTableData& operator=(const QENTTableData& other);

#ifdef QE_INCLUDE_PV_ACCESS
   // Note: we can only read NTTable types for now.
   //
   bool assignFrom (epics::nt::NTTable::const_shared_pointer item);
#endif

   // Clear all table data.
   //
   void clear ();

   // Provides access to the table data fields.
   //
   QStringList getLabels () const;   // get table lables.

   int getRowCount () const;         // get number of rows (the max of all columns)
   int getColCount () const;         // get number of columns

   // Access data as rows, columns or individual element.
   //
   QVariantList getRowData (const int row) const;
   QVariantList getColData (const int col) const;
   QVariant getItem (const int row, const int col) const;

   // Converstion to QVariant
   //
   QVariant toVariant () const;

   // Returns true if item is a QETableData variant, i.e. can be used
   // as a parameter to the assignFromVariant function.
   //
   static bool isAssignableVariant (const QVariant& item);

   // Returns true if the assignment from item is successful.
   //
   bool assignFromVariant (const QVariant& item);

   // Register the QETableData meta type.
   // Note: This function is public for conveniance only, and is invoked by
   // the module itself during program elaboration.
   //
   static bool registerMetaType ();

private:
   QStringList labels;

   // Table data is a list of column data
   // Each column is a (variant) list of row elements.
   // The data is column major to reflect the epics:nt/NTTable type.
   //
   QList<QVariantList> data;
};

// allows qDebug() << QETableData object.
//
QDebug operator<<(QDebug dbg, const QENTTableData &table);

Q_DECLARE_METATYPE (QENTTableData)

#endif  // QE_NT_TABLE_DATA_H
