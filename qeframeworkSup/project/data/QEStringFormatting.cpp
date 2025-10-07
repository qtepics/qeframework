/*  QEStringFormatting.cpp
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
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#include "QEStringFormatting.h"
#include <stdio.h>
#include <math.h>
#include <limits>
#include <QDebug>
#include <QMetaType>
#include <QEPlatform.h>
#include <QECommon.h>
#include <QEPlatform.h>
#include <QEVectorVariants.h>
#include <QENTTableData.h>
#include <QENTNDArrayData.h>
#include <QEOpaqueData.h>

#define DEBUG qDebug () << "QEStringFormatting" << __LINE__ << __FUNCTION__ << " "

// InternalFormats are an extention of Formats used in QEStringFormatting
// and are specials for specific PVA varient types.
// These "formats" are not selectable from within designer.
//
enum InternalFormats {
   NTTable = QE::String + 1,   ///< Format as a NTTable
   NTNDImage,                  ///< Format as a NTNDArray
   PVAOpaque                   ///< Format as opaque, i.e. unknown/unhandled type.
};


//------------------------------------------------------------------------------
// Construction
//
QEStringFormatting::QEStringFormatting ()
{
   // Set up default formatting behaviour
   this->useDbPrecision = true;
   this->precision = 4;
   this->leadingZero = true;
   this->trailingZeros = true;
   this->forceSign = false;
   this->format = QE::Default;
   this->dbFormat = QE::Default;
   this->separator = QE::NoSeparator;
   this->dbFormatArray = false;
   this->notation = QE::Fixed;
   this->radixBase = 10;
   this->addUnits = true;
   this->arrayAction = QE::Ascii;
   this->useRadixPrefix = true;       // only applies to not base 10
   this->leadingZeros = 1;

   // Initialise database information, i.e. meta daya received via CA/PVA.
   this->dbPrecision = 0;
   this->dbEnumerations.clear();
   this->dbFormatArray = false;
}

//------------------------------------------------------------------------------
//
QEStringFormatting::~QEStringFormatting () {}    // place holder

//------------------------------------------------------------------------------
// Set up the precision as specified by the database.
// This precision is used when formatting floating point numbers
// if 'useDbPrecision' is set
//
void QEStringFormatting::setDbPrecision (const unsigned int dbPrecisionIn)
{
   this->dbPrecision = dbPrecisionIn;
}

//------------------------------------------------------------------------------
// Set up the engineering units that will be added to or removed from
// strings if 'addUnits' flag is set.
//
void QEStringFormatting::setDbEgu (const QString eguIn)
{
   this->dbEgu = eguIn;
}

//----------------------------------------------------------------------------
// Set up the enumeration values. Thses are used if avaiable if the formatting
// is QE::Default
//
void QEStringFormatting::setDbEnumerations (const QStringList enumerationsIn)
{
   this->dbEnumerations = enumerationsIn;
}

//------------------------------------------------------------------------------
// Generate a value given a string, using formatting defined within this class.
// If the value can be formatted the formatted value is returned and 'ok' is true.
// If the value can't be formatted an error string is returned and 'ok' is false
//
QVariant QEStringFormatting::formatValue (const QString& text, bool& ok) const
{
   // Init
   ok = false;
   QVariant value;

   // Strip unit if present
   QString unitlessText = text;
   if (this->addUnits) {
      if (this->dbEgu == unitlessText.right (this->dbEgu.length ()))
         unitlessText.chop (this->dbEgu.length ());
   }

   // Use the requested format, unless the requested format is 'default' in
   // which case use the format determined from any value read.
   //
   QE::Formats fmt = this->format;
   if (fmt == QE::Default) {
      fmt = this->dbFormat;
   }

   // Format the value if an enumerated list
   //
   const int enumCount = this->dbEnumerations.size ();
   if ((this->format == QE::Default) && (enumCount > 0)) {
      // If value matched an enumeration, use it
      for (int i = 0; i < this->dbEnumerations.size (); i++) {
         if (unitlessText.compare (this->dbEnumerations[i]) == 0) {
            qulonglong ul = i;
            value = QVariant (ul);
            ok = true;
            return value;
         }
      }

      // Value does not match an enumeration string.
      // If value is an integer and in range >= 0 and < enumCount then
      // treats as if format is integer. This mimics the behaviour of caput.
      //
      bool intValueOk;
      const long intValue = this->toLong (unitlessText, intValueOk);
      if (intValueOk && (intValue >= 0) && (intValue < enumCount)) {
         // Value is integer and in range - use it.
         //
         value = QVariant (int(intValue));
         ok = true;
         return value;
      }
      // Value does not match an enumeration string, nor is an integer value
      // in the range 0 to enumCount - 1.
      //
      ok = false;
      return QVariant ();       // invalid
   }

   // Format the value if a local enumerated list
   //
   if ((this->format == QE::LocalEnumeration) &&
       this->localEnumerations.isDefined ()) {
      return localEnumerations.textToValue (text, ok);
   }

   // If formating as a single value...
   //
   if (!this->dbFormatArray) {
      // Format the value if not enumerated
      switch (fmt) {
         case QE::Default:
            {
               value = QVariant (unitlessText);
               ok = true;
            }
            break;

         case QE::Floating:
            {
               double d = this->toDouble (unitlessText, ok);
               if (ok) {
                  value = QVariant (d);
               }
            }
            break;

         case QE::Integer:
            {
               qlonglong ll = this->toLong (unitlessText, ok);
               if (ok) {
                  value = QVariant (ll);
               }
            }
            break;

         case QE::UnsignedInteger:
            {
               qulonglong ul = this->toULong (unitlessText, ok);
               if (ok) {
                  value = QVariant (ul);
               }
            }
            break;

         case QE::Time:
            // ??? to do
            value = QVariant (unitlessText);
            ok = true;
            break;

         case QE::LocalEnumeration:
            //??? to do
            value = QVariant (unitlessText);
            ok = true;
            break;

         case QE::String:
            value = QVariant (unitlessText);
            ok = true;
            break;

         default:
            ok = false;
            break;
      };
   }

   // Formating as an array...
   // Generally, just interpret the text as a single value and produce an array
   // with a single value in it.  For unsigned int, however, use each character
   // as a value as EPICS records  of arrays of unsigned ints are often used for
   // strings.  Some options don't make a lot of sense (an array of strings?)
   else {
      QVariantList list;
      int len = unitlessText.size ();

      switch (fmt) {
         case QE::Default:
            {
               for (int i = 0; i < len; i++) {
                  list.append (QVariant (unitlessText[i]));
               }

               // Zero terminate. Initially required for writing to area detector file name.
               // (Readback string included all values up to a zero which might include parts of earlier, longer, filename)
               list.append (QVariant (QChar (0)));

               ok = true;
            }
            break;

         case QE::Floating:
            {
               double d = this->toDouble (unitlessText, ok);
               if (ok) {
                  list.append (QVariant (d));
               }
            }
            break;

         case QE::Integer:
            {
               qlonglong ll = this->toLong (unitlessText, ok);
               if (ok) {
                  list.append (QVariant (ll));
               }

               // Zero terminate. Initially required for writing to area detector file name.
               // (Readback string included all values up to a zero which might
               // include parts of earlier, longer, filename).
               //
               list.append (QVariant ((qlonglong) (0)));

            }
            break;

         case QE::UnsignedInteger:
            {
               for (int i = 0; i < len; i++) {
                  qulonglong ul = unitlessText[i].toLatin1 ();
                  list.append (QVariant (ul));
               }

               // Zero terminate. Initially required for writing to area detector file name.
               // (Readback string included all values up to a zero which might include parts of earlier, longer, filename)
               list.append (QVariant ((qulonglong) (0)));

               ok = true;
            }
            break;

         case QE::Time:
            //??? to do
            list.append (QVariant (unitlessText));
            ok = true;
            break;

         case QE::LocalEnumeration:
            //??? to do
            list.append (QVariant (unitlessText));
            ok = true;
            break;

         case QE::String:
            list.append (QVariant (unitlessText));
            ok = true;
            break;

         default:
            ok = false;
            break;
      }
      value = list;
   }
   return value;
}

//------------------------------------------------------------------------------
// Generate a value varry given an array of strings, using formatting defined
// within this class.
// If all the values can be formatted the formatted value is returned and 'ok' is true.
// If any of the values can't be formatted an error string is returned and 'ok' is false
//
QVariant QEStringFormatting::formatValue (const QVector<QString>& text, bool& ok) const
{
   QVariantList result;
   int n = text.count ();
   ok = true;
   for (int j = 0; j < n; j++) {
      bool elok;
      QVariant element = this->formatValue (text.value (j), elok);
      if (!elok)
         ok = false;
      result.append (element);
   }
   return result;
}

//------------------------------------------------------------------------------
// Determine the format that will be used when interpreting a value to write,
// or when presenting a value for which default formatting has been requested.
//
void QEStringFormatting::determineDbFormat (const QVariant& value) const
{
   // Assume default formatting, and only a single value
   // Note: these two members are mutable.
   //
   this->dbFormat = QE::Default;
   this->dbFormatArray = false;

   // Get the value type

   QMetaType::Type vtype = QEPlatform::metaType (value);

   // If the value is a list, get the type of the first element in the list
   //
   if (vtype == QMetaType::QVariantList) {
      // Note that whatever the format, we have an array of them
      this->dbFormatArray = true;

      // Get the list
      const QVariantList valueArray = value.toList ();

      // If the list has anything in it, get the type of the first.
      //
      if (valueArray.count() > 0) {
         vtype = QEPlatform::metaType (valueArray[0]);
      } else {
         formatFailure (QString ("determineDefaultFormatting(). Empty array"));
         return;
      }
   }

   // Determine the formatting type from the variant type
   //
   switch (vtype) {
      case QMetaType::Double:
      case QMetaType::Float:
         this->dbFormat = QE::Floating;
         break;

      case QMetaType::LongLong:
      case QMetaType::Int:
         // Could be an ENUM
         this->dbFormat = QE::Integer;
         break;

      case QMetaType::ULongLong:
      case QMetaType::UInt:
         this->dbFormat = QE::UnsignedInteger;
         break;

      case QMetaType::QString:
         dbFormat = QE::String;
         break;

      default:
         if (vtype >= QMetaType::User) {
            if (QENTTableData::isAssignableVariant (value)) {
               this->dbFormat = static_cast<QE::Formats>(NTTable);
               break;
            }
            if (QENTNDArrayData::isAssignableVariant (value)) {
               this->dbFormat = static_cast<QE::Formats>(NTNDImage);
               break;
            }
            if (QEOpaqueData::isAssignableVariant (value)) {
               this->dbFormat = static_cast<QE::Formats>(PVAOpaque);
               break;
            }
         } else {
            this->formatFailure (QString
                                 ("%1:%2 - unexpected QVariant type '%3' %4.")
                                 .arg (__LINE__).arg (__FUNCTION__)
                                 .arg (value.typeName ()).arg (vtype));
         }
         break;
   }
}

//------------------------------------------------------------------------------
// Generate a string given a value, using formatting defined within this class.
//
QString QEStringFormatting::formatString (const QVariant& value, int arrayIndex) const
{
   QString result;
   bool isNumeric = false;

   const QMetaType::Type valueType = QEPlatform::metaType (value);
   if ((valueType != QMetaType::QVariantList) &&
       (valueType != QMetaType::QStringList) &&
       !QEVectorVariants::isVectorVariant (value)) {
      // "Simple" scalar
      result = this->formatElementString (value, isNumeric);

   } else {
      // Array variable / or vector variant.
      //
      QVariantList valueArray;
      bool okay = false;

      if (valueType == QMetaType::QVariantList) {
         valueArray = value.toList ();
         okay = true;

      } else if (valueType == QMetaType::QStringList) {
         // Convert QVariant::StringList to QVariantList of QString
         // To much conversion - refactor and tidy up.
         //
         QStringList temp = value.toStringList ();
         for (int j = 0; j < temp.count (); j++) {
            valueArray.append (QVariant (temp.value (j, "")));
         }
         okay = true;

      } else {
         // Must be a vector variant.
         valueArray = QEVectorVariants::convertToVariantList (value, okay);
      }

      if (!okay) {
         return this->formatFailure (QString ("Conversion to QVariantList failed"));
      }

      const int number = valueArray.count ();

      switch (this->arrayAction) {

         case QE::Append:
            // Interpret each element in the array as an unsigned integer and append
            // string representations of each element from the array with a space in
            // between each.
            //
            for (int j = 0; j < number; j++) {
               QVariant element = valueArray.value (j);
               QString elementString;
               elementString = this->formatElementString (element, isNumeric);

               if (j > 0)
                  result.append (" ");
               result.append (elementString);
            }
            break;

         case QE::Ascii:
            // Interpret each element from the array as a character in a string.
            // Translate all non printing characters to '?' except for trailing
            // zeros (ignore them)
            //
            for (int j = 0; j < number; j++) {
               QVariant element = valueArray.value (j);
               bool okay;

               int c = element.toInt (&okay);

               if (!okay || (c == 0))
                  break;        // Not an int or got a zero - end of string.

               // Ignore carriage returns.
               // Note this will cause problems when implementing on Commodore 8-bit machines,
               // Acorn BBC, ZX Spectrum, and TRS-80 as they don't use a line feed.
               if (c == '\r') {
               }
               // Translate all non printing characters (except for space and line feed) to a '?'
               else if ((c != '\n') && ((c < ' ') || (c > '~'))) {
                  result.append ("?");
               }
               // Use everything else as is.
               else {
                  result.append (element.toChar ());
               }
            }
            break;

         case QE::Index:
            // Interpret the element selected by setArrayIndex().
            if ((arrayIndex >= 0) && (arrayIndex < number)) {
               QVariant element = valueArray.value (arrayIndex);
               result = this->formatElementString (element, isNumeric);
            }
            break;

         default:
            result = this->formatFailure (QString ("Invalid arrayAction: %1")
                                          .arg ((int) this->arrayAction));
            break;
      }
   }

   // Add units if required, if there are any present, and if the text is not an error message
   // ??? Why cant this be in the 'if' statement?  If it is it never adds an egu
   //
   int eguLen = dbEgu.length ();
   if (isNumeric && this->addUnits && (eguLen > 0) && (this->format != QE::Time)) {
      result.append (" ").append (dbEgu);
   }

   return result;
}

//------------------------------------------------------------------------------
// Generate a string given an element value, using formatting defined within
// this class.
//
QString QEStringFormatting::formatElementString (const QVariant& value,
                                                 bool&isNumeric) const
{
   QString result;
   bool okay = true;
   double d;
   long l;
   unsigned long u;

   isNumeric = false;

   // Examine the value and note the matching format
   // This sets dbFormat which is used by following switch statements
   //
   this->determineDbFormat (value);

   // Format the value as requested
   //
   switch (this->format) {
         // Determine the format from the variant type.
         // Only the types used to store ca data are used. any other type is
         // considered a failure.
      case QE::Default:
         {
            bool haveEnumeratedString = false;  // Successfully converted the value to an enumerated string
            // If a list of enumerated strings is available, attempt to get an enumerated string
            if (this->dbEnumerations.size ()) {
               // Ensure the input value can be used as an index into the list of enumerated strings
               bool convertOk;
               long lValue = value.toLongLong (&convertOk);
               if (convertOk && lValue >= 0) {
                  // Get the appropriate enumerated string
                  if (lValue < dbEnumerations.size ()) {
                     result = dbEnumerations[lValue];
                     haveEnumeratedString = true;
                  }
                  // NOTE: STAT field hard-coded values now set up in QCaObject.cpp - extra values appended to dbEnumerations.
               }
            }

            // If no list of enumerated strings was available, or a string
            // could not be selected, convert the value based on it's type.
            //
            if (!haveEnumeratedString) {
               // avoid the warning related to mixed enums usage
               //
               const int iFormat = int (this->dbFormat);
               switch (iFormat) {
                  case QE::Floating:
                     d = value.toDouble(&okay);
                     result = this->toString (d);
                     isNumeric = true;
                     break;

                  case QE::Integer:
                     l = value.toLongLong (&okay);
                     result = this->toString (l);
                     isNumeric = true;
                     break;

                  case QE::UnsignedInteger:
                     u = value.toULongLong (&okay);
                     result = this->toString (u);
                     isNumeric = true;
                     break;

                  case QE::String:
                     result = value.toString ();
                     break;

                 case NTTable:
                     // Can't display an NT Table as a string.
                     //
                     result = "{{NTTable}}";
                     break;

                 case NTNDImage:
                     // Can't display an NT NDArray (image) as a string.
                     //
                     result = "{{NTNDArray}}";
                     break;

                  case PVAOpaque:
                     // Can't display an opaque (unknown) as a string.
                     //
                     result = "{{opaque}}";
                     break;

                  default:
                     okay = false;
                     result = formatFailure (QString ("%1 - unexpected dbFormat %2.")
                                            .arg (__FUNCTION__).arg (iFormat));
                     break;
               }
            }
            break;
         }

         // Format as requested, ignoring the database type
      case QE::Floating:
         d = value.toDouble (&okay);
         result = this->toString (d);
         isNumeric = true;
         break;

      case QE::Integer:
         l = value.toLongLong (&okay);
         result = this->toString (l);
         isNumeric = true;
         break;

      case QE::UnsignedInteger:
         u = value.toULongLong (&okay);
         result = this->toString (u);
         isNumeric = true;
         break;

      case QE::LocalEnumeration:
         result = this->localEnumerations.valueToText (value, okay);
         break;

      case QE::Time:
         result = this->timeToString (value);
         break;

      case QE::String:
         result = value.toString ();
         // formatFromString (value);
         break;

      default:
         // Don't know how to format.  This is a code error.
         // All cases in QEStringFormatting::formats should be catered for.
         //
         okay = false;
         result = this->formatFailure (QString ("Bug in formatElementString."
                                                " The format type %1 was not expected")
                                       .arg(this->format));
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
// Format a variant value as a string representation of time.
// This is always in decimal, the format is: [days] HH:MM:SS[.FRACTION]
// This method was written to convert any numeric QVariant of type that
// can be converted to a double.
//
QString QEStringFormatting::timeToString (const QVariant& value) const
{
   QString result;
   bool okay;

   double seconds = value.toDouble (&okay);
   if (okay) {
      // Select data base or user precision as appropriate, nand ensure sensible.
      //
      int effectivePrecision = this->useDbPrecision ? this->dbPrecision : this->precision;
      effectivePrecision = LIMIT (effectivePrecision, 0, 9);

      result = QEUtilities::intervalToString (seconds, effectivePrecision, true);
   } else {
      // Just go with the raw string value.
      //
      result = value.toString();
      DEBUG << "Variant" << result << "could not be converted to a double.";
   }

   return result;
}

//------------------------------------------------------------------------------
// Do something with the fact that the value could not be formatted as
// requested.
//
QString QEStringFormatting::formatFailure (const QString message) const
{
   // Log the format failure if required.
   //
   qDebug () << "QEStringFormatting" << message;

   // Return whatever is required for a formatting falure.
   // Something needs to be displayed by a QELabel or similar widget.
   //
   return "---";
}

//==============================================================================
// 'Set' formatting configuration methods
//==============================================================================
//
// Set the precision - the number of significant digits displayed when
// formatting a floating point number.
// Relevent when formatting the string as a floating point number.
// Note, this will only be used if 'useDbPrecision' is false.
//
void QEStringFormatting::setPrecision (const int precisionIn)
{
   // Ensure range is sensible.
   //
   this->precision = LIMIT (precisionIn, 0, 64);
}

//------------------------------------------------------------------------------
// Set or clear a flag to ignore the precision held by this class and use the
// precision read from the database record.
// The precision is the number of significant digits displayed when formatting
// a floating point number.
// Relevent when formatting the string as a floating point number.
//
void QEStringFormatting::setUseDbPrecision (const bool useDbPrecisionIn)
{
   this->useDbPrecision = useDbPrecisionIn;
}

//------------------------------------------------------------------------------
// Set or clear a flag to display a leading zero before a decimal point for
// floating point numbers between 1 and -1
// Relevent when formatting the string as a floating point number.
//
void QEStringFormatting::setLeadingZero (const bool leadingZeroIn)
{
   this->leadingZero = leadingZeroIn;
}

//------------------------------------------------------------------------------
//
void QEStringFormatting::setLeadingZeros (const int leadingZerosIn)
{
   this->leadingZeros = LIMIT (leadingZerosIn, 0, 64);
}

//------------------------------------------------------------------------------
// Set or clear a flag to display trailing zeros at the end up the precision
// for floating point
// Relevent when formatting the string as a floating point number.
//
void QEStringFormatting::setTrailingZeros (const bool trailingZerosIn)
{
   this->trailingZeros = trailingZerosIn;
}

//------------------------------------------------------------------------------
// Set or clear flag to force + sign on numeric values.
//
void QEStringFormatting::setForceSign (const bool forceSignIn)
{
   this->forceSign = forceSignIn;
}

//------------------------------------------------------------------------------
// Set the type of information being displayed (floating point number,
// date/time, etc).
//
void QEStringFormatting::setFormat (const QE::Formats formatIn)
{
   this->format = formatIn;
}

//------------------------------------------------------------------------------
// Set the "thousands" separator.
//
void QEStringFormatting::setSeparator (const QE::Separators separatorIn)
{
   this->separator = separatorIn;
}

//------------------------------------------------------------------------------
// Set the numer system base.
// Relevent when formatting the string as an interger of floating point number.
// Any radix of 2 or more is accepted. Check the conversion code that uses
// this number to see what values are expected.
// At time of writing (08/04/2022) the radix now applies to both integer and
// floating point numbers; and while all bases in the range 2 to 16 are allowed,
// the expected radix bases are 10, 2, 8 and 16.
//
void QEStringFormatting::setRadix (const int radix)
{
   this->radixBase = LIMIT (radix, 2, 16);
}

//------------------------------------------------------------------------------
// Controls if the number has a radix prefix, e.g. "2#", "8#" or "0x".
// Note: numbers represented in decimal are never preceeded by "10#".
//
void QEStringFormatting::setUseRadixPrefix (const bool useRadixPrefixIn)
{
   this->useRadixPrefix = useRadixPrefixIn;
}

//------------------------------------------------------------------------------
// Set the notation (floating, scientific, or automatic)
//
void QEStringFormatting::setNotation (const QE::Notations notationIn)
{
   this->notation = notationIn;
}

//------------------------------------------------------------------------------
// Set how arrays are converted to text (Treates as an array of ascii characters,
// an array of values, etc)
//
void QEStringFormatting::setArrayAction (const QE::ArrayActions arrayActionIn)
{
   this->arrayAction = arrayActionIn;
}

//------------------------------------------------------------------------------
// Set or clear a flag to include the engineering units in a string
//
void QEStringFormatting::setAddUnits (const bool AddUnitsIn)
{
   this->addUnits = AddUnitsIn;
}

//------------------------------------------------------------------------------
// Set the string used to specify local enumeration.
// This is used when a value is to be enumerated and the value is either not
// the VAL field, or the database does not provide any enumeration, or the
// database enumeration is not appropriate.
//
void QEStringFormatting::setLocalEnumeration (const QString localEnumerationIn)
{
   this->localEnumerations.setLocalEnumeration (localEnumerationIn);
}

//==============================================================================
// 'Get' formatting configuration methods
//==============================================================================
//
// Get the precision. See setPrecision() for the use of 'precision'.
//
int QEStringFormatting::getPrecision () const
{
   return this->precision;
}

//------------------------------------------------------------------------------
// Get the 'use DB precision' flag. See setUseDbPrecision() for the use of the
// 'use DB precision' flag.
//
bool QEStringFormatting::getUseDbPrecision () const
{
   return this->useDbPrecision;
}

//------------------------------------------------------------------------------
// Get the 'leading zero' flag. See setLeadingZero() for the use of the
// 'leading zero' flag.
// Deprecated
//
bool QEStringFormatting::getLeadingZero () const
{
   return this->leadingZero;
}

//------------------------------------------------------------------------------
// Get the 'trailing zeros' flag. See setTrailingZeros() for the use of the
// 'trailing zeros' flag.
//
bool QEStringFormatting::getTrailingZeros () const
{
   return this->trailingZeros;
}

//------------------------------------------------------------------------------
// Get or clear flag to force + sign on numeric values.
//
bool QEStringFormatting::getForceSign () const
{
   return this->forceSign;
}

//------------------------------------------------------------------------------
// Get the type of information being formatted. See setFormat() for the use of
// the format type.
//
QE::Formats QEStringFormatting::getFormat () const
{
   return this->format;
}

//------------------------------------------------------------------------------
// Get the thousands separator. See setSeparator for the use of
// the separator type.
//
QE::Separators QEStringFormatting::getSeparator () const
{
   return this->separator;
}

//------------------------------------------------------------------------------
// Get the numerical base. See setRadix() for the use of 'radix'.
//
unsigned int QEStringFormatting::getRadix () const
{
   return this->radixBase;
}

//------------------------------------------------------------------------------
// Return the floating point notation
//
QE::Notations QEStringFormatting::getNotation () const
{
   return notation;
}

//------------------------------------------------------------------------------
// Return the action to take when formatting an array (treat as ascii characters,
// a series of numbers, etc)
//
QE::ArrayActions QEStringFormatting::getArrayAction () const
{
   return arrayAction;
}

//------------------------------------------------------------------------------
// Get the 'include engineering units' flag.
//
bool QEStringFormatting::getAddUnits () const
{
   return addUnits;
}

//------------------------------------------------------------------------------
// Get the local enumeration strings. See setLocalEnumeration() for
// the use of 'localEnumeration'.
//
QString QEStringFormatting::getLocalEnumeration () const
{
   return localEnumerations.getLocalEnumeration ();
}

//------------------------------------------------------------------------------
//
QELocalEnumeration QEStringFormatting::getLocalEnumerationObject () const
{
   return localEnumerations;
}

//------------------------------------------------------------------------------
//
bool QEStringFormatting::getUseRadixPrefix () const
{
   return this->useRadixPrefix;
}

//------------------------------------------------------------------------------
//
int QEStringFormatting::getLeadingZeros () const
{
   return this->leadingZeros;
}

//==============================================================================
// General purpose value to/from string in any radix
//==============================================================================
//
static const char radixChars[] = "0123456789ABCDEF";

// Must be consistant with enum Separators specification.
//
const static char separatorChars[] = "?,_ ";

// The "thousand" gaps/spacing.
// entry [0] and [1] are place holders.
//
static const int separatorGaps[17] = {
   -1, -1,
   /* 2  => */ 8, 5, 5, 5, 5, 5,
   /* 8  => */ 5, 5,
   /* 10 => */ 3, 5, 5, 5, 5, 5,
   /* 16 => */ 4
};

//------------------------------------------------------------------------------
//
bool QEStringFormatting::useScientificNotation (const double value) const
{
   if (this->notation == QE::Fixed) return false;
   if (this->notation == QE::Scientific) return true;

   // Pick best/most approptiate notation based on the value.
   // This is athe same logic as in formatFromFloating
   // Extact precision being used.
   //
   const int prec = LIMIT (this->precision, 0, 15);

   // Example, if prec = 3, when low limit is 0.01
   //
   const double lowFixedLimit = EXP10 (1 - prec);
   const double highFixedLimit = 1.0E+05;

   // Work with the absolute value
   //
   const double absValue = ABS (value);
   const bool inFixedLimits = (absValue >= lowFixedLimit) &&
                              (absValue < highFixedLimit);

   return (absValue != 0.0) && !inFixedLimits;
}

//------------------------------------------------------------------------------
// We could template floating to/from string if ever needs be.
//
QString QEStringFormatting::toString (const double value) const
{
   // Some of these could be pre-computed once.
   //
   const QString sign = (value < 0.0) ? "-" : (this->forceSign ? "+" : "");
   const int zeros = LIMIT (this->leadingZeros, 0, 64);
   const char sepChar = separatorChars[this->separator];
   const int gap = (this->separator == QE::NoSeparator) ? -1 : separatorGaps[this->radixBase];

   const int prec = LIMIT (this->useDbPrecision ? this->dbPrecision : this->precision, 0, 64);

   // Sanity checks/specials.
   //
   if (QEPlatform::isNaN (value)) {
      return "nan";
   }

   if (QEPlatform::isInf (value)) {
      return sign + "inf";
   }

   QString result = "";

   // Do leading sign if needed or requested.
   //
   result.append (sign);

   // Is a radix prefix required?
   //
   if (this->useRadixPrefix && (this->radixBase != 10)) {
      // Yes - do prefix.
      //
      if (this->radixBase == 16) {
         result.append ("0x");
      } else {
         result.append (QString ("%1#").arg (this->radixBase));
      }
   }

   double work = ABS (value);   // working value
   const double dblRadix = this->radixBase;

   if (this->useScientificNotation (value)) {
      // Scientifc representation.
      //
      int exponent = 0;

      if (work != 0.0) {
         // Non-zero value - normalise the value.
         //
         if (work >= dblRadix) {
            while (work >= dblRadix) {
               work /= dblRadix;
               exponent += 1;
            }
         } else if (work < 1.0) {
            while (work < 1.0) {
               work *= dblRadix;
               exponent -= 1;
            }
         }
         // now:  1.0 <= value < 10.0 in the nominated base (unless is zero).

         // Round up by half the value of the least significant digit.
         //
         const double roundUp = pow ((1.0 / dblRadix), prec) * 0.5;
         work = work + roundUp;

         // Check if the round up pushed us into the next radix-decade?
         //
         if (work >= dblRadix) {
            work /= dblRadix;
            exponent += 1;
         }
      }

      // Leading zeros
      for (int j = zeros; j >= 2; j--) {
         if ((gap > 0) && ((j % gap) == 0) && (j < zeros)) {
            result.append (sepChar);
         }
         result.append ('0');
      }

      int r = int (work);   // rounds down towards zero
      result.append (radixChars[r]);

      if (prec > 0) {
         result.append ('.');
         for (int j = 1; j <= prec; j++) {
            work = dblRadix * (work - r);
            r = int (work);
            result.append (radixChars[r]);

            if ((gap > 0) && ((j % gap) == 0) && (j < prec)) {
               result.append (sepChar);
            }
         }
      }

      // Now do the exponent.
      // Be consistant with toFloatingValue re selection of e vs. p
      //
      char xp[20];
      if (this->radixBase >= 11) {
         snprintf (xp, sizeof (xp), "p%+03d", exponent);
      } else {
         snprintf (xp, sizeof (xp), "e%+03d", exponent);
      }
      result.append (xp);

   } else {
      // Fixed point representation.
      //
      // Round up by half the value of the least significant digit.
      //
      const double roundUp = pow ((1.0 / dblRadix), prec) * 0.5;
      work = work + roundUp;

      // Find most significant digit position.
      // Units are 0, tens are 1, etc.
      //
      int mostSig = 0;
      if (work >= dblRadix) {
         double temp = work;
         while (temp >= dblRadix) {
            temp /= dblRadix;
            mostSig += 1;
         }
      }

      // Alternative - how expensive is pow?
      // for (int t = dblRadix; t < work; t = t*dblRadix) {
      //    mostSig++;
      // }

      mostSig = MAX (mostSig, zeros - 1);
      for (int n = mostSig; n >= -prec; n--) {
         double prs = pow (dblRadix, n);
         int r = int (floor (work / prs));
         work = work - r * prs;

         result.append (radixChars[r]);

         // All done?
         //
         if (n <= -prec)
            break;

         if (n == 0) {
            result.append ('.');
         } else if ((gap > 0) && (ABS (n) % gap) == 0) {
            result.append (sepChar);
         }
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
double QEStringFormatting::toDouble (const QString& image, bool& okay) const
{
   const char sepChar = separatorChars[this->separator];

   okay = false;                // hypothesize no good until we know better
   double result = 0.0;

   QString work = image.trimmed ();
   int workBase = this->radixBase;
   int sign = +1;

   // Extract sign if any.
   //
   if (work.startsWith ('+')) {
      work = work.remove (0, 1);

   } else if (work.startsWith ('-')) {
      work = work.remove (0, 1);
      sign = -1;
   }

   // Extract base/radix if specified
   //
   if (work.startsWith ("0x")) {
      workBase = 16;
      work = work.remove (0, 2);
   } else {
      for (int b = 2; b <= 16; b++) {
         QString test = QString ("%1#").arg (b);
         if (work.startsWith (test)) {
            workBase = b;
            work = work.remove (0, b < 10 ? 2 : 3);
            break;
         }
      }
   }

   bool atLeastOne = false;
   bool weHaveFoundAPoint = false;
   double scale = 1.0;
   int exponent = 0;

   for (int j = 0; j < work.length (); j++) {
      const char c = work[j].toLatin1 ();
      int digit = 0;

      if (c == sepChar)
         continue;              // ignore multiple separators for now.

      if (c == '.') {
         if (weHaveFoundAPoint) {
            // multiple points
            return 0.0;
         }
         weHaveFoundAPoint = true;
         continue;
      }

      // In hex (and base 15) we can't use E or e to introduce the
      // exponent as E/e is a valid hex digit.
      // We use p/P for bases > 10  e.g: 0xABD.DEFp-05
      // The use of p/P is consistant with how python addresses this feo base 16.
      // How woolly/flexible should we be?
      //
      const bool isE = ((c == 'e') || (c == 'E'));      // standard
      const bool isP = ((c == 'p') || (c == 'P'));      // alternative
      if ((isE && (workBase < 11)) || (isP && (workBase >= 11))) {
         bool ok;
         exponent = work.mid (j + 1).toInt (&ok);
         if (!ok) {
            return 0.0;
         }
         break;
      }

      if ((c >= '0') && (c <= '9')) {
         digit = int (c) - int ('0');

      } else if ((c >= 'A') && (c <= 'F')) {
         digit = int (c) - int ('A') + 10;

      } else if ((c >= 'a') && (c <= 'f')) {
         digit = int (c) - int ('a') + 10;

      } else {
         return 0.0;            // non-digit
      }

      if ((0 <= digit) && (digit < workBase)) {
         if (!weHaveFoundAPoint) {
            // Ante "decimal" point.
            result = (workBase * result) + digit;
         } else {
            // Post "decimal" point.
            scale /= workBase;
            result = result + (digit * scale);
         }
         atLeastOne = true;
      } else {
         return 0.0;            // not base digit
      }
   }

   // Scale and sign result.
   //
   result = sign * result * pow (workBase, exponent);
   okay = atLeastOne;
   return result;
}

//------------------------------------------------------------------------------
//
template<typename Number>
QString QEStringFormatting::toIntegerStringGeneric (const Number value) const
{
   // Some of these could be pre-computed once.
   //
   const QString sign = (value < 0) ? "-" : (this->forceSign ? "+" : "");
   const int zeros = LIMIT (this->leadingZeros, 0, 64);
   const char sepChar = separatorChars[this->separator];
   const int gap = (this->separator == QE::NoSeparator) ? -1 : separatorGaps[this->radixBase];

   // Big enough for a 64 bit integer on base 2 plus separators, sign and nn#
   // Given zeros are allowed upto 64, then 86 required for decimal.
   // Plus some for any extra I haven't thought about.
   //
   char work[96];
   int p = sizeof (work);
   work[--p] = '\0';            // Fill work in backwards

   int n = 0;                   // Number of digits so far - excluding separators
   Number t = value;              // Working value
   do {
      Number q = t / this->radixBase;
      Number r = t % this->radixBase;

      if (r < 0)
         r = -r;

      work[--p] = radixChars[r];
      n++;

      // Add the separator char if needs be.
      //
      if ((gap > 0) && (n % gap == 0))
         work[--p] = sepChar;

      t = q;
   } while ((t != 0 || n < zeros) && (p > 0));

   if (work[p] == sepChar) {
      p++;                      // no leading separator wanted
   }

   // Do we need to add the radix prefix?
   //
   if (this->useRadixPrefix && (this->radixBase != 10)) {
      // Do the prefix, note the special for base 16.
      //
      if (this->radixBase == 16) {
         // We can't but the 0x directly in the string as %1 becomes %10
         //
         return QString ("%1%2%3").arg (sign).arg ("0x").arg (&work[p]);
      } else {
         return QString ("%1%2#%3").arg (sign).arg (this->radixBase).arg (&work[p]);
      }
   }

   // No radix prefix
   //
   return QString ("%1%2").arg (sign).arg (&work[p]);
}

//------------------------------------------------------------------------------
//
template<typename Number>
Number QEStringFormatting::toIntegerValueGeneric (const QString& image, bool& okay) const
{
   const char sepChar = separatorChars[this->separator];

   okay = false;                // hypothesize no good until we know better
   Number result = 0;

   QString work = image.trimmed ();
   int workBase = this->radixBase;
   int sign = +1;

   // Extract sign if any.
   //
   if (work.startsWith ('+')) {
      work = work.remove (0, 1);

   } else if (work.startsWith ('-')) {
      work = work.remove (0, 1);
      sign = -1;
   }

   // Extract base/radix if specificied
   //
   if (work.startsWith ("0x")) {
      workBase = 16;
      work = work.remove (0, 2);
   } else {
      for (int b = 2; b <= 16; b++) {
         QString test = QString ("%1#").arg (b);
         if (work.startsWith (test)) {
            workBase = b;
            work = work.remove (0, b < 10 ? 2 : 3);
            break;
         }
      }
   }

   // Find min (moost negative) and max (most postive) values so that
   // we can detect overflow.
   //
   static const Number maxValue = std::numeric_limits <Number>::max ();
   static const Number minValue = std::numeric_limits <Number>::min ();

   bool atLeastOne = false;
   for (int j = 0; j < work.length (); j++) {
      const char c = work[j].toLatin1 ();
      int digit = 0;

      if (c == sepChar)
         continue;              // ignore multiple separators for now.

      if ((c >= '0') && (c <= '9')) {
         digit = int (c) - int ('0');

      } else if ((c >= 'A') && (c <= 'F')) {
         digit = int (c) - int ('A') + 10;

      } else if ((c >= 'a') && (c <= 'f')) {
         digit = int (c) - int ('a') + 10;

      } else {
         return 0;              // none digit
      }

      if ((0 <= digit) && (digit < workBase)) {
         // Check going out of range.
         //
         if (sign > 0) {
            if (result > (maxValue - digit) / workBase) {
               // out of +ve range
               return 0;
            }
            result = (workBase * result) + digit;
         } else {
            if (result < (minValue + digit) / workBase) {
               // out of -ve range
               return 0;
            }
            result = (workBase * result) - digit;
         }

         atLeastOne = true;
      } else {
         // non radix digit
         return 0;
      }
   }

   okay = atLeastOne;
   return result;
}

//------------------------------------------------------------------------------
//
QString QEStringFormatting::toString (const long value) const
{
   return this->toIntegerStringGeneric <long> (value);
}

//------------------------------------------------------------------------------
//
QString QEStringFormatting::toString (const unsigned long value) const
{
   return this->toIntegerStringGeneric <unsigned long> (value);
}

//------------------------------------------------------------------------------
//
long QEStringFormatting::toInt (const QString& image, bool& okay) const
{
   return this->toIntegerValueGeneric<int>(image, okay);
}

//------------------------------------------------------------------------------
//
long QEStringFormatting::toLong (const QString& image, bool& okay) const
{
   return this->toIntegerValueGeneric<long>(image, okay);
}

//------------------------------------------------------------------------------
//
unsigned long QEStringFormatting::toULong (const QString& image, bool& okay) const
{
   return this->toIntegerValueGeneric<unsigned long>(image, okay);
}

// end
