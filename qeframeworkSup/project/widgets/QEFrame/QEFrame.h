/*  QEFrame.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2020 Australian Synchrotron
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

#ifndef QE_FRAME_H
#define QE_FRAME_H

#include <QFrame>
#include <QPixmap>
#include <QEWidget.h>
#include <managePixmaps.h>
#include <QEFrameworkLibraryGlobal.h>


// The QEFrame class provides a minimalist extension to the QFrame class
// in that it provides user level enabled and user level visibility control to
// the frame but more significantly to all the widgets enclosed within the
// QEFrame container also.
// See QEGroupBox as well.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEFrame :
      public QFrame,
      public QEWidget,
      public managePixmaps
{
   Q_OBJECT

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

   // QEFrame specific properties.
   //
   /// Pixmap for frame background. Similar operation to pixmap property for a QLabel.
   /// Depricated, and is an  alias for pixmap0
   Q_PROPERTY (QPixmap pixmap READ getPixmap0Property WRITE setPixmap0Property DESIGNABLE false)

   /// Flag the pixmap for the background is to be scaled to fit the frame.
   /// Similar operation to scaledContents property for a QLabel.
   Q_PROPERTY (bool scaledContents READ getScaledContents WRITE setScaledContents)

   // Pixmaps
   /// Pixmap displayed when updateOption property is 'Picture' and data is interpreted as 0.
   ///
   Q_PROPERTY (QPixmap pixmap0 READ getPixmap0Property WRITE setPixmap0Property)

   /// Pixmap displayed when updateOption property is 'Picture' and data is interpreted as 1.
   ///
   Q_PROPERTY (QPixmap pixmap1 READ getPixmap1Property WRITE setPixmap1Property)

   /// Pixmap displayed when updateOption property is 'Picture' and data is interpreted as 2.
   ///
   Q_PROPERTY (QPixmap pixmap2 READ getPixmap2Property WRITE setPixmap2Property)

   /// Pixmap displayed when updateOption property is 'Picture' and data is interpreted as 3.
   ///
   Q_PROPERTY (QPixmap pixmap3 READ getPixmap3Property WRITE setPixmap3Property)

   /// Pixmap displayed when updateOption property is 'Picture' and data is interpreted as 4.
   ///
   Q_PROPERTY (QPixmap pixmap4 READ getPixmap4Property WRITE setPixmap4Property)

   /// Pixmap displayed when updateOption property is 'Picture' and data is interpreted as 5.
   ///
   Q_PROPERTY (QPixmap pixmap5 READ getPixmap5Property WRITE setPixmap5Property)

   /// Pixmap displayed when updateOption property is 'Picture' and data is interpreted as 6.
   ///
   Q_PROPERTY (QPixmap pixmap6 READ getPixmap6Property WRITE setPixmap6Property)

   /// Pixmap displayed when updateOption property is 'Picture' and data is interpreted as 7.
   ///
   Q_PROPERTY (QPixmap pixmap7 READ getPixmap7Property WRITE setPixmap7Property)
   //
   // End QEFrame specific properties.

public:
   explicit QEFrame (QWidget* parent = 0);
   virtual ~QEFrame ();
   QSize sizeHint () const;

   void setScaledContents (bool scaledContentsIn);  // Set the flag used to indicate the background is to be scaled to fit the frame. Similar operation to scaledContents property for a QLabel
   bool getScaledContents () const;                 // Get the flag used to indicate the background is to be scaled to fit the frame. Similar operation to scaledContents property for a QLabel

public slots:
   void setSelectPixmap (const int index);          // Set selected pix map index.
public:
   int getSelectedPixmap () const;                  // Get selected pix map index.

protected:
   void paintEvent (QPaintEvent * event);
   void pixmapUpdated (const int index);

private:
   int selectedPixmapIndex;

   // Flag the pixmap for the background is to be scaled to fit the frame.
   // Similar operation to scaledContents property for a QLabel
   bool scaledContents;
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEFrame::UserLevels)
Q_DECLARE_METATYPE (QEFrame::DisplayAlarmStateOptions)
#endif

#endif // QE_FRAME_H
