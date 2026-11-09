/*  QEPvaData_PVXS.h
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

#ifndef QE_PVA_DATA_PVXS_H
#define QE_PVA_DATA_PVXS_H

#include <cstdint>
#include <string>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <pvxs/sharedArray.h>
#include <pvxs/data.h>

#include <QEFrameworkLibraryGlobal.h>

// Note: keep QEPvaData_PVXS and QEPvaData_PVA aligned AFARP.
//
/// This class provides the means to convert between (to and from) the PVXS
/// PV Access data "value" field and more QE friendly QVariant types.
///
/// Scalar PV data is converted to one of the standard QVariant types.
///
/// Numeric scalar array PV data is converted to one of the QE vector variant types
/// defined in QEVariants; String scalar array PV data is converted to a QStringList
/// variant.
///
/// NTTable structure PV data is forwarded to QETableData for conversion.
/// NT Image structure PV data is still to be processed.
///
/// Other than the "value" field, this class also provides the means to extract other
/// standard fields, i.e. "alarm", "timeStamp", "display", "control" and "valueAlarm".
/// To this end, the QEPvaData class provides a number of inner classes to represent
/// these fields a QE freindly fashion.
///
/// Note: enum data is split into an index "value" and a "choices" standard field.
///

// Short-hand for brevity/clarity
//
using QESharedStringArray = pvxs::shared_array<const std::string>;

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPvaData {
public:
   // Note: the structures member names, apart from isDefined, exactly match
   // the Normative Type field names. We make use of this with some pre-porcessor
   // macros used to perform the extract fuctions.
   //
   //---------------------------------------------------------------------------
   //
   class Enumerated {
   public:
      explicit Enumerated ();
      ~Enumerated ();

      // Assign the members of other to this instance.
      // isMetaUpdate indicates if this is a meta data update.
      //
      void assign (const Enumerated& other, bool& isMetaUpdate);

      // Extract field data if available and return if successful extracted.
      // If fails, class instance is interdeminate.
      //
      bool extract (const pvxs::Value& pv);

      //  bool extract (const pvxs::Value& pv);
      bool isDefined;
      int index;
      QStringList choices;
   };

   //---------------------------------------------------------------------------
   //
   class Alarm {
   public:
      explicit Alarm ();
      ~Alarm ();
      void assign (const Alarm& other);

      // Extract alarm field from the pv data if available and then populate
      // the severity, status and message class instance members.
      // The function returns true if all data is successfully extracted.
      // If it fails, class instance is interdeminate.
      //
      bool extract (const pvxs::Value& pv);

      bool isDefined;
      int severity;
      int status;
      QString message;
   };

   //---------------------------------------------------------------------------
   //
   class TimeStamp {
   public:
      explicit TimeStamp ();
      ~TimeStamp ();
      void assign (const TimeStamp& other);

      // Extract timeStamp field from the pv data if available and then populate
      // the secondsPastEpoch, nanoseconds and userTag class instance members.
      // The function returns true if all data is successfully extracted.
      // If it fails, class instance is interdeminate.
      //
      bool extract (const pvxs::Value& pv);

      bool isDefined;
      qlonglong secondsPastEpoch;
      int nanoseconds;
      int userTag;
   };

   //---------------------------------------------------------------------------
   //
   class Display {
   public:
      explicit Display ();
      ~Display ();

      void assign (const Display& other, bool& isMetaUpdate);

      // Extract display field from the pv data if available and then populate
      // the limitLow, limitHigh, description etc. class instance members.
      // The function returns true if all data is successfully extracted.
      // If it fails, class instance is interdeminate.
      // Note, some display substructures are different (e.g. NTNDArray) so
      // we also need the pv identity, e.g. "epics:nt/NTNDArray:1.0", to
      // fine tune what is extracted from the display structure.
      //
      bool extract (const pvxs::Value& pv);

      bool isDefined;
      double limitLow;
      double limitHigh;
      QString description;
      QString units;
      int precision;
      // QString format;  - replaced by form - this is TBD
   };

   //---------------------------------------------------------------------------
   //
   class Control {
   public:
      explicit Control ();
      ~Control ();

      void assign (const Control& other, bool& isMetaUpdate);

      // Extract control field from the pv data if available and then populate
      // the limitLow, limitHigh and minStep class instance members.
      // The function returns true if all data is successfully extracted.
      // If it fails, class instance is interdeminate.
      //
      bool extract (const pvxs::Value& pv);

      bool isDefined;
      double limitLow;
      double limitHigh;
      double minStep;
   };

   //---------------------------------------------------------------------------
   // While in pvData 'world' the alarm/warning limits are of the appropriate type
   // in QE 'world' this are all represented as double.
   //
   class ValueAlarm {
   public:
      explicit ValueAlarm ();
      ~ValueAlarm ();

      void assign (const ValueAlarm& other, bool& isMetaUpdate);

      // Extract valueAlarm field from the pv data if available and then populate
      // the lowAlarmLimit, lowWarningLimit etc. class instance members.
      // The function returns true if all data is successfully extracted.
      // If it fails, class instance is interdeminate.
      //
      // Note: irrespective of the native type, the limit values are caste
      // to double.
      //
      bool extract (const pvxs::Value& pv);

      bool isDefined;
      bool active;
      double lowAlarmLimit;
      double lowWarningLimit;
      double highWarningLimit;
      double highAlarmLimit;
      int lowAlarmSeverity;
      int lowWarningSeverity;
      int highWarningSeverity;
      int highAlarmSeverity;
      int hysteresis;    // since 7.0.3 this is now byte for all scalar/scalarArray types.
   };

   // Convertys  value to a string if we can.
   // API mimics the pvxs::Value.as functions.
   //
   static bool asQString (const pvxs::Value& pv, QString& value);

   // Converts a PV Access PVXS value field to a QVariant.
   // This may include QE's own user defined QVariants.
   // Type is used to qualify some normative types.
   //
   static bool
   extractValue (const pvxs::Value& pv, QVariant& value, QString& type);

   // New names
   // assignPvToVariant
   // assignVariantToPv

   // assignScalarPvToVariant
   // assignScalarVariantToPv

   // assignArrayPvToVariant
   // assignArrayVariantToPv

   // The opposite of extractValue.
   //
   static bool
   infuseValue (pvxs::Value& pv, const QVariant& value);

   // Converts a std::string shared array to a string list.
   //
   static QStringList toStringList (const QESharedStringArray& source);

   // Converts a ScalarArray to a basic QVariantList and a opposed to a
   // vector variant. Used by QETableData only so far.
   //
   static QVariantList
   scalarArrayToQVariantList (const pvxs::Value& scalarArray);

private:
   explicit QEPvaData ();    // place holder
   virtual ~QEPvaData ();    // place holder

   static bool
   extractScalar (const pvxs::Value& pv, QVariant& value, QString& type);

   static bool
   extractArray (const pvxs::Value& pv, QVariant& value, QString& type);

   static bool
   infuseScalar (pvxs::Value& pv, const QVariant& value);

   static bool
   infuseArray (pvxs::Value& pv, const QVariant& value);
};

#endif  // QE_PVA_DATA_PVXS_H
