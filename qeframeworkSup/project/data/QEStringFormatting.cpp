/*  QEStringFormatting.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2020 Australian Synchrotron
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

#include <QEStringFormatting.h>
#include <stdio.h>
#include <math.h>
#include <QDebug>
#include <QECommon.h>
#include <QEVectorVariants.h>
#include <QENTTableData.h>
#include <QENTNDArrayData.h>
#include <QEOpaqueData.h>

#define DEBUG qDebug () << "QEStringFormatting" << __LINE__ << __FUNCTION__ << "  "

/*
    Construction
*/
QEStringFormatting::QEStringFormatting() {
   // Set up the stream that will perform most conversions
   stream.setString( &outStr );

   // Set up default formatting behaviour
   useDbPrecision = true;
   precision = 4;
   leadingZero = true;
   trailingZeros = true;
   forceSign = false;
   format = FORMAT_DEFAULT;
   dbFormat = FORMAT_DEFAULT;
   separator = SEPARATOR_NONE;
   dbFormatArray = false;
   notation = NOTATION_FIXED;
   stream.setIntegerBase( 10 );
   stream.setRealNumberNotation( QTextStream::FixedNotation );
   addUnits = true;
   arrayAction = ASCII; // INDEX;

   // Initialise database information
   dbPrecision = 0;
}

QEStringFormatting::~QEStringFormatting() {}    // place holder

/*
    Set up the precision specified by the database.
    This precision is used when formatting floating point numbers if 'useDbPrecision' is set
*/
void QEStringFormatting::setDbPrecision( unsigned int dbPrecisionIn )
{
   dbPrecision = dbPrecisionIn;
}

/*
    Set up the engineering units that will be added to or removed from strings if 'addUnits' flag is set
*/
void QEStringFormatting::setDbEgu( QString egu )
{
   dbEgu = egu;
}

/*
    Set up the enumeration values. Thses are used if avaiable if the formatting is FORMAT_DEFAULT
*/
void QEStringFormatting::setDbEnumerations( QStringList enumerations )
{
   dbEnumerations = enumerations;
}

/*
    Generate a value given a string, using formatting defined within this
    class.
    If the value can be formatted the formatted value is returned and 'ok' is true.
    If the value can't be formatted an error string is returned and 'ok' is false
*/
QVariant QEStringFormatting::formatValue( const QString& text, bool& ok ) const
{
   // Init
   ok = false;
   QVariant value;

   // Strip unit if present
   QString unitlessText = text;
   if( addUnits )
   {
      if( dbEgu == unitlessText.right( dbEgu.length() ) )
         unitlessText.chop( dbEgu.length() );
   }

   // Use the requested format, unless the requested format is 'default' in
   // which case use the format determined from any value read.
   //
   formats f = format;
   if( f == FORMAT_DEFAULT )
   {
      f = dbFormat;
   }

   // Format the value if an enumerated list
   //
   const int enumCount = dbEnumerations.size();
   if( format == FORMAT_DEFAULT && (enumCount > 0) )
   {
      // If value matched an enumeration, use it
      for( int i = 0; i < dbEnumerations.size(); i++ )
      {
         if( unitlessText.compare( dbEnumerations[i] ) == 0 )
         {
            qulonglong ul = i;
            value = QVariant( ul );
            ok = true;
            return value;
         }
      }

      // Value does not match an enumeration string.
      // If value is an integer and in range >= 0 and < enumCount then
      // treats as if format is integer. This mimics the behaviour of caput.
      //
      bool intValueOk;
      const int intValue = unitlessText.toInt( &intValueOk );
      if( intValueOk && (intValue >= 0) && (intValue < enumCount) )
      {
         // Value is integer and in range - use it.
         //
         value = QVariant( intValue );
         ok = true;
         return value;
      }

      // Value does not match an enumeration string, nor is an integer value
      // in the range 0 to enumCount - 1.
      //
      ok = false;
      return QVariant();   // invalid
   }

   // Format the value if a local enumerated list
   //
   if( format == FORMAT_LOCAL_ENUMERATE && localEnumerations.isDefined() )
   {
      return localEnumerations.textToValue( text, ok );
   }

   // If formating as a single value...
   if( !dbFormatArray )
   {
      // Format the value if not enumerated
      switch( f )
      {
         case FORMAT_DEFAULT:
            {
               value = QVariant( unitlessText );
               ok = true;
            }
            break;

         case FORMAT_FLOATING:
            {
               double d = unitlessText.toDouble( &ok );
               if( ok )
               {
                  value = QVariant( d );
               }
            }
            break;

         case FORMAT_INTEGER:
            {
               qlonglong ll = unitlessText.toLongLong( &ok );
               if( ok )
               {
                  value = QVariant( ll );
               }
            }
            break;

         case FORMAT_UNSIGNEDINTEGER:
            {
               qulonglong ul = unitlessText.toULongLong( &ok );
               if( ok )
               {
                  value = QVariant( ul );
               }
            }
            break;

         case FORMAT_TIME:
            //??? to do
            value = QVariant( unitlessText );
            ok = true;
            break;

         case FORMAT_LOCAL_ENUMERATE:
            //??? to do
            value = QVariant( unitlessText );
            ok = true;
            break;

         case FORMAT_STRING:
            value = QVariant( unitlessText );
            ok = true;
            break;

         case FORMAT_NT_TABLE:
         case FORMAT_NT_IMAGE:
         case FORMAT_OPAQUE:
            ok = false;
            break;
      };
   }

   // Formating as an array...
   // Generally, just interpret the text as a single value and produce an array with a single value in it
   // For unsigned int, however, use each character as a value as EPICS records of arrays of unsigned ints are often used for strings
   // Some options don't make a lot of sense (an array of strings?)
   else
   {
      QVariantList list;
      int len = unitlessText.size();

      switch( f )
      {
         case FORMAT_DEFAULT:
            {
               for( int i = 0; i < len; i++ )
               {
                  list.append( QVariant( unitlessText[i] ));
               }

               // Zero terminate. Initially required for writing to area detector file name.
               // (Readback string included all values up to a zero which might include parts of earlier, longer, filename)
               list.append( QVariant( QChar( 0 )) );

               ok = true;
            }
            break;

         case FORMAT_FLOATING:
            {
               double d = unitlessText.toDouble( &ok );
               if( ok )
               {
                  list.append( QVariant( d ));
               }
            }
            break;

         case FORMAT_INTEGER:
            {
               qlonglong ll = unitlessText.toLongLong( &ok );
               if( ok )
               {
                  list.append( QVariant( ll ));
               }

               // Zero terminate. Initially required for writing to area detector file name.
               // (Readback string included all values up to a zero which might include parts of earlier, longer, filename)
               list.append( QVariant( (qlonglong)(0)) );

            }
            break;

         case FORMAT_UNSIGNEDINTEGER:
            {
               for( int i = 0; i < len; i++ )
               {
                  qulonglong ul = unitlessText[i].toLatin1();
                  list.append( QVariant( ul ));
               }

               // Zero terminate. Initially required for writing to area detector file name.
               // (Readback string included all values up to a zero which might include parts of earlier, longer, filename)
               list.append( QVariant( (qulonglong)(0) ) );

               ok = true;
            }
            break;

         case FORMAT_TIME:
            //??? to do
            list.append( QVariant( unitlessText ));
            ok = true;
            break;

         case FORMAT_LOCAL_ENUMERATE:
            //??? to do
            list.append( QVariant( unitlessText ));
            ok = true;
            break;

         case FORMAT_STRING:
            list.append( QVariant( unitlessText ));
            ok = true;
            break;

         case FORMAT_NT_TABLE:
         case FORMAT_NT_IMAGE:
         case FORMAT_OPAQUE:
            ok = false;
            break;
      }
      value = list;
   }
   return value;
}

/*
    Generate a value varry given an array of strings, using formatting defined
    within this class.
    If all the values can be formatted the formatted value is returned and 'ok' is true.
    If any of the values can't be formatted an error string is returned and 'ok' is false
 */
QVariant QEStringFormatting::formatValue( const QVector<QString>& text, bool& ok ) const
{
   QVariantList result;
   int n = text.count();
   ok = true;
   for( int j = 0; j < n ;j++ ){
      bool elok;
      QVariant element = formatValue( text.value( j ), elok );
      if (!elok) ok = false;
      result.append( element );
   }
   return result;
}

// Determine the format that will be used when interpreting a value to write,
// or when presenting a value for which default formatting has been requested.
void QEStringFormatting::determineDbFormat( const QVariant &value ) const
{
   // Assume default formatting, and only a single value
   dbFormat = FORMAT_DEFAULT;
   dbFormatArray = false;

   // Get the value type
   QVariant::Type t = value.type();

   // If the value is a list, get the type of the first element in the list
   if( t == QVariant::List )
   {
      // Note that whatever the format, we have an array of them
      dbFormatArray = true;

      // Get the list
      const QVariantList valueArray = value.toList();

      // If the list has anything in it, get the type of the first
      if( valueArray.count() )
      {
         t = valueArray[0].type();
      }
      else
      {
         formatFailure( QString( "Bug in QEStringFormatting::determineDefaultFormatting(). Empty array" ) );
         return;
      }
   }

   // Determine the formatting type from the variant type
   switch( t )
   {
      case QVariant::Double:
         dbFormat = FORMAT_FLOATING;
         break;

      case QVariant::LongLong:
      case QVariant::Int:
         dbFormat = FORMAT_INTEGER;
         break;

      case QVariant::ULongLong:
      case QVariant::UInt:
         dbFormat = FORMAT_UNSIGNEDINTEGER;
         break;

      case QVariant::String:
         dbFormat = FORMAT_STRING;
         break;

      case QVariant::UserType:
         if( QENTTableData::isAssignableVariant( value ) ){
            dbFormat = FORMAT_NT_TABLE;
            break;
         }
         if( QENTNDArrayData::isAssignableVariant( value ) ){
            dbFormat = FORMAT_NT_IMAGE;
            break;
         }
         if( QEOpaqueData::isAssignableVariant( value ) ){
            dbFormat = FORMAT_OPAQUE;
            break;
         }

         ///  ****** else fall through  ******
      default:
         formatFailure( QString( "QEStringFormatting::%1:%2 - unexpected QVariant type '%3' %4." )
                        .arg( __LINE__ ).arg( __FUNCTION__ )
                        .arg( value.typeName() ).arg( t ) );
         break;
   }
}

/*
    Set strem numbers flags
 */
void QEStringFormatting::applyForceSign () const
{
   QTextStream::NumberFlags nf = stream.numberFlags();

   if( forceSign ) {
      // yes - set flag
      nf = nf | QTextStream::ForceSign;
   } else {
      // no - clear flag
      nf = (nf | QTextStream::ForceSign) ^ QTextStream::ForceSign;

   }
   stream.setNumberFlags( nf );
}

/*
    Insert separators iff defined.
 */
QString QEStringFormatting::insertSeparators( const QString& image) const
{
   QString result = image;

   if( separator != SEPARATOR_NONE ){
      static const QChar sc_list [4] = { '?',  ',',  '_',  ' ' };
      const QChar sc = sc_list [separator];

      // For decimal, this is the usual thousands separator.
      const unsigned int r = getRadix();
      const int groupSize = ( r == 2 || r == 16 ) ? 4 : 3;

      int dp;

      // Insert separators before decimal point.
      dp = result.indexOf( ".", 0 );
      if( dp < 0 ) dp = result.length (); // otherwise add virtual point at end of string
      dp -= groupSize;
      while( ( dp > 0 ) && ( result [dp - 1] >= '0' )  && ( result [dp - 1] <= '9') ){
         result.insert (dp, sc);
         dp -= groupSize;
      }

      // Insert separators after decimal point.
      dp = result.indexOf( ".", 0 );
      if( dp >= 0 ) {
         int ep = result.indexOf( "e", dp );           // find exponent if it exists
         if( ep < 0 ) ep = result.indexOf( "E", dp );  // allow for either case
         if( ep < 0 ) ep = result.length ();           // otherwise start at end of string

         dp += groupSize + 1;
         while( dp < ep ){
            result.insert (dp, sc);
            dp += groupSize + 1;
            ep += 1;
         }
      }
   }

   return result;
}

/* Experimental - proof of concept.
   Creates an image of a real floating point number.
   The notation controls meaning of prec:
      when NOTATION_FIXED =>       prec is precision
      when NOTATION_SCIENTIFIC =>  prec is precision
      when NOTATION_AUTOMATIC =>   prec is significance
   When forceSign is true, result always include a leading sign '+' or '-'
   zeros sepifies the minimum number of leading zeros.
 */
QString QEStringFormatting::realImage( const double item,
                                       const notations notation,
                                       const bool forceSign,
                                       const int zeros,
                                       const int prec ) const
{
   double absValue = ABS( item );
   int tw = 0;
   char ntcode = 'f';

   switch( notation ){
      case QEStringFormatting::NOTATION_FIXED:
      default:
         ntcode = 'f';
         tw = prec > 0 ? zeros + 2 + prec : zeros + 1;
         break;

      case QEStringFormatting::NOTATION_SCIENTIFIC:
         ntcode = 'e';
         tw = prec > 0 ? zeros + 6 + prec : zeros + 5;
         break;

      case QEStringFormatting::NOTATION_AUTOMATIC:
         // g does not honor precsion, choose f or e
         ntcode = 'f';
         tw = prec > 0 ? zeros + 2 + prec : zeros + 1;
         if( absValue > 0.0 ){
            if( ( absValue < 1.0E-4 ) || ( absValue >= EXP10( prec ) ) ){
               ntcode = 'e';
               tw = prec > 0 ? zeros + 7 + prec : zeros + 5;
            }
         }
         break;
   }

   const char* sign = forceSign ? "+" : "";

   // Create actual format from format template.
   QString format;
   format.sprintf( "%c%s0%d.%d%c", '%', sign, tw, prec, ntcode );

   QString result;
   result.sprintf( format.toStdString().c_str(), item );

   return insertSeparators( result );
}

/*
    Generate a string given a value, using formatting defined within this class.
*/
QString QEStringFormatting::formatString( const QVariant& value, int arrayIndex ) const
{
   QString result;
   bool isNumeric = false;

   const int valueType = value.type();
   if( (valueType != QVariant::List) &&
       (valueType != QVariant::StringList) &&
       !QEVectorVariants::isVectorVariant(value) )
   {
      // "Simple" scalar
      result = this->formatElementString( value, isNumeric );

   } else {
      // Array variable / or vector variant.
      //
      QVariantList valueArray;
      bool okay =false;

      if( valueType == QVariant::List ){
          valueArray = value.toList();
          okay = true;

      } else if( valueType == QVariant::StringList ){
          // Convert QVariant::StringList to QVariantList of QString
          // To much conversion - refactor and tidy up.
          //
          QStringList temp = value.toStringList();
          for( int j = 0; j < temp.count(); j++ ){
              valueArray.append( QVariant( temp.value( j, "" ) ) );
          }
          okay = true;

      } else {
          // Must be a vector variant.
          valueArray = QEVectorVariants::convertToVariantList( value, okay );
      }

      if (!okay) {
         this->formatFailure( QString ( "Conversion to QVariantList failed") );
         return "---";
      }

      const int number = valueArray.count ();

      switch( arrayAction ) {

         case APPEND:
            // Interpret each element in the array as an unsigned integer and append
            // string representations of each element from the array with a space in
            // between each.
            for( int j = 0; j < number; j++ ){
               QVariant element = valueArray.value (j);
               QString elementString;
               elementString = this->formatElementString( element, isNumeric );

               if( j > 0 )result.append ( " " );
               result.append( elementString );
            }
            break;

         case ASCII:
            // Interpret each element from the array as a character in a string.
            // Translate all non printing characters to '?' except for trailing
            // zeros (ignore them)
            for( int j = 0; j < number; j++ ){
               QVariant element = valueArray.value( j );
               bool okay;

               int c = element.toInt( &okay );

               if( !okay || (c == 0) ) break;  // Not an int or got a zero - end of string.

               // Ignore carriage returns.
               // Note this will cause problems when implementing on Commodore 8-bit machines,
               // Acorn BBC, ZX Spectrum, and TRS-80 as they don't use a line feed.
               if( c == '\r' )
               {
               }
               // Translate all non printing characters (except for space and line feed) to a '?'
               else if( (c!= '\n') && (c < ' ' || c > '~') )
               {
                  result.append( "?" );
               }
               // Use everything else as is.
               else
               {
                  result.append( element.toChar() );
               }
            }
            break;

         case INDEX:
            // Interpret the element selected by setArrayIndex().
            if( ( arrayIndex >= 0 ) && ( arrayIndex < number ) )
            {
               QVariant element = valueArray.value( arrayIndex );
               result = this->formatElementString( element, isNumeric );
            }
            break;

         default:
            this->formatFailure( QString ( "Invalid arrayAction: %d" ).arg ( (int) arrayAction ));
            result = "---";
            break;
      }
   }

   // Add units if required, if there are any present, and if the text is not an error message
   // ??? Why cant this be in the 'if' statement?  If it is it never adds an egu
   //
   int eguLen = dbEgu.length();
   if( isNumeric && addUnits && (eguLen > 0) && (format != FORMAT_TIME) )
   {
      result.append( " " ).append( dbEgu );
   }

   return result;
}

/*
    Generate a string given an element value, using formatting defined within this class.
*/
QString QEStringFormatting::formatElementString( const QVariant& value, bool& isNumeric ) const
{
   // Examine the value and note the matching format
   // This sets dbFormat which is used by following switch statements
   determineDbFormat( value );

   // Initialise
   outStr.clear();

   // Set the precision
   if( useDbPrecision )
      stream.setRealNumberPrecision( dbPrecision );
   else
      stream.setRealNumberPrecision( precision );

   // Format the value as requested
   switch( format )
   {
      // Determine the format from the variant type.
      // Only the types used to store ca data are used. any other type is
      // considered a failure.
      case FORMAT_DEFAULT :
         {
            bool haveEnumeratedString = false;  // Successfully converted the value to an enumerated string
            // If a list of enumerated strings is available, attempt to get an enumerated string
            if( dbEnumerations.size() )
            {
               // Ensure the input value can be used as an index into the list of enumerated strings
               bool convertOk;
               long lValue = value.toLongLong( &convertOk );
               if( convertOk && lValue >= 0 )
               {
                  // Get the appropriate enumerated string
                  if( lValue < dbEnumerations.size() )
                  {
                     outStr = dbEnumerations[lValue];
                     haveEnumeratedString = true;
                  }
                  // NOTE: STAT field hard-coded values now set up in QCaObject.cpp - extra values appended to dbEnumerations.
               }
            }

            // If no list of enumerated strings was available, or a string could not be selected,
            // convert the value based on it's type.
            if( !haveEnumeratedString )
            {
               switch( dbFormat )
               {
                  case FORMAT_FLOATING:
                     formatFromFloating( value );
                     isNumeric = true;
                     break;

                  case FORMAT_INTEGER:
                     formatFromInteger( value );
                     isNumeric = true;
                     break;

                  case FORMAT_UNSIGNEDINTEGER:
                     formatFromUnsignedInteger( value );
                     isNumeric = true;
                     break;

                  case FORMAT_STRING:
                     formatFromString( value );
                     break;

                  case FORMAT_NT_TABLE:
                     // Can't display an NT Table as a string.
                     stream << "{{NTTable}}";
                     break;

                  case FORMAT_NT_IMAGE:
                     // Can't display an NT NDArray (image) as a string.
                     stream << "{{NTNDArray}}";
                     break;

                  case FORMAT_OPAQUE:
                     // Can't display an opaque (unknown) as a string.
                     stream << "{{opaque}}";
                     break;

                  default:
                     formatFailure( QString( "QEStringFormatting::%1 - unexpected dbFormat %2." )
                                    .arg( __FUNCTION__ ).arg ( dbFormat ) );
                     break;
               }
            }
            break;
         }

         // Format as requested, ignoring the database type
      case FORMAT_FLOATING:
         formatFromFloating( value );
         isNumeric = true;
         break;

      case FORMAT_INTEGER:
         formatFromInteger( value );
         isNumeric = true;
         break;

      case FORMAT_UNSIGNEDINTEGER:
         formatFromUnsignedInteger( value );
         isNumeric = true;
         break;

      case FORMAT_LOCAL_ENUMERATE:
         formatFromEnumeration( value );
         break;

      case FORMAT_TIME:
         formatFromTime( value );
         break;

      case FORMAT_STRING:
         formatFromString( value );
         break;

         // Don't know how to format.
         // This is a code error. All cases in QEStringFormatting::formats should be catered for
      default:
         formatFailure( QString( "Bug in QEStringFormatting::format(). The format type was not expected" ) );
         break;
   }

   return outStr;
}

/*
    Format a variant value as a string representation of a floating point
    number.
    First convert the variant value to a double. It may or may not be a
    floating point type variant. If it is - good,
    there will be no conversion problems.
    Then format it as a string using the formatting information stored in this
    class.
*/
void QEStringFormatting::formatFromFloating( const QVariant &value ) const
{
   // Extract the value as a double using whatever conversion the QVariant uses.
   //
   // Note, this will not pick up if the QVariant type is not one of the types used to represent CA data.
   // This is OK as it is not absolutely nessesary to do this sort of check at this point. Also the code is more robust as it will
   // work if the range of QVariant types used expands.
   // Note, this does not give us the freedom to specify what conversions should fail or succeed. For example, does QVariant::toDouble()
   // work if the value it holds is the string 1.234 10^6, or does it work for both - 1.234 and -1.234, and should it?
   // If QVariant::toDouble() does not do exactly what is required, a switch statement for each of the types used to hold CA data
   // will need to be added and the conversions done  manually or using QVariant::toDouble() as required.
   bool convertOk;
   double dValue = value.toDouble( &convertOk );

   if( !convertOk ) {
      formatFailure( QString( "Warning from QEStringFormatting::formatFromFloating(). A variant could not be converted to a long." ) );
      return;
   }

   // Ensure '+' sign added if requested.
   //
   applyForceSign();

   // NOTE: Smart notation (NOTATION_AUTOMATIC) does not honor real number precision.
   // So select FixedNotation or ScientificNotation as appropriate.
   //
   QTextStream::RealNumberNotation rnn = stream.realNumberNotation();
   if( rnn == QTextStream::SmartNotation ){
      int prec;
      double low_fixed_limit;
      double high_fixed_limit;
      double absDbValue;

      // Extact precision being used.
      prec = stream.realNumberPrecision ();
      prec = LIMIT( prec, 0, 15 );

      // Example, if prec = 3, when low limit is 0.01
      low_fixed_limit = EXP10( 1 - prec );

      high_fixed_limit = 1.0E+05;

      // Work with absoloute value
      absDbValue = ABS( dValue );

      if( absDbValue == 0.0 || ( absDbValue >= low_fixed_limit && absDbValue < high_fixed_limit )){
         stream.setRealNumberNotation( QTextStream::FixedNotation );
      } else {
         stream.setRealNumberNotation( QTextStream::ScientificNotation );
      }
   }

   // Generate the text
   stream << dValue;

   stream.setRealNumberNotation( rnn );  // reset

   // Remove leading zero if required
   if( !leadingZero ) {
      if( outStr.left(2) == "0." )
         outStr = outStr.right( outStr.length()-1);
   }

   // Remove trailing zeros if required, but don't leave a naked decimal point, i.e. 4.0000 becomes 4.0 (as opposed to 4.)
   // Also need to be careful to ensure 1.23000e+100 becomes 1.23e+100 (as opposed to 1.23000e+1)
   if( !trailingZeros ) {
      int dp = outStr.indexOf( ".", 0 );
      if( dp >= 0 ){
         int ep = outStr.indexOf( "e", dp );           // find exponent if it exists
         if( ep < 0 ) ep = outStr.indexOf( "E", dp );  // allow for either case
         if( ep < 0 ) ep = outStr.length ();           // otherwise start at end of string
         int sp = ep;
         while( ( sp > dp + 2 ) && ( outStr[sp - 1] == '0' ) ){
            sp--;
         }
         if( sp < ep ){
            outStr.replace( sp, ep - sp, "" );
         }
      }
   }

   // Add sperators if needs be
   outStr = insertSeparators( outStr );
}

/*
    Format a variant value as a string representation of a signed integer.
    This method was written to convert a QVariant of type LongLong, but should cope with a variant of any type.
    First convert the variant value to a long. It may or may not be a longlong type variant. If it is - good,
    there will be no conversion problems.
    Then format it as a string using the formatting information stored in this class.
*/
void QEStringFormatting::formatFromInteger( const QVariant &value ) const
{
   // Extract the value as a long using whatever conversion the QVariant uses.
   //
   // Note, this will not pick up if the QVariant type is not one of the types used to represent CA data.
   // This is OK as it is not absolutely nessesary to do this sort of check at this point. Also the code is more robust as it will
   // work if the range of QVariant types used expands.
   // Note, this does not give us the freedom to specify what conversions should fail or succeed. For example, does QVariant::toLongLong()
   // work if the value it holds is the string 1.000 and should it?
   // If QVariant::toLongLong() does not do exactly what is required, a switch statement for each of the types used to hold CA data
   // will need to be added and the conversions done  manually or using QVariant::toLongLong() as required.
   bool convertOk;
   qlonglong lValue;

   // Use QString conversions is variant is a string.
   // (QVariant toLongLong can't convert strings like "2.000"!)
   if( value.type() == QVariant::String )
   {
      QString str = value.toString();
      double dd = str.toDouble( &convertOk );
      lValue = dd;
   }

   // Use QVariant conversions otherwise
   else
   {
      lValue = value.toLongLong( &convertOk );
   }

   // Error if cant convert
   if( !convertOk )
   {
      formatFailure( QString( "Warning from QEStringFormatting::formatFromInteger(). A variant could not be converted to a long." ) );
      return;
   }

   // Generate the text
   applyForceSign ();
   stream << lValue;

   // Add sperators if needs be
   outStr = insertSeparators( outStr );
}

/*
    Format a variant value as a string representation of an unsigned integer.
    This method was written to convert a QVariant of type ULongLong, but should cope with a variant of any type.
    First convert the variant value to an unsigned long. It may or may not be a ulonglong type variant. If it is - good,
    there will be no conversion problems.
    Then format it as a string using the formatting information stored in this class.
*/
void QEStringFormatting::formatFromUnsignedInteger( const QVariant &value ) const
{
   // Extract the value as an unsigned long using whatever conversion the QVariant uses.
   //
   // Note, this will not pick up if the QVariant type is not one of the types used to represent CA data.
   // This is OK as it is not absolutely nessesary to do this sort of check at this point. Also the code is more robust as it will
   // work if the range of QVariant types used expands.
   // Note, this does not give us the freedom to specify what conversions should fail or succeed. For example, does QVariant::toULongLong()
   // work if the value it holds is the string 1.000 and should it?
   // If QVariant::toULongLong() does not do exactly what is required, a switch statement for each of the types used to hold CA data
   // will need to be added and the conversions done  manually or using QVariant::toULongLong() as required.



   // Use QString conversions is variant is a string.
   // (QVariant toLongLong can't convert strings like "2.000"!)
   bool convertOk;
   unsigned long ulValue;
   if( value.type() == QVariant::String )
   {
      QString str = value.toString();
      double dd = str.toDouble( &convertOk );
      ulValue = dd;
   }

   // Use QVariant conversions otherwise
   else
   {
      ulValue = value.toULongLong( &convertOk );
   }

   if( !convertOk )
   {
      formatFailure( QString( "Warning from QEStringFormatting::formatFromUnsignedInteger(). A variant could not be converted to an unsigned long." ) );
      return;
   }

   // Generate the text
   applyForceSign ();
   stream << ulValue;

   // Add sperators if needs be
   outStr = insertSeparators( outStr );
}

/*
    Format a variant value using local enumeration list.
    If the value is numeric, then the value is compared to the numeric interpretation of the enumeration values,
    if the value is textual, then the value is compared to the textual enumeration values.
*/
void QEStringFormatting::formatFromEnumeration( const QVariant &value ) const
{

   bool match;   // dummy needed by API
   stream << localEnumerations.valueToText( value, match );
}

/*
    Format a variant value as a string representation of time.
    This method was written to convert a QVariant of type ??? (the type used to represent times in CA),
    but should cope with a variant of any type.
*/
void QEStringFormatting::formatFromTime( const QVariant &value ) const
{
   bool okay;
   double seconds;
   double time;
   QString sign;
   int days;
   int hours;
   int mins;
   int secs;
   int nanoSecs;
   QString image;
   int effectivePrecision;
   QString fraction;

   seconds = value.toDouble( &okay );
   if (okay) {
      if (seconds >= 0.0) {
         time = seconds;
         sign= forceSign ? "+" : "";
      } else {
         time = -seconds;
         sign= "-";
      }

#define EXTRACT(item, spi) { item = int (floor (time / spi)); time = time - (spi * item); }

      EXTRACT (days, 86400.0);
      EXTRACT (hours, 3600.0);
      EXTRACT (mins, 60.0);
      EXTRACT (secs, 1.0);
      EXTRACT (nanoSecs, 1.0E-9);

#undef EXTRACT

      // Include days field if rquired or if requested.
      //
      if ((days > 0) || leadingZero) {
         image.sprintf ("%d %02d:%02d:%02d", days, hours, mins, secs);
      } else {
         image.sprintf ("%02d:%02d:%02d", hours, mins, secs);
      }

      // Select data base or user precision as appropriate.
      //
      effectivePrecision = useDbPrecision ? dbPrecision : precision;
      if (effectivePrecision > 9) effectivePrecision = 9;

      if (effectivePrecision > 0) {
         fraction.sprintf (".%09d", nanoSecs);
         fraction.truncate( effectivePrecision + 1 );
      } else {
         fraction = "";
      }

      stream << sign << image << fraction;

   } else {
      formatFailure( QString( "Warning from QEStringFormatting::formatFromTime(). A variant could not be converted to a double." ) );
   }
}

/*
    Format a variant value as a string representation of a string. (Not a big ask!)
*/
void QEStringFormatting::formatFromString( const QVariant& value ) const
{
   // Generate the text
   stream << value.toString(); // No conversion requried. Stored in variant as required type
}

/*
    Do something with the fact that the value could not be formatted as
    requested.
*/
void QEStringFormatting::formatFailure( QString message ) const
{
   // Log the format failure if required.
   //???
   qDebug() << message;

   // Return whatever is required for a formatting falure.
   stream << "---";
}

//========================================================================================
// 'Set' formatting configuration methods

/*
    Set the precision - the number of significant digits displayed when
    formatting a floating point number.
    Relevent when formatting the string as a floating point number.
    Note, this will only be used if 'useDbPrecision' is false.
*/
void QEStringFormatting::setPrecision( int precisionIn ) {
   precision = precisionIn;

   // Ensure range is sensible.
   //
   if (precision < 0) precision = 0;
   if (precision > 18) precision = 18;
}

/*
    Set or clear a flag to ignore the precision held by this class and use the
    precision read from the database record.
    The precision is the number of significant digits displayed when formatting
    a floating point number.
    Relevent when formatting the string as a floating point number.
*/
void QEStringFormatting::setUseDbPrecision( bool useDbPrecisionIn ) {
   useDbPrecision = useDbPrecisionIn;
}

/*
    Set or clear a flag to display a leading zero before a decimal point for
    floating point numbers between 1 and -1
    Relevent when formatting the string as a floating point number.
*/
void QEStringFormatting::setLeadingZero( bool leadingZeroIn ) {
   leadingZero = leadingZeroIn;
}

/*
    Set or clear a flag to display trailing zeros at the end up the precision for
    floating point
    Relevent when formatting the string as a floating point number.
*/
void QEStringFormatting::setTrailingZeros( bool trailingZerosIn ) {
   trailingZeros = trailingZerosIn;
}

/*
    Set or clear flag to force + sign on numeric values.
 */
void QEStringFormatting::setForceSign( bool forceSignIn ) {
   forceSign = forceSignIn;
}

/*
    Set the type of information being displayed (floating point number,
    date/time, etc).
*/
void QEStringFormatting::setFormat( formats formatIn ) {
   format = formatIn;
}

/*
    Set the tousands separator
 */
void QEStringFormatting::setSeparator( const separators separatorIn )
{
   separator = separatorIn;
}

/*
    Set the numer system base.
    Relevent when formatting the string as an interger of floating point
    number.
    Any radix of 2 or more is accepted. Check the conversion code that uses
    this number to see what values are expected.
    At the time of writing (16/2/9) it is anticipated that floating point
    numbers will always be base 10 and integer numbers will
    be base 2, 8, 10, or 16.
*/
void QEStringFormatting::setRadix( const int radix ) {
   stream.setIntegerBase( LIMIT( radix, 2, 16 ) );
}

/*
    Set the notation (floating, scientific, or automatic)
    Note, the notation enumerations match the QTextStream RealNumberNotation values
*/
void QEStringFormatting::setNotation( notations notationIn ) {
   notation = notationIn;
   stream.setRealNumberNotation( (QTextStream::RealNumberNotation)notation );
}

/*
    Set how arrays are converted to text (Treates as an array of ascii characters, an array of values, etc)
*/
void QEStringFormatting::setArrayAction( arrayActions arrayActionIn ) {
   arrayAction = arrayActionIn;
}

/*
    Set or clear a flag to include the engineering units in a string
*/
void QEStringFormatting::setAddUnits( bool AddUnitsIn ) {
   addUnits = AddUnitsIn;
}

/*
    Set the string used to specify local enumeration.
    This is used when a value is to be enumerated and the value is either not the VAL field,
    or the database does not provide any enumeration, or the database enumeration is not appropriate
*/
void QEStringFormatting::setLocalEnumeration( QString/*localEnumerationList*/ localEnumerationIn ) {

   localEnumerations.setLocalEnumeration( localEnumerationIn );
}


//========================================================================================
// 'Get' formatting configuration methods

/*
    Get the precision. See setPrecision() for the use of 'precision'.
*/
int QEStringFormatting::getPrecision() const {
   return precision;
}

/*
    Get the 'use DB precision' flag. See setUseDbPrecision() for the use of the
    'use DB precision' flag.
*/
bool QEStringFormatting::getUseDbPrecision() const {
   return useDbPrecision;
}

/*
    Get the 'leading zero' flag. See setLeadingZero() for the use of the
    'leading zero' flag.
*/
bool QEStringFormatting::getLeadingZero() const {
   return leadingZero;
}

/*
    Get the 'trailing zeros' flag. See setTrailingZeros() for the use of the
    'trailing zeros' flag.
*/
bool QEStringFormatting::getTrailingZeros() const {
   return trailingZeros;
}

/*
    Get or clear flag to force + sign on numeric values.
 */
bool QEStringFormatting::getForceSign() const {
   return forceSign;
}

/*
    Get the type of information being formatted. See setFormat() for the use of
    the format type.
*/
QEStringFormatting::formats QEStringFormatting::getFormat() const {
   return format;
}

/*
    Get the thousands separator. See setSeparator for the use of
    the separator type.
 */
QEStringFormatting::separators QEStringFormatting::getSeparator () const
{
   return separator;
}

/*
    Get the numerical base. See setRadix() for the use of 'radix'.
*/
unsigned int QEStringFormatting::getRadix() const {
   return stream.integerBase();
}

/*
    Return the floating point notation
*/
QEStringFormatting::notations QEStringFormatting::getNotation() const {
   return notation;
}

/*
    Return the action to take when formatting an array (treat as ascii characters, a series of numbers, etc)
*/
QEStringFormatting::arrayActions QEStringFormatting::getArrayAction() const {
   return arrayAction;
}

/*
    Get the 'include engineering units' flag.
*/
bool QEStringFormatting::getAddUnits() const {
   return addUnits;
}

/*
    Get the local enumeration strings. See setLocalEnumeration() for the use of 'localEnumeration'.
*/
QString/*localEnumerationList*/ QEStringFormatting::getLocalEnumeration() const {

   return localEnumerations.getLocalEnumeration();

   /* was returning regenerated localEumeration string
    QString s;
    int i;
    for( i = 0; i < localEnumeration.size(); i++ )
    {
        s.append( localEnumeration[i].value ).append( "=" );
        if( localEnumeration[i].text.contains( ' ' ) )
        {
            s.append( "\"" ).append( localEnumeration[i].text ).append( "\"" );
        }
        else
        {
            s.append( localEnumeration[i].text );
        }
        if( i != localEnumeration.size()+1 )
        {
            s.append( "," );
        }
    }
    return s;
*/
}

QELocalEnumeration QEStringFormatting::getLocalEnumerationObject() const
{
   return localEnumerations;
}

// end
