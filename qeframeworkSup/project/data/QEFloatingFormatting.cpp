/*  QEFloatingFormatting.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2019 Australian Synchrotron
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
#include <QEVectorVariants.h>

#define DEBUG qDebug () << "QEFloatingFormatting" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QEFloatingFormatting::QEFloatingFormatting()
{
    // Default formatting properties.
    format = FORMAT_g;
    precision = 15;
}

//------------------------------------------------------------------------------
//
QEFloatingFormatting::~QEFloatingFormatting() { }

//------------------------------------------------------------------------------
// Generate a value given a floating point number, using formatting defined within this class.
// The formatting mainly applies if formatting as a string. For example, was is
// the number base? should a sign always be included? are leading zeros requried?
// The formatting could include properties related to other types. For example, generate
// an error if attempting to convert a negative floating point number to an unsigned integer.
//
QVariant QEFloatingFormatting::formatValue( const double &floatingValue,
                                            generic::generic_types valueType ) const
{
    switch( valueType ) {
        case generic::GENERIC_DOUBLE :
        case generic::GENERIC_FLOAT :
        {
            QVariant dValue( floatingValue );
            return dValue;
        }
        case generic::GENERIC_LONG :
        {
            qlonglong integerValue;
            if( floatingValue < double( LONG_MIN ) )
                integerValue = LONG_MIN;
            else if( floatingValue > double( LONG_MAX ) )
                integerValue = LONG_MAX;
            else
                integerValue = (qlonglong)floatingValue;

            QVariant lValue( integerValue );
            return lValue;
        }
        case generic::GENERIC_SHORT :
        {
            qlonglong integerValue;
            if( floatingValue < double( SHRT_MIN ) )
                integerValue = SHRT_MIN;
            else if( floatingValue > double( SHRT_MAX ) )
                integerValue = SHRT_MAX;
            else
                integerValue = (qlonglong)floatingValue;

            QVariant lValue( integerValue );
            return lValue;
        }
        case generic::GENERIC_UNSIGNED_LONG :
        {
            qulonglong unsignedIntegerValue;
            if( floatingValue < 0 )
                unsignedIntegerValue = 0;
            else if( floatingValue > double( ULONG_MAX ) )
                unsignedIntegerValue = ULONG_MAX;
            else
                unsignedIntegerValue = qulonglong( floatingValue );

            QVariant ulValue( unsignedIntegerValue );
            return ulValue;
        }
        case generic::GENERIC_UNSIGNED_SHORT :
        {
            qulonglong unsignedIntegerValue;
            if( floatingValue < 0 )
                unsignedIntegerValue = 0;
            else if( floatingValue > double( USHRT_MAX ) )
                unsignedIntegerValue = USHRT_MAX;
            else
                unsignedIntegerValue = qulonglong( floatingValue );

            QVariant ulValue( unsignedIntegerValue );
            return ulValue;
        }
        case generic::GENERIC_UNSIGNED_CHAR :
        {
            qulonglong unsignedIntegerValue;
            if( floatingValue < 0 )
                unsignedIntegerValue = 0;
            else if( floatingValue > double( UCHAR_MAX ) )
                unsignedIntegerValue = UCHAR_MAX;
            else
                unsignedIntegerValue = qulonglong( floatingValue );

            QVariant ulValue( unsignedIntegerValue );
            return ulValue;
        }
        case generic::GENERIC_STRING :
        {
            QString string = QString::number( floatingValue, getFormatChar(), precision );
            QVariant sValue( string );
            return sValue;
        }
        case generic::GENERIC_UNKNOWN :
        {
        }
        default:
        {
            //qDebug() << "QEFloatingFormatting::formatValue() Unknown value 'Generic' type: " << valueType;
            QVariant unknown;
            return unknown;
        }
    }

    //qDebug() << "QEFloatingFormatting::formatValue() Unknown value 'Generic' type: " << valueType;
    QVariant unknown;
    return unknown;
}

/*
    Generate a value given an array of floating point number, using formatting defined within this class.
    The formatting mainly applies if formatting as a string. For example, was is
    the number base? should a sign always be included? are leading zeros requried?
    The formatting could include properties related to other types. For example, generate
    an error if attempting to convert a negative floating point number to an unsigned integer.
*/
QVariant QEFloatingFormatting::formatValue( const QVector<double> &floatingValue,
                                            generic::generic_types valueType ) const
{
    QList<QVariant> array;
    int arraySize = floatingValue.size();
    for( int i = 0; i < arraySize; i++ )
    {
        array.append( formatValue( floatingValue[i], valueType ));
    }
    return array;
}

//------------------------------------------------------------------------------
// Generate an floating point number given a value, using formatting defined within this class.
// The value may be an array of variants or a single variant
//
double QEFloatingFormatting::formatFloating( const QVariant &value ) const
{
    return formatFloating( value, 0 );
}

double QEFloatingFormatting::formatFloating( const QVariant &value, const int arrayIndex ) const
{
    double result;

    // If the value is a list, get the specified item from the list.
    // Otherwise, just use the value as is
    if( value.type() == QVariant::List )
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
// Generate an floating point number array given a value, using formatting defined within this class.
//
QVector<double> QEFloatingFormatting::formatFloatingArray( const QVariant &value )  const
{
    bool okay = true;
    QVector<double> result;

    // If the value is a list, populate a list, converting each of the items to a double
    if( value.type() == QVariant::List )
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

/*
    Wrapper to toDouble with reeor report.
 */
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

/*
    Do something with the fact that the value could not be formatted as requested.
*/
double QEFloatingFormatting::formatFailure( QString message  ) const
{
    // Log the format failure if required.
    qDebug() << "QEFloatingFormatting" << message;

    // Return whatever is required for a formatting falure.
    return 0.0;
}

/*
    Set the precision.
    Relevent when formatting the floating point number as a string.
*/
void QEFloatingFormatting::setPrecision( unsigned int precisionIn )
{
    precision = precisionIn;
}

/*
    Get the precision.
    Relevent when formatting the floating point number as a string.
*/
unsigned int QEFloatingFormatting::getPrecision() const
{
    return precision;
}

/*
    Set the format.
    Relevent when formatting the floating point number as a string.
*/
void QEFloatingFormatting::setFormat( formats formatIn )
{
        format = formatIn;
}

/*
    Get the format.
    Relevent when formatting the floating point number as a string.
*/
QEFloatingFormatting::formats QEFloatingFormatting::getFormat() const
{
    return format;
}

/*
    Get the format character required for the QString::number function.
    Relevent when formatting the floating point number as a string.
*/
char QEFloatingFormatting::getFormatChar() const
{
    switch( format )
    {
        case FORMAT_e: return 'e'; break;
        case FORMAT_E: return 'E'; break;
        case FORMAT_f: return 'f'; break;
        case FORMAT_g: return 'g'; break;
        case FORMAT_G: return 'G'; break;
        default:       return 'g'; break;
    }
}

// end
