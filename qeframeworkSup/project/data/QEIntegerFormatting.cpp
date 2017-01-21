/*  QEIntegerFormatting.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
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
 *  Copyright (c) 2009, 2010, 2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

// Provides textual formatting for QEInteger data.

#include <QEIntegerFormatting.h>
#include <QtDebug>

/*
    ???
*/
QEIntegerFormatting::QEIntegerFormatting() {
    // Default formatting properties.
    radix = 10;
}

/*
    Generate a value given an integer, using formatting defined within this class.
    The formatting mainly applies if formatting as a string. For example, what is
    the number base? should a sign always be included? are leading zeros requried?
    The formatting could include properties related to other types. For example, generate
    an error if attempting to convert a negative integer to an unsigned integer.
*/
QVariant QEIntegerFormatting::formatValue( const long &integerValue, generic::generic_types valueType ) {
    switch( valueType ) {
        case generic::GENERIC_STRING :
        {
            QString string = QString::number( integerValue, radix);
            QVariant sValue( string );
            return sValue;
        }
        case generic::GENERIC_SHORT :
        case generic::GENERIC_LONG :
        {
            QVariant lValue( (qlonglong)integerValue );
            return lValue;
        }
        case generic::GENERIC_UNSIGNED_SHORT :
        case generic::GENERIC_UNSIGNED_CHAR :
        case generic::GENERIC_UNSIGNED_LONG :
        {
            qulonglong unsignedIntegerValue;
            ( integerValue < 0 ) ? unsignedIntegerValue=0 : unsignedIntegerValue=integerValue;
            QVariant ulValue( unsignedIntegerValue );
            return ulValue;
        }
        case generic::GENERIC_FLOAT :
        case generic::GENERIC_DOUBLE :
        {
            QVariant dValue( (double)integerValue );
            return dValue;
        }
        case generic::GENERIC_UNKNOWN :
        default :
        {
            //qDebug() << "QEIntegerFormatting::formatValue() Unknown value 'Generic' type: " << valueType;
            QVariant unknown;
            return unknown;
        }
    }
    //qDebug() << "QEIntegerFormatting::formatValue() Unknown value 'Generic' type: " << valueType;
    QVariant unknown;
    return unknown;
}

/*
    Generate a value given an array of integer numbers, using formatting defined within this class.
    The formatting mainly applies if formatting as a string. For example, what is
    the number base? should a sign always be included? are leading zeros requried?
    The formatting could include properties related to other types. For example, generate
    an error if attempting to convert a negative integer to an unsigned integer.
*/
QVariant QEIntegerFormatting::formatValue( const QVector<long> &integerValue, generic::generic_types valueType )
{
   QList<QVariant> array;
   int arraySize = integerValue.size();
   for( int i = 0; i < arraySize; i++ )
   {
       array.append( formatValue( integerValue[i], valueType ));
   }
   return array;
}

/*
    Generate an integer given a value, using formatting defined within this class.
    The value may be an array of variants or a single variant
*/
long QEIntegerFormatting::formatInteger( const QVariant &value ) {

    // If the value is a list, get the first item from the list.
    // Otherwise, just use the value as is
    if( value.type() == QVariant::List )
    {
        return formatIntegerNonArray( value.toList()[0] );
    }
    else
    {
        return formatIntegerNonArray( value );
    }
}

/*
    Generate an integer array given a value, using formatting defined within this class.
*/
QVector<long> QEIntegerFormatting::formatIntegerArray( const QVariant &value ) {

    QVector<long> returnValue;

    // If the value is a list, populate a list, converting each of the items to a long
    if( value.type() == QVariant::List )
    {
        QVariantList list = value.toList();
        for( long i=0; i < list.count(); i++ )
        {
            returnValue.append( formatIntegerNonArray( list[i] ));
        }
    }

    // The value is not a list so build a list with a single long
    else
    {
        returnValue.append( formatIntegerNonArray( value ));
    }

    return returnValue;
}

/*
    Generate an integer given a value, using formatting defined within this class.
*/
long QEIntegerFormatting::formatIntegerNonArray( const QVariant &value ) {
    // Determine the format from the variant type.
    // Only the types used to store ca data are used. any other type is considered a failure.
    switch( value.type() ) {
        case QVariant::Double :
        {
            return formatFromFloating( value );
        }
        case QVariant::LongLong :
        {
            return value.toLongLong(); // No conversion requried. Stored in variant as required type
        }
        case QVariant::ULongLong :
        {
            return formatFromUnsignedInteger( value );
        }
        case QVariant::String :
        {
            return formatFromString( value );
        }
        default :
        {
            return formatFailure( QString( "Bug in QEIntegerFormatting::formatInteger(). The QVariant type was not expected" ) );
        }
    }
}

/*
    Format a variant value as an integer representation of a floating point number.
    Convert the variant value to a long. It may or may not be a longlong type variant. If it is - good,
    there will be no conversion problems.
*/
long QEIntegerFormatting::formatFromFloating( const QVariant &value ) {
    // Extract the value as an integer using whatever conversion the QVariant uses.
    //
    // Note, this will not pick up if the QVariant type is not one of the types used to represent CA data.
    // This is OK as it is not absolutely nessesary to do this sort of check at this point. Also the code is more robust as it will
    // work if the range of QVariant types used expands.
    // Note, this does not give us the freedom to specify what conversions should fail or succeed. For example, does QVariant::toLongLong()
    // work if the value it holds is the string 1.234a, and should it?
    // If QVariant::toLongLong() does not do exactly what is required, a switch statement for each of the types used to hold CA data
    // will need to be added and the conversions done  manually or using QVariant::toLongLong() as required.
    bool convertOk;
    long iValue = value.toLongLong( &convertOk );

    if( !convertOk )
        return formatFailure( QString( "Warning from QEIntegerFormatting::formatFromFloating(). A variant could not be converted to a long." ) );

    return iValue;
}

/*
    Format a variant value as a (signed) integer representation of an unsigned integer.
    This method was written to convert a QVariant of type ULongLong, but should cope with a variant of any type.
    Convert the variant value to a long. It may or may not be a longlong type variant. If it is - good,
    there will be no conversion problems.
*/
long QEIntegerFormatting::formatFromUnsignedInteger( const QVariant &value ) {
    // Extract the value as a long using whatever conversion the QVariant uses.
    //
    // Note, this will not pick up if the QVariant type is not one of the types used to represent CA data.
    // This is OK as it is not absolutely nessesary to do this sort of check at this point. Also the code is more robust as it will
    // work if the range of QVariant types used expands.
    // Note, this does not give us the freedom to specify what conversions should fail or succeed. For example, does QVariant::toLongLong()
    // work if the value it holds is the string 1.0001 and should it?
    // If QVariant::toLongLong() does not do exactly what is required, a switch statement for each of the types used to hold CA data
    // will need to be added and the conversions done  manually or using QVariant::toLongLong() as required.
    bool convertOk;
    long lValue = value.toLongLong( &convertOk );

    if( !convertOk )
        return formatFailure( QString( "Warning from QEIntegerFormatting::formatFromUnsignedInteger(). A variant could not be converted to an unsigned long." ) );

    return lValue;
}

/*
    Format a variant value as an integer representation of a string.
    This method was written to convert a QVariant of type String, but should cope with a variant of any type.
    Convert the variant value to an unsigned long. It may or may not be a ulonglong type variant. If it is - good,
    there will be no conversion problems.
*/
long QEIntegerFormatting::formatFromString( const QVariant &value ) {
    // Extract the value as a long using whatever conversion the QVariant uses.
    // If that fails, try extracting the value as a double using whatever conversion the QVariant uses, then cast it as a long.
    //
    // Note, this will not pick up if the QVariant type is not one of the types used to represent CA data.
    // This is OK as it is not absolutely nessesary to do this sort of check at this point. Also the code is more robust as it will
    // work if the range of QVariant types used expands.
    // Note, this does not give us the freedom to specify what conversions should fail or succeed. For example, does QVariant::toLongLong()
    // work if the value it holds is the string 1.0001 and should it?
    // If QVariant::toLongLong() does not do exactly what is required, a switch statement for each of the types used to hold CA data
    // will need to be added and the conversions done  manually or using QVariant::toLongLong() as required.
    bool convertOk;
    long lValue = value.toLongLong( &convertOk );

    if( convertOk )
        return lValue;

    double dValue = value.toDouble( &convertOk );
    if( convertOk )
    {
        lValue = (long)dValue;
        return lValue;
    }

    return formatFailure( QString( "Warning from QEIntegerFormatting::formatFromString(). "
                                   "A variant ('%1') could not be converted to an integer." ).
                          arg( value.toString().trimmed() ) );

}

/*
    Format a variant value as an integer representation of time.
    This method was written to convert a QVariant of type ??? (the type used to represent times in CA),
    but should cope with a variant of any type.
    Convert the variant value to a long. It may or may not be a longlong type variant. If it is - good,
    there will be no conversion problems.
*/
long QEIntegerFormatting::formatFromTime( const QVariant &value ) {
    //??? what is the ca time format and how do you convert it to an integer?
    // Should there be conversion properties such as 'convert to minutes', 'convert to hours'.
    return value.toLongLong();
}

/*
    Do something with the fact that the value could not be formatted as requested.
*/
long QEIntegerFormatting::formatFailure( QString message ) {
    // Log the format failure if required.
    qDebug() << message;

    // Return whatever is required for a formatting falure.
    return 0;
}

/*
    Set the numer system base.
    Relevent when formatting the string as an interger.
    Any radix of 2 or more is accepted. Check the conversion code that uses this number to see what values are expected.
    At the time of writing (16/2/9) it is anticipated that floating point numbers will always be base 10 and integer numbers will
    be base 2, 8, 10, or 16.
    ??? if radix processing beocomes significant, create a radix class that can be used for both QEIntegerFormatting and QEStringFormatting.
*/
void QEIntegerFormatting::setRadix( unsigned int radixIn ) {
    if( radixIn >= 2 )
        radix = radixIn;
}

/*
    Get the numerical base. See setRadix() for the use of 'radix'.
*/
unsigned int QEIntegerFormatting::getRadix() {
    return radix;
}

// end
