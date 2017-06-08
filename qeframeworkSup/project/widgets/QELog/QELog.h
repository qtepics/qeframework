/*  QELog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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
 *  Copyright (c) 2012,2016 Australian Synchrotron
 *
 *  Author:
 *    Ricardo Fernandes
 *  Contact details:
 *    ricardo.fernandes@synchrotron.org.au
 */

#ifndef QE_LOG_H
#define QE_LOG_H

#include <QWidget>
#include <QCheckBox>
#include <QTableWidget>
#include <QPushButton>
#include <QEWidget.h>


// ============================================================
//  _QTABLEWIDGETLOG CLASS
// ============================================================
class _QTableWidgetLog : public QTableWidget {
   Q_OBJECT
private:
   bool initialized;

public:
   explicit _QTableWidgetLog (QWidget * parent = 0);
   void refreshSize ();
   void resizeEvent (QResizeEvent *);
   void resize (int w, int h);
};


// ============================================================
//  _QELOG METHODS
// ============================================================
class QEPLUGINLIBRARYSHARED_EXPORT QELog : public QWidget, public QEWidget {
   Q_OBJECT
private:
   void newMessage (QString msg, message_types type);

protected:
   _QTableWidgetLog * qTableWidgetLog;
   QCheckBox *qCheckBoxInfoMessage;
   QCheckBox *qCheckBoxWarningMessage;
   QCheckBox *qCheckBoxErrorMessage;
   QPushButton *qPushButtonClear;
   QPushButton *qPushButtonSave;
   QColor qColorInfo;
   QColor qColorWarning;
   QColor qColorError;
   bool scrollToBottom;
   int optionsLayout;

public:
   explicit QELog (QWidget* parent = 0);
   virtual ~QELog ();

   void setShowColumnTime (bool pValue);
   bool getShowColumnTime ();

   void setShowColumnType (bool pValue);
   bool getShowColumnType ();

   void setShowColumnMessage (bool pValue);
   bool getShowColumnMessage ();

   void setShowMessageFilter (bool pValue);
   bool getShowMessageFilter ();

   void setShowClear (bool pValue);
   bool getShowClear ();

   void setShowSave (bool pValue);
   bool getShowSave ();

   void setOptionsLayout (int pValue);
   int getOptionsLayout ();

   void setScrollToBottom (bool pValue);
   bool getScrollToBottom ();

   void setInfoColor (QColor pValue);
   QColor getInfoColor ();

   void setWarningColor (QColor pValue);
   QColor getWarningColor ();

   void setErrorColor (QColor pValue);
   QColor getErrorColor ();

   void clearLog ();

   void addLog (int pType, QString pMessage);

   void refreshLog ();


   Q_PROPERTY (bool showColumnTime READ getShowColumnTime WRITE setShowColumnTime)

   Q_PROPERTY (bool showColumnType READ getShowColumnType WRITE setShowColumnType)

   Q_PROPERTY (bool showColumnMessage READ getShowColumnMessage WRITE setShowColumnMessage)

   Q_PROPERTY (bool showMessageFilter READ getShowMessageFilter WRITE setShowMessageFilter)

   Q_PROPERTY (bool showClear READ getShowClear WRITE setShowClear)

   Q_PROPERTY (bool showSave READ getShowSave WRITE setShowSave)

   Q_PROPERTY (bool scrollToBottom READ getScrollToBottom WRITE setScrollToBottom)


   enum optionsLayoutProperty {
      Top,
      Bottom,
      Left,
      Right
   };

   Q_ENUMS (optionsLayoutProperty)

   Q_PROPERTY (optionsLayoutProperty optionsLayout READ getOptionsLayoutProperty WRITE
               setOptionsLayoutProperty)

   void setOptionsLayoutProperty (optionsLayoutProperty pOptionsLayout);

   optionsLayoutProperty getOptionsLayoutProperty ();


   Q_PROPERTY (QColor infoColor READ getInfoColor WRITE setInfoColor)

   Q_PROPERTY (QColor warningColor READ getWarningColor WRITE setWarningColor)

   Q_PROPERTY (QColor errorColor READ getErrorColor WRITE setErrorColor)

   // Message properties !!!Include these by a common definition (QCA_MESSAGE_PROPERTIES for example)
   // Not all QE widgets need these properties (only those that do something with messages - like the logging widget)
   Q_ENUMS (MessageFilterOptions)
   Q_PROPERTY (MessageFilterOptions messageFormFilter READ getMessageFormFilter WRITE
               setMessageFormFilter)
   Q_PROPERTY (MessageFilterOptions messageSourceFilter READ getMessageSourceFilter WRITE
               setMessageSourceFilter)
   enum MessageFilterOptions { Any = UserMessage::MESSAGE_FILTER_ANY,
                               Match = UserMessage::MESSAGE_FILTER_MATCH,
                               None = UserMessage::MESSAGE_FILTER_NONE
                             };

   MessageFilterOptions getMessageFormFilter ();
   void setMessageFormFilter (MessageFilterOptions messageFormFilter);

   MessageFilterOptions getMessageSourceFilter ();
   void setMessageSourceFilter (MessageFilterOptions messageSourceFilter);

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

private slots:
   void checkBoxInfoToggled (bool);

   void checkBoxWarningToggled (bool);

   void checkBoxErrorToggled (bool);

   void buttonClearClicked ();

   void buttonSaveClicked ();
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QELog::optionsLayoutProperty)
Q_DECLARE_METATYPE (QELog::MessageFilterOptions)
Q_DECLARE_METATYPE (QELog::UserLevels)
Q_DECLARE_METATYPE (QELog::DisplayAlarmStateOptions)
#endif

#endif   // QE_LOG_H
