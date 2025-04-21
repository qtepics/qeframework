/*  QEFloatingFormatting.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2024 Australian Synchrotron
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

// Provides textual formatting for QEFloating data.

#include "QEFloatingFormatting.h"
#include <QDebug>
#include <QMetaType>
#include <QEPlatform.h>
#include <QEVectorVariants.h>

#define DEBUG qDebug () << "QEFloatingFormatting" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// Dummy constructor/destructor
//
QEFloatingFormatting::QEFloatingFormatting() { }

//------------------------------------------------------------------------------
//
QEFloatingFormatting::~QEFloatingFormatting() { }

//------------------------------------------------------------------------------
// Generate a value given a floating point number, using formatting defined
// within this class.
// The formatting mainly applies if formatting as a string. For example, was is
// the number base? should a sign always be included? are leading zeros requried?
// The formatting could include properties related to other types. For example, generate
// an error if attempting to convert a negative floating point number to an unsigned integer.
//
QVariant QEFloatingFormatting::formatValue( const double &floatingValue) const
{
   return QVariant (floatingValue);
}

//------------------------------------------------------------------------------
// Generate a value given an array of floating point number, using formatting
// defined within this class.
//
QVariant QEFloatingFormatting::formatValue( const QVector<double> &floatingValue) const
{
   QList<QVariant> array;
   int arraySize = floatingValue.size();
   for( int i = 0; i < arraySize; i++ )
   {
      array.append( formatValue( floatingValue[i] ));
   }
   return array;
}

//------------------------------------------------------------------------------
// Generate an floating point number given a value, using formatting defined
// within this class. The value may be an array of variants or a single variant.
//
double QEFloatingFormatting::formatFloating( const QVariant &value ) const
{
   return formatFloating( value, 0 );
}

//------------------------------------------------------------------------------
//
double QEFloatingFormatting::formatFloating( const QVariant &value,
                                             const int arrayIndex ) const
{
   double result;

   // If the value is a list, get the specified item from the list.
   // Otherwise, just use the value as is.
   //
   const QMetaType::Type vtype = QEPlatform::metaType (value);
   if( vtype == QMetaType::QVariantList )
   {
      const QVariantList list = value.toList();

      if (arrayIndex >= 0 && arrayIndex < list.count()) {
         const QVariant element = list.value (arrayIndex);
         result = varToDouble ( element );
      } else {
         result = formatFailure ("array index out of range" );
      }

   } else if( QEVectorVariants::isVectorVariant( value ) ){
      // This is one of our vector variants.
      //
      result = QEVectorVariants::getDoubleValue ( value, arrayIndex, 0.0 );
   }
   else
   {
      result = varToDouble ( value );
   }

   return result;
}

//------------------------------------------------------------------------------
// Generate an floating point number array given a value, using formatting
// defined within this class.
//
QVector<double> QEFloatingFormatting::formatFloatingArray( const QVariant &value )  const
{
   bool okay = true;
   QVector<double> result;

   // If the value is a list, populate a list, converting each of the
   // items to a double.
   //
   const QMetaType::Type vtype = QEPlatform::metaType (value);
   if( vtype == QMetaType::QVariantList )
   {
      const QVariantList list = value.toList();
      for( int i=0; i < list.count(); i++ )
      {
         const QVariant element = list.value (i);
         bool elementOkay;
         result.append( element.toDouble( &elementOkay ) );
         okay &= elementOkay;
      }
   }
   else if( QEVectorVariants::isVectorVariant( value ) ){

      // This is one of our vectors variant.
      // We can convert to a QVector<double>
      //
      result = QEVectorVariants::convertToFloatingVector (value, okay);

   } else {
      // The value is not a list/vector so build a list with a single double.
      //
      result.append( value.toDouble( &okay ) );
   }

   return result;
}

//------------------------------------------------------------------------------
// Wrapper to toDouble with error report.
//
double QEFloatingFormatting::varToDouble( const QVariant& item ) const
{
   const QString name = item.typeName();

   bool okay;
   double temp;
   temp = item.toDouble ( &okay );
   if ( !okay ) {
      return formatFailure (name + " to double conversion failure" );
   }

   return double (temp);
}

//------------------------------------------------------------------------------
// Do something with the fact that the value could not be formatted as requested.
//
double QEFloatingFormatting::formatFailure( QString message ) const
{
   // Log the format failure if required.
   qDebug() << "QEFloatingFormatting" << message;

   // Return whatever is required for a formatting falure.
   return 0.0;
}

// end
