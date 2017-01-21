/*  Generic.cpp
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
 *  Copyright (c) 2009, 2010 Australian Synchrotron
 *
 *  Author:
 *    Anthony Owen
 *  Contact details:
 *    anthony.owen@gmail.com
 */

// Provides a generic holder for different types.

#include <Generic.h>
#include <stdlib.h>
#include <string.h>

using namespace generic;

/*
    Creates unknown type
*/
Generic::Generic() { 
    value = NULL;
    arrayCount = 0;
    type = generic::GENERIC_UNKNOWN;
}

/*
    Creates overloaded constructor of type string
*/
Generic::Generic( std::string newValue ) {
    value = NULL;
    arrayCount = 0;
    setString( newValue );
}

/*
    Creates overloaded constructor of type short
*/
Generic::Generic( short newValue ) {
    value = NULL;
    arrayCount = 0;
    setShort( newValue );
}

Generic::Generic( short* newValue, unsigned long arrayCountIn ) {
    value = NULL;
    arrayCount = 0;
    setShort( newValue, arrayCountIn );
}

/*
    Creates overloaded constructor of type unsigned short
*/
Generic::Generic( unsigned short newValue ) {
    value = NULL;
    arrayCount = 0;
    setUnsignedShort( newValue );
}

Generic::Generic( unsigned short* newValue, unsigned long arrayCountIn ) {
    value = NULL;
    arrayCount = 0;
    setUnsignedShort( newValue, arrayCountIn );
}

/*
    Creates overloaded constructor of type char
*/
Generic::Generic( unsigned char newValue ) {
    value = NULL;
    arrayCount = 0;
    setUnsignedChar( newValue );
}

Generic::Generic( unsigned char* newValue, unsigned long arrayCountIn ) {
    value = NULL;
    arrayCount = 0;
    setUnsignedChar( newValue, arrayCountIn );
}

/*
    Creates overloaded constructor of type unsigned long
*/
Generic::Generic( quint32 newValue ) {
    value = NULL;
    arrayCount = 0;
    setUnsignedLong( newValue );
}

Generic::Generic( quint32* newValue, unsigned long arrayCountIn ) {
    value = NULL;
    arrayCount = 0;
    setUnsignedLong( newValue, arrayCountIn );
}

/*
    Creates overloaded constructor of type float
*/
Generic::Generic( float newValue ) {
    value = NULL;
    arrayCount = 0;
    setFloat( newValue );
}

Generic::Generic( float* newValue, unsigned long arrayCountIn ) {
    value = NULL;
    arrayCount = 0;
    setFloat( newValue, arrayCountIn );
}

/*
    Creates overloaded constructor of type double
*/
Generic::Generic( double newValue ) {
    value = NULL;
    arrayCount = 0;
    setDouble( newValue );
}

Generic::Generic( double* newValue, unsigned long arrayCountIn ) {
    value = NULL;
    arrayCount = 0;
    setDouble( newValue, arrayCountIn );
}

/*
    Actively delete the stored value and set flags
*/
Generic::~Generic() {
    deleteValue();
}

/*
    Copy constructor for deep copy
*/
Generic::Generic( Generic &param ) {
    cloneValue( &param );
}

/*
    = Operator for assignment
*/
Generic& Generic::operator= ( Generic &param ) {
    cloneValue( &param );
    return *this;
}

/*
    Creates and records new type string
*/
void Generic::setString( std::string newValue ) {
    deleteValue();
    value = new std::string( newValue );
    type = GENERIC_STRING;
}

/*
    Creates and records new type short
*/
void Generic::setShort( short newValue ) { 
    setShort( &newValue, 1 );
}

/*
    Creates and records new type short (an array larger than 1)
*/
void Generic::setShort( short* newValueArray, unsigned long arrayCountIn ) {
    deleteValue();
    value = new short[arrayCountIn];
    if( newValueArray )
    {
        memcpy( value, newValueArray, sizeof(short)*arrayCountIn );
    }
    arrayCount = arrayCountIn;
    type = GENERIC_SHORT;
}

void Generic::updateShort( short newValue, unsigned long arrayIndex ) {
    if( arrayIndex >= arrayCount )
        return;

    short* valueArray = (short*)value;
    valueArray[arrayIndex] = newValue;
}

/*
    Creates and records new type unsigned short
*/
void Generic::setUnsignedShort( unsigned short newValue ) {
    setUnsignedShort( &newValue, 1 );
}

/*
    Creates and records new type unsigned short (an array larger than 1)
*/
void Generic::setUnsignedShort( unsigned short* newValueArray, unsigned long arrayCountIn ) {
    deleteValue();
    value = new unsigned short[arrayCountIn];
    if( newValueArray )
    {
        memcpy( value, newValueArray, sizeof(unsigned short)*arrayCountIn );
    }
    arrayCount = arrayCountIn;
    type = GENERIC_UNSIGNED_SHORT;
}

void Generic::updateUnsignedShort( unsigned short newValue, unsigned long arrayIndex ) {
    if( arrayIndex >= arrayCount )
        return;

    unsigned short* valueArray = (unsigned short*)value;
    valueArray[arrayIndex] = newValue;
}

/*
    Creates and records new type unsigned char
*/
void Generic::setUnsignedChar( unsigned char newValue ) {
    setUnsignedChar( &newValue, 1 );
}

/*
    Creates and records new type unsigned char (an array larger than 1)
*/
void Generic::setUnsignedChar( unsigned char* newValueArray, unsigned long arrayCountIn ) {
    deleteValue();
    value = new unsigned char[arrayCountIn];
    if( newValueArray )
    {
        memcpy( value, newValueArray, sizeof(unsigned char)*arrayCountIn );
    }
    arrayCount = arrayCountIn;
    type = GENERIC_UNSIGNED_CHAR;
}

void Generic::updateUnsignedChar( unsigned char newValue, unsigned long arrayIndex ) {
    if( arrayIndex >= arrayCount )
        return;

    unsigned char* valueArray = (unsigned char*)value;
    valueArray[arrayIndex] = newValue;
}

/*
    Creates and records new type  long
*/
void Generic::setLong( qint32 newValue ) {
    setLong( &newValue, 1 );
}

/*
    Creates and records new type long (an array larger than 1)
*/
void Generic::setLong( qint32* newValueArray, unsigned long arrayCountIn ) {
    deleteValue();
    value = new qint32[arrayCountIn];
    if( newValueArray )
    {
        memcpy( value, newValueArray, sizeof(qint32)*arrayCountIn );
    }
    arrayCount = arrayCountIn;
    type = GENERIC_LONG;
}

void Generic::updateLong( qint32 newValue, unsigned long arrayIndex ) {
    if( arrayIndex >= arrayCount )
        return;

    qint32* valueArray = (qint32*)value;
    valueArray[arrayIndex] = newValue;
}

/*
    Creates and records new type unsigned long
*/
void Generic::setUnsignedLong( quint32 newValue ) {
    setUnsignedLong( &newValue, 1 );
}

/*
    Creates and records new type unsigned long (an array larger than 1)
*/
void Generic::setUnsignedLong( quint32* newValueArray, unsigned long arrayCountIn ) {
    deleteValue();
    value = new quint32[arrayCountIn];
    if( newValueArray )
    {
        memcpy( value, newValueArray, sizeof(quint32)*arrayCountIn );
    }
    arrayCount = arrayCountIn;
    type = GENERIC_UNSIGNED_LONG;
}

void Generic::updateUnsignedLong( quint32 newValue, unsigned long arrayIndex ) {
    if( arrayIndex >= arrayCount )
        return;

    quint32* valueArray = (quint32*)value;
    valueArray[arrayIndex] = newValue;
}
/*
    Creates and records new type float
*/
void Generic::setFloat( float newValue ) {
    setFloat( &newValue, 1 );
}

/*
    Creates and records new type float (an array larger than 1)
*/
void Generic::setFloat( float* newValueArray, unsigned long arrayCountIn ) {
    deleteValue();
    value = new float[arrayCountIn];
    if( newValueArray )
    {
        memcpy( value, newValueArray, sizeof(float)*arrayCountIn );
    }
    arrayCount = arrayCountIn;
    type = GENERIC_FLOAT;
}

void Generic::updateFloat( float newValue, unsigned long arrayIndex ) {
    if( arrayIndex >= arrayCount )
        return;

    float* valueArray = (float*)value;
    valueArray[arrayIndex] = newValue;
}

/*
    Creates and records new type double
*/
void Generic::setDouble( double newValue ) {
    setDouble( &newValue, 1 );
}

/*
    Creates and records new type double (an array larger than 1)
*/
void Generic::setDouble( double* newValueArray, unsigned long arrayCountIn ) {
    deleteValue();
    value = new double[arrayCountIn];
    if( newValueArray )
    {
        memcpy( value, newValueArray, sizeof(double)*arrayCountIn );
    }
    arrayCount = arrayCountIn;
    type = GENERIC_DOUBLE;
}

void Generic::updateDouble( double newValue, unsigned long arrayIndex ) {
    if( arrayIndex >= arrayCount )
        return;

    double* valueArray = (double*)value;
    valueArray[arrayIndex] = newValue;
}

/*
    Returns type string or invalid
*/
std::string Generic::getString() {
    if( getType() == GENERIC_STRING ) {
        return *(std::string*)value;
    }
    return "";
}

/*
    Returns type string array (char array) or invalid
*/
void Generic::getString( char** valueArray, unsigned long* arrayCountOut ){
    if( getType() == GENERIC_STRING ) {
        *valueArray = (char*)value;
        if( arrayCountOut )
            *arrayCountOut = arrayCount;
        return;
    }
    *valueArray = NULL;
    if( arrayCountOut )
        *arrayCountOut = 0;
}

/*
    Returns type short or invalid
*/
short Generic::getShort() { 
    if( getType() == GENERIC_SHORT ) {
        return *(short*)value;
    }
    return 0;
}

/*
    Returns type short array or invalid
*/
void Generic::getShort( short** valueArray, unsigned long* arrayCountOut ) {
    if( getType() == GENERIC_SHORT ) {
        *valueArray = (short*)value;
        if( arrayCountOut )
            *arrayCountOut = arrayCount;
        return;
    }
    *valueArray = NULL;
    if( arrayCountOut )
        *arrayCountOut = 0;
}

/*
    Returns type unsigned short or invalid
*/
unsigned short Generic::getUnsignedShort() {
    if( getType() == GENERIC_UNSIGNED_SHORT ) {
        return *(unsigned short*)value;
    }
    return 0;
}

/*
    Returns type short array or invalid
*/
void Generic::getUnsignedShort( unsigned short** valueArray, unsigned long* arrayCountOut ) {
    if( getType() == GENERIC_UNSIGNED_SHORT ) {
        *valueArray = (unsigned short*)value;
        if( arrayCountOut )
            *arrayCountOut = arrayCount;
        return;
    }
    *valueArray = NULL;
    if( arrayCountOut )
        *arrayCountOut = 0;
}

/*
    Returns type char or invalid
*/
unsigned char Generic::getUnsignedChar() {
    if( getType() == GENERIC_UNSIGNED_CHAR ) {
        return *(char*)value;
    }
    return 0;
}

/*
    Returns type char array or invalid
*/
void Generic::getUnsignedChar( unsigned char** valueArray, unsigned long* arrayCountOut ) {
    if( getType() == GENERIC_UNSIGNED_CHAR ) {
        *valueArray = (unsigned char*)value;
        if( arrayCountOut )
            *arrayCountOut = arrayCount;
        return;
    }
    *valueArray = NULL;
    if( arrayCountOut )
        *arrayCountOut = 0;
}

/*
    Returns type long or invalid
*/
qint32 Generic::getLong() {
    if( getType() == GENERIC_LONG ) {
        return *(qint32*)value;
    }
    return 0;
}

/*
    Returns type long array or invalid
*/
void Generic::getLong( qint32** valueArray, unsigned long* arrayCountOut ) {
    if( getType() == GENERIC_LONG ) {
        *valueArray = (qint32*)value;
        if( arrayCountOut )
            *arrayCountOut = arrayCount;
        return;
    }
    *valueArray = NULL;
    if( arrayCountOut )
        *arrayCountOut = 0;
}

/*
    Returns type unsigned long or invalid
*/
quint32 Generic::getUnsignedLong() {
    if( getType() == GENERIC_UNSIGNED_LONG ) {
        return *(quint32*)value;
    }
    return 0;
}

/*
    Returns type unsigned long array or invalid
*/
void Generic::getUnsignedLong( quint32** valueArray, unsigned long* arrayCountOut ) {
    if( getType() == GENERIC_UNSIGNED_LONG ) {
        *valueArray = (quint32*)value;
        if( arrayCountOut )
            *arrayCountOut = arrayCount;
        return;
    }
    *valueArray = NULL;
    if( arrayCountOut )
        *arrayCountOut = 0;
}

/*
    Returns type float or invalid
*/
float Generic::getFloat() {
    if( getType() == GENERIC_FLOAT ) {
        return *(float*)value;
    }
    return 0;
}

/*
    Returns type double array or invalid
*/
void Generic::getFloat( float** valueArray, unsigned long* arrayCountOut ) {
    if( getType() == GENERIC_FLOAT ) {
        *valueArray = (float*)value;
        if( arrayCountOut )
            *arrayCountOut = arrayCount;
        return;
    }
    *valueArray = NULL;
    if( arrayCountOut )
        *arrayCountOut = 0;
}

/*
    Returns type double or invalid
*/
double Generic::getDouble() { 
    if( getType() == GENERIC_DOUBLE ) {
        return *(double*)value;
    }
    return 0;
}

/*
    Returns type double array or invalid
*/
void Generic::getDouble( double** valueArray, unsigned long* arrayCountOut ) {
    if( getType() == GENERIC_DOUBLE ) {
        *valueArray = (double*)value;
        if( arrayCountOut )
            *arrayCountOut = arrayCount;
        return;
    }
    *valueArray = NULL;
    if( arrayCountOut )
        *arrayCountOut = 0;
}

/*
    Returns value type
*/
generic_types Generic::getType() {
    return type;
}

/*
    Returns array size
*/
unsigned long Generic::getArrayCount() {
    return arrayCount;
}



/*
    Sets the value type
*/
void Generic::setType( generic_types newType ) {

    // Update the array count to reflect the element size change
    int currentSize = getSize( type );
    int newSize = getSize( newType );
    arrayCount = arrayCount * currentSize / newSize;

    // Change the type
    type = newType;
}

/*
    Deletes stored value 
*/
void Generic::deleteValue() {
    if( value == NULL ) {
        type = GENERIC_UNKNOWN;
        return;
    }
    switch( getType() ) {
        case GENERIC_STRING :
            delete (std::string*)value;
        break;
        case GENERIC_SHORT :
            delete (short*)value;
        break;
        case GENERIC_UNSIGNED_SHORT :
            delete (unsigned short*)value;
        break;
        case GENERIC_UNSIGNED_CHAR :
            delete (char*)value;
        break;
        case GENERIC_LONG :
            delete (qint32*)value;
        break;
        case GENERIC_UNSIGNED_LONG :
            delete (quint32*)value;
        break;
        case GENERIC_FLOAT :
            delete (float*)value;
        break;
        case GENERIC_DOUBLE :
            delete (double*)value;
        break;
        case GENERIC_UNKNOWN :
            value = NULL;
            return;
        break;
    }
    value = NULL;
    type = GENERIC_UNKNOWN;
}

/*
    Clone from given Generic
*/
void Generic::cloneValue( Generic *param ) {
    switch( param->getType() ) {
        case GENERIC_STRING :
            setString( param->getString() );
        break;
        case GENERIC_UNSIGNED_SHORT :
            {
                unsigned short* paramValue;
                unsigned long paramCount;
                param->getUnsignedShort( &paramValue, &paramCount );
                setUnsignedShort( paramValue, paramCount );
            }
        break;
        case GENERIC_SHORT :
            {
                short* paramValue;
                unsigned long paramCount;
                param->getShort( &paramValue, &paramCount );
                setShort( paramValue, paramCount );
            }
        break;
        case GENERIC_UNSIGNED_CHAR :
            {
                unsigned char* paramValue;
                unsigned long paramCount;
                param->getUnsignedChar( &paramValue, &paramCount );
                setUnsignedChar( paramValue, paramCount );
            }
        break;
        case GENERIC_LONG :
            {
                qint32* paramValue;
                unsigned long paramCount;
                param->getLong( &paramValue, &paramCount );
                setLong( paramValue, paramCount );
            }
        break;
         case GENERIC_UNSIGNED_LONG :
            {
                quint32* paramValue;
                unsigned long paramCount;
                param->getUnsignedLong( &paramValue, &paramCount );
                setUnsignedLong( paramValue, paramCount );
            }
        break;
        case GENERIC_FLOAT :
            {
                float* paramValue;
                unsigned long paramCount;
                param->getFloat( &paramValue, &paramCount );
                setFloat( paramValue, paramCount );
            }
        break;
        case GENERIC_DOUBLE :
            {
                double* paramValue;
                unsigned long paramCount;
                param->getDouble( &paramValue, &paramCount );
                setDouble( paramValue, paramCount );
            }
        break;
        case GENERIC_UNKNOWN :
            deleteValue();
        break;
    }
}

/*
    Return the size in bytes of the data elements in each of the Generic types
*/
int Generic::getSize( generic_types type )
{
    // Return the size
    switch( type )
    {
        case GENERIC_STRING:
        case GENERIC_UNSIGNED_CHAR:
        case GENERIC_UNKNOWN:
            return 1;

        case GENERIC_SHORT:
        case GENERIC_UNSIGNED_SHORT:
            return 2;

        case GENERIC_LONG:
        case GENERIC_UNSIGNED_LONG:
        case GENERIC_FLOAT:
            return 4;

        case GENERIC_DOUBLE:
            return 8;
    }

    // Avoid compilation warnings only.
    // All cases are handled above, and if not then compiler warnings should let us know.
    return 1;
}
