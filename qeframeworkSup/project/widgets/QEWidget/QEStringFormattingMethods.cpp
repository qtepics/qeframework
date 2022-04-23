/*  QEStringFormattingMethods.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2022 Australian Synchrotron
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

/*
  This class is used by QEWidgets when setting properties managed by the QEStringformatting class.
  This class calls matching set and get methods from the QEStringformatting class but also calls back
  the QEWidget when setting properties so the widget can update itself based on the changes.
 */

#include <QEStringFormattingMethods.h>

// String formatting properties

// precision
void QEStringFormattingMethods::setPrecision( int precision )
{
    stringFormatting.setPrecision( precision );
    stringFormattingChange();
}
int QEStringFormattingMethods::getPrecision() const
{
    return stringFormatting.getPrecision();
}

// useDbPrecision
void QEStringFormattingMethods::setUseDbPrecision( bool useDbPrecision )
{
    stringFormatting.setUseDbPrecision( useDbPrecision);
    stringFormattingChange();
}
bool QEStringFormattingMethods::getUseDbPrecision() const
{
    return stringFormatting.getUseDbPrecision();
}

// leadingZeros
void QEStringFormattingMethods::setLeadingZeros( const int leadingZeros )
{
   stringFormatting.setLeadingZeros( leadingZeros );
   stringFormattingChange();
}
int QEStringFormattingMethods::getLeadingZeros() const
{
   return stringFormatting.getLeadingZeros();
}

// leadingZero - deprecated
void QEStringFormattingMethods::setLeadingZero( bool leadingZero )
{
}
bool QEStringFormattingMethods::getLeadingZero() const
{
    return true;
}

// trailingZeros - deprecated
void QEStringFormattingMethods::setTrailingZeros( bool trailingZeros )
{
}
bool QEStringFormattingMethods::getTrailingZeros() const
{
    return true;
}

// forceSign
void QEStringFormattingMethods::setForceSign( bool forceSign )
{
    stringFormatting.setForceSign( forceSign );
    stringFormattingChange();
}
bool QEStringFormattingMethods::getForceSign() const
{
    return stringFormatting.getForceSign();
}

// addUnits
void QEStringFormattingMethods::setAddUnits( bool addUnits )
{
    stringFormatting.setAddUnits( addUnits );
    stringFormattingChange();
}
bool QEStringFormattingMethods::getAddUnits() const
{
    return stringFormatting.getAddUnits();
}

// localEnumeration
void QEStringFormattingMethods::setLocalEnumeration( QString localEnumeration )
{
    stringFormatting.setLocalEnumeration( localEnumeration );
    stringFormattingChange();
}
QString QEStringFormattingMethods::getLocalEnumeration() const
{
    return stringFormatting.getLocalEnumeration();
}

// format
void QEStringFormattingMethods::setFormat( QEStringFormatting::formats format )
{
    stringFormatting.setFormat( format );
    stringFormattingChange();
}
QEStringFormatting::formats QEStringFormattingMethods::getFormat() const
{
    return stringFormatting.getFormat();
}

// separator
void QEStringFormattingMethods::setSeparator( const QEStringFormatting::separators separator )
{
    stringFormatting.setSeparator( separator );
    stringFormattingChange();
}
QEStringFormatting::separators QEStringFormattingMethods::getSeparator() const
{
   return stringFormatting.getSeparator();
}

// radix
void QEStringFormattingMethods::setRadix( const int radix )
{
    stringFormatting.setRadix( radix);
    stringFormattingChange();
}
unsigned int QEStringFormattingMethods::getRadix() const
{
    return stringFormatting.getRadix();
}

// notation
void QEStringFormattingMethods::setNotation( QEStringFormatting::notations notation )
{
    stringFormatting.setNotation( notation );
    stringFormattingChange();
}
QEStringFormatting::notations QEStringFormattingMethods::getNotation() const
{
    return stringFormatting.getNotation();
}

// arrayAction
void QEStringFormattingMethods::setArrayAction( QEStringFormatting::arrayActions arrayAction )
{
    stringFormatting.setArrayAction( arrayAction );
    stringFormattingChange();
}
QEStringFormatting::arrayActions QEStringFormattingMethods::getArrayAction() const
{
    return stringFormatting.getArrayAction();
}

// local enuerations
QELocalEnumeration QEStringFormattingMethods::getLocalEnumerationObject() const
{
    return stringFormatting.getLocalEnumerationObject();
}

// end
