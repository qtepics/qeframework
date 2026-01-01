/*  QEPvaData.h
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

#ifndef QE_PVA_DATA_H
#define QE_PVA_DATA_H

#include <QEPvaCheck.h>
#ifdef QE_INCLUDE_PV_ACCESS

#include <QString>
#include <QStringList>
#include <QVariant>
#include <pv/pvIntrospect.h>
#include <pv/pvData.h>
#include <QEFrameworkLibraryGlobal.h>

/// This class provides the means to convert between (to and from) the PV Access
/// data "value" field and more QE friendly QVariant types.
///
/// Scalar PV data is converted to one of the standard QVariant types.
///
/// Numeric scalar array PV data is converted to one of the QE vector varient types
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
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPvaData {
public:
   // A bit of short hand
   //
   typedef epics::pvData::PVStructure::const_shared_pointer  PVStructureConstPtr;
   typedef epics::pvData::PVStructure::shared_pointer        PVStructureSharedPtr;

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
      bool extract (const PVStructureConstPtr& pv);

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
      bool extract (const PVStructureConstPtr& pv);

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
      bool extract (const PVStructureConstPtr& pv);

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
      bool extract (const PVStructureConstPtr& pv, const QString& identity);

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
      bool extract (const PVStructureConstPtr& pv);

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
      bool extract (const PVStructureConstPtr& pv);

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

   // Converts a PV Access PV value field to a QVariant.
   // This may include QE's own user defined QVariants.
   // Type is used to qualify some normative types.
   //
   static bool
   extractValue (PVStructureSharedPtr& pv, QVariant& value, QString& type);

   // The opposite of extractValue
   static bool
   infuseValue (PVStructureSharedPtr& pv, const QVariant& value);


   // Converts a PVScalar to QVariant
   // If scalarType invalid, then returned QVariant type is Invalid
   //
   static bool
   extractScalar (const epics::pvData::PVScalar::const_shared_pointer& pvScalar,
                  QVariant& value, QString& type);

   // The opposite of extractScalar
   static bool
   infuseScalar (epics::pvData::PVScalar::shared_pointer& pvScalar, const QVariant& value);


   // Converts a PVScalarArray to QVariant using QE's own defined vector variant
   // types for example: QEInt32Vector or QEDoubleVector.
   //
   static bool
   extractScalarArray (const epics::pvData::PVScalarArray::const_shared_pointer& pvArray,
                       QVariant& value, QString& type);

   // The opposite of extractScalarArray
   static bool
   infuseScalarArray (epics::pvData::PVScalarArray::shared_pointer& pvArray,
                      const QVariant& value);

   // This handles QVariantList variants.
   static bool
   infuseScalarArrayList (epics::pvData::PVScalarArray::shared_pointer& pvArray,
                          const QVariantList& value);

   // This handles own QEVector variants.
   static bool
   infuseScalarArrayVector (epics::pvData::PVScalarArray::shared_pointer& pvArray,
                            const QVariant& value);

   // Converts a PVScalarArray to a basic QVariantList and a opposed to a vector variant.
   // Used by QETableData only so far. Relocate
   //
   static QVariantList
   scalarArrayToQVariantList (const epics::pvData::PVScalarArray::const_shared_pointer& scalarArray);

private:
   explicit QEPvaData ();    // place holder
   virtual ~QEPvaData ();    // place holder
};

#endif  // QE_INCLUDE_PV_ACCESS

#endif  // QE_PVA_DATA_H
