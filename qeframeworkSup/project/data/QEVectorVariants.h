/*  QEVectorVariants.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2018-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
/// These vector types are registered as QVariant types, and can be set/get
/// like this, using QEInt32Vector as an example:
///
///   QEInt32Vector vector;
///   QVariant variant;
///
///   variant = setValue (vector);
///
///   vector = qvariant_cast<QEInt32Vector>(variant);
///   vector = variant.value<QEInt32Vector>();
///
// The choice of:   typedef QEVector<float> QFloatVector;
// instead of:      class   QEFloatVector : QVector<float> { };
// is currently an arbitary decision and may change.
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
// Note: these QEUint... , not QEUInt... - diffent from epicsTypes.
//
typedef QVector<uint8_t> QEUint8Vector;
Q_DECLARE_METATYPE (QEUint8Vector)

typedef QVector<uint16_t> QEUint16Vector;
Q_DECLARE_METATYPE (QEUint16Vector)

typedef QVector<uint32_t> QEUint32Vector;
Q_DECLARE_METATYPE (QEUint32Vector)

typedef QVector<uint64_t> QEUint64Vector;
Q_DECLARE_METATYPE (QEUint64Vector)

// Support/utilitiy class for owb vector variants.
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

   // Returns a byte array that holds the raw vector data.
   // Note: each element of the vector is not converted to a byte, rather
   // each element becomes 1, 2, 4 or 8 bytes in the byte array.
   // okay indicates success or otherwise.
   //
   static QByteArray getAsByteArray (const QVariant& vector, bool& okay);

   // Returns 1, 2, 4, 8 as appropriate or 0 if vector is not a vector variant.
   //
   static unsigned getElementSize (const QVariant& vector);

   // Conveniance (and quicker) functions to extract a single element from a vector variant.
   // If index out of range or variant parameter is not a vector variant then default value is returned.
   //
   static double   getDoubleValue  (const QVariant& vector, const int index, const double&   defaultValue);
   static long     getIntegerValue (const QVariant& vector, const int index, const long&     defaultValue);
   static QVariant getVariantValue (const QVariant& vector, const int index, const QVariant& defaultValue);

   // Conveniance functions to replace an element of a vector variant.
   // If not a vector variant or index out of range or value cannot be converted
   // to the appropriate element type, the return value is false.
   //
   static bool replaceValue (QVariant& vector, const int index, const QVariant& value);

   // Register these meta types.
   // Note: This function is public for conveniance only, and is invoked by the
   // module itself during program elaboration.
   //
   static bool registerMetaTypes ();

private:
   explicit QEVectorVariants ();
   ~QEVectorVariants ();

   // Maps QVariant's userType int values to OwnTypes.
   //
   static QHash<int, OwnTypes> typeMap;
};

#endif  // QE_VECTOR_VARIANTS_H
