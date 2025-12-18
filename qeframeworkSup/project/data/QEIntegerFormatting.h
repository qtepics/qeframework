/*  QEIntegerFormatting.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2009-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

// Provides integer (long) values for QEInteger data.

#ifndef QE_INTEGER_FORMATTING_H
#define QE_INTEGER_FORMATTING_H

#include <QString>
#include <QVariant>

#include <QEFrameworkLibraryGlobal.h>

/*!
  This class holds formatting instructions and uses them to convert to/from an
  integer and a QVariant of any type.
  It is generally set up with it's formatting instructions and then passed to a
  QEInteger class that will sink and source integer data to widgets or other code.
  It is used to convert data to and from a QCaObject (which sources and sinks data
  in the form of a QVariant where the QVariant reflects the underlying variable
  data type) and the QEInteger class.
  An example of a requirement for integer data is a combo box which must
  determine an integer index to select a menu option.
 */
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEIntegerFormatting {
public:

   // Construction
   explicit QEIntegerFormatting();
   ~QEIntegerFormatting();

   //===============================================
   // Main functions of this class:
   //   - Format a double based on a value
   //   - Translate a double and generate a value
   //===============================================
   /// This is used to convert the QVariant value received from a QCaObject,
   /// which is still based on the data variable type, to an integer.
   long formatInteger( const QVariant &value ) const;
   long formatInteger( const QVariant &value, int arrayIndex ) const;

   /// This is used to convert the QVariant value received from a QCaObject,
   /// which is still based on the data variable type, to an integer array.
   /// Typically used where the input QVariant value is an array of data values,
   /// but will work for any QVariant type.
   QVector<long> formatIntegerArray( const QVariant &value ) const;

   /// Given an integer value, format it as a data value of the specified type,
   /// according to the formatting instructions held by the class.
   /// This is used when writing integer data to a QCaObject.
   QVariant formatValue( const long &integerValue) const;
   QVariant formatValue( const QVector<long> &integerValue ) const;

private:
   // Utility function to convert variant to long.
   long varToLong( const QVariant& item ) const;

   // Error reporting
   long formatFailure( const QString& message ) const;
};

#endif // QE_INTEGER_FORMATTING_H
