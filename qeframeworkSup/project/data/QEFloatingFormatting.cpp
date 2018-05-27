/*  QEFloatingFormatting.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2018 Australian Synchrotron
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

#include <QEFloatingFormatting.h>
#include <QtDebug>

/*
    ???
*/
QEFloatingFormatting::QEFloatingFormatting() {
    // Default formatting properties.
    format = FORMAT_g;
    precision = 6;
}

/*
    Generate a value given a floating point number, using formatting defined within this class.
    The formatting mainly applies if formatting as a string. For example, was is
    the number base? should a sign always be included? are leading zeros requried?
    The formatting could include properties related to other types. For example, generate
    an error if attempting to convert a negative floating point number to an unsigned integer.
*/
QVariant QEFloatingFormatting::formatValue( const double &floatingValue, generic::generic_types valueType ) {
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
            if( floatingValue < (double)LONG_MIN )
                integerValue = LONG_MIN;
            else if( floatingValue > (double)LONG_MAX )
                integerValue = LONG_MAX;
            else
                integerValue = (qlonglong)floatingValue;

            QVariant lValue( integerValue );
            return lValue;
        }
        case generic::GENERIC_SHORT :
        {
            qlonglong integerValue;
            if( floatingValue < (double)SHRT_MIN )
                integerValue = SHRT_MIN;
            else if( floatingValue > (double)SHRT_MAX )
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
            else if( floatingValue > (double)ULONG_MAX )
                unsignedIntegerValue = ULONG_MAX;
            else
                unsignedIntegerValue = (qulonglong)floatingValue;

            QVariant ulValue( unsignedIntegerValue );
            return ulValue;
        }
        case generic::GENERIC_UNSIGNED_SHORT :
        {
            qulonglong unsignedIntegerValue;
            if( floatingValue < 0 )
                unsignedIntegerValue = 0;
            else if( floatingValue > (double)USHRT_MAX )
                unsignedIntegerValue = USHRT_MAX;
            else
                unsignedIntegerValue = (qulonglong)floatingValue;

            QVariant ulValue( unsignedIntegerValue );
            return ulValue;
        }
        case generic::GENERIC_UNSIGNED_CHAR :
        {
            qulonglong unsignedIntegerValue;
            if( floatingValue < 0 )
                unsignedIntegerValue = 0;
            else if( floatingValue > (double)UCHAR_MAX )
                unsignedIntegerValue = UCHAR_MAX;
            else
                unsignedIntegerValue = (qulonglong)floatingValue;

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
        default :
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
QVariant QEFloatingFormatting::formatValue( const QVector<double> &floatingValue, generic::generic_types valueType ) {
    QList<QVariant> array;
    int arraySize = floatingValue.size();
    for( int i = 0; i < arraySize; i++ )
    {
        array.append( formatValue( floatingValue[i], valueType ));
    }
    return array;
}

/*
    Generate an floating point number given a value, using formatting defined within this class.
    The value may be an array of variants or a single variant
*/
double QEFloatingFormatting::formatFloating( const QVariant &value, const int arrayIndex )
{

    // If the value is a list, get the first item from the list.
    // Otherwise, just use the value as is
    if( value.type() == QVariant::List )
    {
        QVariant defValue( (double) 0.0 );
        return formatFloatingNonArray( value.toList().value( arrayIndex, defValue ) );
    }
    else
    {
        return formatFloatingNonArray( value );
    }
}

/*
    Generate an floating point number array given a value, using formatting defined within this class.
*/
QVector<double> QEFloatingFormatting::formatFloatingArray( const QVariant &value ) {

    QVector<double> returnValue;

    // If the value is a list, populate a list, converting each of the items to a double
    if( value.type() == QVariant::List )
    {
        QVariantList list = value.toList();
        for( long i=0; i < list.count(); i++ )
        {
            returnValue.append( formatFloatingNonArray( list[i] ));
        }
    }

    // The value is not a list so build a list with a single double
    else
    {
        returnValue.append( formatFloatingNonArray( value ));
    }

    return returnValue;
}

/*
    Generate an floating point number given a value, using formatting defined within this class.
    The value must be a single variant.
    This is used when formatting a single value, or for each value in an array of values.
*/
double QEFloatingFormatting::formatFloatingNonArray( const QVariant &value ) {

    // Determine the format from the variant type.
    // Only the types used to store ca data are used. any other type is considered a failure.
    switch( value.type() ) {
        case QVariant::Double :
        {
            return value.toDouble(); // No conversion requried. Stored in variant as required type
        }
        case QVariant::LongLong :
        {
            return formatFromInteger( value );
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
            return formatFailure( QString( "Bug in QEFloatingFormatting::formatFloating(). The QVariant type was not expected" ) );
        }
    }
}

/*
    Format a variant value as a floating point representation of a signed integer.
    This method was written to convert a QVariant of type LongLong, but should cope with a variant of any type.
    Convert the variant value to a double. It may or may not be a double type variant. If it is - good,
    there will be no conversion problems.
*/
double QEFloatingFormatting::formatFromInteger( const QVariant &value ) {
    // Extract the value as a double using whatever conversion the QVariant uses.
    //
    // Note, this will not pick up if the QVariant type is not one of the types used to represent CA data.
    // This is OK as it is not absolutely nessesary to do this sort of check at this point. Also the code is more robust as it will
    // work if the range of QVariant types used expands.
    // Note, this does not give us the freedom to specify what conversions should fail or succeed. For example, does QVariant::toDouble()
    // work if the value it holds is the string 1.234a, and should it?
    // If QVariant::toDouble() does not do exactly what is required, a switch statement for each of the types used to hold CA data
    // will need to be added and the conversions done  manually or using QVariant::toDouble() as required.
    bool convertOk;
    double dValue = value.toDouble( &convertOk );

    if( !convertOk )
        return formatFailure( QString( "Warning from QEFloatingFormatting::formatFromFloating(). A variant could not be converted to a double." ) );

    return dValue;
}

/*
    Format a variant value as a floating point representation of an unsigned integer.
    This method was written to convert a QVariant of type ULongLong, but should cope with a variant of any type.
    Convert the variant value to a double. It may or may not be a double type variant. If it is - good,
    there will be no conversion problems.
*/
double QEFloatingFormatting::formatFromUnsignedInteger( const QVariant &value ) {
    // Extract the value as a double using whatever conversion the QVariant uses.
    //
    // Note, this will not pick up if the QVariant type is not one of the types used to represent CA data.
    // This is OK as it is not absolutely nessesary to do this sort of check at this point. Also the code is more robust as it will
    // work if the range of QVariant types used expands.
    // Note, this does not give us the freedom to specify what conversions should fail or succeed. For example, does QVariant::toDouble()
    // work if the value it holds is the string 'twenty two' and should it?
    // If QVariant::toDouble() does not do exactly what is required, a switch statement for each of the types used to hold CA data
    // will need to be added and the conversions done  manually or using QVariant::toDouble() as required.
    bool convertOk;
    double dValue = value.toDouble( &convertOk );

    if( !convertOk )
        return formatFailure( QString( "Warning from QEFloatingFormatting::formatFromUnsignedInteger(). A variant could not be converted to a double." ) );

    return dValue;
}

/*
    Format a variant value as a floating point representation of a string.
    This method was written to convert a QVariant of type String, but should cope with a variant of any type.
    Convert the variant value to a double. It may or may not be a double type variant. If it is - good,
    there will be no conversion problems.
*/
double QEFloatingFormatting::formatFromString( const QVariant &value ) {
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
    double dValue = value.toDouble( &convertOk );

    if( !convertOk )
        return formatFailure( QString( "Warning from QEFloatingFormatting::formatFromString(). "
                                       "A variant ('%1') could not be converted to a double." ).
                              arg( value.toString().trimmed() ) );

    return dValue;
}

/*
    Format a variant value as a floating point representation of time.
    This method was written to convert a QVariant of type ??? (the type used to represent times in CA),
    but should cope with a variant of any type.
    Convert the variant value to a double. It may or may not be a double type variant. If it is - good,
    there will be no conversion problems.
*/
double QEFloatingFormatting::formatFromTime( const QVariant &value ) {
    //??? what is the ca time format and how do you convert it to an double?
    // Should there be conversion properties such as 'convert to minutes', 'convert to hours'.
    return value.toDouble();
}

/*
    Do something with the fact that the value could not be formatted as requested.
*/
double QEFloatingFormatting::formatFailure( QString message ) {
    // Log the format failure if required.
    qDebug() << message;

    // Return whatever is required for a formatting falure.
    return 0.0;
}

/*
    Set the precision.
    Relevent when formatting the floating point number as a string.
*/
void QEFloatingFormatting::setPrecision( unsigned int precisionIn ) {
        precision = precisionIn;
}

/*
    Get the precision.
    Relevent when formatting the floating point number as a string.
*/
unsigned int QEFloatingFormatting::getPrecision() {
    return precision;
}

/*
    Set the format.
    Relevent when formatting the floating point number as a string.
*/
void QEFloatingFormatting::setFormat( formats formatIn ) {
        format = formatIn;
}

/*
    Get the format.
    Relevent when formatting the floating point number as a string.
*/
int QEFloatingFormatting::getFormat() {
    return format;
}

/*
    Get the format character required for the QString::number function.
    Relevent when formatting the floating point number as a string.
*/
char QEFloatingFormatting::getFormatChar() {
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
