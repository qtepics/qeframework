/*  QEGenericEdit.h
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
 *  Copyright (c) 2009,2010,2012,2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QE_GENERIC_EDIT_H
#define QE_GENERIC_EDIT_H

#include <QLineEdit>
#include <QVariant>
#include <QEWidget.h>
#include <QESingleVariableMethods.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEFrameworkLibraryGlobal.h>

// Provides the base class for QELineEdit
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEGenericEdit :
    public QLineEdit,
    public QESingleVariableMethods,
    public QEWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText DESIGNABLE false)

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


    //=================================================================================
    // Control widget properties
    // These properties should be similar for all widgets with control functionality (writing to variables, not just displaying).
    // WHEN MAKING CHANGES: search for CONTROLVARIABLEPROPERTIES and change all relevent occurances.
public:
    /// Sets if this widget subscribes for data updates and displays current data.
    /// Default is 'true' (subscribes for and displays data updates)
    Q_PROPERTY(bool subscribe READ getSubscribe WRITE setSubscribe)
    /// Sets if this widget automatically writes any changes when it loses focus.
    /// Default is 'false' (does not write any changes when it loses focus).
    Q_PROPERTY(bool writeOnLoseFocus READ getWriteOnLoseFocus WRITE setWriteOnLoseFocus)
    /// Sets if this widget writes any changes when the user presses 'enter'.
    /// Note, the current value will be written even if the user has not changed it.
    /// Default is 'true' (writes any changes when the user presses 'enter').
    Q_PROPERTY(bool writeOnEnter READ getWriteOnEnter WRITE setWriteOnEnter)
    /// Sets if this widget writes any changes when the user finished editing (the QLineEdit 'editingFinished' signal is emitted).
    /// No writing occurs if no changes were made.
    /// Default is 'true' (writes any changes when the QLineEdit 'editingFinished' signal is emitted).
    Q_PROPERTY(bool writeOnFinish READ getWriteOnFinish WRITE setWriteOnFinish)
    /// Sets if this widget will ask for confirmation (using a dialog box) prior to writing data.
    /// Default is 'false' (will not ask for confirmation (using a dialog box) prior to writing data).
    Q_PROPERTY(bool confirmWrite READ getConfirmWrite WRITE setConfirmWrite)
    /// Allow updated while widget has focus - defaults to false
    Q_PROPERTY(bool allowFocusUpdate READ getAllowFocusUpdate WRITE setAllowFocusUpdate)
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
    enum UserLevels { User      = userLevelTypes::USERLEVEL_USER,          ///< Refer to USERLEVEL_USER for details
                      Scientist = userLevelTypes::USERLEVEL_SCIENTIST,     ///< Refer to USERLEVEL_SCIENTIST for details
                      Engineer  = userLevelTypes::USERLEVEL_ENGINEER       ///< Refer to USERLEVEL_ENGINEER for details
                              };
    Q_ENUMS(UserLevels)

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

    /// DEPRECATED. USE displayAlarmStateOption INSTEAD.
    /// If set (default) widget will indicate the alarm state of any variable data it is displaying.
    /// If clear widget will never indicate the alarm state of any variable data it is displaying.
    /// Typically the background colour is set to indicate the alarm state.
    /// Note, this property is included in the set of standard properties as it applies to most widgets. It
    /// will do nothing for widgets that don't display data.
    Q_PROPERTY(bool displayAlarmState READ getDisplayAlarmState WRITE setDisplayAlarmState DESIGNABLE false)

    /// \enum DisplayAlarmStateOptions
    /// User friendly enumerations for #displayAlarmStateOption property - refer to #displayAlarmStateOption property and displayAlarmStateOptions enumeration for details.
    enum DisplayAlarmStateOptions { Never       = standardProperties::DISPLAY_ALARM_STATE_NEVER,          ///< Refer to DISPLAY_ALARM_STATE_NEVER for details
                                    Always      = standardProperties::DISPLAY_ALARM_STATE_ALWAYS,         ///< Refer to DISPLAY_ALARM_STATE_ALWAYS for details
                                    WhenInAlarm = standardProperties::DISPLAY_ALARM_STATE_WHEN_IN_ALARM   ///< Refer to DISPLAY_ALARM_STATE_WHEN_IN_ALARM for details
                              };
    Q_ENUMS(DisplayAlarmStateOptions)
    /// If 'Always' (default) widget will indicate the alarm state of any variable data it is displaying, including 'No Alarm'
    /// If 'Never' widget will never indicate the alarm state of any variable data it is displaying.
    /// If 'WhenInAlarm' widget only indicate the alarm state of any variable data it is displaying if it is 'in alarm'.
    /// Typically the background colour is set to indicate the alarm state.
    /// Note, this property is included in the set of standard properties as it applies to most widgets. It
    /// will do nothing for widgets that don't display data.
    Q_PROPERTY(DisplayAlarmStateOptions displayAlarmStateOption READ getDisplayAlarmStateOptionProperty WRITE setDisplayAlarmStateOptionProperty)

    DisplayAlarmStateOptions getDisplayAlarmStateOptionProperty() { return (DisplayAlarmStateOptions)getDisplayAlarmStateOption(); }            ///< Access function for #displayAlarmStateOption property - refer to #displayAlarmStateOption property for details
    void setDisplayAlarmStateOptionProperty( DisplayAlarmStateOptions option ) { setDisplayAlarmStateOption( (displayAlarmStateOptions)option ); }///< Access function for #displayAlarmStateOption property - refer to #displayAlarmStateOption property for details

public:
    // END-STANDARD-PROPERTIES ========================================================


public:
    /// Create without a variable.
    /// Use setVariableNameProperty() and setSubstitutionsProperty() to define a variable and, optionally, macro substitutions later.
    QEGenericEdit( QWidget *parent = 0 );

    /// Create with a variable.
    /// A connection is automatically established.
    /// If macro substitutions are required, create without a variable and set the variable and macro substitutions after creation.
    QEGenericEdit( const QString &variableName, QWidget *parent = 0 );

    // Property convenience functions

    // write on lose focus
    /// Sets if this widget automatically writes any changes when it loses focus.
    /// Default is 'false' (does not write any changes when it loses focus).
    void setWriteOnLoseFocus( bool writeOnLoseFocus );

    /// Returns 'true' if this widget automatically writes any changes when it loses focus.
    ///
    bool getWriteOnLoseFocus();

    // write on enter
    /// Sets if this widget writes any changes when the user presses 'enter'.
    /// Note, the current value will be written even if the user has not changed it.
    /// Default is 'true' (writes any changes when the user presses 'enter').
    void setWriteOnEnter( bool writeOnEnter );

    /// Returns 'true' if this widget writes any changes when the user presses 'enter'.
    ///
    bool getWriteOnEnter();

    // write on finish
    /// Sets if this widget writes any changes when the user finished editing (the QLineEdit 'editingFinished' signal is emitted).
    /// No writing occurs if no changes were made.
    /// Default is 'true' (writes any changes when the QLineEdit 'editingFinished' signal is emitted).
    void setWriteOnFinish( bool writeOnFinish );

    /// Returns 'true' if this widget writes any changes when the user finished editing (the QLineEdit 'editingFinished' signal is emitted).
    ///
    bool getWriteOnFinish();

    // confirm write
    /// Sets if this widget will ask for confirmation (using a dialog box) prior to writing data.
    /// Default is 'false' (will not ask for confirmation (using a dialog box) prior to writing data).
    void setConfirmWrite( bool confirmWrite );

    /// Returns 'true' if this widget will ask for confirmation (using a dialog box) prior to writing data.
    ///
    bool getConfirmWrite();

    // set/get allow focus update
    /// Sets if this widget configured to allow updates while it has focus.
    /// Default is 'false'.
    void setAllowFocusUpdate( bool allowFocusUpdate );

    /// Returns 'true' if this widget configured to allow updates while it has focus.
    bool getAllowFocusUpdate() const;

    // subscribe
    /// Sets if this widget subscribes for data updates and displays current data.
    /// Default is 'true' (subscribes for and displays data updates)
    void setSubscribe( bool subscribe );
    
    /// Returns 'true' if this widget subscribes for data updates and displays current data.
    ///
    bool getSubscribe();

    void writeValue (qcaobject::QCaObject *qca, QVariant newValue );

public slots:
    /// Update the default style applied to this widget.
    void setDefaultStyle( const QString& style ) { setStyleDefault( style ); }

private:
    bool writeOnLoseFocus;        // Write changed value to database when widget object loses focus (user moves from widget)
    bool writeOnEnter;            // Write changed value to database when enter is pressed with focus on the widget
    bool writeOnFinish;           // Write changed value to database when user finishes editing (leaves a widget)
    bool confirmWrite;            // Request confirmation before writing a value
    bool isAllowFocusUpdate;      // Controls whether updated while widger has focus allowed.
    bool allowDrop;
    bool isFirstUpdate;

    void setup();

    QMenu* getDefaultContextMenu();                 // Return the Qt default context menu to add to the QE context menu


private slots:
    void connectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex );
    void useNewVariableNameProperty( QString variableNameIn, QString variableNameSubstitutionsIn, unsigned int variableIndex );
    void userReturnPressed();                       // Act on the user pressing return in the widget
    void userEditingFinished();                     // Act on the user signaling text editing is complete (pressing return)

signals:
    // Note, the following signals are common to many QE widgets,
    // if changing the doxygen comments, ensure relevent changes are migrated to all instances
    //
    /// Internal use only. Used by QEConfiguredLayout to be notified when one of its widgets has written something
    void userChange( const QVariant& oldValue,
                     const QVariant& newValue,
                     const QVariant& lastValue );    // Signal a user attempt to change a value. Values are strings as the user sees them

    /// Internal use only. Used when changing a property value to force a re-display to reflect the new property value.
    void requestResend();

protected:
    QVariant lastValue;                    // Last updated value (may have arrived while user is editing field)
    QVariant lastUserValue;                // Last updated value seen by the user (same as lastValue unless the user is editing the text)

    bool messageDialogPresent;             // True if dialog box for the widget is present. When this dialog box is present the line edit no longer have focus, but the user is still effectivly interacting with it
    bool writeFailMessageDialogPresent;    // True if the write faile message dialog has been created. Cleared in spurious 'editing finished' callback caused by the 'write failed' dialog

    bool isConnected;

    void setDataIfNoFocus( const QVariant& value, QCaAlarmInfo& alarmInfo, QCaDateTime& dateTime );

    bool getIsConnected() const;
    bool getIsFirstUpdate() const;

    // These abstract functions must provided by derived sub-classes.
    //
    virtual void setValue( const QVariant & value ) = 0;
    virtual QVariant getValue() = 0;
    virtual bool writeData( const QVariant & value, QString& message ) = 0;

public:
    // write the text value (of the underlying QLineEdit object) into the PV immediately
    void writeNow ();

    // Drag and Drop
private:
    void dragEnterEvent(QDragEnterEvent *event) { qcaDragEnterEvent( event ); }
    void dropEvent(QDropEvent *event)           { qcaDropEvent( event ); }
    // Don't drag from interactive widget void mousePressEvent(QMouseEvent *event)    { qcaMousePressEvent( event ); }
    void setDrop( QVariant drop );
    QVariant getDrop();

    // Copy paste
    QString copyVariable();
    QVariant copyData();
    void paste( QVariant s );

};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEGenericEdit::UserLevels)
Q_DECLARE_METATYPE (QEGenericEdit::DisplayAlarmStateOptions)
#endif

#endif // QE_GENERIC_EDIT_H
