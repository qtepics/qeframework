/*  QEVectorVariants.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at
 *  the Australian Synchrotron.
 *
 *  Copyright (c) 2018-2024 Australian Synchrotron
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

#include "QEVectorVariants.h"
#include <QDebug>
#include <QMetaType>
#include <QEPlatform.h>

#define DEBUG qDebug() << "QEArrayVariants" << __LINE__ << __FUNCTION__ << "  "

// static
QHash<int, QEVectorVariants::OwnTypes> QEVectorVariants::typeMap;

//-----------------------------------------------------------------------------
//
QEVectorVariants::QEVectorVariants () {}    // place holder

//------------------------------------------------------------------------------
//
QEVectorVariants::~QEVectorVariants () {}   // place holder


//------------------------------------------------------------------------------
// static
QEVectorVariants::OwnTypes QEVectorVariants::getOwnType (const QVariant& item)
{
   OwnTypes result = QEVectorVariants::Invalid;

   // Is it a user type, i.e. could it be one of ours ?
   //
   const QMetaType::Type mtype = QEPlatform::metaType (item);
   if (mtype >= QMetaType::User) {
      const int userType = item.userType ();
      result = QEVectorVariants::typeMap.value (userType, QEVectorVariants::Invalid);
   }
   return result;
}

//------------------------------------------------------------------------------
// static
bool QEVectorVariants::isVectorVariant (const QVariant& item)
{
   const QMetaType::Type mtype = QEPlatform::metaType (item);
   if (mtype < QMetaType::User) return false;

   const int userType = item.userType ();

   // All own types *are* vector variants (for now).
   //
   return QEVectorVariants::typeMap.contains (userType);
}

//------------------------------------------------------------------------------
//
#define GET_VECTOR_COUNT(kind) {                                               \
   const kind temp = qvariant_cast<kind>(vector);                              \
   result = temp.count();                                                      \
}


// static
int QEVectorVariants::vectorCount (const QVariant& vector)
{
   int result = 0;

   const OwnTypes type = QEVectorVariants::getOwnType (vector);
   switch (type) {

      case DoubleVector:
         GET_VECTOR_COUNT (QEDoubleVector);
         break;

      case FloatVector:
         GET_VECTOR_COUNT (QEFloatVector);
         break;

      case BoolVector:
         GET_VECTOR_COUNT (QEBoolVector);
         break;

      case Int8Vector:
         GET_VECTOR_COUNT (QEInt8Vector);
         break;

      case Int16Vector:
         GET_VECTOR_COUNT (QEInt16Vector);
         break;

      case Int32Vector:
         GET_VECTOR_COUNT (QEInt32Vector);
         break;

      case Int64Vector:
         GET_VECTOR_COUNT (QEInt64Vector);
         break;

      case Uint8Vector:
         GET_VECTOR_COUNT (QEUint8Vector);
         break;

      case Uint16Vector:
         GET_VECTOR_COUNT (QEUint16Vector);
         break;

      case Uint32Vector:
         GET_VECTOR_COUNT (QEUint32Vector);
         break;

      case Uint64Vector:
         GET_VECTOR_COUNT (QEUint64Vector);
         break;

      case Invalid:
      default:
         result = 0;
         break;
   }

   return result;
}

#undef  GET_VECTOR_COUNT


//------------------------------------------------------------------------------
//
#define VAR_TO_DOUBLE_VECTOR(kind) {                                           \
   const kind temp = qvariant_cast<kind>(vector);                              \
   const int n = temp.count();                                                 \
   result.reserve (n);                                                         \
   for (int j = 0; j < n; j++) {                                               \
      result.append (double (temp [j]));                                       \
   }                                                                           \
   okay = true;                                                                \
}

// static
QVector<double> QEVectorVariants::convertToFloatingVector (const QVariant& vector, bool& okay)
{
   QVector<double> result;
   okay = false;

   const OwnTypes type = QEVectorVariants::getOwnType (vector);

   switch (type) {

      case DoubleVector:
         // direct assignment
         result = qvariant_cast<QEDoubleVector>(vector);
         okay = true;
         break;

      case FloatVector:
         VAR_TO_DOUBLE_VECTOR (QEFloatVector);
         break;

      case BoolVector:
         VAR_TO_DOUBLE_VECTOR (QEBoolVector);
         break;

      case Int8Vector:
         VAR_TO_DOUBLE_VECTOR (QEInt8Vector);
         break;

      case Int16Vector:
         VAR_TO_DOUBLE_VECTOR (QEInt16Vector);
         break;

      case Int32Vector:
         VAR_TO_DOUBLE_VECTOR (QEInt32Vector);
         break;

      case Int64Vector:
         VAR_TO_DOUBLE_VECTOR (QEInt64Vector);
         break;

      case Uint8Vector:
         VAR_TO_DOUBLE_VECTOR (QEUint8Vector);
         break;

      case Uint16Vector:
         VAR_TO_DOUBLE_VECTOR (QEUint16Vector);
         break;

      case Uint32Vector:
         VAR_TO_DOUBLE_VECTOR (QEUint32Vector);
         break;

      case Uint64Vector:
         VAR_TO_DOUBLE_VECTOR (QEUint64Vector);
         break;

      case Invalid:
      default:
         okay = false;
         break;
   }

   return result;
}

#undef VAR_TO_DOUBLE_VECTOR


//------------------------------------------------------------------------------
// TODO Overflow from Double/Float
//
#define VAR_TO_LONG_VECTOR(kind) {                                             \
   const kind temp = qvariant_cast<kind>(vector);                              \
   const int n = temp.count();                                                 \
   result.reserve (n);                                                         \
   for (int j = 0; j < n; j++) {                                               \
      result.append (long (temp [j]));                                         \
   }                                                                           \
   okay = true;                                                                \
}

// static
QVector<long> QEVectorVariants::convertToIntegerVector (const QVariant& vector, bool& okay)
{
   QVector<long> result;
   okay = false;

   const OwnTypes type = QEVectorVariants::getOwnType (vector);

   switch (type) {

      case DoubleVector:
         VAR_TO_LONG_VECTOR (QEDoubleVector);
         break;

      case FloatVector:
         VAR_TO_LONG_VECTOR (QEFloatVector);
         break;

      case BoolVector:
         VAR_TO_LONG_VECTOR (QEBoolVector);
         break;

      case Int8Vector:
         VAR_TO_LONG_VECTOR (QEInt8Vector);
         break;

      case Int16Vector:
         VAR_TO_LONG_VECTOR (QEInt16Vector);
         break;

      case Int32Vector:
         VAR_TO_LONG_VECTOR (QEInt32Vector);
         break;

      case Int64Vector:
         VAR_TO_LONG_VECTOR (QEInt64Vector);
         break;

      case Uint8Vector:
         VAR_TO_LONG_VECTOR (QEUint8Vector);
         break;

      case Uint16Vector:
         VAR_TO_LONG_VECTOR (QEUint16Vector);
         break;

      case Uint32Vector:
         VAR_TO_LONG_VECTOR (QEUint32Vector);
         break;

      case Uint64Vector:
         VAR_TO_LONG_VECTOR (QEUint64Vector);
         break;

      case Invalid:
      default:
         okay = false;
         break;
   }

   return result;
}

#undef VAR_TO_LONG_VECTOR


//------------------------------------------------------------------------------
// convert is type name for cast or a convert function
//
#define VAR_TO_VAR_LIST(kind,convert) {                                        \
   const kind temp = qvariant_cast<kind>(vector);                              \
   const int n = temp.count();                                                 \
   result.reserve (n);                                                         \
   for (int j = 0; j < n; j++) {                                               \
      result.append (QVariant (convert(temp [j])));                            \
   }                                                                           \
   okay = true;                                                                \
}


// static
QVariantList QEVectorVariants::convertToVariantList (const QVariant& vector, bool& okay)
{
   QVariantList result;
   okay = false;

   const OwnTypes type = QEVectorVariants::getOwnType (vector);

   switch (type) {

      case DoubleVector:
         VAR_TO_VAR_LIST (QEDoubleVector, double);
         break;

      case FloatVector:
         VAR_TO_VAR_LIST (QEFloatVector, float);
         break;

      case BoolVector:
         VAR_TO_VAR_LIST (QEBoolVector, bool);
         break;

      case Int8Vector:
         VAR_TO_VAR_LIST (QEInt8Vector, char);
         break;

      case Int16Vector:
         VAR_TO_VAR_LIST (QEInt16Vector, int);
         break;

      case Int32Vector:
         VAR_TO_VAR_LIST (QEInt32Vector, int);
         break;

      case Int64Vector:
         VAR_TO_VAR_LIST (QEInt64Vector, qlonglong);
         break;

      case Uint8Vector:
         VAR_TO_VAR_LIST (QEUint8Vector, uint);
         break;

      case Uint16Vector:
         VAR_TO_VAR_LIST (QEUint16Vector, uint);
         break;

      case Uint32Vector:
         VAR_TO_VAR_LIST (QEUint32Vector, uint);
         break;

      case Uint64Vector:
         VAR_TO_VAR_LIST (QEUint64Vector, qulonglong);
         break;

      case Invalid:
      default:
         okay = false;
         break;
   }

   return result;
}

#undef VAR_TO_VAR_LIST


//------------------------------------------------------------------------------
// static
double   QEVectorVariants::getDoubleValue  (const QVariant& vector, const int index,
                                      const double&   defaultValue)
{
   const OwnTypes type = QEVectorVariants::getOwnType (vector);
   double result = defaultValue;

#define GET_ELEMENT(kind) {                                                    \
   const kind temp = qvariant_cast<kind>(vector);                              \
   result = double (temp.value(index, defaultValue));                          \
}


   switch (type) {

      case DoubleVector:
         GET_ELEMENT (QEDoubleVector);
         break;

      case FloatVector:
         GET_ELEMENT (QEFloatVector);
         break;

      case BoolVector:
         GET_ELEMENT (QEBoolVector);
         break;

      case Int8Vector:
         GET_ELEMENT (QEInt8Vector);
         break;

      case Int16Vector:
         GET_ELEMENT (QEInt16Vector);
         break;

      case Int32Vector:
         GET_ELEMENT (QEInt32Vector);
         break;

      case Int64Vector:
         GET_ELEMENT (QEInt64Vector);
         break;

      case Uint8Vector:
         GET_ELEMENT (QEUint8Vector);
         break;

      case Uint16Vector:
         GET_ELEMENT (QEUint16Vector);
         break;

      case Uint32Vector:
         GET_ELEMENT (QEUint32Vector);
         break;

      case Uint64Vector:
         GET_ELEMENT (QEUint64Vector);
         break;

      case Invalid:
      default:
         break;
   }

   return result;

#undef GET_ELEMENT
}


//------------------------------------------------------------------------------
// static
long     QEVectorVariants::getIntegerValue (const QVariant& vector, const int index,
                                      const long&     defaultValue)
{
   const OwnTypes type = QEVectorVariants::getOwnType (vector);
   long result = defaultValue;


#define GET_ELEMENT(kind) {                                                    \
   const kind temp = qvariant_cast<kind>(vector);                              \
   result = long (temp.value (index, defaultValue));                           \
}


   switch (type) {

      case DoubleVector:
         GET_ELEMENT (QEDoubleVector);
         break;

      case FloatVector:
         GET_ELEMENT (QEFloatVector);
         break;

      case BoolVector:
         GET_ELEMENT (QEBoolVector);
         break;

      case Int8Vector:
         GET_ELEMENT (QEInt8Vector);
         break;

      case Int16Vector:
         GET_ELEMENT (QEInt16Vector);
         break;

      case Int32Vector:
         GET_ELEMENT (QEInt32Vector);
         break;

      case Int64Vector:
         GET_ELEMENT (QEInt64Vector);
         break;

      case Uint8Vector:
         GET_ELEMENT (QEUint8Vector);
         break;

      case Uint16Vector:
         GET_ELEMENT (QEUint16Vector);
         break;

      case Uint32Vector:
         GET_ELEMENT (QEUint32Vector);
         break;

      case Uint64Vector:
         GET_ELEMENT (QEUint64Vector);
         break;

      case Invalid:
      default:
         break;
   }

   return result;

#undef GET_ELEMENT
}

//------------------------------------------------------------------------------
// static
QVariant QEVectorVariants::getVariantValue (const QVariant& vector, const int index,
                                      const QVariant& defaultValue)
{
   const OwnTypes type = QEVectorVariants::getOwnType (vector);
   QVariant result = defaultValue;


#define GET_ELEMENT(kind, convert) {                                           \
   const kind temp = qvariant_cast<kind>(vector);                              \
   if ((index >= 0) && (index < temp.size()))                                  \
      result = QVariant (convert (temp.value (index)));                        \
}


   switch (type) {

      case DoubleVector:
         GET_ELEMENT (QEDoubleVector, double);
         break;

      case FloatVector:
         GET_ELEMENT (QEFloatVector, float);
         break;

      case BoolVector:
         GET_ELEMENT (QEBoolVector, bool);
         break;

      case Int8Vector:
         GET_ELEMENT (QEInt8Vector, char);
         break;

      case Int16Vector:
         GET_ELEMENT (QEInt16Vector, int);
         break;

      case Int32Vector:
         GET_ELEMENT (QEInt32Vector, int);
         break;

      case Int64Vector:
         GET_ELEMENT (QEInt64Vector, qlonglong);
         break;

      case Uint8Vector:
         GET_ELEMENT (QEUint8Vector, uint);
         break;

      case Uint16Vector:
         GET_ELEMENT (QEUint16Vector, uint);
         break;

      case Uint32Vector:
         GET_ELEMENT (QEUint32Vector, uint);
         break;

      case Uint64Vector:
         GET_ELEMENT (QEUint64Vector, qulonglong);
         break;

      case Invalid:
      default:
         break;
   }
#undef GET_ELEMENT

   return result;
}

//------------------------------------------------------------------------------
// Register own meta types and create type to OwnTypes map.
// static
bool QEVectorVariants::registerMetaTypes ()
{
   int t;

   t = qRegisterMetaType<QEDoubleVector>();
   QEVectorVariants::typeMap.insert (t, DoubleVector);

   t = qRegisterMetaType<QEFloatVector>();
   QEVectorVariants::typeMap.insert (t, FloatVector);

   t = qRegisterMetaType<QEBoolVector>();
   QEVectorVariants::typeMap.insert (t, BoolVector);

   t = qRegisterMetaType<QEInt8Vector>();
   QEVectorVariants::typeMap.insert (t, Int8Vector);

   t = qRegisterMetaType<QEInt16Vector>();
   QEVectorVariants::typeMap.insert (t, Int16Vector);

   t = qRegisterMetaType<QEInt32Vector>();
   QEVectorVariants::typeMap.insert (t, Int32Vector);

   t = qRegisterMetaType<QEInt64Vector>();
   QEVectorVariants::typeMap.insert (t, Int64Vector);

   t = qRegisterMetaType<QEUint8Vector>();
   QEVectorVariants::typeMap.insert (t, Uint8Vector);

   t = qRegisterMetaType<QEUint16Vector>();
   QEVectorVariants::typeMap.insert (t, Uint16Vector);

   t = qRegisterMetaType<QEUint32Vector>();
   QEVectorVariants::typeMap.insert (t, Uint32Vector);

   t = qRegisterMetaType<QEUint64Vector>();
   QEVectorVariants::typeMap.insert (t, Uint64Vector);

   return true;
}

// Elaborate on start up.
//
static const bool _elaborate = QEVectorVariants::registerMetaTypes ();

// end

