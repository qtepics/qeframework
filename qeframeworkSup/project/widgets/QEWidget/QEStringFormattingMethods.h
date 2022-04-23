/*  QEStringFormattingMethods.h
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

#ifndef QE_STRING_FORMATTING_METHODS_H
#define QE_STRING_FORMATTING_METHODS_H

#include <QEFrameworkLibraryGlobal.h>
#include <QEStringFormatting.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEStringFormattingMethods {

public:
    virtual ~QEStringFormattingMethods(){}

    // Must be overriden by concrete sub-classes.
    //
    virtual void stringFormattingChange() = 0;

    // String formatting properties

    // precision
    void setPrecision( int precision );
    int getPrecision() const;

    // useDbPrecision
    void setUseDbPrecision( bool useDbPrecision );
    bool getUseDbPrecision() const;

    // leadingZeros
    void setLeadingZeros( const int leadingZeros );
    int getLeadingZeros() const;

    // leadingZero
    Q_DECL_DEPRECATED
    void setLeadingZero( bool leadingZero );
    Q_DECL_DEPRECATED
    bool getLeadingZero() const;

    // trailingZeros
    Q_DECL_DEPRECATED
    void setTrailingZeros( bool trailingZeros );
    Q_DECL_DEPRECATED
    bool getTrailingZeros() const;

    // forceSign
    void setForceSign( bool forceSign );
    bool getForceSign() const;

    // addUnits
    void setAddUnits( bool addUnits );
    bool getAddUnits() const;

    // localEnumeration
    void setLocalEnumeration( QString localEnumeration );
    QString getLocalEnumeration() const;

    // format
    void setFormat( QEStringFormatting::formats format );
    QEStringFormatting::formats getFormat() const;

    // separator
    void setSeparator( const QEStringFormatting::separators separator );
    QEStringFormatting::separators getSeparator() const;

    // radix
    void setRadix( const int radix );
    unsigned int getRadix() const;

    // notation
    void setNotation( QEStringFormatting::notations notation );
    QEStringFormatting::notations getNotation() const;

    // arrayAction
    void setArrayAction( QEStringFormatting::arrayActions arrayAction );
    QEStringFormatting::arrayActions getArrayAction() const;

    // Access underlying local enumerations object (as opposed to property string)
    QELocalEnumeration getLocalEnumerationObject() const;

protected:
    QEStringFormatting stringFormatting;
};

#endif // QE_STRING_FORMATTING_METHODS_H
