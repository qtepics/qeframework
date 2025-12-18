/*  QESlider.h
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

#ifndef QE_SLIDER_H
#define QE_SLIDER_H

#include <QSlider>
#include <QEEnums.h>
#include <QEWidget.h>
#include <QEFloating.h>
#include <QEFloatingFormatting.h>
#include <QESingleVariableMethods.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QCaVariableNamePropertyManager.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QESlider :
      public QSlider,
      public QESingleVariableMethods,
      public QEWidget
{
   Q_OBJECT

public:
   explicit QESlider (QWidget * parent = 0);
   explicit QESlider (const QString & variableName, QWidget * parent = 0);

   // Property convenience functions

   // write on change
   void setWriteOnChange (bool writeOnChange);
   bool getWriteOnChange () const;

   // subscribe
   void setSubscribe (bool subscribe);
   bool getSubscribe () const;

   // Set scale and offset (used to scale data when inteter scale bar min and max are not suitable)
   void setScale (double scaleIn);
   double getScale () const;

   void setOffset (double offsetIn);
   double getOffset () const;

   // set/get allow focus update
   void setAllowFocusUpdate (bool allowFocusUpdate);
   bool getAllowFocusUpdate () const;

   // Auto scale control
   void setAutoScale (const bool autoScale);
   bool getAutoScale () const;

public slots:
   // write the value (of the underlying QSlider object) into the PV immediately
   void writeNow ();

   /// Update the widget and write to the main associated control process variable.
   /// Supplied values are converted to the approiate widget value type.
   /// If conversion not possible or resultant value out of range, no widget/PV update occurs.
   //
   void setPvValue (const QString& text);
   void setPvValue (const int value);
   void setPvValue (const double value);
   void setPvValue (const bool value);

   /// Update the default style applied to this widget.
   void setDefaultStyle (const QString & style);

   /// Slot to set the visibility of a QE widget, taking into account the user level.
   /// Widget will be hidden if hidden by a call this slot, by will only be made
   /// visible by a calll to this slot if the user level allows.
   ///
   void setManagedVisible (bool v);

signals:
   // Note, the following signals are common to many QE widgets,
   // if changing the doxygen comments, ensure relevent changes are migrated to all instances
   // These signals are emitted using the QEEmitter::emitDbValueChanged function.
   /// Sent when the widget is updated following a data change
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
   void dbValueChanged ();      // signal event
   void dbValueChanged (const QString & out);   // signal as formatted text
   void dbValueChanged (const int &out);        // signal as int if applicable
   void dbValueChanged (const long &out);       // signal as long if applicable
   void dbValueChanged (const qlonglong & out); // signal as qlonglong if applicable
   void dbValueChanged (const double &out);     // signal as floating if applicable
   void dbValueChanged (const bool &out);       // signal as bool: value != 0 if applicable

   // This signal is emitted using the QEEmitter::emitDbConnectionChanged function.
   /// Sent when the widget state updated following a channel connection change
   /// Applied to provary varible.
   void dbConnectionChanged (const bool &isConnected);

protected:
   // Drag and Drop
   void dragEnterEvent (QDragEnterEvent * event) { qcaDragEnterEvent (event);   }
   void dropEvent (QDropEvent * event)           { qcaDropEvent (event);   }
   // Don't drag from interactive widget
   // void mousePressEvent(QMouseEvent *event) { qcaMousePressEvent( event ); }
   void setDrop (QVariant drop);
   QVariant getDrop ();

   // Copy paste
   QString copyVariable ();
   QVariant copyData ();
   void paste (QVariant s);

   qcaobject::QCaObject * createQcaItem (unsigned int variableIndex);
   void establishConnection (unsigned int variableIndex);

private:
   enum Constatnts {
      pvVariableIndex = 0
   };

   void setup ();

   // Use scale and offset to transform between interal interger widget
   // slider value and external process variable double value.
   //
   double sliderToPv (const int) const;
   int pvToSlider (const double) const;

   bool updateInProgress;       // Ignore changes during updates, it isn't the user changing the slider.
   bool ignoreSingleShotRead;
   bool m_writeOnChange;        // Write changed value to database when ever the position changes.
   bool m_isAllowFocusUpdate;
   bool m_autoScale;

   double m_scale;
   double m_offset;
   double m_currentValue;       // Value or last update or write

   QEFloatingFormatting floatingFormatting;     // Floating formatting options.

private slots:
   void connectionChanged (QCaConnectionInfo & connectionInfo, const unsigned int &);
   void setValueIfNoFocus (const double &value, QCaAlarmInfo &, QCaDateTime &,
                           const unsigned int &);
   void userValueChanged (const int &newValue);
   void useNewVariableNameProperty (QString pvName, QString substitutions, unsigned int index);


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
   Q_PROPERTY (QString variableSubstitutions READ getVariableNameSubstitutionsProperty WRITE
               setVariableNameSubstitutionsProperty)

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
   Q_PROPERTY (bool subscribe READ getSubscribe WRITE setSubscribe)

   /// Sets if this widget writes any changes as the user moves the slider (the QSlider 'valueChanged' signal is emitted).
   /// Default is 'true' (writes any changes when the QSlider 'valueChanged' signal is emitted).
   Q_PROPERTY (bool writeOnChange READ getWriteOnChange WRITE setWriteOnChange)

   /// Allow updated while widget has focus - defaults to false
   ///
   Q_PROPERTY (bool allowFocusUpdate READ getAllowFocusUpdate WRITE setAllowFocusUpdate)

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
   Q_PROPERTY (bool variableAsToolTip READ getVariableAsToolTip WRITE setVariableAsToolTip)

   /// Allow drag/drops operations to this widget. Default is false.
   /// Any dropped text will be used as a new variable name.
   ///
   Q_PROPERTY (bool allowDrop READ getAllowDrop WRITE setAllowDrop)

   /// Display the widget. Default is true.
   /// Setting this property false is usefull if widget is only used to provide
   /// a signal - for example, when supplying data to a QELink widget.
   /// Note, when false the widget will still be visible in Qt Designer.
   ///
   Q_PROPERTY (bool visible READ getRunVisible WRITE setRunVisible)

   /// Set the ID used by the message filtering system. Default is zero.
   /// Widgets or applications that use messages from the framework have the option
   /// of filtering on this ID. For example, by using a unique message source ID
   /// a QELog widget may be set up to only log messages from a select set of widgets.
   ///
   Q_PROPERTY (unsigned int messageSourceId READ getMessageSourceId WRITE setMessageSourceId)

   /// Hide style sheet from designer as style calculation by the styleManager
   /// and not directly setable per se.
   /// This also stops transient styles being saved to the ui file.
   Q_PROPERTY (QString styleSheet READ styleSheet WRITE setStyleSheet DESIGNABLE false)

   /// Style Sheet string to be applied before, i.e. lower priority than, any other
   /// style, e.g. alarm style and/or user level style.
   /// Default is an empty string.
   ///
   Q_PROPERTY (QString defaultStyle READ getStyleDefault WRITE setStyleDefault)

   /// Style Sheet string to be applied when the widget is displayed in 'User' mode. Default is an empty string.
   /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
   /// This Style Sheet string will be applied by the styleManager class.
   /// Refer to the styleManager class for details about how this Style Sheet
   /// string will be merged with any pre-existing Style Sheet string
   /// and any Style Sheet strings generated during the display of data.
   ///
   Q_PROPERTY (QString userLevelUserStyle READ getStyleUser WRITE setStyleUser)

   /// Style Sheet string to be applied when the widget is displayed in 'Scientist' mode. Default is an empty string.
   /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
   /// This Style Sheet string will be applied by the styleManager class.
   /// Refer to the styleManager class for details about how this Style Sheet
   /// string will be merged with any pre-existing Style Sheet string
   /// and any Style Sheet strings generated during the display of data.
   ///
   Q_PROPERTY (QString userLevelScientistStyle READ getStyleScientist WRITE setStyleScientist)

   /// Style Sheet string to be applied when the widget is displayed in 'Engineer' mode. Default is an empty string.
   /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
   /// This Style Sheet string will be applied by the styleManager class.
   /// Refer to the styleManager class for details about how this Style Sheet string
   /// will be merged with any pre-existing Style Sheet string
   /// and any Style Sheet strings generated during the display of data.
   ///
   Q_PROPERTY (QString userLevelEngineerStyle READ getStyleEngineer WRITE setStyleEngineer)

   /// Lowest user level at which the widget is visible. Default is 'User'.
   /// Used when designing GUIs that display more and more detail according to the user mode.
   /// The user mode is set application wide through the QELogin widget, or programatically through setUserLevel()
   /// Widgets that are always visible should be visible at 'User'.
   /// Widgets that are only used by scientists managing the facility should be visible at 'Scientist'.
   /// Widgets that are only used by engineers maintaining the facility should be visible at 'Engineer'.
   ///
   Q_PROPERTY (QE::UserLevels userLevelVisibility READ getUserLevelVisibility
                                                  WRITE setUserLevelVisibility)

   /// Lowest user level at which the widget is enabled. Default is 'User'.
   /// Used when designing GUIs that allow access to more and more detail according to the user mode.
   /// The user mode is set application wide through the QELogin widget, or programatically through setUserLevel()
   /// Widgets that are always accessable should be visible at 'User'.
   /// Widgets that are only accessable to scientists managing the facility should be visible at 'Scientist'.
   /// Widgets that are only accessable to engineers maintaining the facility should be visible at 'Engineer'.
   ///
   Q_PROPERTY (QE::UserLevels userLevelEnabled READ getUserLevelEnabled WRITE setUserLevelEnabled)

   /// If 'Always' (default) widget will indicate the alarm state of any variable data it is displaying, including 'No Alarm'
   /// If 'WhenInAlarm' widget only indicate the alarm state of any variable data it is displaying if it is 'in alarm' or 'Out of Service'.
   /// If 'WhenInvalid' widget only indicate the alarm state of any variable data it is in the 'Invalid' alarm state or 'Out of Service'.
   /// If 'Never' widget will never indicate the alarm state of any variable data it is displaying.
   /// Typically the background colour is set to indicate the alarm state.
   /// Note, this property is included in the set of standard properties as it applies to most widgets.
   /// It will do nothing for widgets that don't display data.
   ///
   Q_PROPERTY (QE::DisplayAlarmStateOptions displayAlarmStateOption
               READ getDisplayAlarmStateOption WRITE setDisplayAlarmStateOption)

   /// Indicates whether the widget should repond to the underlying PV(s) being declared Out of Service.
   /// The default oosAware value is most often true, however is false for some widgets.
   ///
   Q_PROPERTY (bool oosAware READ getOosAware WRITE setOosAware)

public:
   // END-STANDARD-PROPERTIES ========================================================

   // Widget specific properties
   //
   Q_PROPERTY (double scale  READ getScale WRITE setScale)
   Q_PROPERTY (double offset READ getOffset WRITE setOffset)

   // If true, slider users the PV's control limits supplied with the data.
   // If false (default), the minimum and maximum property values are used.
   // Note: most widgets are autoScale true by default, however backward
   // compatibility rules.
   //
   Q_PROPERTY (bool autoScale  READ getAutoScale  WRITE setAutoScale)

   // Make the value/siider position properties non-designable. This both hides the
   // properties within designer and stops the values from being written to the .ui file.
   //
   Q_PROPERTY (double value READ value WRITE setValue DESIGNABLE false)
   Q_PROPERTY (int sliderPosition READ sliderPosition WRITE setSliderPosition DESIGNABLE false)
};

#endif   // QE_SLIDER_H
