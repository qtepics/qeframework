/*  QEFloatingFormatting.h
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

// Provides floating (double) values for QEFloating data.

#ifndef QE_FLOATING_FORMATTING_H
#define QE_FLOATING_FORMATTING_H

#include <QString>
#include <QVariant>
#include <Generic.h>

#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEFloatingFormatting {
public:

    // Formatting enumerations
    enum formats { FORMAT_e = 'e',   // format as [-]9.9e[+|-]999
                   FORMAT_E = 'E',   // format as [-]9.9E[+|-]999
                   FORMAT_f = 'f',   // format as [-]9.9
                   FORMAT_g = 'g',   // use e or f format, whichever is the most concise
                   FORMAT_G = 'G' }; // use E or f format, whichever is the most concise


    // Construction
    explicit QEFloatingFormatting();
    ~QEFloatingFormatting();

    //===============================================
    // Main functions of this class:
    //   - Format a double based on a value
    //   - Translate a double and generate a value
    //===============================================
    /// Given a data value of any type, format it as an double according to the formatting instructions held by the class.
    /// This is used to convert the QVariant value received from a QCaObject, which is still based on the data variable type, to a double.
    double formatFloating( const QVariant &value, int arrayIndex = 0 ) const;

    /// Given a data value of any type, format it as an array of doubles according to the formatting instructions held by the class.
    /// This is used to convert the QVariant value received from a QCaObject, which is still based on the data variable type, to a double array.
    /// Typically used where the input QVariant value is an array of data values, but will work for any QVariant type.
    QVector<double> formatFloatingArray( const QVariant &value ) const;

    QVariant formatValue( const double &floatingValue, generic::generic_types valueType ) const;
    QVariant formatValue( const QVector<double> &floatingValue, generic::generic_types valueType ) const;

    // Functions to configure the formatting
    void setPrecision( unsigned int precision );
    void setFormat( formats format );

    // Functions to read the formatting configuration
    unsigned int getPrecision() const;
    formats getFormat() const;

private:
    // Private functions to read the formatting configuration
    char getFormatChar() const;

    // Type specific conversion functions
    double formatFromFloating( const QVariant &value ) const;
    double formatFromInteger( const QVariant &value ) const;
    double formatFromUnsignedInteger( const QVariant &value ) const;
    double formatFromString( const QVariant &value ) const;
    double formatFromTime( const QVariant &value ) const;

    // Common functions
    double formatFloatingNonArray( const QVariant &value ) const;

    // Error reporting
    double formatFailure( QString message ) const;

    // Formatting configuration
    unsigned int precision;
    formats format;
};

#endif // QE_FLOATING_FORMATTING_H
