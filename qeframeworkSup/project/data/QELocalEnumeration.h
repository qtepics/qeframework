/*  QELocalEnumeration.h
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

#ifndef QELOCALENUMERATION_H
#define QELOCALENUMERATION_H

#include <QList>
#include <QString>
#include <QVariant>

#include <QEFrameworkLibraryGlobal.h>

/// This class allows a user defined two-way value to enumeration map.
/// The map is define using a single string, typically a widget property string.
/// This may then be used to replace the enumeration values provided by EPICS and/or
/// provide an enueration set of more that 16 values.
/// See setLocalEnumeration() for the use of 'localEnumeration'.
///
/// This functionality that this class provided was formerly embedded within
/// QEStringFormatting.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QELocalEnumeration {
public:

    /// Constructors
    ///
    QELocalEnumeration();

    /// Constructor with localEnumeration
    ///
    QELocalEnumeration( const QString &  localEnumeration );

    /// Parse the local enumeration string.
    ///
    /// Format is:
    ///
    ///  [[<|<=|=|!=|>=|>]value1|*] : string1 , [[<|<=|=|!=|>=|>]value2|*] : string2 , [[<|<=|=|!=|>=|>]value3|*] : string3 , ...
    ///
    /// Where:
    ///   <  Less than
    ///   <= Less than or equal
    ///   =  Equal (default if no operator specified)
    ///   >= Greather than or equal
    ///   >  Greater than
    ///   *  Always match (used to specify default text)
    ///
    /// Values may be numeric or textual
    /// Values do not have to be in any order, but first match wins
    /// Values may be quoted
    /// Strings may be quoted
    /// Consecutive values do not have to be present.
    /// Operator is assumed to be equality if not present.
    /// White space is ignored except within quoted strings.
    /// \n may be included in a string to indicate a line break
    ///
    /// Examples are:
    ///
    /// 0:Off,1:On
    /// 0 : "Pump Running", 1 : "Pump not running"
    /// 0:"", 1:"Warning!\nAlarm"
    /// <2:"Value is less than two", =2:"Value is equal to two", >2:"Value is grater than 2"
    /// 3:"Beamline Available", *:""
    /// "Pump Off":"OH NO!, the pump is OFF!","Pump On":"It's OK, the pump is on"
    ///
    /// The data value is converted to a string if no enumeration for that value is available.
    /// For example, if the local enumeration is '0:off,1:on', and a value of 10 is processed, the text generated is '10'.
    /// If a blank string is required, this should be explicit. for example, '0:off,1:on,10:""'
    ///
    /// A range of numbers can be covered by a pair of values as in the following example: >=4:"Between 4 and 8",<=8:"Between 4 and 8"
    ///
    /// Will completely re-initialises the object.
    ///
    void setLocalEnumeration( const QString &  localEnumeration );


    /// Get the local enumeration strings. See setLocalEnumeration() for the use of 'localEnumeration'.
    ///
    QString getLocalEnumeration() const;


    /// Evaluates: getLocalEnumeration.count() > 0
    ///
    bool isDefined() const;

    /// Format a variant value using local enumeration list.
    /// If the value is numeric, then the value is compared to the numeric
    /// interpretation of the enumeration values, if the value is textual,
    /// then the value is compared to the textual enumeration values.
    ///
    QString valueToText( const QVariant & value, bool& match ) const;

    /// Generate a value given a string, using formatting defined within this class.
    /// If the value can be formatted the formatted value is returned and 'ok' is true.
    /// If the value can't be formatted an error string is returned and 'ok' is false
    ///
    QVariant textToValue( const QString & text, bool& ok ) const;

    // Convenience wrapper functions.
    //
    /// Generate an integer value given a string, using formatting defined within this class.
    /// If the value can be formatted the formatted value is returned and 'ok' is true.
    /// If the value can't be formatted then 0 is returned and 'ok' is false.
    ///
    int textToInt( const QString & text, bool& ok ) const;

    /// Generate a double value given a string, using formatting defined within this class.
    /// If the value can be formatted the formatted value is returned and 'ok' is true.
    /// If the value can't be formatted then 0.0 is returned and 'ok' is false.
    ///
    double textToDouble( const QString & text, bool& ok ) const;

private:
    // Support class used to build the localEnumeration list in the QEStringFormatting class
    //
    class localEnumerationItem {
        public:
        enum operations { LESS,
                          LESS_EQUAL,
                          EQUAL,
                          NOT_EQUAL,
                          GREATER_EQUAL,
                          GREATER,
                          ALWAYS,
                          UNKNOWN
                      };
        double dValue;        // Numeric value that numeric data is compared to (derived from sValue if possible)
        QString sValue;       // Text value that textual data is compared with
        operations op;        // Operator used for comparison used between data and value (=,<,>)
        QString text;         // Text used if the data value matches
    };

    typedef QList<localEnumerationItem>  LocalEnumerationLItemList;


    QString localEnumerationString;   // Raw string as supplied to constructor

    LocalEnumerationLItemList localEnumeration; // Local enumerations (example: 0="Not referencing",1=Referencing)
};

#endif
