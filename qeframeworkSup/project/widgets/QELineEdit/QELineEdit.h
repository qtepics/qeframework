/*  QELineEdit.h
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
 *  Copyright (c) 2009,2010,2012,2016,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QE_LINE_EDIT_H
#define QE_LINE_EDIT_H

#include <QLineEdit>
#include <QEWidget.h>
#include <QEString.h>
#include <QEStringFormatting.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QEStringFormattingMethods.h>
#include <QEGenericEdit.h>
#include <QCaVariableNamePropertyManager.h>


class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QELineEdit :
      public QEGenericEdit,
      public QEStringFormattingMethods
{
    Q_OBJECT

    // Single variable properties, control widget properties and standard properties are
    // all defined in QEGenericEdit

    // BEGIN-STRING-FORMATTING-PROPERTIES =============================================
    // String formatting properties
    // These properties should be identical for every widget managing strings.
    // WHEN MAKING CHANGES: Use the update_widget_properties script in the
    // resources directory.
public:
    /// Precision used when formatting floating point numbers. The default is 4.
    /// This is only used if useDbPrecision is false.
    Q_PROPERTY(int  precision READ getPrecision WRITE setPrecision)

    /// If true (default), format floating point numbers using the precision supplied with the data.
    /// If false, the precision property is used.
    Q_PROPERTY(bool useDbPrecision READ getUseDbPrecision WRITE setUseDbPrecision)

    /// If true (default), always add a leading zero when formatting numbers.
    ///
    Q_PROPERTY(bool leadingZero READ getLeadingZero WRITE setLeadingZero)

    /// If true (default), always remove any trailing zeros when formatting numbers.
    ///
    Q_PROPERTY(bool trailingZeros READ getTrailingZeros WRITE setTrailingZeros)

    /// If true (default), add engineering units supplied with the data.
    ///
    Q_PROPERTY(bool addUnits READ getAddUnits WRITE setAddUnits)

    /// If false (default), no "+" sign, when true always add a sign.
    ///
    Q_PROPERTY(bool forceSign READ getForceSign WRITE setForceSign)

    // NOTE, keep in sync. The documentation below is repeated in QEStringFormatting::setLocalEnumeration() (in QEStringformatting.cpp)
    /// An enumeration list used to data values. Used only when the formatting option is 'local enumeration'.
    /// Value is converted to an integer and used to select a string from this list.
    ///
    /// Format is:
    ///
    ///   [[<|<=|=|!=|>=|>]value1|*] : string1 , [[<|<=|=|!=|>=|>]value2|*] : string2 , [[<|<=|=|!=|>=|>]value3|*] : string3 , ...
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
    Q_PROPERTY(QString/*localEnumerationList*/ localEnumeration READ getLocalEnumeration WRITE setLocalEnumeration)

    /// \enum    Formats
    /// User friendly enumerations for format property - refer to QEStringFormatting::formats for details.
    enum Formats { Default          = QEStringFormatting::FORMAT_DEFAULT,            ///< Format as best appropriate for the data type
                   Floating         = QEStringFormatting::FORMAT_FLOATING,           ///< Format as a floating point number
                   Integer          = QEStringFormatting::FORMAT_INTEGER,            ///< Format as an integer
                   UnsignedInteger  = QEStringFormatting::FORMAT_UNSIGNEDINTEGER,    ///< Format as an unsigned integer
                   Time             = QEStringFormatting::FORMAT_TIME,               ///< Format as a time
                   LocalEnumeration = QEStringFormatting::FORMAT_LOCAL_ENUMERATE     ///< Format as a selection from the #localEnumeration property
                };
    void setFormatProperty( Formats format ){ setFormat( (QEStringFormatting::formats)format ); }  ///< Access function for #format property - refer to #format property for details
    Formats getFormatProperty(){ return (Formats)getFormat(); }                                    ///< Access function for #format property - refer to #format property for details
    Q_ENUMS(Formats)

    /// Format to apply to data. Default is 'Default' in which case the data type supplied with the data determines how the data is formatted.
    /// For all other options, an attempt is made to format the data as requested (whatever its native form).
    Q_PROPERTY(Formats format READ getFormatProperty WRITE setFormatProperty)

    /// Base used for when formatting integers. Default is 10 (duh!)
    ///
    Q_PROPERTY(int radix READ getRadix WRITE setRadix)

    /// \enum Separators
    /// User friendly enumerations for seprator property - refer to QEStringFormatting::formats for details.
    enum Separators { NoSeparator = QEStringFormatting::SEPARATOR_NONE,         ///< Use no separator
                      Comma       = QEStringFormatting::SEPARATOR_COMMA,        ///< Use ',' as separator
                      Underscore  = QEStringFormatting::SEPARATOR_UNDERSCORE,   ///< Use '_' as separator
                      Space       = QEStringFormatting::SEPARATOR_SPACE         ///< Use ' ' as separator
                    };
    Q_ENUMS (Separators)
    void setSeparatorProperty( const Separators notation ){ setSeparator( (QEStringFormatting::separators)notation ); }  ///< Access function for #separator property - refer to #separator property for details
    Separators getSeparatorProperty() const { return (Separators)getSeparator(); }                                       ///< Access function for #separator property - refer to #separator property for details

    /// Seperators used for interger and fixed point formatting. Default is None.
    ///
    Q_PROPERTY(Separators separator READ getSeparatorProperty WRITE setSeparatorProperty)

    /// \enum Notations
    /// User friendly enumerations for notation property - refer to QEStringFormatting::notations for details.
    enum Notations { Fixed       = QEStringFormatting::NOTATION_FIXED,              ///< Refer to QEStringFormatting::NOTATION_FIXED for details
                     Scientific  = QEStringFormatting::NOTATION_SCIENTIFIC,         ///< Refer to QEStringFormatting::NOTATION_SCIENTIFIC for details
                     Automatic   = QEStringFormatting::NOTATION_AUTOMATIC           ///< Refer to QEStringFormatting::NOTATION_AUTOMATIC for details
                };
    void setNotationProperty( Notations notation ){ setNotation( (QEStringFormatting::notations)notation ); }  ///< Access function for #notation property - refer to #notation property for details
    Notations getNotationProperty(){ return (Notations)getNotation(); }                                        ///< Access function for #notation property - refer to #notation property for details
    Q_ENUMS(Notations)
    /// Notation used for numerical formatting. Default is fixed.
    ///
    Q_PROPERTY(Notations notation READ getNotationProperty WRITE setNotationProperty)

    /// \enum ArrayActions
    /// User friendly enumerations for arrayAction property - refer to QEStringFormatting::arrayActions for details.
    enum ArrayActions { Append = QEStringFormatting::APPEND,            ///< Refer to QEStringFormatting::APPEND for details
                        Ascii  = QEStringFormatting::ASCII,             ///< Refer to QEStringFormatting::ASCII for details
                        Index  = QEStringFormatting::INDEX              ///< Refer to QEStringFormatting::INDEX for details
                    };
    void setArrayActionProperty( ArrayActions arrayAction ){ setArrayAction( (QEStringFormatting::arrayActions)arrayAction ); }    ///< Access function for #arrayAction property - refer to #arrayAction property for details
    ArrayActions getArrayActionProperty(){ return (ArrayActions)getArrayAction(); }                                                ///< Access function for #arrayAction property - refer to #arrayAction property for details
    Q_ENUMS(ArrayActions)

    /// Text formatting option for array data. Default is ASCII. Options are:
    /// \li ASCII - treat array as a single text string. For example an array of three characters 'a' 'b' 'c' will be formatted as 'abc'.
    /// \li APPEND - treat array as an array of numbers and format a string containing them all with a space between each. For example, an array of three numbers 10, 11 and 12 will be formatted as '10 11 12'.
    /// \li INDEX - Extract a single item from the array. The item is then formatted as any other non array data would be. The item selected is determined by the arrayIndex property. For example, if arrayIndex property is 1, an array of three numbers 10, 11 and 12 will be formatted as '11'.
    Q_PROPERTY(ArrayActions arrayAction READ getArrayActionProperty WRITE setArrayActionProperty)
public:
    // END-STRING-FORMATTING-PROPERTIES ===============================================

public:
    /// Create without a variable.
    /// Use setVariableNameProperty() and setSubstitutionsProperty() to define a variable and, optionally, macro substitutions later.
    QELineEdit( QWidget *parent = 0 );

    /// Create with a variable.
    /// A connection is automatically established.
    /// If macro substitutions are required, create without a variable and set the variable and macro substitutions after creation.
    QELineEdit( const QString &variableName, QWidget *parent = 0 );

private:
    void establishConnection( unsigned int variableIndex );
    void setup();

private slots:
    void setTextIfNoFocus( const QString& value, QCaAlarmInfo&, QCaDateTime&, const unsigned int& );  // Update the text in the widget as long as the user is not entering data in it

signals:
    // Note, the following signals are common to many QE widgets,
    // if changing the doxygen comments, ensure relevent changes are migrated to all instances
    // These signals are emitted using the QEEmitter::emitDbValueChanged function.
    /// Sent when the widget is updated following a data change
    /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
    /// For example a QList widget could log updates from this widget.
    void dbValueChanged( const QString& out );   // signal as enumeration text
    void dbValueChanged( const int& out );       // signal as int if applicable
    void dbValueChanged( const long& out );      // signal as long if applicable
    void dbValueChanged( const qlonglong& out ); // signal as qlonglong if applicable
    void dbValueChanged( const double& out );    // signal as floating if applicable
    void dbValueChanged( const bool& out );      // signal as bool: value != 0 if applicable

    // This signal is emitted using the QEEmitter::emitDbConnectionChanged function.
    /// Sent when the widget state updated following a channel connection change
    void dbConnectionChanged (const bool& isConnected);

    /// Internal use only. Used by QEConfiguredLayout to be notified when one of its widgets has written something
    void userChange( const QString& oldValue, const QString& newValue, const QString& lastValue );    // Signal a user attempt to change a value. Values are strings as the user sees them

    /// Internal use only. Used when changing a property value to force a re-display to reflect the new property value.
    void requestResend();

private:
    qcaobject::QCaObject* createQcaItem( unsigned int variableIndex );

    void stringFormattingChange() { requestResend(); }

    // Override QEGenericEdit abstract functions
    //
    /// Sets the undelying QLineEdit widget to the given value.
    void setValue( const QVariant & value );

    /// Gets the undelying value.
    QVariant getValue();

    /// Write the data to the channel.
    bool writeData( const QVariant & value, QString& message );

};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QELineEdit::Formats)
Q_DECLARE_METATYPE (QELineEdit::Separators)
Q_DECLARE_METATYPE (QELineEdit::Notations)
Q_DECLARE_METATYPE (QELineEdit::ArrayActions)
#endif

#endif   // QE_LINE_EDIT_H
