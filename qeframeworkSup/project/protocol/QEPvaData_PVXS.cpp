/*  QEPvaData_PVXS.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEPvaData_PVXS.h"

#include <iostream>
#include <QDebug>
#include <QMetaType>
#include <QEPlatform.h>
#include <QEVectorVariants.h>
#include <QENTTableData.h>
#include <QENTNDArrayData.h>
#include <QEOpaqueData.h>

#define DEBUG qDebug() << "QEPvaData_PVXS" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// Inner function macro functions
//------------------------------------------------------------------------------
//
#define ASSERT(condition, message) {                                           \
   if (!(condition)) {                                                         \
      DEBUG << message;                                                        \
      return false;                                                            \
   }                                                                           \
}

// We attempt to get the inner structure, say display first using ASSIGN_STRUCT
// and then read sub fields, say limitLow, using ASSIGN_MEMBER as opposed to
// attempting to access "display.limitLow" directly.
//
#define ASSIGN_STRUCT(classname, innername)                                   \
   pvxs::Value value = pv[innername];                                         \
   if (!value) {                                                              \
      /* DEBUG <<  #classname << ": no " << innername; */                     \
      return false;                                                           \
   }


// Note: member name and field are often the same, but not always.
//
#define ASSIGN_MEMBER(classname, membername, field)                           \
   if (!value[membername].as(field)) {                                        \
      /* DEBUG << #classname << " no " << membername; */                      \
      return false;                                                           \
   }


// Like above, only optional.  Either returns true, or does not return.
//
#define OPTIONAL_ASSIGN_STRUCT(classname, innername)                          \
   pvxs::Value value = pv[innername];                                         \
   if (!value) {                                                              \
      /* DEBUG <<  #classname << ": no " << innername; */                     \
      return true;                                                            \
   }

#define OPTIONAL_ASSIGN_MEMBER(classname, membername, field)                  \
   value[membername].as(field)


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
// static
QStringList QEPvaData::toStringList (const QESharedStringArray& source)
{
   QStringList result;

   result.reserve (source.size());
   for (auto item = source.begin(); item != source.end(); ++item) {
      result.push_back (QString::fromStdString(*item));
   }

   return result;
}

//------------------------------------------------------------------------------
// Similar to extractArray (604) - keep alligned
// static
QVariantList QEPvaData::scalarArrayToQVariantList (const pvxs::Value& scalarArray)
{
   QVariantList result;

   const pvxs::TypeCode tc = scalarArray.type();

   // dtype is the array element type acceptable to pvxs
   // convert convert from pvxs data type to a QVariant type - often blank.
   //
#define ARRAY_TO_LIST(dtype, convert) {                                    \
   pvxs::shared_array<const dtype> source;                                 \
   bool status = scalarArray.as (source);                                  \
   if (!status) return result;                                             \
   result.reserve (source.size());                                         \
   for (auto element : source) {                                           \
      QVariant value;                                                      \
      value.setValue (convert (element));                                  \
      result.append (value);                                               \
   }                                                                       \
}

   switch (tc.code) {

      case pvxs::TypeCode::BoolA:
         ARRAY_TO_LIST (bool, );
         break;

      case pvxs::TypeCode::Int8A:
      case pvxs::TypeCode::Int16A:
      case pvxs::TypeCode::Int32A:
         ARRAY_TO_LIST (int32_t, );
         break;

      case pvxs::TypeCode::Int64A:
         ARRAY_TO_LIST (int64_t, );
         break;

      case pvxs::TypeCode::UInt8A:
      case pvxs::TypeCode::UInt16A:
      case pvxs::TypeCode::UInt64A:
         ARRAY_TO_LIST (uint32_t, );
         break;

      case pvxs::TypeCode::UInt32A:
         ARRAY_TO_LIST (uint64_t, );
         break;

      case pvxs::TypeCode::Float32A:
      case pvxs::TypeCode::Float64A:
         ARRAY_TO_LIST (double, );
         break;

      case pvxs::TypeCode::StringA:
         ARRAY_TO_LIST (std::string, QString::fromStdString);
         break;

      default:
         DEBUG << "scalarArrayToQVariantList: unhandled "
               <<  tc.name() << " (" << tc.code << ")\n";
         break;
   }

#undef ARRAY_TO_LIST

   return result;
}

//------------------------------------------------------------------------------
//
QEPvaData::QEPvaData () { }   // place holder

//------------------------------------------------------------------------------
//
QEPvaData::~QEPvaData () { }  // place holder


//==============================================================================
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
bool QEPvaData::Enumerated::extract (const pvxs::Value& pv)
{
   // Hypothesize it fails until we know otherwise.
   //
   this->isDefined = false;

   // Verify this is a, or at least perports to be, an NTEnum type.
   //
   if (!pv.idStartsWith ("epics:nt/NTEnum:")) return false;

   ASSIGN_STRUCT (Enumerated, "value");
   ASSIGN_MEMBER (Enumerated, "index", this->index);

   // Can't use ASSIGN_MEMBER for arrays.
   //
   QESharedStringArray ssa;
   bool status = value["choices"].as(ssa);
   if (!status) {
      DEBUG << "Enumerated: no valid choices\n";
      return false;
   }

   this->choices = QEPvaData:: toStringList (ssa);
   this->isDefined = true;
   return this->isDefined;
}


//==============================================================================
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
bool QEPvaData::Alarm::extract (const pvxs::Value& pv)
{
   // Hypothesize it fails until we know otherwise.
   //
   this->isDefined = false;

   ASSIGN_STRUCT (Alarm, "alarm");
   ASSIGN_MEMBER (Alarm, "severity", this->severity);
   ASSIGN_MEMBER (Alarm, "status",   this->status);
   std::string temp;
   ASSIGN_MEMBER (Alarm, "message",  temp);
   this->message = QString::fromStdString (temp);

   this->isDefined = true;
   return this->isDefined;
}


//==============================================================================
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
bool QEPvaData::TimeStamp::extract (const pvxs::Value& pv)
{
   // Hypothesize it fails until we know otherwise.
   //
   this->isDefined = false;

   ASSIGN_STRUCT (TimeStamp, "timeStamp");
   ASSIGN_MEMBER (TimeStamp, "secondsPastEpoch", this->secondsPastEpoch);
   ASSIGN_MEMBER (TimeStamp, "nanoseconds",      this->nanoseconds);
   ASSIGN_MEMBER (TimeStamp, "userTag",          this->userTag);

   this->isDefined = true;
   return this->isDefined;
}


//==============================================================================
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
bool QEPvaData::Display::extract (const pvxs::Value& pv)
{
   const bool isNTTable =   pv.idStartsWith ("epics:nt/NTTable:");
   const bool isNTNDArray = pv.idStartsWith ("epics:nt/NTNDArray:");

   // Hypothesize it fails until we know otherwise.
   //
   this->isDefined = false;

   if (isNTTable) {
      // The NT table deos not usually have these items.
      //
      std::string temp;

      OPTIONAL_ASSIGN_STRUCT (Display, "display");
      OPTIONAL_ASSIGN_MEMBER (Display, "limitLow",    this->limitLow );
      OPTIONAL_ASSIGN_MEMBER (Display, "limitHigh",   this->limitHigh);

      temp = "";   // Ensure we clear temp
      OPTIONAL_ASSIGN_MEMBER (Display, "description", temp);
      this->description = QString::fromStdString (temp);

      temp = "";   // Ensure we clear temp
      OPTIONAL_ASSIGN_MEMBER (Display, "units",       temp);
      this->units = QString::fromStdString (temp);

      OPTIONAL_ASSIGN_MEMBER (Display, "precision",   this->precision);

   } else {
      std::string temp;

      ASSIGN_STRUCT (Display, "display");
      ASSIGN_MEMBER (Display, "limitLow",    this->limitLow );
      ASSIGN_MEMBER (Display, "limitHigh",   this->limitHigh);
      ASSIGN_MEMBER (Display, "description", temp);
      this->description = QString::fromStdString (temp);
      ASSIGN_MEMBER (Display, "units",       temp);
      this->units = QString::fromStdString (temp);

      if (isNTNDArray) {
         // NTNDArrays don't normally have a precision.
         OPTIONAL_ASSIGN_MEMBER (Display, "precision",   this->precision);
      }
      // format replaced by form - TBD
   }

   this->isDefined = true;
   return true;
}


//==============================================================================
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
bool QEPvaData::Control::extract (const pvxs::Value& pv)
{
   // Hypothesize it fails until we know otherwise.
   //
   this->isDefined = false;

   ASSIGN_STRUCT (Control, "control");
   ASSIGN_MEMBER (Control, "limitLow",  this->limitLow );
   ASSIGN_MEMBER (Control, "limitHigh", this->limitHigh );
   ASSIGN_MEMBER (Control, "minStep",   this->minStep );

   this->isDefined = true;
   return this->isDefined;
}


//==============================================================================
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

//------------------------------------------------------------------------------
//
bool QEPvaData::ValueAlarm::extract (const pvxs::Value& pv)
{
   // Hypothesize it fails until we know otherwise.
   //
   this->isDefined = false;

   ASSIGN_STRUCT (ValueAlarm, "valueAlarm");
   ASSIGN_MEMBER (ValueAlarm, "active",              this->active);

   ASSIGN_MEMBER (ValueAlarm, "lowAlarmLimit",       this->lowAlarmLimit);
   ASSIGN_MEMBER (ValueAlarm, "lowWarningLimit",     this->lowWarningLimit);
   ASSIGN_MEMBER (ValueAlarm, "highWarningLimit",    this->highWarningLimit);
   ASSIGN_MEMBER (ValueAlarm, "highAlarmLimit",      this->highAlarmLimit);

   ASSIGN_MEMBER (ValueAlarm, "lowAlarmSeverity",    this->lowAlarmSeverity);
   ASSIGN_MEMBER (ValueAlarm, "lowWarningSeverity",  this->lowWarningSeverity);
   ASSIGN_MEMBER (ValueAlarm, "highWarningSeverity", this->highWarningSeverity);
   ASSIGN_MEMBER (ValueAlarm, "highAlarmSeverity",   this->highAlarmSeverity);
   ASSIGN_MEMBER (ValueAlarm, "hysteresis",          this->hysteresis);

   this->isDefined = true;
   return this->isDefined;
}

#undef ASSIGN_STRUCT
#undef ASSIGN_MEMBER


static const QVariant nullVariant;

//------------------------------------------------------------------------------
// static
bool QEPvaData::extractScalar (const pvxs::Value& pv, QVariant& value, QString& type)
{
   bool result = false;

   pvxs::Value item = pv["value"];
   if (!item) return false;   // there is no value

   const pvxs::TypeCode tc = item.type();
   type = tc.name();   // e.g. "double"

   switch (tc.code) {

      case pvxs::TypeCode::Bool:
         {
            bool temp = false;
            result = item.as (temp);
            if (result) value = QVariant (temp);
         }
         break;

      case pvxs::TypeCode::Int8:
      case pvxs::TypeCode::Int16:
      case pvxs::TypeCode::Int32:
         {
            int32_t temp = 0;
            result = item.as (temp);
            if (result) value = QVariant (int(temp));
            break;
         }

      case pvxs::TypeCode::Int64:
         {
            int64_t temp = 0;
            result = item.as (temp);
            if (result) value = QVariant (qlonglong(temp));
            break;
         }

      case pvxs::TypeCode::UInt8:
      case pvxs::TypeCode::UInt16:
      case pvxs::TypeCode::UInt32:
         {
            uint32_t temp = 0;
            result = item.as (temp);
            if (result) value = QVariant (uint (temp));
         }
         break;

      case pvxs::TypeCode::UInt64:
         {
            uint64_t temp = 0;
            result = item.as (temp);
            if (result) value = QVariant (qulonglong (temp));
         }
         break;

      case pvxs::TypeCode::Float32:
      case pvxs::TypeCode::Float64:
         {
            double temp = 0.0;
            result = item.as (temp);
            if (result) value = QVariant(temp);
         }
         break;

      case pvxs::TypeCode::String:
         {
            QString temp;
            result = asQString (item, temp);
            if (result) value = QVariant (temp);
         }
         break;

      default:
         DEBUG << "unhandled " << type << " (" << tc.code << ")";
         break;
   }

   return result;
}


//------------------------------------------------------------------------------
// elementType - e.g. bool, int16, uint32, double
// vectorType  - associated vector variant type, e.g. QEDoubleVector
//
template <typename elementType, typename vectorType>
static bool extractArrayTemplate (const pvxs::Value& item, QVariant& value)
{
   pvxs::shared_array<const elementType> source;
   bool result = item.as (source);
   if (result) {
      vectorType target;
      target.reserve (source.size());
      for (auto element : source) {
         target.push_back (element);
      }
      value.setValue (target);
   }

   return result;
}

//------------------------------------------------------------------------------
// static
bool QEPvaData::extractArray (const pvxs::Value& pv, QVariant& value, QString& type)
{
   bool result = false;

   pvxs::Value item = pv["value"];
   if (!item) return false;   // there is no value

   const pvxs::TypeCode tc = item.type();
   type = tc.name();   // e.g. "double"

   switch (tc.code) {
      case pvxs::TypeCode::BoolA:
         result = extractArrayTemplate<bool, QEBoolVector>(item, value);
         break;

      case pvxs::TypeCode::Int8A:
         result = extractArrayTemplate<int8_t, QEInt8Vector>(item, value);
         break;

      case pvxs::TypeCode::Int16A:
         result = extractArrayTemplate<int16_t, QEInt16Vector>(item, value);
         break;

      case pvxs::TypeCode::Int32A:
         result = extractArrayTemplate<int32_t, QEInt32Vector>(item, value);
         break;

      case pvxs::TypeCode::Int64A:
         result = extractArrayTemplate<int64_t, QEInt64Vector>(item, value);
         break;

      case pvxs::TypeCode::UInt8A:
         result = extractArrayTemplate<uint8_t, QEUint8Vector>(item, value);
         break;

      case pvxs::TypeCode::UInt16A:
         result = extractArrayTemplate<uint16_t, QEUint16Vector>(item, value);
         break;

      case pvxs::TypeCode::UInt32A:
         result = extractArrayTemplate<uint32_t, QEUint32Vector>(item, value);
         break;

      case pvxs::TypeCode::UInt64A:
         result = extractArrayTemplate<uint64_t, QEUint64Vector>(item, value);
         break;

      case pvxs::TypeCode::Float32A:
         result = extractArrayTemplate<float, QEFloatVector>(item, value);
         break;

      case pvxs::TypeCode::Float64A:
         result = extractArrayTemplate<double, QEDoubleVector>(item, value);
         break;

      case pvxs::TypeCode::StringA:
         {
            pvxs::shared_array<const std::string> source;
            result = item.as (source);
            if (result) {
               QStringList list = toStringList(source);
               value.setValue (list);
            }
         }
         break;

      default:
         DEBUG << "unhandled " << type << " (" << tc.code << ")";
         break;
   }

   return result;

#undef EXTRACT_ARRAY
}

//------------------------------------------------------------------------------
// static
bool QEPvaData::asQString (const pvxs::Value& pv, QString& value)
{
   std::string temp;
   bool ok = pv.as (temp);
   value = ok ? QString::fromStdString (temp) : QString ("");
   return ok;
}

//------------------------------------------------------------------------------
// static
bool QEPvaData::extractValue (const pvxs::Value& pv, QVariant& value, QString& type)
{
   // Hypothesize it fails until we know otherwise.
   //
   bool result = false;

   value = nullVariant;
   type = "";

   if (pv.idStartsWith ("epics:nt/NTScalar:")) {
      result = QEPvaData::extractScalar (pv, value, type);

   } else if (pv.idStartsWith ("epics:nt/NTScalarArray:")) {
      result = QEPvaData::extractArray (pv, value, type);

   } else if (pv.idStartsWith ("epics:nt/NTEnum:")) {
      QEPvaData::Enumerated enumeration;
      result = enumeration.extract (pv);

      type = "enum";

      // Just grab the index value here
      //
      if (result) {
         value = QVariant (static_cast<int>(enumeration.index));
      }

   } else if (pv.idStartsWith ("epics:nt/NTTable:")) {
      QENTTableData item;
      result = item.assignFrom (pv);
      if (result) {
         value.setValue (item);
         type = "table";
      }

   } else if (pv.idStartsWith ("epics:nt/NTNDArray:")) {
      QENTNDArrayData item;
      result = item.assignFrom (pv);
      if (result) {
         value.setValue (item);
         type = "NDArray";
      }

   } else {
      // Is opaque
      //
      QEOpaqueData item;
      result = item.assignFrom (pv);
      if (result) {
         value.setValue (item);
         type = "opaque";
      }
   }

   return result;
}

//------------------------------------------------------------------------------
// static
bool QEPvaData::infuseScalar (pvxs::Value& pv, const QVariant& value)
{
   bool result = true;

   pvxs::Value item = pv["value"];
   const pvxs::TypeCode tc = item.type();

   switch (tc.code) {
      case pvxs::TypeCode::Bool:
         item = value.toBool();
         break;

      case pvxs::TypeCode::Int8:
      case pvxs::TypeCode::Int16:
         // range checks?
      case pvxs::TypeCode::Int32:
         item = value.toInt (&result);
         break;

      case pvxs::TypeCode::Int64:
         item = value.toLongLong (&result);
         break;

      case pvxs::TypeCode::UInt8:
      case pvxs::TypeCode::UInt16:
      case pvxs::TypeCode::UInt32:
         item = value.toUInt (&result);
         break;

      case pvxs::TypeCode::UInt64:
         item = value.toULongLong (&result);
         break;

      case pvxs::TypeCode::Float32:
      case pvxs::TypeCode::Float64:
         item = value.toDouble (&result);
         break;

      case pvxs::TypeCode::String:
         item = value.toString().toStdString();
         break;

      default:
         DEBUG << "undandled type:" << tc.name();
         result = false;
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
// static
bool QEPvaData::infuseArray (pvxs::Value& pv, const QVariant& value)
{
   pvxs::Value item = pv["value"];

   // Alas we dont have a QEVectorVariants for std string.
   //
   if (value.canConvert<QStringList>()) {
       //Extract the QStringList
       //
       const QStringList list = value.toStringList();
       const int size = list.size();

       // First we need to convert the QStringList QList of std::string;
       //
       QList<std::string>data;
       data.reserve (size);
       for (int s = 0; s < size; s++) {
          data.append (list.value(s).toStdString());
       }

       pvxs::shared_array<std::string> array (data.begin(), data.end());
       item = array.freeze();
       return true;
   }

   if (!QEVectorVariants::isVectorVariant (value)) {
      DEBUG << value << " is not a vector variant nor a string list";
      return false;
   }

   const QEVectorVariants::OwnTypes ownType = QEVectorVariants::getOwnType (value);
   switch (ownType) {
      case QEVectorVariants::DoubleVector:
         {
            const QEDoubleVector data = qvariant_cast<QEDoubleVector>(value);
            pvxs::shared_array<double> array (data.begin(), data.end());
            item = array.freeze();
         }
         break;

      case QEVectorVariants::FloatVector:
         {
            const QEFloatVector data = qvariant_cast<QEFloatVector>(value);
            pvxs::shared_array<float> array (data.begin(), data.end());
            item = array.freeze();
         }
         break;

      case QEVectorVariants::BoolVector:
         {
            const QEBoolVector data = qvariant_cast<QEBoolVector>(value);
            pvxs::shared_array<bool> array (data.begin(), data.end());
            item = array.freeze();
         }
         break;

      case QEVectorVariants::Int8Vector:
         {
            const QEInt8Vector data = qvariant_cast<QEInt8Vector>(value);
            pvxs::shared_array<int8_t> array (data.begin(), data.end());
            item = array.freeze();
         }
         break;

      case QEVectorVariants::Int16Vector:
         {
            const QEInt16Vector data = qvariant_cast<QEInt16Vector>(value);
            pvxs::shared_array<int16_t> array (data.begin(), data.end());
            item = array.freeze();
         }
         break;

      case QEVectorVariants::Int32Vector:
         {
            const QEInt32Vector data = qvariant_cast<QEInt32Vector>(value);
            pvxs::shared_array<int32_t> array (data.begin(), data.end());
            item = array.freeze();
         }
         break;

      case QEVectorVariants::Int64Vector:
         {
            const QEInt64Vector data = qvariant_cast<QEInt64Vector>(value);
            pvxs::shared_array<int64_t> array (data.begin(), data.end());
            item = array.freeze();
         }
         break;

      case QEVectorVariants::Uint8Vector:
         {
            const QEUint8Vector data = qvariant_cast<QEUint8Vector>(value);
            pvxs::shared_array<uint8_t> array (data.begin(), data.end());
            item = array.freeze();
         }
         break;

      case QEVectorVariants::Uint16Vector:
         {
            const QEUint16Vector data = qvariant_cast<QEUint16Vector>(value);
            pvxs::shared_array<uint16_t> array (data.begin(), data.end());
            item = array.freeze();
         }
         break;

      case QEVectorVariants::Uint32Vector:
         {
            const QEUint32Vector data = qvariant_cast<QEUint32Vector>(value);
            pvxs::shared_array<uint32_t> array (data.begin(), data.end());
            item = array.freeze();
         }
         break;

      case QEVectorVariants::Uint64Vector:
         {
            const QEUint64Vector data = qvariant_cast<QEUint64Vector>(value);
            pvxs::shared_array<uint64_t> array (data.begin(), data.end());
            item = array.freeze();
         }
         break;

      case QEVectorVariants::Invalid:
         DEBUG << value << " is an invalid vector type";
         return false;
         break;

      default:
         DEBUG << value << " is an unknwon vector type";
         return false;
         break;
   }

   return true;
}

//------------------------------------------------------------------------------
// static
bool QEPvaData::infuseValue (pvxs::Value& pv, const QVariant& value)
{
   bool result;

   if (pv.idStartsWith ("epics:nt/NTScalar:")) {
      result = QEPvaData::infuseScalar (pv, value);

   } else if (pv.idStartsWith ("epics:nt/NTScalarArray:")) {
      result = QEPvaData::infuseArray (pv, value);

   } else if (pv.idStartsWith ("epics:nt/NTEnum:")) {
      pv["value"]["index"] = value.toInt (&result);

   } else {
      result = false;
   }

   return result;
}

// end
