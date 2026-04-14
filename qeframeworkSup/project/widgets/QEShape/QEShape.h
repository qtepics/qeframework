/*  QEShape.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2009-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_SHAPE_H
#define QE_SHAPE_H

#include <QEChannel.h>
#include <QEInteger.h>
#include <QEEnums.h>
#include <QEWidget.h>
#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QWidget>
#include <QEFrameworkLibraryGlobal.h>
#include <QCaVariableNamePropertyManager.h>

// Maximum number of variables.
#define QESHAPE_NUM_VARIABLES 6

/*!
  This class is a EPICS aware shape widget based on the Qt widget.
  One of several shapes can be drawn within the widget, and up to 6 variables can be used to animate various attributes of the shape.
  For example to represent beam positino and size, an elipse can be drawn with four variables animating its vertcal and horizontal size and position.
  It is tighly integrated with the base class QEWidget which provides generic support such as macro substitutions, drag/drop, and standard properties.
 */
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEShape :
      public QWidget, public QEWidget
{
   Q_OBJECT

public:
   /// Create without a variable.
   /// Use setVariableNameProperty() and setSubstitutionsProperty() to define a variable and, optionally, macro substitutions later.
   explicit QEShape (QWidget *parent = 0);

   /// Create with a single variable. (Note, the QEShape widget can use up to 6 variables)
   /// A connection is automatically established.
   /// If macro substitutions are required, create without a variable and set the variable and macro substitutions after creation.
   explicit QEShape (const QString& variableName, QWidget *parent = 0);

   QSize sizeHint () const;

   /// Options for the type of shape.
   ///
   enum shapeOptions { Line, Points, Polyline, Polygon, Rect, RoundedRect, Ellipse, Arc, Chord, Pie, Path };
   Q_ENUM (shapeOptions)

   /// Options for how a variable will animate the shape.
   ///
   enum animationOptions {Width, Height, X, Y, Transperency, Rotation, ColourHue, ColourSaturation, ColourValue, ColourIndex, Penwidth};
   Q_ENUM (animationOptions)

   /// Scale the widgets my m/d.
   void scaleBy (const int m, const int d);

   // Property convenience functions

   // variable animations
   /// Access function for #animation' properties - refer to animation' properties for details
   void setAnimation (animationOptions animation, const int index);

   /// Access function for #animation' properties - refer to animation' properties for details
   animationOptions getAnimation (const int index) const;

   // scales
   /// Access function for #scale' properties - refer to scale' properties for details
   void setScale (const double scale, const int index);

   /// Access function for #scale' properties - refer to scale' properties for details
   double getScale (const int index) const;

   // offsets
   /// Access function for #offset' properties - refer to offset' properties for details
   void setOffset (const double offset, const int index);

   /// Access function for #offset' properties - refer to offset' properties for details
   double getOffset (const int index) const;

   // border
   /// Access function for #border' properties - refer to border' properties for details
   void setBorder (const bool border);

   /// Access function for #border' properties - refer to border' properties for details
   bool getBorder() const;

   // fill
   /// Access function for #fill' properties - refer to fill' properties for details
   void setFill (const bool fill);

   /// Access function for #fill' properties - refer to fill' properties for details
   bool getFill() const;

   // shape
   /// Access function for #shape' properties - refer to shape' properties for details
   void setShape (shapeOptions shape);

   /// Access function for #shape' properties - refer to shape' properties for details
   shapeOptions getShape() const;

   // number of points
   /// Access function for #number of points' properties - refer to number of points' properties for details
   void setNumPoints (const unsigned int numPoints);

   /// Access function for #number of points' properties - refer to number of points' properties for details
   unsigned int getNumPoints() const;

   // Origin translation
   /// Access function for #origin translation' properties - refer to origin translation' properties for details
   void setOriginTranslation (const QPoint& originTranslation);

   /// Access function for #origin translation' properties - refer to origin translation' properties for details
   QPoint getOriginTranslation() const;

   // points
   /// Access function for #point' properties - refer to point' properties for details
   void setPoint (const QPoint& point, const int index);

   /// Access function for #point' properties - refer to point' properties for details
   QPoint getPoint(const int index) const;

   // colors
   /// Access function for #colour' properties - refer to colour' properties for details
   void setColor (const QColor& color, const int index);

   /// Access function for #colour' properties - refer to colour' properties for details
   QColor getColor (const int index) const;

   // draw border
   /// Access function for #draw border' properties - refer to draw border' properties for details
   void setDrawBorder (const bool drawBorder);

   /// Access function for #draw border' properties - refer to draw border' properties for details
   bool getDrawBorder() const;

   // line width
   /// Access function for #line width' properties - refer to line width' properties for details
   void setLineWidth (const unsigned int lineWidth);

   /// Access function for #line width' properties - refer to line width' properties for details
   unsigned int getLineWidth() const;

   // start angle
   /// Access function for #start angle' properties - refer to start angle' properties for details
   void setStartAngle (const double startAngle);

   /// Access function for #start angle' properties - refer to start angle' properties for details
   double getStartAngle() const;

   // rotation
   /// Access function for #rotation' properties - refer to rotation' properties for details
   void setRotation (const double rotation);

   /// Access function for #rotation' properties - refer to rotation' properties for details
   double getRotation() const;

   // arc length
   /// Access function for #arc length' properties - refer to arc length' properties for details
   void setArcLength (const double arcLength);

   /// Access function for #arc length' properties - refer to arc length' properties for details
   double getArcLength() const;

private:

   double lastValue[QESHAPE_NUM_VARIABLES];

   QEIntegerFormatting integerFormatting;                     // Integer formatting options
#define OFFSETS_SIZE QESHAPE_NUM_VARIABLES
   double offsets[OFFSETS_SIZE];

#define SCALES_SIZE QESHAPE_NUM_VARIABLES
   double scales[SCALES_SIZE];

   shapeOptions shape;
   QPoint originTranslation;

#define POINTS_SIZE 10
   QPoint points[POINTS_SIZE];
   unsigned int numPoints;

#define COLORS_SIZE 10
   QColor colors[COLORS_SIZE];

   animationOptions animations[6];

   double startAngle;
   double arcLength;
   double rotation;
   unsigned int lineWidth;
   bool fill;
   bool drawBorder;

   void colorChange (unsigned int index);                     // Act on a color property change. (will update shape if using the color)
   unsigned int currentColor;                                 // Index into colorsProperty last used when setting brush color

   void establishConnection (unsigned int variableIndex);     // Create a CA connection and initiates updates if required

private slots:
   void connectionUpdated (const QEConnectionUpdate& update);
   void setValue (const QEIntegerValueUpdate& update);
   void usePvNameProperties (const QEPvNameProperties& pvNameProperties);

signals:
   // Note, the following signals are common to many QE widgets,
   // if changing the doxygen comments, ensure relevent changes are migrated to all instances
   /// Sent when the widget is updated following a data change for the first variable
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
   void dbValueChanged1 (const qlonglong& out);

   /// Sent when the widget is updated following a data change for the second variable
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
   void dbValueChanged2 (const qlonglong& out);

   /// Sent when the widget is updated following a data change for the third variable
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
   void dbValueChanged3 (const qlonglong& out);

   /// Sent when the widget is updated following a data change for the fourth variable
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
   void dbValueChanged4 (const qlonglong& out);

   /// Sent when the widget is updated following a data change for the fifth variable
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
   void dbValueChanged5 (const qlonglong& out);

   /// Sent when the widget is updated following a data change for the sixth variable
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
   void dbValueChanged6 (const qlonglong& out);

public slots:
   /// Slot to set the visibility of a QE widget, taking into account the user level.
   /// Widget will be hidden if hidden by a call this slot, by will only be made
   /// visible by a calll to this slot if the user level allows.
   ///
   void setManagedVisible (bool v) { setRunVisible (v); }

private:
   void setup();                                               // Initialisation common to all constructors
   QEChannel* createQcaItem (unsigned int variableIndex);
   void refreshData (const int index);

   bool isConnected;

   void paintEvent(QPaintEvent *event);
   QPen pen;
   QBrush brush;
   bool antialiased;

   double painterCurrentScaleX;
   double painterCurrentScaleY;
   double painterCurrentTranslateX;
   double painterCurrentTranslateY;

   QPoint scaledOriginTranslation;

   // Drag and Drop
private:
   void dragEnterEvent(QDragEnterEvent *event) { qcaDragEnterEvent (event); }
   void dropEvent(QDropEvent *event)           { qcaDropEvent (event); }
   void mousePressEvent(QMouseEvent *event)    { qcaMousePressEvent (event); }
   void setDrop (QVariant drop);
   QVariant getDrop();

   // Copy paste
   QString copyVariable();
   QVariant copyData();
   void paste (QVariant s);

public:
   //=================================================================================
   // Multiple Variable properties
   // These properties should be similar for every widget using multiple variables (The number of variables may vary).
   // WHEN MAKING CHANGES: search for MULTIPLEVARIABLEPROPERTIESBASE and change all occurances.
private:
   QCaVariableNamePropertyManager variableNamePropertyManagers[QESHAPE_NUM_VARIABLES];
public:

   // Define a variable
   // Note, the QPROPERTY declaration itself can't be in this macro
#define VARIABLE_PROPERTY_ACCESS(VAR_INDEX) \
   void    setVariableName##VAR_INDEX##Property (const QString& variableName) { variableNamePropertyManagers[VAR_INDEX].setVariableNameProperty (variableName); } \
   QString getVariableName##VAR_INDEX##Property() const { return variableNamePropertyManagers[VAR_INDEX].getVariableNameProperty(); }

   VARIABLE_PROPERTY_ACCESS(0)
   /// EPICS variable name (CA PV).
   /// This variable is read and used to animate an attribute of the shape.
   /// The value read is first scaled and offset by properties scale1 and offset1 then the attribute selected for animation is selected by the property animation1.
   Q_PROPERTY(QString variable1 READ getVariableName0Property WRITE setVariableName0Property)

   VARIABLE_PROPERTY_ACCESS(1)
   /// EPICS variable name (CA PV).
   /// This variable is read and used to animate an attribute of the shape.
   /// The value read is first scaled and offset by properties scale2 and offset2 then the attribute selected for animation is selected by the property animation2.
   Q_PROPERTY(QString variable2 READ getVariableName1Property WRITE setVariableName1Property)

   VARIABLE_PROPERTY_ACCESS(2)
   /// EPICS variable name (CA PV).
   /// This variable is read and used to animate an attribute of the shape.
   /// The value read is first scaled and offset by properties scale3 and offset3 then the attribute selected for animation is selected by the property animation3.
   Q_PROPERTY(QString variable3 READ getVariableName2Property WRITE setVariableName2Property)

   VARIABLE_PROPERTY_ACCESS(3)
   /// EPICS variable name (CA PV).
   /// This variable is read and used to animate an attribute of the shape.
   /// The value read is first scaled and offset by properties scale4 and offset4 then the attribute selected for animation is selected by the property animation4.
   Q_PROPERTY(QString variable4 READ getVariableName3Property WRITE setVariableName3Property)

   VARIABLE_PROPERTY_ACCESS(4)
   /// EPICS variable name (CA PV).
   /// This variable is read and used to animate an attribute of the shape.
   /// The value read is first scaled and offset by properties scale5 and offset5 then the attribute selected for animation is selected by the property animation5.
   Q_PROPERTY(QString variable5 READ getVariableName4Property WRITE setVariableName4Property)

   VARIABLE_PROPERTY_ACCESS(5)
   /// EPICS variable name (CA PV).
   /// This variable is read and used to animate an attribute of the shape.
   /// The value read is first scaled and offset by properties scale6 and offset6 then the attribute selected for animation is selected by the property animation6.
   Q_PROPERTY(QString variable6 READ getVariableName5Property WRITE setVariableName5Property)

#undef VARIABLE_PROPERTY_ACCESS

   /// Macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2... Values may be quoted strings. For example, 'SAMPLE=SAM1, NAME = "Ref foil"'
   /// These substitutions are applied to all the variable names.
   Q_PROPERTY(QString variableSubstitutions READ getVariableNameSubstitutionsProperty WRITE setVariableNameSubstitutionsProperty)

   /// Property access function for #variableSubstitutions property. This has special behaviour to work well within designer.
   void setVariableNameSubstitutionsProperty (QString variableNameSubstitutions)
   {
      for (int i = 0; i < QESHAPE_NUM_VARIABLES; i++)
      {
         variableNamePropertyManagers[i].setSubstitutionsProperty (variableNameSubstitutions);
      }
   }
   /// Property access function for #variableSubstitutions property. This has special behaviour to work well within designer.
   QString getVariableNameSubstitutionsProperty()
   {
      return variableNamePropertyManagers[0].getSubstitutionsProperty();
   }
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
   Q_PROPERTY(unsigned int messageSourceId READ getMessageSourceId WRITE setMessageSourceId)

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

   // Widget specific properties

   /// Animation to be effected by the 1st variable.
   /// This is used to select what the effect changing data for the 1st variable will have on the shape.
   Q_PROPERTY(animationOptions animation1 READ getAnimation1Property WRITE setAnimation1Property)
   /// Animation to be effected by the 2nd variable.
   /// This is used to select what the effect changing data for the 2nd variable will have on the shape.
   Q_PROPERTY(animationOptions animation2 READ getAnimation2Property WRITE setAnimation2Property)
   /// Animation to be effected by the 3rd variable.
   /// This is used to select what the effect changing data for the 3rd variable will have on the shape.
   Q_PROPERTY(animationOptions animation3 READ getAnimation3Property WRITE setAnimation3Property)
   /// Animation to be effected by the 4th variable.
   /// This is used to select what the effect changing data for the 4th variable will have on the shape.
   Q_PROPERTY(animationOptions animation4 READ getAnimation4Property WRITE setAnimation4Property)
   /// Animation to be effected by the 5th variable.
   /// This is used to select what the effect changing data for the 5th variable will have on the shape.
   Q_PROPERTY(animationOptions animation5 READ getAnimation5Property WRITE setAnimation5Property)
   /// Animation to be effected by the 6th variable.
   /// This is used to select what the effect changing data for the 6th variable will have on the shape.
   Q_PROPERTY(animationOptions animation6 READ getAnimation6Property WRITE setAnimation6Property)

   /// Scale factor applied to data from the 1st variable before it is used to animate the shape
   Q_PROPERTY(double scale1 READ getScale1Property WRITE setScale1Property)
   /// Scale factor applied to data from the 2nd variable before it is used to animate the shape
   ///
   Q_PROPERTY(double scale2 READ getScale2Property WRITE setScale2Property)
   /// Scale factor applied to data from the 3rd variable before it is used to animate the shape
   ///
   Q_PROPERTY(double scale3 READ getScale3Property WRITE setScale3Property)
   /// Scale factor applied to data from the 4th variable before it is used to animate the shape
   ///
   Q_PROPERTY(double scale4 READ getScale4Property WRITE setScale4Property)
   /// Scale factor applied to data from the 5th variable before it is used to animate the shape
   ///
   Q_PROPERTY(double scale5 READ getScale5Property WRITE setScale5Property)
   /// Scale factor applied to data from the 6th variable before it is used to animate the shape
   ///
   Q_PROPERTY(double scale6 READ getScale6Property WRITE setScale6Property)

   /// Offset applied to data from the 1st variable before it is used to animate the shape
   ///
   Q_PROPERTY(double offset1 READ getOffset1Property WRITE setOffset1Property)
   /// Offset applied to data from the 2nd variable before it is used to animate the shape
   ///
   Q_PROPERTY(double offset2 READ getOffset2Property WRITE setOffset2Property)
   /// Offset applied to data from the 3rd variable before it is used to animate the shape
   ///
   Q_PROPERTY(double offset3 READ getOffset3Property WRITE setOffset3Property)
   /// Offset applied to data from the 4th variable before it is used to animate the shape
   ///
   Q_PROPERTY(double offset4 READ getOffset4Property WRITE setOffset4Property)
   /// Offset applied to data from the 5th variable before it is used to animate the shape
   ///
   Q_PROPERTY(double offset5 READ getOffset5Property WRITE setOffset5Property)
   /// Offset applied to data from the 6th variable before it is used to animate the shape
   ///
   Q_PROPERTY(double offset6 READ getOffset6Property WRITE setOffset6Property)

   /// Particular shape implemented by this widget.
   /// For example: line, rectangle, elipse. The coordinates used to draw the shape are taken from properties 'point1' to 'point10' as appropriate.
   Q_PROPERTY(shapeOptions shape READ getShapeProperty WRITE setShapeProperty)

   /// The number of points to use when drawing shapes that are defined by a variable number of points, such as polyline, polygon, path, and series of points.
   ///
   Q_PROPERTY(unsigned int numPoints READ getNumPoints WRITE setNumPoints)

   /// Moves the origin (where 0,0 is) when drawing the shape.
   /// This is usefull when the top left of the widget is not the natural origin for the shape.
   /// For example, if a rectangular shape was used to represent the opening of a set of slits,
   /// the origin is in the center of the widget. While the correct effect may be achieved with
   /// appropriate initial coordinates and data offsets, it can be easier to set up correct operation
   /// if this property is set to the natural origin of the object being represented.
   Q_PROPERTY(QPoint originTranslation READ getOriginTranslation WRITE setOriginTranslation)

   /// 1st coordinate used when drawing the shape. Used for the following shapes: Line, Points, Polyline, Polygon, Rect, RoundedRect, Ellipse, Arc, Chord, Pie, Path, Text, Pixmap
   ///
   Q_PROPERTY(QPoint point1 READ getPoint1Property WRITE setPoint1Property)
   /// 2nd coordinate used when drawing the shape. Used for the following shapes: Line, Points, Polyline, Polygon, Rect, RoundedRect, Ellipse, Arc, Chord, Pie, Path, Pixmap
   ///
   Q_PROPERTY(QPoint point2 READ getPoint2Property WRITE setPoint2Property)
   /// 3rd coordinate used when drawing the shape. Used for the following shapes: Points, Polyline, Polygon, Path
   ///
   Q_PROPERTY(QPoint point3 READ getPoint3Property WRITE setPoint3Property)
   /// 4th coordinate used when drawing the shape. Used for the following shapes: Points, Polyline, Polygon, Path
   ///
   Q_PROPERTY(QPoint point4 READ getPoint4Property WRITE setPoint4Property)
   /// 5th coordinate used when drawing the shape. Used for the following shapes: Points, Polyline, Polygon, Path
   ///
   Q_PROPERTY(QPoint point5 READ getPoint5Property WRITE setPoint5Property)
   /// 6th coordinate used when drawing the shape. Used for the following shapes: Points, Polyline, Polygon, Path
   ///
   Q_PROPERTY(QPoint point6 READ getPoint6Property WRITE setPoint6Property)
   /// 7th coordinate used when drawing the shape. Used for the following shapes: Points, Polyline, Polygon, Path
   ///
   Q_PROPERTY(QPoint point7 READ getPoint7Property WRITE setPoint7Property)
   /// 8th coordinate used when drawing the shape. Used for the following shapes: Points, Polyline, Polygon, Path
   ///
   Q_PROPERTY(QPoint point8 READ getPoint8Property WRITE setPoint8Property)
   /// 9th coordinate used when drawing the shape. Used for the following shapes: Points, Polyline, Polygon, Path
   ///
   Q_PROPERTY(QPoint point9 READ getPoint9Property WRITE setPoint9Property)
   /// 10th coordinate used when drawing the shape. Used for the following shapes: Points, Polyline, Polygon, Path
   ///
   Q_PROPERTY(QPoint point10 READ getPoint10Property WRITE setPoint10Property)

   /// Used by the color animation to determine the color based on a data value. The scaled and offset data is used as an index to select color properties 'color1' to 'color10'.
   ///
   Q_PROPERTY(QColor color1 READ getColor1Property WRITE setColor1Property)
   /// Used by the color animation to determine the color based on a data value. The scaled and offset data is used as an index to select color properties 'color1' to 'color10'.
   ///
   Q_PROPERTY(QColor color2 READ getColor2Property WRITE setColor2Property)
   /// Used by the color animation to determine the color based on a data value. The scaled and offset data is used as an index to select color properties 'color1' to 'color10'.
   ///
   Q_PROPERTY(QColor color3 READ getColor3Property WRITE setColor3Property)
   /// Used by the color animation to determine the color based on a data value. The scaled and offset data is used as an index to select color properties 'color1' to 'color10'.
   ///
   Q_PROPERTY(QColor color4 READ getColor4Property WRITE setColor4Property)
   /// Used by the color animation to determine the color based on a data value. The scaled and offset data is used as an index to select color properties 'color1' to 'color10'.
   ///
   Q_PROPERTY(QColor color5 READ getColor5Property WRITE setColor5Property)
   /// Used by the color animation to determine the color based on a data value. The scaled and offset data is used as an index to select color properties 'color1' to 'color10'.
   ///
   Q_PROPERTY(QColor color6 READ getColor6Property WRITE setColor6Property)
   /// Used by the color animation to determine the color based on a data value. The scaled and offset data is used as an index to select color properties 'color1' to 'color10'.
   ///
   Q_PROPERTY(QColor color7 READ getColor7Property WRITE setColor7Property)
   /// Used by the color animation to determine the color based on a data value. The scaled and offset data is used as an index to select color properties 'color1' to 'color10'.
   ///
   Q_PROPERTY(QColor color8 READ getColor8Property WRITE setColor8Property)
   /// Used by the color animation to determine the color based on a data value. The scaled and offset data is used as an index to select color properties 'color1' to 'color10'.
   ///
   Q_PROPERTY(QColor color9 READ getColor9Property WRITE setColor9Property)
   /// Used by the color animation to determine the color based on a data value. The scaled and offset data is used as an index to select color properties 'color1' to 'color10'.
   ///
   Q_PROPERTY(QColor color10 READ getColor10Property WRITE setColor10Property)


   /// Draw a border on the shape. Used for the following shapes: Polygon, Rect, RoundedRect, Ellipse, Pie
   /// Note, if both 'border' and 'fill' properties are false, then nothing will be drawn
   Q_PROPERTY(bool drawBorder READ getDrawBorder WRITE setDrawBorder)

   /// Fill the shape. Used for the following shapes: Polygon, Rect, RoundedRect, Ellipse, Pie
   /// Note, if both 'border' and 'fill' properties are false, then nothing will be drawn
   Q_PROPERTY(bool fill READ getFill WRITE setFill)

   /// Sets the width of the pen. Used for the following shapes: Line, Points, Polyline, Polygon, Rect, RoundedRect, Ellipse, Arc, Chord, Pie, Path
   ///
   Q_PROPERTY(unsigned int lineWidth READ getLineWidth WRITE setLineWidth)

   /// Sets the start angle when drawing the following shapes: Ellipse, Arc, Chord
   ///
   Q_PROPERTY(double startAngle READ getStartAngle WRITE setStartAngle)

   /// Rotates the shape around the origin (top left corner unless modified with the 'origin translation' property. Units are degrees clockwise.
   /// Applies to all shapes.
   Q_PROPERTY(double rotation READ getRotation WRITE setRotation)

   /// Sector span of all arc related shapes, including Arc, Chord, Pie.
   ///
   Q_PROPERTY(double arcLength READ getArcLength WRITE setArcLength)

private:
   // Conveniancne macros.
   //
#define ANIMATION_ACCESS(INDEX) \
   void setAnimation##INDEX##Property (const animationOptions animation) { setAnimation (animation, INDEX - 1); } \
   animationOptions getAnimation##INDEX##Property() const { return getAnimation (INDEX - 1); }

#define SCALE_ACCESS(INDEX) \
   void   setScale##INDEX##Property (const double scale) { setScale (scale, INDEX - 1); } \
   double getScale##INDEX##Property() const { return getScale (INDEX - 1); }

#define OFFSET_ACCESS(INDEX) \
   void   setOffset##INDEX##Property (const double offset) { setOffset (offset, INDEX - 1); } \
   double getOffset##INDEX##Property() const { return getOffset (INDEX - 1); }

#define POINT_ACCESS(INDEX) \
   void   setPoint##INDEX##Property (const QPoint& point) { setPoint (point, INDEX - 1); } \
   QPoint getPoint##INDEX##Property() const { return getPoint (INDEX - 1);  }

#define COLOR_ACCESS(INDEX) \
   void   setColor##INDEX##Property (const QColor& color) { setColor (color, INDEX - 1); } \
   QColor getColor##INDEX##Property() const { return getColor (INDEX - 1);  }


   ANIMATION_ACCESS (1)
   ANIMATION_ACCESS (2)
   ANIMATION_ACCESS (3)
   ANIMATION_ACCESS (4)
   ANIMATION_ACCESS (5)
   ANIMATION_ACCESS (6)

   SCALE_ACCESS (1)
   SCALE_ACCESS (2)
   SCALE_ACCESS (3)
   SCALE_ACCESS (4)
   SCALE_ACCESS (5)
   SCALE_ACCESS (6)

   OFFSET_ACCESS (1)
   OFFSET_ACCESS (2)
   OFFSET_ACCESS (3)
   OFFSET_ACCESS (4)
   OFFSET_ACCESS (5)
   OFFSET_ACCESS (6)

   void setShapeProperty (shapeOptions shape) { setShape (shape); }
   shapeOptions getShapeProperty() const { return getShape(); }

   POINT_ACCESS (1)
   POINT_ACCESS (2)
   POINT_ACCESS (3)
   POINT_ACCESS (4)
   POINT_ACCESS (5)
   POINT_ACCESS (6)
   POINT_ACCESS (7)
   POINT_ACCESS (8)
   POINT_ACCESS (9)
   POINT_ACCESS(10)

   COLOR_ACCESS (1)
   COLOR_ACCESS (2)
   COLOR_ACCESS (3)
   COLOR_ACCESS (4)
   COLOR_ACCESS (5)
   COLOR_ACCESS (6)
   COLOR_ACCESS (7)
   COLOR_ACCESS (8)
   COLOR_ACCESS (9)
   COLOR_ACCESS (10)

   // Tidy up macros for includers
   //
#undef VARIABLE_PROPERTY_ACCESS
#undef ANIMATION_ACCESS
#undef SCALE_ACCESS
#undef OFFSET_ACCESS
#undef POINT_ACCESS
#undef COLOR_ACCESS
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEShape::animationOptions)
Q_DECLARE_METATYPE (QEShape::shapeOptions)
#endif

#endif // QE_SHAPE_H
