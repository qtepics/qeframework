/*  QESlider.h
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

#ifndef QE_SLIDER_H
#define QE_SLIDER_H

#include <QSlider>
#include <QEWidget.h>
#include <QEFloating.h>
#include <QEFloatingFormatting.h>
#include <QESingleVariableMethods.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QCaVariableNamePropertyManager.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QESlider :
      public QSlider,
      public QESingleVariableMethods,
      public QEWidget {
    Q_OBJECT

public:
    QESlider( QWidget *parent = 0 );
    QESlider( const QString& variableName, QWidget *parent = 0 );

    // Property convenience functions

    // write on change
    void setWriteOnChange( bool writeOnChange );
    bool getWriteOnChange() const;

    // subscribe
    void setSubscribe( bool subscribe );
    bool getSubscribe() const;

    // Set scale and offset (used to scale data when inteter scale bar min and max are not suitable)
    void setScale( double scaleIn );
    double getScale() const;
    void setOffset( double offsetIn );
    double getOffset() const;

    // set/get allow focus update
    void setAllowFocusUpdate( bool allowFocusUpdate );
    bool getAllowFocusUpdate() const;

public slots:
    // write the value (of the underlying QSlider object) into the PV immediately
    void writeNow();

    /// Update the default style applied to this widget.
    void setDefaultStyle( const QString& style ) { setStyleDefault( style ); }

protected:
    QEFloatingFormatting floatingFormatting; // Floating formatting options.
    bool writeOnChange;             // Write changed value to database when ever the position changes.

    void establishConnection( unsigned int variableIndex );

private slots:
    void connectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& );
    void setValueIfNoFocus( const double& value, QCaAlarmInfo&, QCaDateTime&, const unsigned int& );
    void userValueChanged( const int& newValue );
    void useNewVariableNameProperty( QString variableNameIn, QString variableNameSubstitutionsIn, unsigned int variableIndex )// !! move into Standard Properties section??
    {
        setVariableNameAndSubstitutions(variableNameIn, variableNameSubstitutionsIn, variableIndex);
    }

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
    /// Applied to provary varible.
    void dbConnectionChanged( const bool& isConnected );

private:
    void setup();
    qcaobject::QCaObject* createQcaItem( unsigned int variableIndex );

    bool updateInProgress;                  // Ignore changes during updates, it isn't the user changing the slider.

    bool isConnected;

    bool ignoreSingleShotRead;

    double scale;
    double offset;
    bool isAllowFocusUpdate;

    double currentValue;        // Value or last update or write

    // Drag and Drop
protected:
    void dragEnterEvent(QDragEnterEvent *event) { qcaDragEnterEvent( event ); }
    void dropEvent(QDropEvent *event)           { qcaDropEvent( event ); }
    // Don't drag from interactive widget void mousePressEvent(QMouseEvent *event)    { qcaMousePressEvent( event ); }
    void setDrop( QVariant drop );
    QVariant getDrop();

    // Copy paste
    QString copyVariable();
    QVariant copyData();
    void paste( QVariant s );

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

    //=================================================================================
    // Control widget properties
    // These properties should be similar for all widgets with control functionality (writing to variables, not just displaying).
    // WHEN MAKING CHANGES: search for CONTROLVARIABLEPROPERTIES and change all relevent occurances.
public:
    /// Sets if this widget subscribes for data updates and displays current data.
    /// Default is 'true' (subscribes for and displays data updates)
    Q_PROPERTY(bool subscribe READ getSubscribe WRITE setSubscribe)

    /// Sets if this widget writes any changes as the user moves the slider (the QSlider 'valueChanged' signal is emitted).
    /// Default is 'true' (writes any changes when the QSlider 'valueChanged' signal is emitted).
    Q_PROPERTY(bool writeOnChange READ getWriteOnChange WRITE setWriteOnChange)

    /// Allow updated while widget has focus - defaults to false
    ///
    Q_PROPERTY( bool allowFocusUpdate READ getAllowFocusUpdate WRITE setAllowFocusUpdate )
public:
    //=================================================================================

    // BEGIN-STANDARD-PROPERTIES ======================================================
    // Standard properties
    // These properties should be identical for every widget using them.
    // WHEN MAKING CHANGES: Use the update_widget_properties script in the
    // resources directory.
public slots:
    /// Slot to set the visibility of a QE widget, taking into account the user level.
    /// Widget will be hidden if hidden by a call this slot, by will only be made visible by a calll to this slot if the user level allows.
    void setManagedVisible( bool v ){ setRunVisible( v ); }
public:
    /// Use the variable as the tool tip. Default is true. Tool tip property will be overwritten by the variable name.
    ///
    Q_PROPERTY(bool variableAsToolTip READ getVariableAsToolTip WRITE setVariableAsToolTip)

    /// Allow drag/drops operations to this widget. Default is false. Any dropped text will be used as a new variable name.
    ///
    Q_PROPERTY(bool allowDrop READ getAllowDrop WRITE setAllowDrop)

    /// Display the widget. Default is true.
    /// Setting this property false is usefull if widget is only used to provide a signal - for example, when supplying data to a QELink widget.
    /// Note, when false the widget will still be visible in Qt Designer.
    Q_PROPERTY(bool visible READ getRunVisible WRITE setRunVisible)

    /// Set the ID used by the message filtering system. Default is zero.
    /// Widgets or applications that use messages from the framework have the option of filtering on this ID.
    /// For example, by using a unique message source ID a QELog widget may be set up to only log messages from a select set of widgets.
    Q_PROPERTY(unsigned int messageSourceId READ getMessageSourceId WRITE setMessageSourceId )

    /// Hide style sheet from designer as style calculation by the styleManager and not directly setable per se.
    /// This also stops transient styles being saved to the ui file.
    Q_PROPERTY(QString styleSheet   READ styleSheet       WRITE setStyleSheet  DESIGNABLE false)

    /// Style Sheet string to be applied before, i.e. lower priority than, any other style, e.g. alarm style and/or user level style.
    /// Default is an empty string.
    Q_PROPERTY(QString defaultStyle READ getStyleDefault  WRITE setStyleDefault)

    /// Style Sheet string to be applied when the widget is displayed in 'User' mode. Default is an empty string.
    /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
    /// This Style Sheet string will be applied by the styleManager class.
    /// Refer to the styleManager class for details about how this Style Sheet string will be merged with any pre-existing Style Sheet string
    /// and any Style Sheet strings generated during the display of data.
    Q_PROPERTY(QString userLevelUserStyle READ getStyleUser WRITE setStyleUser)

    /// Style Sheet string to be applied when the widget is displayed in 'Scientist' mode. Default is an empty string.
    /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
    /// This Style Sheet string will be applied by the styleManager class.
    /// Refer to the styleManager class for details about how this Style Sheet string will be merged with any pre-existing Style Sheet string
    /// and any Style Sheet strings generated during the display of data.
    Q_PROPERTY(QString userLevelScientistStyle READ getStyleScientist WRITE setStyleScientist)

    /// Style Sheet string to be applied when the widget is displayed in 'Engineer' mode. Default is an empty string.
    /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
    /// This Style Sheet string will be applied by the styleManager class.
    /// Refer to the styleManager class for details about how this Style Sheet string will be merged with any pre-existing Style Sheet string
    /// and any Style Sheet strings generated during the display of data.
    Q_PROPERTY(QString userLevelEngineerStyle READ getStyleEngineer WRITE setStyleEngineer)

    /// \enum UserLevels
    /// User friendly enumerations for #userLevelVisibility and #userLevelEnabled properties - refer to #userLevelVisibility and #userLevelEnabled properties and userLevel enumeration for details.
    enum UserLevels {
        User      = userLevelTypes::USERLEVEL_USER,          ///< Refer to USERLEVEL_USER for details
        Scientist = userLevelTypes::USERLEVEL_SCIENTIST,     ///< Refer to USERLEVEL_SCIENTIST for details
        Engineer  = userLevelTypes::USERLEVEL_ENGINEER       ///< Refer to USERLEVEL_ENGINEER for details
    };
    Q_ENUM (UserLevels)

    /// Lowest user level at which the widget is visible. Default is 'User'.
    /// Used when designing GUIs that display more and more detail according to the user mode.
    /// The user mode is set application wide through the QELogin widget, or programatically through setUserLevel()
    /// Widgets that are always visible should be visible at 'User'.
    /// Widgets that are only used by scientists managing the facility should be visible at 'Scientist'.
    /// Widgets that are only used by engineers maintaining the facility should be visible at 'Engineer'.
    Q_PROPERTY(UserLevels userLevelVisibility READ getUserLevelVisibilityProperty WRITE setUserLevelVisibilityProperty)

    /// Lowest user level at which the widget is enabled. Default is 'User'.
    /// Used when designing GUIs that allow access to more and more detail according to the user mode.
    /// The user mode is set application wide through the QELogin widget, or programatically through setUserLevel()
    /// Widgets that are always accessable should be visible at 'User'.
    /// Widgets that are only accessable to scientists managing the facility should be visible at 'Scientist'.
    /// Widgets that are only accessable to engineers maintaining the facility should be visible at 'Engineer'.
    Q_PROPERTY(UserLevels userLevelEnabled READ getUserLevelEnabledProperty WRITE setUserLevelEnabledProperty)

    UserLevels getUserLevelVisibilityProperty() { return (UserLevels)getUserLevelVisibility(); }            ///< Access function for #userLevelVisibility property - refer to #userLevelVisibility property for details
    void setUserLevelVisibilityProperty( UserLevels level ) { setUserLevelVisibility( (userLevelTypes::userLevels)level ); }///< Access function for #userLevelVisibility property - refer to #userLevelVisibility property for details
    UserLevels getUserLevelEnabledProperty() { return (UserLevels)getUserLevelEnabled(); }                  ///< Access function for #userLevelEnabled property - refer to #userLevelEnabled property for details
    void setUserLevelEnabledProperty( UserLevels level ) { setUserLevelEnabled( (userLevelTypes::userLevels)level ); }      ///< Access function for #userLevelEnabled property - refer to #userLevelEnabled property for details

    /// \enum DisplayAlarmStateOptions
    /// User friendly enumerations for #displayAlarmStateOption property - refer to #displayAlarmStateOption
    /// property and displayAlarmStateOptions enumeration for details.
    enum DisplayAlarmStateOptions {
       Always      = standardProperties::DISPLAY_ALARM_STATE_ALWAYS,              ///< Refer to DISPLAY_ALARM_STATE_ALWAYS for details
       WhenInAlarm = standardProperties::DISPLAY_ALARM_STATE_WHEN_IN_ALARM,       ///< Refer to DISPLAY_ALARM_STATE_WHEN_IN_ALARM for details
       WhenInvalid = standardProperties::DISPLAY_ALARM_STATE_WHEN_INVALID,        ///< Refer to DISPLAY_ALARM_STATE_WHEN_INVALID for details
       Never       = standardProperties::DISPLAY_ALARM_STATE_NEVER                ///< Refer to DISPLAY_ALARM_STATE_NEVER for details
    };
    Q_ENUM (DisplayAlarmStateOptions)
    /// If 'Always' (default) widget will indicate the alarm state of any variable data it is displaying, including 'No Alarm'
    /// If 'WhenInAlarm' widget only indicate the alarm state of any variable data it is displaying if it is 'in alarm' or 'Out of Service'.
    /// If 'WhenInvalid' widget only indicate the alarm state of any variable data it is in the 'Invalid' alarm state or 'Out of Service'.
    /// If 'Never' widget will never indicate the alarm state of any variable data it is displaying.
    /// Typically the background colour is set to indicate the alarm state.
    /// Note, this property is included in the set of standard properties as it applies to most widgets.
    /// It will do nothing for widgets that don't display data.
    Q_PROPERTY(DisplayAlarmStateOptions displayAlarmStateOption READ getDisplayAlarmStateOptionProperty WRITE setDisplayAlarmStateOptionProperty)

    /// Indicates whether the widget should repond to the underlying PV(s) being declared Out of Service.
    /// The default oosAware value is most often true, however is false for some widgets.
    Q_PROPERTY(bool oosAware READ getOosAware WRITE setOosAware)

    DisplayAlarmStateOptions getDisplayAlarmStateOptionProperty() { return (DisplayAlarmStateOptions)getDisplayAlarmStateOption(); }            ///< Access function for #displayAlarmStateOption property - refer to #displayAlarmStateOption property for details
    void setDisplayAlarmStateOptionProperty( DisplayAlarmStateOptions option ) { setDisplayAlarmStateOption( (displayAlarmStateOptions)option ); }///< Access function for #displayAlarmStateOption property - refer to #displayAlarmStateOption property for details

public:
    // END-STANDARD-PROPERTIES ========================================================

    // Widget specific properties


    Q_PROPERTY(double scale READ getScale WRITE setScale)
    Q_PROPERTY(double offset READ getOffset WRITE setOffset)

    // Make the value/siider position proerties non-designable. This both hides the
    // properties within designer and stops the values from being written to the .ui file.
    //
    Q_PROPERTY(double value           READ value           WRITE setValue           DESIGNABLE false)
    Q_PROPERTY(int    sliderPosition  READ sliderPosition  WRITE setSliderPosition  DESIGNABLE false)

};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QESlider::UserLevels)
Q_DECLARE_METATYPE (QESlider::DisplayAlarmStateOptions)
#endif

#endif // QE_SLIDER_H
