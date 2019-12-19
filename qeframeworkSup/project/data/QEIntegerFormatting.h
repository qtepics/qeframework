/*  QEIntegerFormatting.h
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

// Provides integer (long) values for QEInteger data.

#ifndef QE_INTEGER_FORMATTING_H
#define QE_INTEGER_FORMATTING_H

#include <QString>
#include <QVariant>
#include <Generic.h>

#include <QEFrameworkLibraryGlobal.h>

/*!
    This class holds formatting instructions and uses them to convert to/from an integer and a QVariant of any type.
    It is generally set up with it's formatting instructions and then passed to a QEInteger class that will sink and source integer data to widgets or other code.
    It is used to convert data to and from a QCaObject (which sources and sinks data in the form of a QVariant where
    the QVariant reflects the underlying variable data type) and the QEInteger class.
    An example of a requirement for integer data is a combo box which must determine an integer index to select a menu option.
 */
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEIntegerFormatting {
public:

    // Construction
   explicit QEIntegerFormatting();
   ~QEIntegerFormatting();

    //===============================================
    // Main functions of this class:
    //   - Format a double based on a value
    //   - Translate a double and generate a value
    //===============================================
    /// Given a data value of any type, format it as an integer according to the formatting instructions held by the class.
    /// This is used to convert the QVariant value received from a QCaObject, which is still based on the data variable type, to an integer.
   long formatInteger( const QVariant &value ) const;
   long formatInteger( const QVariant &value, int arrayIndex ) const;

    /// Given a data value of any type, format it as an array of integers according to the formatting instructions held by the class.
    /// This is used to convert the QVariant value received from a QCaObject, which is still based on the data variable type, to an integer array.
    /// Typically used where the input QVariant value is an array of data values, but will work for any QVariant type.
    QVector<long> formatIntegerArray( const QVariant &value ) const;

    /// Given an integer value, format it as a data value of the specified type, according to the formatting instructions held by the class.
    /// This is used when writing integer data to a QCaObject.
    QVariant formatValue( const long &integerValue, generic::generic_types valueType ) const;
    QVariant formatValue( const QVector<long> &integerValue, generic::generic_types valueType ) const;

    // Functions to configure the formatting
    /// Set the radix used for all conversions. Default is 10.
    void setRadix( unsigned int radix );

    /// Get the radix used for all conversions.
    unsigned int getRadix() const;

private:
    // Utility function to convert variant to long.
    long varToLong( const QVariant& item ) const;

    // Error reporting
    long formatFailure( QString message ) const;

    // Formatting configuration
    int radix;      // Positional base system to display data.
};

#endif // QE_INTEGER_FORMATTING_H
