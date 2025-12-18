/*  QEIntegerFormatting.cpp
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

// Provides textual formatting for QEInteger data.

#include "QEIntegerFormatting.h"
#include <QDebug>
#include <QMetaType>
#include <QEPlatform.h>
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
   const QMetaType::Type vtype = QEPlatform::metaType (value);

   if( vtype == QMetaType::QVariantList )
   {
      const QVariantList list = value.toList();

      if (arrayIndex >= 0 && arrayIndex < list.count()) {
         const QVariant element = list.value (arrayIndex);
         result = this->varToLong ( element );
      } else {
         result = this->formatFailure ("array index out of range" );
      }

   } else if( QEVectorVariants::isVectorVariant( value ) ){
      // This is one of our vector variants.
      //
      result = QEVectorVariants::getIntegerValue ( value, arrayIndex, 0 );

   } else if( vtype == QMetaType::QStringList ){
      // This is a string list
      //
      const QStringList list = value.toStringList();
      if (arrayIndex >= 0 && arrayIndex < list.count()) {
         const QVariant element = list.value (arrayIndex);
         result = this->varToLong ( element );
      } else {
         result = this->formatFailure ("array index out of range" );
      }

   } else {
      // Otherwise is a simple scalar or non convertable type.
      result = this->varToLong ( value );
   }

   return result;
}

//------------------------------------------------------------------------------
// Generate an integer array given a value,
// using formatting defined within this class.
//
QVector<long> QEIntegerFormatting::formatIntegerArray( const QVariant &value ) const
{
   QVector<long> result;

   // If the value is a list, populate a list, converting each of the
   // items to a long.
   //
   const QMetaType::Type vtype = QEPlatform::metaType (value);
   if( vtype == QMetaType::QVariantList )
   {
      const QVariantList list = value.toList();
      for( int i=0; i < list.count(); i++ )
      {
         const QVariant element = list.value (i);
         result.append( this->varToLong ( element ) );
      }
   }

   else if( QEVectorVariants::isVectorVariant( value ) ){
      // This is one of our vectors variant.
      // We can convert to a QVector<long>
      //
      bool okay;  // dummy
      result = QEVectorVariants::convertToIntegerVector ( value, okay );

   } else if( vtype == QMetaType::QStringList ){
      // This is a string list
      //
      const QStringList list = value.toStringList();
      for( int i=0; i < list.count(); i++ )
      {
         const QVariant element = list.value (i);
         result.append( this->varToLong ( element ) );
      }

   } else  {
      // The value is not a list/vector so build a list with a single long.
      //
      result.append( this->varToLong ( value ) );
   }

   return result;
}

//------------------------------------------------------------------------------
// Wrapper to toLongLong with error report.
// QVariant provides a toLongLong function, but not a toLong function with
// validation, i.e. out of range.
//
long QEIntegerFormatting::varToLong (const QVariant& item ) const
{
   const QString name = item.typeName();

   bool okay;
   qlonglong temp = 0;
   temp = item.toLongLong ( &okay );
   if ( !okay ) {
      return this->formatFailure (name + " to long conversion failure" );
   }

   if ((temp < LONG_MIN) || (temp > LONG_MAX)) {
      return this->formatFailure (name + " out of range" );
   }

   return long (temp);
}

//------------------------------------------------------------------------------
// Do something with the fact that the value could not be formatted as requested.
//
long QEIntegerFormatting::formatFailure( const QString& message ) const
{
   // Log the format failure if required.
   qDebug() << "QEIntegerFormatting" << message;

   // Return whatever is required for a formatting falure.
   return 0;
}

// end
