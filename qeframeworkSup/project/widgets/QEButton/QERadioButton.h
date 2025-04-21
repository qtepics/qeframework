/*  QERadioButton.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2024 Australian Synchrotron
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

#ifndef QE_RADIO_BUTTON_H
#define QE_RADIO_BUTTON_H

#include <QRadioButton>
#include <QEEnums.h>
#include <QEWidget.h>
#include <QEString.h>
#include <QEStringFormatting.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QEForm.h>
#include <managePixmaps.h>
#include <QEGenericButton.h>
#include <QCaVariableNamePropertyManager.h>


class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QERadioButton : 
   public QRadioButton, 
   public QEGenericButton 
{
    Q_OBJECT

    // BEGIN-SINGLE-VARIABLE-V3-PROPERTIES ===============================================
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

    /// Index used to select a single item of data for processing. The default is 0.
    ///
    Q_PROPERTY (int arrayIndex READ getArrayIndex WRITE setArrayIndex)
    //
    // END-SINGLE-VARIABLE-V3-PROPERTIES =================================================

    
    // BEGIN-GENERIC-BUTTON-PROPERTIES ===================================================
    // Generic button properties
    // These properties should be identical for specific button widgets (QEPushButton, QERadioButton and QECheckBox)
    // WHEN MAKING CHANGES: search for GENERIC-BUTTON-PROPERTIES  and change all occurances.
public:
    // Important, as in most commonly used, properties first.
    //
    /// Button label text (prior to substitution).
    /// Macro substitutions will be applied to this text and the result will be set as the button text.
    /// Used when data updates are not being represented in the button text.
    /// IF NOT LEFT EMPTY, THIS TEXT WILL TAKE PRIORITY OVER THE PUSH BUTTON 'text' PROPERTY!
    /// For example, a button in a sub form may have a 'labelText' property of 'Turn Pump $(PUMPNUM) On'.
    /// When the sub form is used twice in a main form with substitutions PUMPNUM=1 and PUMPNUM=2 respectively,
    /// the two identical buttons in the sub forms will have the labels 'Turn Pump 1 On' and 'Turn Pump 2 On' respectively.
    Q_PROPERTY(QString labelText READ getLabelTextProperty WRITE setLabelTextProperty)

    /// Value written when user clicks button if 'writeOnClick' property is true.
    /// Note: Set format to Integer when writing numeric values to enum PVs like the bo and mbbo records. 
    ///
    Q_PROPERTY(QString clickText READ getClickText WRITE setClickText)

    /// If true, the 'clickText' property is written when the button is clicked. Default is true
    ///
    Q_PROPERTY(bool writeOnClick READ getWriteOnClick WRITE setWriteOnClick)

    //--------------------------------------------------------------------------
    // Opening new gui ui file.
    //
    // Note, a property macro in the form 'Q_PROPERTY(QString guiName READ ...' doesn't work.
    // A property name ending with 'Name' results in some sort of string variable being displayed,
    // but will only accept alphanumeric and won't generate callbacks on change.
    /// File name of GUI to be presented on button click.
    /// File name can be absolute, relative to the path of the QEForm in which the button widget is located,
    /// relative to the any path in the path list published in the ContainerProfile class, or relative to the current path.
    /// See QEWidget::openQEFile() in QEWidget.cpp for details.
    Q_PROPERTY(QString guiFile READ getGuiName WRITE setGuiName)

    /// Overriding macro substitutions. These macro substitions take precedence over any existing
    /// macro substitutions defined by the variableSubstitutions property, any parent forms, or
    /// the application containing the button. These macro substitutions are particularly usefull when
    /// the button's function is to reload the same form but with different macro substitutions.
    /// The variableSubstitutions property cannot be used for this since, although they are
    /// added to the list of macro substittions applied to the new form, they are appended
    /// to the list and the existing macro substitutions take precedence.
    Q_PROPERTY(QString prioritySubstitutions READ getPrioritySubstitutions WRITE setPrioritySubstitutions)

    /// Creation options when opening a new GUI. Open a new window, open a new tab, or replace the current window.
    /// the creation option is supplied when the button generates a newGui signal.
    /// Application code connected to this signal should honour this request if possible.
    /// When used within the QEGui application, the QEGui application creates a new window, new tab, or replaces the current window as appropriate.
    Q_PROPERTY(QE::CreationOptions creationOption READ getCreationOption WRITE setCreationOption)

    
    /// Window customisation name. This name will be used to select a set of window customisations including menu items and tool bar buttons.
    /// Applications such as QEGui can load .xml files containing named sets of window customisations. This property is used to select a set loaded from these files.
    /// The selected set of customisations will be applied to the main window containing the new GUI.
    /// Customisations are not applied if the GUI is opened as a dock.
    Q_PROPERTY(QString customisationName READ getCustomisationName WRITE setCustomisationName)


    //--------------------------------------------------------------------------
    /// Program to run when the button is clicked.
    /// No attempt to run a program is made if this property is empty.
    /// Example: firefox
    Q_PROPERTY(QString program READ getProgram WRITE setProgram)

    /// Arguments for program specified in the 'program' property.
    ///
    Q_PROPERTY(QStringList arguments READ getArguments WRITE setArguments)

    /// Startup options. Just run the command, run the command within a terminal, or display the output in QE message system.
    ///
    Q_PROPERTY(QE::ProgramStartupOptions programStartupOption
               READ getProgramStartupOption WRITE setProgramStartupOption)

    //--------------------------------------------------------------------------
    // Other button properties
    //    
    /// Set the widget's disabled record policy, i.e. the action to be taken when the under lying record is disabled,
    /// i.e. when the assiociated record's STAT field value is Disabled.
    /// When the policy is ignore, then no special action is taken. This is the default policy.
    /// When the policy is grayout, the widget is style is set as if disconnected when the record is disabled.
    /// When the policy is disable, the widget is disabled when the record is disabled.
    ///
    Q_PROPERTY(QE::DisabledRecordPolicy disabledRecordPolicy READ getDisabledRecordPolicy WRITE setDisabledRecordPolicy)

    /// Set the buttons text alignment.
    /// Left justification is particularly useful when displaying quickly changing numeric data updates.
    Q_PROPERTY(Qt::Alignment alignment READ getTextAlignment WRITE setTextAlignment )

    /// Update options (text, pixmap, both, or state (checked or unchecked)
    ///
    Q_PROPERTY(QE::UpdateOptions updateOption READ getUpdateOption WRITE setUpdateOption)

    /// Pixmap to display if updateOption is Icon or TextAndIcon and data value translates to an index of 0
    ///
    Q_PROPERTY(QPixmap pixmap0 READ getPixmap0Property WRITE setPixmap0Property)

    /// Pixmap to display if updateOption is Icon or TextAndIcon and data value translates to an index of 1
    ///
    Q_PROPERTY(QPixmap pixmap1 READ getPixmap1Property WRITE setPixmap1Property)

    /// Pixmap to display if updateOption is Icon or TextAndIcon and data value translates to an index of 2
    ///
    Q_PROPERTY(QPixmap pixmap2 READ getPixmap2Property WRITE setPixmap2Property)

    /// Pixmap to display if updateOption is Icon or TextAndIcon and data value translates to an index of 3
    ///
    Q_PROPERTY(QPixmap pixmap3 READ getPixmap3Property WRITE setPixmap3Property)

    /// Pixmap to display if updateOption is Icon or TextAndIcon and data value translates to an index of 4
    ///
    Q_PROPERTY(QPixmap pixmap4 READ getPixmap4Property WRITE setPixmap4Property)

    /// Pixmap to display if updateOption is Icon or TextAndIcon and data value translates to an index of 5
    ///
    Q_PROPERTY(QPixmap pixmap5 READ getPixmap5Property WRITE setPixmap5Property)

    /// Pixmap to display if updateOption is Icon or TextAndIcon and data value translates to an index of 6
    ///
    Q_PROPERTY(QPixmap pixmap6 READ getPixmap6Property WRITE setPixmap6Property)

    /// Pixmap to display if updateOption is Icon or TextAndIcon and data value translates to an index of 7
    ///
    Q_PROPERTY(QPixmap pixmap7 READ getPixmap7Property WRITE setPixmap7Property)

    /// Password user will need to enter before any action is taken
    ///
    Q_PROPERTY(QString password READ getPassword WRITE setPassword)

    /// If true, a dialog will be presented asking the user to confirm if the button action should be carried out
    ///
    Q_PROPERTY(bool confirmAction READ getConfirmAction WRITE setConfirmAction)

    /// Text used to confirm acion if confirmation dialog is presented
    ///
    Q_PROPERTY(QString confirmText READ getConfirmText WRITE setConfirmText)

    /// If true, the 'pressText' property is written when the button is pressed. Default is false
    ///
    Q_PROPERTY(bool writeOnPress READ getWriteOnPress WRITE setWriteOnPress)

    /// If true, the 'releaseText' property is written when the button is released. Default is false
    ///
    Q_PROPERTY(bool writeOnRelease READ getWriteOnRelease WRITE setWriteOnRelease)

    /// Value written when user presses button if 'writeOnPress' property is true
    ///
    Q_PROPERTY(QString pressText READ getPressText WRITE setPressText)

    /// Value written when user releases button if 'writeOnRelease' property is true
    ///
    Q_PROPERTY(QString releaseText READ getReleaseText WRITE setReleaseText)

    /// Text used to compare with text written or read to determine if push button should be marked as checked.
    /// Note, must be an exact match following formatting of data updates.
    /// When writing values, the 'pressText', 'ReleaseText', or 'clickedtext' must match this property to cause the button to be checked when the write occurs.
    ///
    /// Good example: formatting set to diaplay a data value of '1' as 'On', clickCheckedText is 'On', clickText is 'On'.
    /// In this example, the push button will be checked when a data update occurs with a value of 1 or when the button is clicked.
    ///
    /// Bad example: formatting set to diaplay a data value of '1' as 'On', clickCheckedText is 'On', clickText is '1'.
    /// In this example, the push button will be checked when a data update occurs with a value of 1 but, although 
    /// a valid value will be written when clicked, the button will not be checked when clicked as '1' is not the same as 'On'.
    Q_PROPERTY(QString clickCheckedText READ getClickCheckedText WRITE setClickCheckedText)
public:
    // END-GENERIC-BUTTON-PROPERTIES ==================================================
    
    
    //=================================================================================
    // Control widget properties
    // These properties should be similar for all widgets with control functionality (writing to variables, not just displaying).
    // WHEN MAKING CHANGES: search for CONTROLVARIABLEPROPERTIES and change all relevent occurances.
public:
    /// Sets if this widget subscribes for data updates and displays current data.
    /// Default is 'true' (subscribes for and displays data updates)
    Q_PROPERTY(bool subscribe READ getSubscribe WRITE setSubscribe)
public:
    //=================================================================================


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
    explicit QERadioButton( QWidget *parent = 0 );

    /// Create with a variable.
    /// A connection is automatically established.
    /// If macro substitutions are required, create without a variable and
    /// set the variable and macro substitutions after creation.
    ///
    explicit QERadioButton( const QString& variableName, QWidget *parent = 0 );

    /// Destructor
    ~QERadioButton();

public slots:
    // Write the click/clockChecked/press/release text value (of the 
    // associated button object) into the PV immediately.
    //
    void writeNow() { processWriteNow( isChecked () ); }

    // Note, keep in sync. The text below is repeated in QEPushButton.h, QERadioButton.h and QECheckBox.h
    /// Default slot used to create a new GUI if there is no slot indicated in the ContainerProfile class.
    /// This slot is typically used when the button is pressed within the Designer preview window to allow
    /// the operation of the button to be tested. If an application does not specify a slot to use for
    /// creating new windows (through the ContainerProfile class) a window will still be created through
    /// this slot, but it will not respect the window creation options or any other window related
    /// application constraints.  For example, the QEGui application does provide a slot for creating 
    /// new GUIs in the ContainerProfile class which respects the creation options, knows how to add tabs
    /// in the application, and extend the application's window menu in the menu bar.
    ///
    void requestAction( const QEActionRequests& request ){ startGui( request ); }

    /// Update the default style applied to this widget.
    ///
    void setDefaultStyle( const QString& style ) { setStyleDefault( style ); }

   /// Slot to set the visibility of a QE widget, taking into account the user level.
   /// Widget will be hidden if hidden by a call this slot, by will only be made
   /// visible by a calll to this slot if the user level allows.
   ///
   void setManagedVisible( bool v ){ setRunVisible( v ); }

signals:
    // Note, the following signals are common to many QE widgets,
    // if changing the doxygen comments, ensure relevent changes are migrated to all instances
    // These signals are emitted using the QEEmitter::emitDbValueChanged function.
    /// Sent when the widget is updated following a data change
    /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
    /// For example a QList widget could log updates from this widget.
    void dbValueChanged();                       // signal event
    void dbValueChanged( const QString& out );   // signal as formatted text
    void dbValueChanged( const int& out );       // signal as int if applicable
    void dbValueChanged( const long& out );      // signal as long if applicable
    void dbValueChanged( const qlonglong& out ); // signal as qlonglong if applicable
    void dbValueChanged( const double& out );    // signal as floating if applicable
    void dbValueChanged( const bool& out );      // signal as bool: value != 0 if applicable

    // This signal is emitted using the QEEmitter::emitDbConnectionChanged function.
    /// Sent when the widget state updated following a channel connection change
    void dbConnectionChanged( const bool& isConnected );

    /// Internal use only. Used when changing a property value to force
    /// a re-display to reflect the new property value.
    void requestResend();

    /// Internal use only. Request a new GUI is created.
    /// Typically, this is caught by the QEGui application.
    void newGui( const QEActionRequests& request );

    /// Button has been Pressed.
    /// The value emitted is the integer interpretation of the pressText property
    void pressed( int value );

    /// Button has been Released
    /// The value emitted is the integer interpretation of the releaseText property
    void released( int value );

    /// Button has been Clicked.
    /// The value emitted is the integer interpretation of the clickText property
    /// (or the clickCheckedText property if the button was checked)
    void clicked( int value );

    /// Program started by button has compelted
    void programCompleted();

private:
    void setup();

    void setButtonState( bool checked ){ QRadioButton::setChecked( checked ); }
    void setButtonText( QString text ){ QRadioButton::setText( text ); }
    QString getButtonText(){ return text(); }
    void setButtonIcon( QIcon& icon ) {setIcon( icon ); }

    void emitNewGui( const QEActionRequests& request ){ emit newGui( request ); }

    QAbstractButton* getButtonQObject(){ return this; }

    void stringFormattingChange(){ requestResend(); }

    QE::UpdateOptions getDefaultUpdateOption();

    void emitPressed( int value ){ emit pressed( value ); }
    void emitReleased( int value ){ emit released( value ); }
    void emitClicked( int value ){ emit clicked( value ); }

    // Drag and Drop (See QEGenericButton.cpp for common button drag and drop)
    void dragEnterEvent(QDragEnterEvent *event) { qcaDragEnterEvent( event ); }
    void dropEvent(QDropEvent *event)           { qcaDropEvent( event ); }
    // Don't drag from interactive widget void mousePressEvent(QMouseEvent *event)    { qcaMousePressEvent( event ); }

    // Copy paste (See QEGenericButton.cpp for common button copy and paste)
    QVariant copyData();

private slots:
    // Hand-ball to QEGenericButton.
    void connectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex ) { QEGenericButton::connectionChanged( connectionInfo, variableIndex ); }
    void setButtonText( const QString& text, QCaAlarmInfo& alarmInfo, QCaDateTime& timestamp, const unsigned int& variableIndex ) { setGenericButtonText( text, alarmInfo, timestamp, variableIndex); }
    void userPressed() { QEGenericButton::userPressed(); }
    void userReleased() { QEGenericButton::userReleased(); }
    void userClicked( bool checked ) { QEGenericButton::userClicked( checked ); }

    // Note, keep in sync. The text below is repeated in QERadioButton.h, QERadioButton.h and QECheckBox.h
    /// Slot to receiver a 'process completed' signal from the application launcher
    //
    void programCompletedSlot();
    
    void useNewVariableNameProperty( QString variableNameIn, QString variableNameSubstitutionsIn, unsigned int variableIndex )
    {
        useGenericNewVariableName( variableNameIn, variableNameSubstitutionsIn, variableIndex );
    }

};

#endif // QE_RADIO_BUTTON_H
