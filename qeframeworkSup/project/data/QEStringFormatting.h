/*  QEStringFormatting.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2025 Australian Synchrotron
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
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

// Formats the string for QEString data.

#ifndef QE_STRING_FORMATTING_H
#define QE_STRING_FORMATTING_H

#include <QVariant>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QEEnums.h>
#include <QELocalEnumeration.h>
#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEStringFormatting {
public:
   // Construction/destruction
   //
   explicit QEStringFormatting ();
   ~QEStringFormatting ();

   //===============================================
   // Main functions of this class:
   //   - Format a string based on a value and array index (typically 0)
   //   - Translate a string and generate a value
   //===============================================
   //
   QString formatString (const QVariant& value, int arrayIndex) const;

   /// This is used to convert the QVariant value received from a QCaObject,
   /// which is still based on the data variable type, to a string array.
   /// Typically used where the input QVariant value is an array of data values,
   /// but will work for any QVariant type.
   QVector<QString> formatStringArray( const QVariant &value ) const;


   QVariant formatValue (const QString& text, bool& ok) const;
   QVariant formatValue (const QVector<QString>& text, bool& ok) const;


   // Functions to set up formatting information from the database
   //
   // Units to be added (or removed) from the formatted string if 'addUnits' flag is set.
   void setDbEgu (const QString egu);

   void setDbEnumerations (const QStringList enumerations);
   void setDbPrecision (const unsigned int dbPrecisionIn);

   // Functions to configure the formatting
   //
   void setPrecision (const int precision);
   void setUseDbPrecision (const bool useDbPrecision);
   void setLeadingZero (const bool leadingZero);
   void setTrailingZeros (const bool trailingZeros);
   void setForceSign (const bool forceSign);
   void setFormat (const QE::Formats format);
   void setSeparator (const QE::Separators separator);
   void setRadix (const int radix);
   void setNotation (const QE::Notations notation);
   void setArrayAction (const QE::ArrayActions arrayActionIn);
   void setAddUnits (const bool addUnits);
   void setLocalEnumeration (const QString /*localEnumerationList */ localEnumerationIn);
   void setUseRadixPrefix (const bool useRadixPrefix);
   void setLeadingZeros (const int leadingZeros);

   // Functions to read the formatting configuration
   //
   int getPrecision () const;
   bool getUseDbPrecision () const;
   bool getLeadingZero () const;
   bool getTrailingZeros () const;
   bool getForceSign () const;
   QE::Formats getFormat () const;
   QE::Separators getSeparator () const;
   unsigned int getRadix () const;
   QE::Notations getNotation () const;
   QE::ArrayActions getArrayAction () const;
   bool getAddUnits () const;
   QString getLocalEnumeration () const;
   QELocalEnumeration getLocalEnumerationObject () const;
   bool getUseRadixPrefix () const;
   int getLeadingZeros () const;

   // Primative conversion functions.
   // While these are primarily intended for use internally, they have
   // be made public as may be useful to bespoke applications and plugins.
   //
   // Provides integer and floating point number to QString.
   // Note: overloaded function names.
   //
   QString toString (const long value) const;
   QString toString (const unsigned long value) const;
   QString toString (const double value) const;

   // Provides QString to numeric type.
   // The returned value only meaningful/valid if the okay argument is set true.
   // Note: the base/radix value of the formatting object is used unless the
   // input string overrides this with a radix base identification prefix.
   // E.g. "8#dddd" for octal, "16#dddd" or "0xdddd" for hexadecimal numbers.
   // Note: 10# may be used for decimal.
   //
   long toInt (const QString& image, bool& okay) const;
   long toLong (const QString& image, bool& okay) const;
   unsigned long toULong (const QString& image, bool& okay) const;
   double toDouble (const QString& image, bool& okay) const;

private:
   // isNumeric set true iff value is numeric data.
   //
    QString formatElementString (const QVariant& value, bool& isNumeric) const;

   // Time specific conversion function.
   // The value parameter must be in seconds.
   //
   QString timeToString (const QVariant& value) const;

   // This function determines and updates dbFormat and dbFormatArray.
   //
   void determineDbFormat (const QVariant& value) const;

   // These templates are private - only instatiated internally.
   //
   template<typename Number>
   QString toIntegerStringGeneric (const Number) const;

   template<typename Number>
   Number toIntegerValueGeneric (const QString& image, bool& okay) const;

   // If the nominated notation is NOTATION_FIXED or NOTATION_SCIENTIFIC
   // this function just returns false or true respectively.
   // However, if the nominated is NOTATION_AUTOMATIC it will return true if
   // scientific notation is the most appropriate notation for the given value.
   //
   bool useScientificNotation (const double value) const;

   // Error reporting
   QString formatFailure (const QString& message) const;

   // Formatted output string
   mutable QE::Formats dbFormat; // Format determined from read value (Floating, integer, etc).
   mutable bool dbFormatArray;   // True if read value is an array

   // Database information
   QString dbEgu;
   QStringList dbEnumerations;
   unsigned int dbPrecision;

   // Formatting configuration
   bool useDbPrecision;         // Use the number of decimal places displayed as defined in the database.
   bool leadingZero;            // Add a leading zero when required.
   bool trailingZeros;          // Add trailing zeros when required (up to the precision).
   bool forceSign;              // Add "+" for numeric values >= 0
   QE::Formats format;          // Presentation required (Floating, integer, etc).
   QE::Notations notation;      // Required notation for floating point formats
   QE::Separators separator;    // Thousands separator (applies to numeric values only)
   bool addUnits;               // Flag use engineering units from database
   int precision;               // Floating point precision. Used if 'useDbPrecision' is false.
   QELocalEnumeration localEnumerations;    // Local enumeration values.
   QE::ArrayActions arrayAction;    // Action to take when processing array or waveform data
   int leadingZeros;            // number of leading zeros
   int radixBase;               // Radix value: 2 - 16
   bool useRadixPrefix;         // Only applies to non-base 10 representations
};

#endif                          // QE_STRING_FORMATTING_H
