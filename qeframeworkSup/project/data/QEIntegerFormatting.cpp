/*  QEIntegerFormatting.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2023 Australian Synchrotron
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

// Provides textual formatting for QEInteger data.

#include "QEIntegerFormatting.h"
#include <QDebug>
#include <QEVectorVariants.h>

#define DEBUG qDebug () << "QEIntegerFormatting" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// Dummy constructor/destructor
//
QEIntegerFormatting::QEIntegerFormatting() { }

//------------------------------------------------------------------------------
//
QEIntegerFormatting::~QEIntegerFormatting() { }

//------------------------------------------------------------------------------
// Generate a value given an integer, using formatting defined within this class.
// The formatting mainly applies if formatting as a string. For example, what is
// the number base? should a sign always be included? are leading zeros requried?
// The formatting could include properties related to other types. For example, generate
// an error if attempting to convert a negative integer to an unsigned integer.
//
QVariant QEIntegerFormatting::formatValue( const long &integerValue) const
{
   // Qt has a hidden Long variant type which we do not use.
   return QVariant (qlonglong (integerValue));
}

//------------------------------------------------------------------------------
// Generate a value given an array of integer numbers, using formatting defined within this class.
// The formatting mainly applies if formatting as a string. For example, what is
// the number base? should a sign always be included? are leading zeros requried?
// The formatting could include properties related to other types. For example, generate
// an error if attempting to convert a negative integer to an unsigned integer.
//
QVariant QEIntegerFormatting::formatValue( const QVector<long> &integerValue) const
{
   QList<QVariant> array;
   int arraySize = integerValue.size();
   for( int i = 0; i < arraySize; i++ )
   {
      array.append( formatValue( integerValue[i] ));
   }
   return array;
}

//------------------------------------------------------------------------------
// Generate an integer given a value, using formatting defined within this class.
// The value may be an array of variants or a single variant
//
long QEIntegerFormatting::formatInteger( const QVariant &value ) const
{
   return formatInteger( value, 0 );
}

//------------------------------------------------------------------------------
//
long QEIntegerFormatting::formatInteger( const QVariant &value,
                                         const int arrayIndex ) const
{
   long result;

   // If the value is a list, get the specified item from the list.
   // Otherwise, just use the value as is.
   //
   if( value.type() == QVariant::List )
   {
      const QVariantList list = value.toList();

      if (arrayIndex >= 0 && arrayIndex < list.count()) {
         const QVariant element = list.value (arrayIndex);
         result = varToLong ( element );
      } else {
         result = formatFailure ("array index out of range" );
      }
   }

   else if( QEVectorVariants::isVectorVariant( value ) ){
      // This is one of our vector variants.
      //
      result = QEVectorVariants::getIntegerValue ( value, arrayIndex, 0 );

   } else {
      // Otherwise is a scaler or non convertable type.
      result = varToLong ( value );
   }

   return result;
}

//------------------------------------------------------------------------------
// Generate an integer array given a value, using formatting defined within this class.
//
QVector<long> QEIntegerFormatting::formatIntegerArray( const QVariant &value ) const
{
   QVector<long> result;

   // If the value is a list, populate a list, converting each of the
   // items to a long.
   //
   if( value.type() == QVariant::List )
   {
      const QVariantList list = value.toList();
      for( int i=0; i < list.count(); i++ )
      {
         const QVariant element = list.value (i);
         result.append( varToLong ( element ) );
      }
   }

   else if( QEVectorVariants::isVectorVariant( value ) ){

      // This is one of our vectors variant.
      // We can convert to a QVector<long>
      //
      bool okay;
      result = QEVectorVariants::convertToIntegerVector ( value, okay );

   } else  {
      // The value is not a list/vector so build a list with a single long.
      //
      result.append( varToLong ( value ) );
   }

   return result;
}

//------------------------------------------------------------------------------
// QVariant provides a toLongLong function, but not a toLong function with
//  valiation, i.e. out of range.
//
long QEIntegerFormatting::varToLong (const QVariant& item ) const
{
   const QString name = item.typeName();

   bool okay;
   qlonglong temp;
   temp = item.toLongLong ( &okay );
   if ( !okay ) {
      return formatFailure (name + " to long conversion failure" );
   }

   if (temp < LONG_MIN || temp > LONG_MAX) {
      return formatFailure (name + " out of range" );
   }

   return long (temp);
}

//------------------------------------------------------------------------------
// Do something with the fact that the value could not be formatted as requested.
//
long QEIntegerFormatting::formatFailure( QString message ) const
{
   // Log the format failure if required.
   qDebug() << "QEIntegerFormatting" << message;

   // Return whatever is required for a formatting falure.
   return 0;
}

// end
