/*  QENTTableData.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (C) 2018-2019 Australian Synchrotron
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

#include "QENTTableData.h"
#include <iostream>
#include <QEPvaData.h>

#define DEBUG qDebug() << "QENTTableData" << __LINE__ << __FUNCTION__ << "  "


//------------------------------------------------------------------------------
//
QENTTableData::QENTTableData ()
{
   // needed for types to be registrered as meta type.
}

//------------------------------------------------------------------------------
//
QENTTableData::QENTTableData (const QENTTableData & other)
{
   // needed for types to be registrered as meta type.
   *this = other;
}

//------------------------------------------------------------------------------
//
QENTTableData::~QENTTableData ()
{
   // needed for types to be registrered as meta type.
}

//------------------------------------------------------------------------------
//
#ifdef QE_INCLUDE_PV_ACCESS

namespace TR1 = std::tr1;
namespace pvd = epics::pvData;
namespace nmt = epics::nt;

bool QENTTableData::assignFrom (nmt::NTTable::const_shared_pointer table)
{
   static bool verbose = true;

   // Local macro function
   //
#define ASSERT(condition, message) {                                           \
      if (!(condition)) {                                                      \
         if(verbose) {                                                         \
            DEBUG << "QETableData" << __LINE__ << __FUNCTION__ << message;     \
         }                                                                     \
         return false;                                                         \
      }                                                                        \
   }

   // Temp data variables - so that the object is all or nothing.
   //
   QStringList labelsTemp;
   QList < QVariantList > dataTemp;
   pvd::shared_vector<const std::string> labelNames;

   table->getLabels()->getAs (labelNames);
   const int numberLabels = labelNames.size();

   // NOTE: I had to fix this in 7.0.2 - line 274 of nttable.cpp
   //
   //  return pvValue->getStructure()->getFieldNames();
   //  // return pvNTTable->getStructure()->getFieldNames();
   //
   pvd::StringArray colNames = table->getColumnNames ();
   const int numberColumns = colNames.size();

   // We assume the getLabels and the getColumnNames are the same order.
   //
   // DEBUG << "labelsVector.size()" << numberLabels << "colNames.size()"  << numberColumns;
   ASSERT (numberLabels == numberColumns, "labels and columns sizes must match")

   for (int col = 0; col < numberColumns; col++) {

      labelsTemp.append (QString::fromStdString (labelNames [col]));

      const std::string columnName = colNames [col];

      epics::pvData::PVFieldPtr colDataField = table->getColumn (columnName);
      ASSERT (colDataField.get(), "Null column data");

      const pvd::Type fieldType = colDataField->getField ()->getType ();
      ASSERT (fieldType == pvd::scalarArray, "column data field is not a scalarArray");

      pvd::PVScalarArray::const_shared_pointer colDataArray =
            TR1::static_pointer_cast < const pvd::PVScalarArray > (colDataField);

      // MAYBE: The variant could become one of the array vector type variants
      // defined in QEVariants or a QStringList variant.
      //
      const QVariantList colData = QEPvaData::scalarArrayToQVariantList (colDataArray);
      dataTemp.append (colData);
   }

   this->labels = labelsTemp;
   this->data = dataTemp;

   return true;

#undef ASSERT
}

#endif  // QE_INCLUDE_PV_ACCESS

//------------------------------------------------------------------------------
//
void QENTTableData::clear ()
{
   this->labels.clear ();
   this->data.clear ();
}

//------------------------------------------------------------------------------
//
QStringList QENTTableData::getLabels () const
{
   return this->labels;
}

//------------------------------------------------------------------------------
//
int QENTTableData::getRowCount () const
{
   int result = 0;
   for (int j = 0; j < this->data.count (); j++) {
      int rc = this->data.value (j).count ();
      if (result < rc) {
         result = rc;
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
int QENTTableData::getColCount () const
{
   int result = 0;
   // Based on data, not column lables.
   result = this->data.count ();
   return result;
}

//------------------------------------------------------------------------------
//
QVariantList QENTTableData::getRowData (const int row) const
{
   QVariantList result;
   QVariant none;

   const int nr = this->getRowCount ();
   const int nc = this->getColCount ();

   if ((row >= 0) && (row < nr)) {
      for (int col = 0; col < nc; col++) {
         QVariant item;
         item = this->data.value (col).value (row, none);
         result.append (item);
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
QVariantList QENTTableData::getColData (const int col) const
{
   QVariantList result;
   if (col >= 0 && col < this->data.count ()) {
      result = this->data.value (col);
   }
   return result;
}

//------------------------------------------------------------------------------
//
QVariant QENTTableData::getItem (const int row, const int col) const
{
   static const QVariant none;
   QVariant result;

   const int nr = this->getRowCount ();
   const int nc = this->getColCount ();

   if ((row >= 0) && (row < nr)) {
      if (col >= 0 && col < nc) {
         result = this->data.value (col).value (row, none);
      }
   }
   return result;
}


//------------------------------------------------------------------------------
//
QVariant QENTTableData::toVariant () const
{
   QVariant result;
   result.setValue < QENTTableData > (*this);
   return result;
}

//------------------------------------------------------------------------------
//
bool QENTTableData::assignFromVariant (const QVariant & item)
{
   bool result = QENTTableData::isAssignableVariant (item);
   if (result) {
      this->clear ();
      *this = item.value < QENTTableData > ();
   }
   return result;
}

//------------------------------------------------------------------------------
// static
bool QENTTableData::isAssignableVariant (const QVariant & item)
{
   return item.canConvert < QENTTableData > ();
}

//------------------------------------------------------------------------------
// static
bool QENTTableData::registerMetaType ()
{
   qRegisterMetaType < QENTTableData > ();
   return true;
}

// Elaborate on start up.
//
static const bool _elaborate = QENTTableData::registerMetaType ();

//------------------------------------------------------------------------------
//
QDebug operator<< (QDebug dbg, const QENTTableData & table)
{
   const int nr = table.getRowCount ();

   dbg << "Table:" << "\n";
   dbg << "Labels:" << table.getLabels () << "\n";
   dbg << "Data:" << (nr == 0 ? "empty" : "") << "\n";
   for (int row = 0; row < nr; row++) {
      dbg << row << ":" << table.getRowData (row) << "\n";
   }
   return dbg.maybeSpace ();
}

// end
