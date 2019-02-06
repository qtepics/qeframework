/*  QEVectorVariants.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2018 Australian Synchrotron
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

#ifndef QE_VECTOR_VARIANTS_H
#define QE_VECTOR_VARIANTS_H

#include <stdint.h>
#include <QMetaType>
#include <QHash>
#include <QVector>
#include <QVariant>
#include <QVariantList>
#include <QEFrameworkLibraryGlobal.h>

/// Defines QE's own variant types.  This includes Vector types for all int/uint
/// sizes, float/double and boolean. We use QStringList instead of defining a
/// QVector<QString>. For scalars we just use the existing int/uint types and
/// don't worry about the size of small integer types being rounded up.
///
/// While these variant types and the QEVariants were introduced to support the use
/// of PV Access, nothing in this class depends on artefacts from pvAccess/pvData.
///
// The choice of:   typedef QEVector<float> QFloatVector;
// instead of:      class   QEFloatVector : QVector<float> { };
// is currently an arbitary descision and may change.
//
typedef QVector<double> QEDoubleVector;
Q_DECLARE_METATYPE (QEDoubleVector)

typedef QVector<float> QEFloatVector;
Q_DECLARE_METATYPE (QEFloatVector)

typedef QVector<bool> QEBoolVector;
Q_DECLARE_METATYPE (QEBoolVector)

// Signed
//
typedef QVector<int8_t> QEInt8Vector;
Q_DECLARE_METATYPE (QEInt8Vector)

typedef QVector<int16_t> QEInt16Vector;
Q_DECLARE_METATYPE (QEInt16Vector)

typedef QVector<int32_t> QEInt32Vector;
Q_DECLARE_METATYPE (QEInt32Vector)

typedef QVector<int64_t> QEInt64Vector;
Q_DECLARE_METATYPE (QEInt64Vector)

// UnSigned
//
typedef QVector<uint8_t> QEUint8Vector;
Q_DECLARE_METATYPE (QEUint8Vector)

typedef QVector<uint16_t> QEUint16Vector;
Q_DECLARE_METATYPE (QEUint16Vector)

typedef QVector<uint32_t> QEUint32Vector;
Q_DECLARE_METATYPE (QEUint32Vector)

typedef QVector<uint64_t> QEUint64Vector;
Q_DECLARE_METATYPE (QEUint64Vector)

//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEVectorVariants {
public:
   // Own QEVariants type identifiers.
   //
   enum OwnTypes {
      Invalid,
      DoubleVector,
      FloatVector,
      BoolVector,
      Int8Vector,
      Int16Vector,
      Int32Vector,
      Int64Vector,
      Uint8Vector,
      Uint16Vector,
      Uint32Vector,
      Uint64Vector,
   };

   // Returns own type. If not one of the types defined here then returns Invalid.
   //
   static OwnTypes getOwnType (const QVariant& item);

   // Returns true if item is one of our own vector variants.
   //
   static bool isVectorVariant (const QVariant& item);

   // If vector variant, returns the number of elements, otherwise returns 0
   //
   static int vectorCount(const QVariant& vector);

   // Converts a vector variant, e.g. QEInt16Vector to a double QVector.
   // okay indicates success or otherwise.
   //
   static QVector<double> convertToFloatingVector (const QVariant& vector, bool& okay);

   // Converts a vector variant, e.g. QEInt16Vector to a long QVector.
   // okay indicates success or otherwise.
   //
   static QVector<long> convertToIntegerVector (const QVariant& vector, bool& okay);

   // Converts a vector variant, e.g. QEInt16Vector to a QVariantList.
   // okay indicates success or otherwise.
   //
   static QVariantList convertToVariantList (const QVariant& vector, bool& okay);

   // Conveniance (and quicker) functions to extract a single element from a vector variant.
   // If index out of range of variant parameter is not a vector variant then default value is returned.
   //
   static double   getDoubleValue  (const QVariant& vector, const int index, const double&   defaultValue);
   static long     getIntegerValue (const QVariant& vector, const int index, const long&     defaultValue);
   static QVariant getVariantValue (const QVariant& vector, const int index, const QVariant& defaultValue);

   // Register these meta types.
   // Note: This function is public for conveniance only, and is invoked by the
   // module itself during program elaboration.
   //
   static bool registerMetaTypes ();

private:
   explicit QEVectorVariants ();
   ~QEVectorVariants ();

   static QHash<int, OwnTypes> typeMap;  // Maps userType int value to OwnTypes.
};

#endif  // QE_VECTOR_VARIANTS_H
