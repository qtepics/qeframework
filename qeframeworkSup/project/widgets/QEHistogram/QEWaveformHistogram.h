/*  QEWaveformHistogram.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2021  Australian Synchrotron.
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
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_WAVEFORM_HISTOGRAM_H
#define QE_WAVEFORM_HISTOGRAM_H

#include <QString>
#include <QVector>

#include <QECommon.h>
#include <QEHistogram.h>
#include <QEWidget.h>
#include <QEFloating.h>
#include <QEFloatingFormatting.h>
#include <QEStringFormatting.h>
#include <QEStringFormattingMethods.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEFrameworkLibraryGlobal.h>

class QEPVNameSelectDialog;  // differed

/// The QEWaveformHistogram class is a EPICS aware histogram widget.
/// The value of, i.e. the length of each bar of the histogram is controlled by
/// the corresponding element of an array, e.g. waveform, process variable.
/// When the variable is defined (connected), the bar lengths are updated, and
/// optionally the bar colours set to reflect the variable's severity status.
/// The bar is 'grayed-out' when the variable is disconnected (although the bars
/// retains their last known values/lengths).
/// The histogram nature of the this widget is provided by a QEHistogram widget.
/// The QEWaveformHistogram widget is tighly integrated with the base class QEWidget,
/// via QEFrame, which provides generic support such as macro substitutions,
/// drag/drop, and standard properties.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEWaveformHistogram :
      public QEHistogram,
      public QEWidget,
      public QEStringFormattingMethods
{
   Q_OBJECT

public:
   /// \enum ScaleModes
   enum ScaleModes {
      Manual,              ///< Use property minimum/maximum to scale histogram
      Auto,                ///< Dynamically scale based on minimum/maximum displayed value
      OperationalRange     ///< Use process variable operational range (LOPR/HOPR).
   };
   Q_ENUMS (ScaleModes)

   /// \enum
   /// User friendly enumerations for format property - refer to QEStringFormatting::formats for details.
   ///
   enum Formats {
      Default          = QEStringFormatting::FORMAT_DEFAULT,            ///< Format as best appropriate for the data type
      Floating         = QEStringFormatting::FORMAT_FLOATING,           ///< Format as a floating point number
      Integer          = QEStringFormatting::FORMAT_INTEGER,            ///< Format as an integer
      UnsignedInteger  = QEStringFormatting::FORMAT_UNSIGNEDINTEGER,    ///< Format as an unsigned integer
      Time             = QEStringFormatting::FORMAT_TIME,               ///< Format as a time
      LocalEnumeration = QEStringFormatting::FORMAT_LOCAL_ENUMERATE     ///< Format as a selection from the #localEnumeration property
   };
   Q_ENUMS (Formats)

   /// \enum
   /// User friendly enumerations for notation property - refer to QEStringFormatting::notations for details.
   ///
   enum Notations {
      Fixed       = QEStringFormatting::NOTATION_FIXED,              ///< Refer to QEStringFormatting::NOTATION_FIXED for details
      Scientific  = QEStringFormatting::NOTATION_SCIENTIFIC,         ///< Refer to QEStringFormatting::NOTATION_SCIENTIFIC for details
      Automatic   = QEStringFormatting::NOTATION_AUTOMATIC           ///< Refer to QEStringFormatting::NOTATION_AUTOMATIC for details
   };
   Q_ENUMS (Notations)


   // QEWaveformHistogram specific properties ========================================
   //
   Q_PROPERTY (QString variable READ getVariableNameProperty WRITE setVariableNameProperty)
   Q_PROPERTY (QString variableSubstitutions READ getPvNameSubstitutions WRITE
               setPvNameSubstitutions)

   Q_PROPERTY (ScaleModes scaleMode    READ getScaleMode        WRITE setScaleMode)

   // Readout formatting
   Q_PROPERTY (int       readoutPrecision  READ getReadoutPrecision WRITE setReadoutPrecision)
   Q_PROPERTY (Formats   readoutFormat     READ getReadoutFormat    WRITE setReadoutFormat)
   Q_PROPERTY (Notations readoutNotation   READ getReadoutNotation  WRITE setReadoutNotation)
   //
   // End QEWaveformHistogram specific properties ===================================


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
   /// User friendly enumerations for #displayAlarmStateOption property - refer to #displayAlarmStateOption
   /// property and displayAlarmStateOptions enumeration for details.
   enum DisplayAlarmStateOptions {
      Always      = standardProperties::DISPLAY_ALARM_STATE_ALWAYS,              ///< Refer to DISPLAY_ALARM_STATE_ALWAYS for details
      WhenInAlarm = standardProperties::DISPLAY_ALARM_STATE_WHEN_IN_ALARM,       ///< Refer to DISPLAY_ALARM_STATE_WHEN_IN_ALARM for details
      WhenInvalid = standardProperties::DISPLAY_ALARM_STATE_WHEN_INVALID,        ///< Refer to DISPLAY_ALARM_STATE_WHEN_INVALID for details
      Never       = standardProperties::DISPLAY_ALARM_STATE_NEVER                ///< Refer to DISPLAY_ALARM_STATE_NEVER for details
   };
   Q_ENUMS(DisplayAlarmStateOptions)
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


   // QEWaveformHistogram context menu values
   //
   enum OwnContextMenuOptions {
      QEWH_NONE = CM_SPECIFIC_WIDGETS_START_HERE,
      QEWH_PV_NAME_SELECT_DIALOG,
      QEWH_SUB_CLASS_WIDGETS_START_HERE
   };

public:
   explicit QEWaveformHistogram (QWidget* parent = 0);
   ~QEWaveformHistogram () { }

   void setScaleMode (const ScaleModes scaleMode);
   ScaleModes getScaleMode () const;

   void setReadoutPrecision (const int readoutPrecision);
   int getReadoutPrecision () const;

   void setReadoutFormat (const Formats format);
   Formats getReadoutFormat() const;

   void setReadoutNotation (const Notations notation);
   Notations getReadoutNotation () const;

signals:
   // This signal is emitted using the QEEmitter::emitDbConnectionChanged function.
   /// Sent when the widget state updated following a channel connection change
   void dbConnectionChanged (const bool& isConnected);

   // This signal is emitted using the QEEmitter::emitDbValueChanged function.
   /// Sent when the widget is updated following a data change
   void dbValueChanged ();             // signal event

protected:
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);
   void establishConnection (unsigned int variableIndex);

   // Drag and Drop
   void dragEnterEvent (QDragEnterEvent* event) { this->qcaDragEnterEvent (event);  }
   void dropEvent (QDropEvent* event)           { this->qcaDropEvent (event);       }
   void mousePressEvent (QMouseEvent* event)    { this->qcaMousePressEvent (event); }
   // This widget uses the setDrop/getDrop defined in QEWidget which is copy/paste.

   void stringFormattingChange() { }

   // Copy paste
   QString copyVariable ();
   QVariant copyData ();
   void paste (QVariant v);

   QMenu* buildContextMenu ();                        // Build the specific context menu
   void contextMenuTriggered (int selectedItemNum);   // An action was selected from the context menu

   void onMouseIndexChanged (const int index);
   void onMouseIndexPressed (const int index, const Qt::MouseButton button);

   // Sets (or replaces) the pvName and establish the connection.
   //
   void setPvName (const QString& pvName);

private:
   void setVariableNameProperty (const QString& variableName);
   QString getVariableNameProperty ();
   void setPvNameSubstitutions (const QString& subs);
   QString getPvNameSubstitutions () const;
   void updateHistogramScale ();
   void setReadOut (const QString& text);
   void genReadOut (const int index);

   QEPVNameSelectDialog* pvNameSelectDialog;
   QEHistogram* histogram;   // sef ref alias
   QCaVariableNamePropertyManager vnpm;
   QEFloatingFormatting floatingFormatting;
   int selectedChannel;         //
   ScaleModes mScaleMode;
   bool isFirstUpdate;

private slots:
   void newVariableNameProperty (QString pvName, QString subs, unsigned int variableIndex);

   void connectionChanged (QCaConnectionInfo& connectionInfo, const unsigned int&variableIndex);

   void setChannelArrayValue (const QVector <double>& value, QCaAlarmInfo&,
                              QCaDateTime&, const unsigned int&);
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEWaveformHistogram::ScaleModes)
Q_DECLARE_METATYPE (QEWaveformHistogram::Formats)
Q_DECLARE_METATYPE (QEWaveformHistogram::Notations)
#endif

#endif // QE_WAVEFORM_HISTOGRAM_H
