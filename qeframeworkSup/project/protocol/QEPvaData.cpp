/*  QEPvaData.cpp
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

#include "QEPvaData.h"

// Must at least include include own header
#ifdef QE_INCLUDE_PV_ACCESS

#include <QDebug>
#include <QMetaType>
#include <pv/nt.h>

#include <pv/ntscalar.h>
#include <pv/ntscalarArray.h>
#include <pv/ntenum.h>
#include <pv/nttable.h>
#include <pv/ntndarray.h>

#include <pv/pvType.h>
#include <pv/pvIntrospect.h>
#include <QEPlatform.h>
#include <QEVectorVariants.h>
#include <QENTTableData.h>
#include <QENTNDArrayData.h>
#include <QEOpaqueData.h>


#define DEBUG qDebug() << "QEPvaData" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// Essentially != however caters for NaN values.
// By defn, NaN is != NaN. which is not what we want.
//
static bool hasChanged (const double self, const double other)
{
   // If both NaN then considered unchanged, i.e. quazi equal.
   // We don't seem to need to worry about infinites.
   //
   if (QEPlatform::isNaN (self) && QEPlatform::isNaN (other)) return false;

   return self != other;
}

//------------------------------------------------------------------------------
//
QEPvaData::QEPvaData () { }   // place holder

//------------------------------------------------------------------------------
//
QEPvaData::~QEPvaData () { }  // place holder

//------------------------------------------------------------------------------
//
QEPvaData::Enumerated::Enumerated ()
{
   this->isDefined = false;
   this->index = 0;
   this->choices.clear ();
}

QEPvaData::Enumerated::~Enumerated () { } // place holder

//------------------------------------------------------------------------------
//
void QEPvaData::Enumerated::assign (const Enumerated& other, bool& isMetaUpdate)
{
   if (other.isDefined) {
      isMetaUpdate = isMetaUpdate ||
                     (!this->isDefined) ||
                     (this->choices != other.choices);
      *this = other;
   } else {
      this->isDefined = false;
      this->index = 0;
      this->choices.clear();
   }
}

//------------------------------------------------------------------------------
//
QEPvaData::Alarm::Alarm ()
{
   this->isDefined = false;
   this->severity = 0;
   this->status = 0;
   this->message = "";
}

QEPvaData::Alarm::~Alarm () { } // place holder

//------------------------------------------------------------------------------
//
void QEPvaData::Alarm::assign (const Alarm& other)
{
   if (other.isDefined) {
      *this = other;
   } else {
      this->isDefined = false;
      this->severity = 0;
      this->status = 0;
      this->message = "";
   }
}

//------------------------------------------------------------------------------
//
QEPvaData::TimeStamp::TimeStamp ()
{
   this->isDefined = false;
   this->secondsPastEpoch = 0;
   this->nanoseconds = 0;
   this->userTag = 0;
}

QEPvaData::TimeStamp::~TimeStamp () { } // place holder

//------------------------------------------------------------------------------
//
void QEPvaData::TimeStamp::assign (const TimeStamp& other)
{
   if (other.isDefined) {
      *this = other;
   } else {
      this->isDefined = false;
      this->secondsPastEpoch = 0;
      this->nanoseconds = 0;
      this->userTag = 0;
   }
}

//------------------------------------------------------------------------------
//
QEPvaData::Display::Display ()
{
   this->isDefined = false;
   this->limitLow = 0.0;
   this->limitHigh = 0.0;
   this->description = "";
   this->units = "";
   this->precision = 0;
}

QEPvaData::Display::~Display () { } // place holder

//------------------------------------------------------------------------------
//
void QEPvaData::Display::assign (const Display& other, bool& isMetaUpdate)
{
   if (other.isDefined) {
      isMetaUpdate = isMetaUpdate ||
                     (!this->isDefined) ||
                     hasChanged (this->limitLow, other.limitLow) ||
                     hasChanged (this->limitHigh, other.limitHigh) ||
                     (this->description != other.description) ||
                     (this->units != other.units) ||
                     (this->precision != other.precision);
      *this = other;
   } else {
      this->isDefined = false;
      this->limitLow = 0.0;
      this->limitHigh = 0.0;
      this->description = "";
      this->units = "";
      this->precision = 0;
   }
}

//------------------------------------------------------------------------------
//
QEPvaData::Control::Control ()
{
   this->isDefined = false;
   this->limitLow = 0.0;
   this->limitHigh = 0.0;
   this->minStep = 0.0;
}

QEPvaData::Control::~Control () { } // place holder

//------------------------------------------------------------------------------
//
void QEPvaData::Control::assign (const Control& other, bool& isMetaUpdate)
{
   if (other.isDefined) {
      isMetaUpdate = isMetaUpdate ||
                     (!this->isDefined) ||
                     hasChanged (this->limitLow, other.limitLow) ||
                     hasChanged (this->limitHigh, other.limitHigh);
      *this = other;
   } else {
      this->isDefined = false;
      this->limitLow = 0.0;
      this->limitHigh = 0.0;
      this->minStep = 0.0;
   }
}

//------------------------------------------------------------------------------
//
QEPvaData::ValueAlarm::ValueAlarm ()
{
   this->isDefined = false;
   this->active = false;
   this->lowAlarmLimit = 0.0;
   this->lowWarningLimit = 0.0;
   this->highWarningLimit = 0.0;
   this->highAlarmLimit = 0.0;
   this->lowAlarmSeverity = 0;
   this->lowWarningSeverity = 0;
   this->highWarningSeverity = 0;
   this->highAlarmSeverity = 0;
   this->hysteresis = 0;
}

QEPvaData::ValueAlarm::~ValueAlarm () { } // place holder

//------------------------------------------------------------------------------
//
void QEPvaData::ValueAlarm::assign (const ValueAlarm& other, bool& isMetaUpdate)
{
   if (other.isDefined) {
      isMetaUpdate = isMetaUpdate ||
                     (!this->isDefined) ||
                     hasChanged (this->lowAlarmLimit, other.lowAlarmLimit) ||
                     hasChanged (this->lowWarningLimit, other.lowWarningLimit) ||
                     hasChanged (this->highWarningLimit, other.highWarningLimit) ||
                     hasChanged (this->highAlarmLimit, other.highAlarmLimit) ||
                     (this->lowAlarmSeverity!= other.lowAlarmSeverity) ||
                     (this->lowWarningSeverity != other.lowWarningSeverity) ||
                     (this->highWarningSeverity != other.highWarningSeverity) ||
                     (this->highAlarmSeverity != other.highAlarmSeverity);
      *this = other;
   } else {
      this->isDefined = false;
      this->active = false;
      this->lowAlarmLimit = 0.0;
      this->lowWarningLimit = 0.0;
      this->highWarningLimit = 0.0;
      this->highAlarmLimit = 0.0;
      this->lowAlarmSeverity = 0;
      this->lowWarningSeverity = 0;
      this->highWarningSeverity = 0;
      this->highAlarmSeverity = 0;
      this->hysteresis = 0;
   }
}

//==============================================================================
// QEPvaData
//==============================================================================
//
namespace TR1 = std::tr1;
namespace pvd = epics::pvData;
namespace nmt = epics::nt;

// Expected type identifiers - less version number
//
static const QString scalarTypeId = "epics:nt/NTScalar:";              //
static const QString arrayTypeId  = "epics:nt/NTScalarArray:";         //
static const QString enumTypeId   = "epics:nt/NTEnum:";                //
static const QString tableTypeId  = "epics:nt/NTTable:";               //
static const QString imageTypeId  = "epics:nt/NTNDArray:";             //

static const QVariant nullVariant;

#define ASSERT(condition, message) {                                           \
      if (!(condition)) {                                                      \
         DEBUG << "QEPvaData" << __LINE__ << __FUNCTION__ << message;          \
         return false;                                                         \
      }                                                                        \
   }


//------------------------------------------------------------------------------
// static
// Extract the value part of the PV. The alarm info and timestamp info are
// extracted elsewhere.
//
bool QEPvaData::extractValue (PVStructureSharedPtr& pv,
                              QVariant& value, QString& type)
{
   bool result = false;
   value = nullVariant;

   // Note: since base-7.0.3, had to switch to wrapUnsafe for NTScalar and NTScalarArray.
   //
   if (epics::nt::NTScalar::is_a (pv)) {
      epics::nt::NTScalar::const_shared_pointer item = epics::nt::NTScalar::wrapUnsafe (pv);
      ASSERT (item.get() != NULL, "NTScalar::wrapUnsafe yielded null");

      pvd::PVScalar::const_shared_pointer scalar = item->getValue<const pvd::PVScalar>();
      result = QEPvaData::extractScalar (scalar, value, type);


   } else if (epics::nt::NTScalarArray::is_a (pv)) {
      epics::nt::NTScalarArray::const_shared_pointer item = epics::nt::NTScalarArray::wrapUnsafe (pv);
      ASSERT (item.get() != NULL, "NTScalarArray::wrapUnsafe yielded null");
      pvd::PVScalarArray::const_shared_pointer scalarArray = item->getValue<const pvd::PVScalarArray>();
      result = QEPvaData::extractScalarArray (scalarArray, value, type);


   } else if (epics::nt::NTEnum::is_a (pv)) {
      epics::nt::NTEnum::const_shared_pointer item = epics::nt::NTEnum::wrap (pv);
      ASSERT (item.get() != NULL, "NTEnum::wrap yielded null");
      QEPvaData::Enumerated enumeration;
      result = enumeration.extract (pv);

      // Just grab the index value here
      if (result) {
         value = QVariant (enumeration.index);
         type = "";
      } else {
         DEBUG << "epics::nt::NTEnum item not valid";
      }


   } else if (epics::nt::NTTable::is_a (pv)) {

      epics::nt::NTTable::const_shared_pointer item = epics::nt::NTTable::wrap (pv);
      ASSERT (item.get() != NULL, "NTTable::wrap yielded null");

      QENTTableData table;
      result = table.assignFrom (item);
      if (result) {
         value = table.toVariant ();
         type = "";
      } else {
         DEBUG << "epics::nt::NTTable item not valid";
      }


   } else if (epics::nt::NTNDArray::is_a (pv)) {
      epics::nt::NTNDArray::const_shared_pointer item = epics::nt::NTNDArray::wrap (pv);
      ASSERT (item.get() != NULL, ":NTNDArray::wrap yielded null");
      // This is a NTNDArray/image type.
      //
      QENTNDArrayData image;
      result = image.assignFrom (item);
      if (result) {
         value = image.toVariant ();
         type = "";
      } else {
         DEBUG << "epics::nt::NTNDArray item not valid";
      }

   }

   else {
      // Unknown/unhandled types.
      //
      const pvd::StructureConstPtr ptr = pv->getStructure();

      QEOpaqueData opaque;
      result = opaque.assignFrom (ptr);
      if (result) {
         value = opaque.toVariant ();
         type = "opaque";
      } else {
         DEBUG << "opaque to varient failed.";
      }
   }

   return result;
}

//------------------------------------------------------------------------------
// static
bool QEPvaData::extractScalar (const pvd::PVScalar::const_shared_pointer& scalar,
                               QVariant& value, QString& type)
{
   bool result = true;   // hypothesize all okay

   pvd::ScalarConstPtr ptr = scalar->getScalar();
   pvd::ScalarType scalarType = ptr->getScalarType();

   // Look at pv/convert.h
   //
   switch (scalarType) {
      case pvd::pvString:
         value = QVariant (QString::fromStdString (scalar->getAs <std::string>()));
         type = "string";
         break;

      case pvd::pvDouble:
         value = QVariant (double(scalar->getAs <double> ()));
         type = "double";
         break;

      case pvd::pvFloat:
         value = QVariant (scalar->getAs <float> ());
         type = "float";
         break;

      case pvd::pvBoolean:
         value = QVariant (bool (scalar->getAs <pvd::boolean> ()));
         type = "boolean";
         break;

         // signed
         //
      case pvd::pvByte:
         value = QVariant (int (scalar->getAs <pvd::int8> ()));
         type = "byte";
         break;

      case pvd::pvShort:
         // QVariant do not support short
         value = QVariant (int (scalar->getAs <pvd::int16> ()));
         type = "short";
         break;

      case pvd::pvInt:
         value = QVariant (int (scalar->getAs <pvd::int32> ()));
         type = "int";
         break;

      case pvd::pvLong:
         value = QVariant (qlonglong (scalar->getAs <pvd::int64> ()));
         type = "long";
         break;

         // unsigned
         //
      case pvd::pvUByte:
         value = QVariant (uint (scalar->getAs <pvd::uint8> ()));
         type = "ubyte";
         break;

      case pvd::pvUShort:
         // QVariant do not support unsigned short
         value = QVariant (uint (scalar->getAs <pvd::uint16> ()));
         type = "ushort";
         break;

      case pvd::pvUInt:
         value = QVariant (uint (scalar->getAs <pvd::uint32> ()));
         break;

      case pvd::pvULong:
         value = QVariant (qulonglong (scalar->getAs <pvd::uint64> ()));
         type = "ulong";
         break;

      default:
         value = nullVariant;
         type = "unknown";
         result = false;
         DEBUG << scalarType << "unhandled";
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
// Converts a PVScalarArray to QVariant
// Modify QE to handle arrays better.
//
//static
bool QEPvaData::extractScalarArray
   (const pvd::PVScalarArray::const_shared_pointer& scalarArray,
    QVariant& value, QString& type)
{
   const std::size_t number = scalarArray->getLength ();

   bool result = true;   // hypothesize all okay

   pvd::ScalarArrayConstPtr ptr = scalarArray->getScalarArray ();
   pvd::ScalarType elementType = ptr->getElementType ();

// pvdet is pv data scalar array element type
// qevec is QE vector type with element compatible with pvdet
// con is type name of qevec element for cast or a convert function
// E.g. ATOV (pvd::int16, QEInt16Vector, int16_t);
//
#define ATOV(pvdet,qevec,con) {                           \
   pvd::shared_vector<const pvdet> data;                  \
   qevec qdata;                                           \
   qdata.reserve (number);                                \
   scalarArray->getAs (data);                             \
   for (std::size_t j = 0; j < number; ++j) {             \
      qdata.append (con (data [j]));                      \
   }                                                      \
   value.setValue (qdata);                                \
}

   switch (elementType) {
      case pvd::pvString:
         ATOV (std::string, QStringList, QString::fromStdString);
         type = "string";
         break;

      case pvd::pvDouble:
         ATOV (double, QEDoubleVector, double);
         type = "double";
         break;

      case pvd::pvFloat:
         ATOV (float, QEFloatVector, float);
         type = "float";
         break;

      case pvd::pvBoolean:
         ATOV (pvd::boolean, QEBoolVector, bool);
         type = "boolean";
         break;

         // signed integers
         //
      case pvd::pvByte:
         ATOV (pvd::int8, QEInt8Vector, int8_t);
         type = "byte";
         break;

      case pvd::pvShort:
         ATOV (pvd::int16, QEInt16Vector, int16_t);
         type = "short";
         break;

      case pvd::pvInt:
         ATOV (pvd::int32, QEInt32Vector, int32_t);
         type = "int";
         break;

      case pvd::pvLong:
         ATOV (pvd::int64, QEInt64Vector, int64_t);
         type = "long";
         break;

         // unsigned integers
         //
      case pvd::pvUByte:
         ATOV (pvd::uint8, QEUint8Vector,   uint8_t);
         type = "ubyte";
         break;

      case pvd::pvUShort:
         ATOV (pvd::uint16, QEUint16Vector, uint16_t);
         type = "ushort";
         break;

      case pvd::pvUInt:
         ATOV (pvd::uint32, QEUint32Vector, uint32_t);
         type = "uint";
         break;

      case pvd::pvULong:
         ATOV (pvd::uint64, QEUint64Vector, uint64_t);
         type = "ulong";
         break;

      default:
         value = nullVariant;
         type = "unknown";
         result = false;
         DEBUG << elementType << "unhandled";
         break;
   }

   return result;

#undef ATOV
}


//------------------------------------------------------------------------------
// static
bool QEPvaData::infuseValue (PVStructureSharedPtr& pv,
                             const QVariant& value)
{
   bool result = false;

   pvd::PVField::shared_pointer valueField = pv->getSubField ("value");
   if (valueField.get() == NULL) {
      DEBUG << "no value field\n";
      return false;
   }

   const pvd::Type valueType = valueField->getField()->getType();
   if (valueType == pvd::scalar) {

      pvd::PVScalar::shared_pointer scalarField;
      scalarField = TR1::static_pointer_cast<pvd::PVScalar> (valueField);
      result = QEPvaData::infuseScalar (scalarField, value);

   } else if (valueType == pvd::scalarArray) {

      pvd::PVScalarArray::shared_pointer scalarArrayField;
      scalarArrayField = TR1::static_pointer_cast<pvd::PVScalarArray> (valueField);
      result = QEPvaData::infuseScalarArray (scalarArrayField, value);

   } else if (valueType == pvd::structure) {
      QEPvaData::PVStructureSharedPtr structureField;
      structureField = TR1::static_pointer_cast<pvd::PVStructure> (valueField);

      const QString id = QString::fromStdString (structureField->getField()->getID ());
      if (id == "enum_t") {
         pvd::PVInt::shared_pointer pvIndex = structureField->getSubField<pvd::PVInt> ("index");
         pvIndex->put (value.toInt());
         result = true;
      } else {
         DEBUG << "TBD structure type: " << id;
         result = false;
      }

   } else {
      DEBUG << "TBD value type: " << valueType;
      std::cout << valueField << valueField->getField()->getID() << "\n";
      result = false;
   }

   return result;
}


//------------------------------------------------------------------------------
//static
bool QEPvaData::infuseScalar (pvd::PVScalar::shared_pointer& scalarField,
                              const QVariant& value)
{
   bool result = true;  // hypothosize okay

   const pvd::ScalarType scalarType = scalarField->getScalar()->getScalarType ();

   switch (scalarType) {
      case pvd::pvBoolean:
         scalarField->putFrom <pvd::boolean>(value.toBool());
         break;

      case pvd::pvByte:
         scalarField->putFrom <pvd::int8>(value.toInt());
         break;

      case pvd::pvShort:
         scalarField->putFrom <pvd::int16>(value.toInt());
         break;

      case pvd::pvInt:
         scalarField->putFrom <pvd::int32>(value.toInt());
         break;

      case pvd::pvLong:
         scalarField->putFrom <pvd::int64>(value.toLongLong());
         break;

      case pvd::pvUByte:
         scalarField->putFrom <pvd::uint8>(value.toUInt());
         break;

      case pvd::pvUShort:
         scalarField->putFrom <pvd::uint16>(value.toUInt());
         break;

      case pvd::pvUInt:
         scalarField->putFrom <pvd::uint32>(value.toUInt());
         break;

      case pvd::pvULong:
         scalarField->putFrom <pvd::uint64>(value.toULongLong());
         break;

      case pvd::pvFloat:
         scalarField->putFrom <float>(value.toFloat());
         break;

      case pvd::pvDouble:
         scalarField->putFrom <double>(value.toDouble());
         break;

      case pvd::pvString:
         scalarField->putFrom <std::string>(value.toString().toStdString());
         break;

      default:
         result = false;
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
// static
//
bool QEPvaData::infuseScalarArray (pvd::PVScalarArray::shared_pointer& pvArray,
                                   const QVariant& value)
{
   bool result;

   const QMetaType::Type mtype = QEPlatform::metaType (value);

   if (QEVectorVariants::isVectorVariant (value)) {
      result = QEPvaData::infuseScalarArrayVector (pvArray, value);

   } else if (mtype == QMetaType::QStringList) {
      // Convert the QStringList variant to a list of variants (which are all QString)
      //
      QVariantList vl = value.toList();
      result = QEPvaData::infuseScalarArrayList (pvArray, vl);

   } else if (mtype == QMetaType::QVariantList) {
      QVariantList vl = value.toList();
      result = QEPvaData::infuseScalarArrayList (pvArray, vl);

   } else {
      // a scalar - convert to a list of one.
      QVariantList vl;
      vl.append (value);
      result = QEPvaData::infuseScalarArrayList (pvArray, vl);
   }

   return result;
}

//------------------------------------------------------------------------------
// static
bool QEPvaData::infuseScalarArrayList (epics::pvData::PVScalarArray::shared_pointer& pvArray,
                                       const QVariantList& valueList)
{
   const std::size_t offset = 0;
   const std::size_t number = valueList.count ();

   bool result = true;   // hypothesize all okay

   pvd::ScalarArrayConstPtr ptr = pvArray->getScalarArray ();
   pvd::ScalarType elementType = ptr->getElementType ();

   // pvdet is pv data scalar array element type (e.g. double,pvd::boolean,pvd::uint16)
   // totype is variant extraction funtion (e.g. toBouble, toBool, toUint)
   // extra is any additonal conversion (i.e.  .toStdString() )
   // E.G. VLTOA (std::string, toDouble, );
   //
   #define VLTOA(pvdet,totype,extra) {                                         \
      pvdet* ptr = new pvdet [number];                                         \
      for (std::size_t j = 0; j < number; ++j) {                               \
         ptr [j] = valueList.value(j).totype()extra;                           \
      }                                                                        \
      pvd::shared_vector<const pvdet> data (ptr, offset, number);              \
      pvArray->putFrom (data);                                                 \
   }

   QVariant v;
   v.toLongLong();

   switch (elementType) {
      case pvd::pvString:
         VLTOA (std::string, toString, .toStdString());
         break;

      case pvd::pvDouble:
         VLTOA (double, toDouble, );
         break;

      case pvd::pvFloat:
         VLTOA (float, toFloat, );
         break;

      case pvd::pvBoolean:
         VLTOA (pvd::boolean, toBool, );
         break;

      case pvd::pvByte:
         VLTOA (pvd::int8,  toInt, );
         break;

      case pvd::pvShort:
         VLTOA (pvd::int16, toInt, );
         break;

      case pvd::pvInt:
         VLTOA (pvd::int32, toInt, );
         break;

      case pvd::pvLong:
         VLTOA (pvd::int64, toLongLong, );
         break;

      case pvd::pvUByte:
         VLTOA (pvd::uint8,  toUInt, );
         break;

      case pvd::pvUShort:
         VLTOA (pvd::uint16, toUInt, );
         break;

      case pvd::pvUInt:
         VLTOA (pvd::uint32, toUInt, );
         break;

      case pvd::pvULong:
         VLTOA (pvd::uint64, toULongLong, );
         break;

      default:
         result = false;
         break;
   }

   return result;

#undef VLTOA
}

//------------------------------------------------------------------------------
// static
bool QEPvaData::infuseScalarArrayVector (epics::pvData::PVScalarArray::shared_pointer& pvArray,
                                         const QVariant& vector)
{
   if (!QEVectorVariants::isVectorVariant (vector)) {
      const QMetaType::Type mtype = QEPlatform::metaType (vector);
      DEBUG << " called with non vector variant type:" << mtype;
      return false;
   }

   const std::size_t offset = 0;
   const std::size_t number = QEVectorVariants::vectorCount (vector);

   bool result = true;   // hypothesize all okay

   pvd::ScalarArrayConstPtr ptr = pvArray->getScalarArray ();
   pvd::ScalarType elementType = ptr->getElementType ();

   // Dropped the types match (for now) - it created anomolous behaviour.
   // We must convert each element.
   //
#define VVTOA(qevec, qetype, type) {                                           \
   qevec vec = vector.value <qevec>();                                         \
   type* ptr = new type [number];                                              \
   for (std::size_t j = 0; j < number; ++j) {                                  \
      ptr [j] = type (vec [j]);                                                \
   }                                                                           \
   pvd::shared_vector<const type> data (ptr, offset, number);                  \
   pvArray->putFrom (data);                                                    \
}


   switch (elementType) {

      case pvd::pvDouble:
         VVTOA (QEDoubleVector, QEVectorVariants::DoubleVector, double)
         break;

      case pvd::pvFloat:
         VVTOA (QEFloatVector, QEVectorVariants::FloatVector, float);
         break;

      case pvd::pvBoolean:
         // VVTOA (QEBoolVector, QEVariants::BoolVector, pvd::boolean);
         // Boolean is weird
         {
            QEBoolVector vec = vector.value <QEBoolVector>();
            pvd::boolean* ptr = new pvd::boolean [number];
            for (std::size_t j = 0; j < number; ++j) {
               ptr [j] = pvd::boolean (vec [j]);
            }
            pvd::shared_vector<const pvd::boolean> data (ptr, offset, number);
            pvArray->putFrom (data);
         }
         break;

      case pvd::pvByte:
         VVTOA (QEInt8Vector, QEVectorVariants::Int8Vector, pvd::int8);
         break;

      case pvd::pvShort:
         VVTOA (QEInt16Vector, QEVectorVariants::Int16Vector, pvd::int16);
         break;

      case pvd::pvInt:
         VVTOA (QEInt32Vector, QEVectorVariants::Int32Vector, pvd::int32);
         break;

      case pvd::pvLong:
         VVTOA (QEInt64Vector, QEVectorVariants::Int64Vector, pvd::int64);
         break;

      case pvd::pvUByte:
         VVTOA (QEUint8Vector, QEVectorVariants::Uint8Vector, pvd::uint8);
         break;

      case pvd::pvUShort:
         VVTOA (QEUint16Vector, QEVectorVariants::Uint16Vector, pvd::uint16);
         break;

      case pvd::pvUInt:
         VVTOA (QEUint32Vector, QEVectorVariants::Uint32Vector, pvd::uint32);
         break;

      case pvd::pvULong:
         VVTOA (QEUint64Vector, QEVectorVariants::Uint64Vector, pvd::uint64);
         break;

      default:
         result = false;
         break;
   }

   return result;

#undef VVTOA
}

//------------------------------------------------------------------------------
// static - special used by QETableData
//
QVariantList QEPvaData::scalarArrayToQVariantList
    (const pvd::PVScalarArray::const_shared_pointer& scalarArray)
{
   const std::size_t number = scalarArray->getLength ();

   QVariantList result;

   pvd::ScalarArrayConstPtr ptr = scalarArray->getScalarArray ();
   pvd::ScalarType elementType = ptr->getElementType ();

// pvet is pv scalar array element type
// con  is type name for null cast or a convert function
//
#define ATOL(pvet,con) {                                  \
   pvd::shared_vector<const pvet> data;                   \
   scalarArray->getAs (data);                             \
   for (std::size_t j = 0; j < number; ++j) {             \
      result.append (con (data [j]));                     \
   }                                                      \
}


   switch (elementType) {
      case pvd::pvString:
         ATOL (std::string, QString::fromStdString);
         break;

      case pvd::pvDouble:
         ATOL (double, double);
         break;

      case pvd::pvFloat:
         ATOL (float, float);
         break;

      case pvd::pvBoolean:
         ATOL (pvd::boolean, bool);
         break;

         // signed
         //
      case pvd::pvByte:
         ATOL (int8_t, int);
         break;

      case pvd::pvShort:
         ATOL (int16_t, int);
         break;

      case pvd::pvInt:
         ATOL (int32_t, int);
         break;

      case pvd::pvLong:
         ATOL (int64_t, qlonglong);
         break;


         // unsigned
         //
      case pvd::pvUByte:
         ATOL (uint8_t,  uint);
         break;

      case pvd::pvUShort:
         ATOL (uint16_t, uint);
         break;

      case pvd::pvUInt:
         ATOL (uint32_t, uint);
         break;

      case pvd::pvULong:
         ATOL (uint64_t, qulonglong);
         break;

      default:
         break;
   }

   return result;

#undef ATOL
}


//------------------------------------------------------------------------------
// Inner function macro functions
//------------------------------------------------------------------------------
// We attempt to get the inner strcture, say display first using ASSIGN_STRUCT
// and then read sub fields, say limitLow, using ASSIGN_MEMBER as opposed to
// attempting to access "display.limitLow" directly.
//
#define ASSIGN_STRUCT(target, field)                                           \
   this->isDefined = false;                                                    \
   pvd::PVField::const_shared_pointer itemField;                               \
   PVStructureConstPtr structField;                                            \
   /* */                                                                       \
   itemField = pv->getSubField (#field);                                       \
   if (!itemField) {                                                           \
      return false;                                                            \
   }                                                                           \
   /* */                                                                       \
   const pvd::Type valueType = itemField->getField()->getType ();              \
   if (valueType != pvd::structure) {                                          \
      DEBUG << #field " is not a structure";                                   \
      return false;                                                            \
   }                                                                           \
   /* */                                                                       \
   structField = TR1::static_pointer_cast<const pvd::PVStructure>(itemField);
                        \

//------------------------------------------------------------------------------
//
#define ASSIGN_MEMBER(target, member, type, convert) {                         \
   type::const_shared_pointer memberField;                                     \
   memberField = structField->getSubField<type>(#member);                      \
   if (memberField.get() == NULL) {                                            \
      DEBUG << "no "#target"."#member" field";                                 \
      return false;                                                            \
   }                                                                           \
   this->member = convert (memberField->get());                                \
}


//------------------------------------------------------------------------------
// static
bool QEPvaData::Enumerated::extract (const PVStructureConstPtr& pv)
{
   this->isDefined = false;

   const pvd::StructureConstPtr ptr = pv->getStructure();
   if (!ptr) return false;
   const QString typeName = QString::fromStdString (ptr->getID ());

   // Verify this is a, or at least perports to be, ab NTEnum type.
   //
   if (typeName.left (enumTypeId.length ()) != enumTypeId) {
      return false;
   }

   ASSIGN_STRUCT (enumeration, value);
   ASSIGN_MEMBER (enumeration, index, pvd::PVInt,    int);

   pvd::PVField::const_shared_pointer choicesField;
   choicesField = structField->getSubField ("choices");
   if (!choicesField) {
      std::cout << "no value.choices field\n";
      return false;
   }

   pvd::PVScalarArray::const_shared_pointer choicesArray =
         TR1::static_pointer_cast<const pvd::PVScalarArray> (choicesField);

   const std::size_t number = choicesArray->getLength ();

   pvd::shared_vector<const std::string> choicesData;
   choicesArray->getAs (choicesData);

   this->choices.clear();
   for (std::size_t j = 0; j < number; ++j) {
      this->choices.append (QString::fromStdString (choicesData [j]));
   }

   this->isDefined = true;
   return true;
}

//------------------------------------------------------------------------------
// static
bool QEPvaData::Alarm::extract (const PVStructureConstPtr& pv)
{
   ASSIGN_STRUCT (alarm, alarm);
   ASSIGN_MEMBER (alarm, severity, pvd::PVInt,    int);
   ASSIGN_MEMBER (alarm, status,   pvd::PVInt,    int);
   ASSIGN_MEMBER (alarm, message,  pvd::PVString, QString::fromStdString);
   this->isDefined = true;
   return true;
}

//------------------------------------------------------------------------------
// static
bool QEPvaData::Display::extract (const PVStructureConstPtr& pv)
{
   ASSIGN_STRUCT (display, display);
   ASSIGN_MEMBER (display, limitLow,    pvd::PVDouble, double);
   ASSIGN_MEMBER (display, limitHigh,   pvd::PVDouble, double);
   ASSIGN_MEMBER (display, description, pvd::PVString, QString::fromStdString);
   ASSIGN_MEMBER (display, units,       pvd::PVString, QString::fromStdString);
   ASSIGN_MEMBER (display, precision,   pvd::PVInt,    int);
   // format replaced by form - TBD
   this->isDefined = true;
   return true;
}

//------------------------------------------------------------------------------
// static
bool QEPvaData::TimeStamp::extract (const PVStructureConstPtr& pv)
{
   ASSIGN_STRUCT (timeStamp, timeStamp);
   ASSIGN_MEMBER (timeStamp, secondsPastEpoch, pvd::PVLong, long);
   ASSIGN_MEMBER (timeStamp, nanoseconds,      pvd::PVInt,  int);
   ASSIGN_MEMBER (timeStamp, userTag,          pvd::PVInt,  int);
   this->isDefined = true;
   return true;
}

//------------------------------------------------------------------------------
// static
bool QEPvaData::Control::extract (const PVStructureConstPtr& pv)
{
   ASSIGN_STRUCT (control, control);
   ASSIGN_MEMBER (control, limitLow,  pvd::PVDouble, double);
   ASSIGN_MEMBER (control, limitHigh, pvd::PVDouble, double);
   ASSIGN_MEMBER (control, minStep,   pvd::PVDouble, double);
   this->isDefined = true;
   return true;
}

//------------------------------------------------------------------------------
// static
bool QEPvaData::ValueAlarm::extract (const PVStructureConstPtr& pv)
{
   ASSIGN_STRUCT (valueAlarm, valueAlarm);
   ASSIGN_MEMBER (valueAlarm, active,              pvd::PVBoolean, bool);
   ASSIGN_MEMBER (valueAlarm, lowAlarmSeverity,    pvd::PVInt,     int);
   ASSIGN_MEMBER (valueAlarm, lowWarningSeverity,  pvd::PVInt,     int);
   ASSIGN_MEMBER (valueAlarm, highWarningSeverity, pvd::PVInt,     int);
   ASSIGN_MEMBER (valueAlarm, highAlarmSeverity,   pvd::PVInt,     int);

   // The high/low alarm/warning limits vary depending on the value type.
   // As all the same, just use lowAlarmLimit as guidence.
   //
   pvd::PVField::const_shared_pointer flal = structField->getSubField ("lowAlarmLimit");
   if (flal.get() == NULL) {
      DEBUG << "no valueAlarm.lowAlarmLimit field";
      return false;
   }
   const pvd::Type fType = flal->getField()->getType();
   if (fType != pvd::scalar) {
      DEBUG << "valueAlarm.lowAlarmLimit is not scalar";
      return false;
   }

   pvd::PVScalar::const_shared_pointer sflal =  TR1::static_pointer_cast<const pvd::PVScalar> (flal);
   pvd::ScalarType scalarType = sflal->getScalar()->getScalarType();

   // Look at pv/convert.h
   //
   switch (scalarType) {
      case pvd::pvBoolean:
         ASSIGN_MEMBER (valueAlarm, lowAlarmLimit,       pvd::PVBoolean,  double);
         ASSIGN_MEMBER (valueAlarm, lowWarningLimit,     pvd::PVBoolean,  double);
         ASSIGN_MEMBER (valueAlarm, highWarningLimit,    pvd::PVBoolean,  double);
         ASSIGN_MEMBER (valueAlarm, highAlarmLimit,      pvd::PVBoolean,  double);
         ASSIGN_MEMBER (valueAlarm, hysteresis,          pvd::PVByte,     int);
         break;

      case pvd::pvByte:
         ASSIGN_MEMBER (valueAlarm, lowAlarmLimit,       pvd::PVByte,   double);
         ASSIGN_MEMBER (valueAlarm, lowWarningLimit,     pvd::PVByte,   double);
         ASSIGN_MEMBER (valueAlarm, highWarningLimit,    pvd::PVByte,   double);
         ASSIGN_MEMBER (valueAlarm, highAlarmLimit,      pvd::PVByte,   double);
         ASSIGN_MEMBER (valueAlarm, hysteresis,          pvd::PVByte,   int);
         break;

      case pvd::pvShort:
         ASSIGN_MEMBER (valueAlarm, lowAlarmLimit,       pvd::PVShort,  double);
         ASSIGN_MEMBER (valueAlarm, lowWarningLimit,     pvd::PVShort,  double);
         ASSIGN_MEMBER (valueAlarm, highWarningLimit,    pvd::PVShort,  double);
         ASSIGN_MEMBER (valueAlarm, highAlarmLimit,      pvd::PVShort,  double);
         ASSIGN_MEMBER (valueAlarm, hysteresis,          pvd::PVByte,   int);
         break;

      case pvd::pvInt:
         ASSIGN_MEMBER (valueAlarm, lowAlarmLimit,       pvd::PVInt,    double);
         ASSIGN_MEMBER (valueAlarm, lowWarningLimit,     pvd::PVInt,    double);
         ASSIGN_MEMBER (valueAlarm, highWarningLimit,    pvd::PVInt,    double);
         ASSIGN_MEMBER (valueAlarm, highAlarmLimit,      pvd::PVInt,    double);
         ASSIGN_MEMBER (valueAlarm, hysteresis,          pvd::PVByte,   int);
         break;

      case pvd::pvLong:
         ASSIGN_MEMBER (valueAlarm, lowAlarmLimit,       pvd::PVLong,   double);
         ASSIGN_MEMBER (valueAlarm, lowWarningLimit,     pvd::PVLong,   double);
         ASSIGN_MEMBER (valueAlarm, highWarningLimit,    pvd::PVLong,   double);
         ASSIGN_MEMBER (valueAlarm, highAlarmLimit,      pvd::PVLong,   double);
         ASSIGN_MEMBER (valueAlarm, hysteresis,          pvd::PVByte,   int);
         break;

      case pvd::pvUByte:
         ASSIGN_MEMBER (valueAlarm, lowAlarmLimit,       pvd::PVUByte,  double);
         ASSIGN_MEMBER (valueAlarm, lowWarningLimit,     pvd::PVUByte,  double);
         ASSIGN_MEMBER (valueAlarm, highWarningLimit,    pvd::PVUByte,  double);
         ASSIGN_MEMBER (valueAlarm, highAlarmLimit,      pvd::PVUByte,  double);
         ASSIGN_MEMBER (valueAlarm, hysteresis,          pvd::PVByte,   int);
         break;

      case pvd::pvUShort:
         ASSIGN_MEMBER (valueAlarm, lowAlarmLimit,       pvd::PVUShort, double);
         ASSIGN_MEMBER (valueAlarm, lowWarningLimit,     pvd::PVUShort, double);
         ASSIGN_MEMBER (valueAlarm, highWarningLimit,    pvd::PVUShort, double);
         ASSIGN_MEMBER (valueAlarm, highAlarmLimit,      pvd::PVUShort, double);
         ASSIGN_MEMBER (valueAlarm, hysteresis,          pvd::PVByte,   int);
         break;

      case pvd::pvUInt:
         ASSIGN_MEMBER (valueAlarm, lowAlarmLimit,       pvd::PVUInt,   double);
         ASSIGN_MEMBER (valueAlarm, lowWarningLimit,     pvd::PVUInt,   double);
         ASSIGN_MEMBER (valueAlarm, highWarningLimit,    pvd::PVUInt,   double);
         ASSIGN_MEMBER (valueAlarm, highAlarmLimit,      pvd::PVUInt,   double);
         ASSIGN_MEMBER (valueAlarm, hysteresis,          pvd::PVByte,   int);
         break;

      case pvd::pvULong:
         ASSIGN_MEMBER (valueAlarm, lowAlarmLimit,       pvd::PVULong,  double);
         ASSIGN_MEMBER (valueAlarm, lowWarningLimit,     pvd::PVULong,  double);
         ASSIGN_MEMBER (valueAlarm, highWarningLimit,    pvd::PVULong,  double);
         ASSIGN_MEMBER (valueAlarm, highAlarmLimit,      pvd::PVULong,  double);
         ASSIGN_MEMBER (valueAlarm, hysteresis,          pvd::PVByte,   int);
         break;

      case pvd::pvFloat:
         ASSIGN_MEMBER (valueAlarm, lowAlarmLimit,       pvd::PVFloat,  double);
         ASSIGN_MEMBER (valueAlarm, lowWarningLimit,     pvd::PVFloat,  double);
         ASSIGN_MEMBER (valueAlarm, highWarningLimit,    pvd::PVFloat,  double);
         ASSIGN_MEMBER (valueAlarm, highAlarmLimit,      pvd::PVFloat,  double);
         ASSIGN_MEMBER (valueAlarm, hysteresis,          pvd::PVByte,   int);
         break;

      case pvd::pvDouble:
         ASSIGN_MEMBER (valueAlarm, lowAlarmLimit,       pvd::PVDouble,  double);
         ASSIGN_MEMBER (valueAlarm, lowWarningLimit,     pvd::PVDouble,  double);
         ASSIGN_MEMBER (valueAlarm, highWarningLimit,    pvd::PVDouble,  double);
         ASSIGN_MEMBER (valueAlarm, highAlarmLimit,      pvd::PVDouble,  double);
         ASSIGN_MEMBER (valueAlarm, hysteresis,          pvd::PVByte,    int);
         break;

      case pvd::pvString:
         DEBUG << "unexpected valueAlarm limit type" << scalarType;
         return false;
         break;

      default:
         return false;
         break;
   }

   this->isDefined = true;
   return true;
}

#undef ASSIGN_STRUCT
#undef ASSIGN_MEMBER

#endif  // QE_INCLUDE_PV_ACCESS

// end

