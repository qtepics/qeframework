/*  QEAnalogProgressBar.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2011-2024 Australian Synchrotron
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_ANALOG_PROGRESS_BAR_H
#define QE_ANALOG_PROGRESS_BAR_H

#include <QString>
#include <QVector>
#include <QEEnums.h>
#include <QEAnalogIndicator.h>
#include <QEWidget.h>
#include <QEFloating.h>
#include <QEFloatingFormatting.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QESingleVariableMethods.h>
#include <QEStringFormattingMethods.h>


class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEAnalogProgressBar :
      public QEAnalogIndicator,
      public QEWidget,
      public QESingleVariableMethods,
      public QEStringFormattingMethods
{
    Q_OBJECT

    // BEGIN-SINGLE-VARIABLE-V2-PROPERTIES ===============================================
    // Single Variable properties
    // These properties should be identical for every widget using a single variable.
    // WHEN MAKING CHANGES: Use the update_widget_properties script in the resources
    // directory.
    //
    // Note, a property macro in the form 'Q_PROPERTY(QString variableName READ ...' doesn't work.
    // A property name ending with 'Name' results in some sort of string a variable being displayed,
    // but will only accept alphanumeric and won't generate callbacks on change.
public:
    /// EPICS variable name (CA PV)
    ///
    Q_PROPERTY (QString variable READ getVariableNameProperty WRITE setVariableNameProperty)

    /// Macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2...
    /// Values may be quoted strings. For example, 'PUMP=PMP3, NAME = "My Pump"'
    /// These substitutions are applied to variable names for all QE widgets.
    /// In some widgets are are also used for other purposes.
    ///
    Q_PROPERTY (QString variableSubstitutions READ getVariableNameSubstitutionsProperty WRITE setVariableNameSubstitutionsProperty)

    /// The number of elements required to be subscribed for from the PV host (IOC).
    /// The default is 0 which means subscribed for all elements.
    /// Note: changing this value causes the unsubscribe/re-subscribe just as if the
    /// variable name changed.
    ///
    Q_PROPERTY (int elementsRequired READ getElementsRequired WRITE setElementsRequired)

    /// Index used to select a single item of data for processing. The default is 0.
    ///
    Q_PROPERTY (int arrayIndex READ getArrayIndex WRITE setArrayIndex)
    //
    // END-SINGLE-VARIABLE-V2-PROPERTIES =================================================

    // BEGIN-STANDARD-PROPERTIES ======================================================
    // Standard properties
    // These properties should be identical for every widget using them.
    // WHEN MAKING CHANGES: Use the update_widget_properties script in the
    // resources directory.
public:
    /// Use the variable as the tool tip. Default is true. Tool tip property
    /// will be overwritten by the variable name.
    ///
    Q_PROPERTY(bool variableAsToolTip READ getVariableAsToolTip WRITE setVariableAsToolTip)

    /// Allow drag/drops operations to this widget. Default is false.
    /// Any dropped text will be used as a new variable name.
    ///
    Q_PROPERTY(bool allowDrop READ getAllowDrop WRITE setAllowDrop)

    /// Display the widget. Default is true.
    /// Setting this property false is usefull if widget is only used to provide
    /// a signal - for example, when supplying data to a QELink widget.
    /// Note, when false the widget will still be visible in Qt Designer.
    ///
    Q_PROPERTY(bool visible READ getRunVisible WRITE setRunVisible)

    /// Set the ID used by the message filtering system. Default is zero.
    /// Widgets or applications that use messages from the framework have the option
    /// of filtering on this ID. For example, by using a unique message source ID
    /// a QELog widget may be set up to only log messages from a select set of widgets.
    ///
    Q_PROPERTY(unsigned int messageSourceId READ getMessageSourceId WRITE setMessageSourceId )

    /// Hide style sheet from designer as style calculation by the styleManager
    /// and not directly setable per se.
    /// This also stops transient styles being saved to the ui file.
    Q_PROPERTY(QString styleSheet   READ styleSheet       WRITE setStyleSheet  DESIGNABLE false)

    /// Style Sheet string to be applied before, i.e. lower priority than, any other
    /// style, e.g. alarm style and/or user level style.
    /// Default is an empty string.
    ///
    Q_PROPERTY(QString defaultStyle READ getStyleDefault  WRITE setStyleDefault)

    /// Style Sheet string to be applied when the widget is displayed in 'User' mode. Default is an empty string.
    /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
    /// This Style Sheet string will be applied by the styleManager class.
    /// Refer to the styleManager class for details about how this Style Sheet
    /// string will be merged with any pre-existing Style Sheet string
    /// and any Style Sheet strings generated during the display of data.
    ///
    Q_PROPERTY(QString userLevelUserStyle READ getStyleUser WRITE setStyleUser)

    /// Style Sheet string to be applied when the widget is displayed in 'Scientist' mode. Default is an empty string.
    /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
    /// This Style Sheet string will be applied by the styleManager class.
    /// Refer to the styleManager class for details about how this Style Sheet
    /// string will be merged with any pre-existing Style Sheet string
    /// and any Style Sheet strings generated during the display of data.
    ///
    Q_PROPERTY(QString userLevelScientistStyle READ getStyleScientist WRITE setStyleScientist)

    /// Style Sheet string to be applied when the widget is displayed in 'Engineer' mode. Default is an empty string.
    /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
    /// This Style Sheet string will be applied by the styleManager class.
    /// Refer to the styleManager class for details about how this Style Sheet string
    /// will be merged with any pre-existing Style Sheet string
    /// and any Style Sheet strings generated during the display of data.
    ///
    Q_PROPERTY(QString userLevelEngineerStyle READ getStyleEngineer WRITE setStyleEngineer)

    /// Lowest user level at which the widget is visible. Default is 'User'.
    /// Used when designing GUIs that display more and more detail according to the user mode.
    /// The user mode is set application wide through the QELogin widget, or programatically through setUserLevel()
    /// Widgets that are always visible should be visible at 'User'.
    /// Widgets that are only used by scientists managing the facility should be visible at 'Scientist'.
    /// Widgets that are only used by engineers maintaining the facility should be visible at 'Engineer'.
    ///
    Q_PROPERTY(QE::UserLevels userLevelVisibility READ getUserLevelVisibility WRITE setUserLevelVisibility)

    /// Lowest user level at which the widget is enabled. Default is 'User'.
    /// Used when designing GUIs that allow access to more and more detail according to the user mode.
    /// The user mode is set application wide through the QELogin widget, or programatically through setUserLevel()
    /// Widgets that are always accessable should be visible at 'User'.
    /// Widgets that are only accessable to scientists managing the facility should be visible at 'Scientist'.
    /// Widgets that are only accessable to engineers maintaining the facility should be visible at 'Engineer'.
    ///
    Q_PROPERTY(QE::UserLevels userLevelEnabled READ getUserLevelEnabled WRITE setUserLevelEnabled)

    /// If 'Always' (default) widget will indicate the alarm state of any variable data it is displaying, including 'No Alarm'
    /// If 'WhenInAlarm' widget only indicate the alarm state of any variable data it is displaying if it is 'in alarm' or 'Out of Service'.
    /// If 'WhenInvalid' widget only indicate the alarm state of any variable data it is in the 'Invalid' alarm state or 'Out of Service'.
    /// If 'Never' widget will never indicate the alarm state of any variable data it is displaying.
    /// Typically the background colour is set to indicate the alarm state.
    /// Note, this property is included in the set of standard properties as it applies to most widgets.
    /// It will do nothing for widgets that don't display data.
    ///
    Q_PROPERTY(QE::DisplayAlarmStateOptions displayAlarmStateOption
               READ getDisplayAlarmStateOption WRITE setDisplayAlarmStateOption)

    /// Indicates whether the widget should repond to the underlying PV(s) being declared Out of Service.
    /// The default oosAware value is most often true, however is false for some widgets.
    ///
    Q_PROPERTY(bool oosAware READ getOosAware WRITE setOosAware)

public:
    // END-STANDARD-PROPERTIES ========================================================


    // Analog Progress Bar specific properties ========================================
    // We want to colocate displayAlarmState and AlarmSeverityDisplayMode within
    // designer.
    //
public:
    enum AlarmSeverityDisplayModes { foreground, background };
    Q_ENUM (AlarmSeverityDisplayModes)

    /// Visualise the EPICS alarm severity
    ///
    Q_PROPERTY( AlarmSeverityDisplayModes alarmSeverityDisplayMode
                READ getAlarmSeverityDisplayMode WRITE setAlarmSeverityDisplayMode )

    // Display properties
    /// Use the EPICS database display limits
    ///
    Q_PROPERTY( bool useDbDisplayLimits READ getUseDbDisplayLimits WRITE setUseDbDisplayLimits )

    // Make the value and isActive proerties non-designable. This both hides the properties
    // within designer and stops the values from being written to the .ui file.
    //
    Q_PROPERTY (int    value    READ getValue     WRITE setValue    DESIGNABLE false)
    Q_PROPERTY (bool   isActive READ getIsActive  WRITE setIsActive DESIGNABLE false)
    //
    // End of Analog Progress Bar specific properties =================================


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

    /// The minimum number of leading digits before any decimal/radix point. The defult is 1.
    ///
    Q_PROPERTY(int leadingZeros READ getLeadingZeros WRITE setLeadingZeros)

    /// If true (default), add engineering units supplied with the data.
    ///
    Q_PROPERTY(bool addUnits READ getAddUnits WRITE setAddUnits)

    /// If false (default), no "+" sign, when true always add a sign.
    ///
    Q_PROPERTY(bool forceSign READ getForceSign WRITE setForceSign)

    // NOTE, keep in sync. The documentation below is repeated in
    // QEStringFormatting::setLocalEnumeration() (in QEStringformatting.cpp)
    //
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

    /// Format to apply to data. Default is 'Default' in which case the data type supplied with the data determines how the data is formatted.
    /// For all other options, an attempt is made to format the data as requested (whatever its native form).
    Q_PROPERTY(QE::Formats format READ getFormatProperty WRITE setFormatProperty)

    void setFormatProperty (QE::Formats format) { this->setFormat(format); }  ///< Access function for #format property - refer to #format property for details
    QE::Formats getFormatProperty() const { return this->getFormat(); }       ///< Access function for #format property - refer to #format property for details

    /// Base used for when formatting integers. Default is 10 (duh!)
    ///
    Q_PROPERTY(int radix READ getRadix WRITE setRadix)

    /// Seperators used for interger and fixed point formatting. Default is None.
    ///
    Q_PROPERTY(QE::Separators separator READ getSeparator WRITE setSeparator)

    /// Notation used for numerical formatting. Default is fixed.
    ///
    Q_PROPERTY(QE::Notations notation READ getNotation WRITE setNotation)

    /// Text formatting option for array data. Default is ASCII. Options are:
    /// \li ASCII - treat array as a single text string. For example an array of three characters 'a' 'b' 'c' will be formatted as 'abc'.
    /// \li APPEND - treat array as an array of numbers and format a string containing them all with a space between each.
    ///              For example, an array of three numbers 10, 11 and 12 will be formatted as '10 11 12'.
    /// \li INDEX - Extract a single item from the array. The item is then formatted as any other non array data would be.
    ///             The item selected is determined by the arrayIndex property. For example, if arrayIndex property is 1,
    ///             an array of three numbers 10, 11 and 12 will be formatted as '11'.
    ///
    Q_PROPERTY(QE::ArrayActions arrayAction READ getArrayAction WRITE setArrayAction)

public:
    // END-STRING-FORMATTING-PROPERTIES ===============================================

public:
    /// Create without a variable.
    /// Use setVariableNameProperty() and setSubstitutionsProperty() to define
    /// a variable and, optionally, macro substitutions later.
    ///
    QEAnalogProgressBar( QWidget *parent = 0 );

    /// Create with a variable.
    /// A connection is automatically established.
    /// If macro substitutions are required, create without a variable and set
    /// the variable and macro substitutions after creation.
    ///
    QEAnalogProgressBar( const QString &variableName, QWidget *parent = 0 );

    /// Destruction
    virtual ~QEAnalogProgressBar() {}

    // useDbDisplayLimits, e.g. as specified by LOPR and HOPR fields for ai, ao, longin
    // and longout record types, to call setAnalogMinimum and setAnalogMaximum.
    //
    void setUseDbDisplayLimits (bool useDbDisplayLimitsIn);                  ///< Access function for #useDbDisplayLimits property - refer to #useDbDisplayLimits property for details
    bool getUseDbDisplayLimits () const;                                     ///< Access function for #useDbDisplayLimits property - refer to #useDbDisplayLimits property for details

    void setAlarmSeverityDisplayMode (const AlarmSeverityDisplayModes mode); ///< Access function for #AlarmSeverityDisplayModes property - refer to #AlarmSeverityDisplayModes property for details
    AlarmSeverityDisplayModes getAlarmSeverityDisplayMode () const;          ///< Access function for #AlarmSeverityDisplayModes property - refer to #AlarmSeverityDisplayModes property for details

signals:
    // Note, the following signals are common to many QE widgets,
    // if changing the doxygen comments, ensure relevent changes are migrated to all instances
    // These signals are emitted using the QEEmitter::emitDbValueChanged function.
    /// Sent when the widget is updated following a data change
    /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
    /// For example a QList widget could log updates from this widget.
    void dbValueChanged ();                     // signal event
    void dbValueChanged (const QString& out);   // signal as formatted text
    void dbValueChanged (const int& out);       // signal as int if applicable
    void dbValueChanged (const long& out);      // signal as long if applicable
    void dbValueChanged (const qlonglong& out); // signal as qlonglong if applicable
    void dbValueChanged (const double& out);    // signal as floating if applicable
    void dbValueChanged (const bool& out);      // signal as bool: value != 0 if applicable

    // This signal is emitted using the QEEmitter::emitDbConnectionChanged function.
    /// Sent when the widget state updated following a channel connection change
    /// Applied to provary varible.
    void dbConnectionChanged (const bool& isConnected);

    /// Internal use only. Used when changing a property value to force a
    /// re-display to reflect the new property value.
    void requestResend();

public slots:
    /// Slot to set the visibility of a QE widget, taking into account the user level.
    /// Widget will be hidden if hidden by a call this slot, by will only be made
    /// visible by a calll to this slot if the user level allows.
    ///
    void setManagedVisible (bool v) { this->setRunVisible(v); }

protected:
    void establishConnection( unsigned int variableIndex );
    qcaobject::QCaObject* createQcaItem( unsigned int variableIndex );
    void stringFormattingChange() { emit requestResend();  }

    // Drag and Drop
    void dragEnterEvent(QDragEnterEvent *event) { qcaDragEnterEvent( event ); }
    void dropEvent(QDropEvent *event)           { qcaDropEvent( event ); }
    void mousePressEvent(QMouseEvent *event)    { qcaMousePressEvent( event ); }
    // Use default getDrop/setDrop

    // Copy paste
    QString copyVariable ();
    QVariant copyData ();
    void paste (QVariant s);

    // Override QEAnalogIndicator hook functions.
    //
    QColor getBackgroundPaintColour () const;
    QColor getForegroundPaintColour () const;

    QString getTextImage () const;
    BandList getBandList () const;

private:
    void setup();
    bool useAlarmColours (QCaAlarmInfo& alarmInfo) const;
    Band createBand (const double lower, const double upper,
                     unsigned short severity) const;

    QEFloatingFormatting floatingFormatting;
    bool useDbPrecison;
    bool useDbDisplayLimits;
    AlarmSeverityDisplayModes alarmSeverityDisplayMode;
    bool isFirstUpdate;
    QString theImage;

private slots:
    void connectionChanged (QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex);

    void setProgressBarValue (const double& value, QCaAlarmInfo&, QCaDateTime&, const unsigned int&);

    void useNewVariableNameProperty (QString variableNameIn,
                                     QString variableNameSubstitutionsIn,
                                     unsigned int variableIndex);
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEAnalogProgressBar::AlarmSeverityDisplayModes)
#endif

#endif // QE_ANALOG_PROGRESS_BAR_H
