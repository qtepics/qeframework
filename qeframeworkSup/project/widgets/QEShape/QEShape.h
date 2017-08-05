/*  QEShape.h
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
 *  Copyright (c) 2009, 2010 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QE_SHAPE_H
#define QE_SHAPE_H

#include <QEInteger.h>
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
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEShape : public QWidget, public QEWidget {
    Q_OBJECT

  public:
    /// Create without a variable.
    /// Use setVariableNameProperty() and setSubstitutionsProperty() to define a variable and, optionally, macro substitutions later.
    QEShape( QWidget *parent = 0 );
    /// Create with a single variable. (Note, the QEShape widget can use up to 6 variables)
    /// A connection is automatically established.
    /// If macro substitutions are required, create without a variable and set the variable and macro substitutions after creation.
    QEShape( const QString& variableName, QWidget *parent = 0 );

    /// Options for the type of shape.
    ///
    enum shapeOptions { Line, Points, Polyline, Polygon, Rect, RoundedRect, Ellipse, Arc, Chord, Pie, Path };
    /// Options for how a variable will animate the shape.
    ///
    enum animationOptions {Width, Height, X, Y, Transperency, Rotation, ColourHue, ColourSaturation, ColourValue, ColourIndex, Penwidth};

    /// Scale the widgets my m/d.
    void scaleBy (const int m, const int d);

    // Property convenience functions

    // variable animations
    /// Access function for #animation' properties - refer to animation' properties for details
    void setAnimation( animationOptions animation, const int index );
    /// Access function for #animation' properties - refer to animation' properties for details
    animationOptions getAnimation( const int index );

    // scales
    /// Access function for #scale' properties - refer to scale' properties for details
    void setScale( const double scale, const int index );
    /// Access function for #scale' properties - refer to scale' properties for details
    double getScale( const int index );

    // offsets
    /// Access function for #offset' properties - refer to offset' properties for details
    void setOffset( const double offset, const int index );
    /// Access function for #offset' properties - refer to offset' properties for details
    double getOffset( const int index );

    // border
    /// Access function for #border' properties - refer to border' properties for details
    void setBorder( const bool border );
    /// Access function for #border' properties - refer to border' properties for details
    bool getBorder();

    // fill
    /// Access function for #fill' properties - refer to fill' properties for details
    void setFill( const bool fill );
    /// Access function for #fill' properties - refer to fill' properties for details
    bool getFill();

    // shape
    /// Access function for #shape' properties - refer to shape' properties for details
    void setShape( shapeOptions shape );
    /// Access function for #shape' properties - refer to shape' properties for details
    shapeOptions getShape();

    // number of points
    /// Access function for #number of points' properties - refer to number of points' properties for details
    void setNumPoints( const unsigned int numPoints );
    /// Access function for #number of points' properties - refer to number of points' properties for details
    unsigned int getNumPoints();

    // Origin translation
    /// Access function for #origin translation' properties - refer to origin translation' properties for details
    void setOriginTranslation( const QPoint originTranslation );
    /// Access function for #origin translation' properties - refer to origin translation' properties for details
    QPoint getOriginTranslation();

    // points
    /// Access function for #point' properties - refer to point' properties for details
    void setPoint( const QPoint point, const int index );
    /// Access function for #point' properties - refer to point' properties for details
    QPoint getPoint(const int index);

    // colors
    /// Access function for #colour' properties - refer to colour' properties for details
    void setColor( const QColor color, const int index );
    /// Access function for #colour' properties - refer to colour' properties for details
    QColor getColor( const int index );

    // draw border
    /// Access function for #draw border' properties - refer to draw border' properties for details
    void setDrawBorder( const bool drawBorder );
    /// Access function for #draw border' properties - refer to draw border' properties for details
    bool getDrawBorder();

    // line width
    /// Access function for #line width' properties - refer to line width' properties for details
    void setLineWidth( const unsigned int lineWidth );
    /// Access function for #line width' properties - refer to line width' properties for details
    unsigned int getLineWidth();

    // start angle
    /// Access function for #start angle' properties - refer to start angle' properties for details
    void setStartAngle( const double startAngle );
    /// Access function for #start angle' properties - refer to start angle' properties for details
    double getStartAngle();

    // rotation
    /// Access function for #rotation' properties - refer to rotation' properties for details
    void setRotation( const double rotation );
    /// Access function for #rotation' properties - refer to rotation' properties for details
    double getRotation();

    // arc length
    /// Access function for #arc length' properties - refer to arc length' properties for details
    void setArcLength( const double arcLength );
    /// Access function for #arc length' properties - refer to arc length' properties for details
    double getArcLength();

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

    void colorChange( unsigned int index );                     // Act on a color property change. (will update shape if using the color)
    unsigned int currentColor;                                  // Index into colorsProperty last used when setting brush color

    void establishConnection( unsigned int variableIndex );     // Create a CA connection and initiates updates if required

  private slots:
    void connectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex );
    void setValue( const long& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex );
    // !! move this functionality into QEWidget???
    // !! needs one for single variables and one for multiple variables, or just the multiple variable one for all
    void useNewVariableNameProperty( QString variableNameIn, QString variableNameSubstitutionsIn, unsigned int variableIndex )// !! move into Standard Properties section??
    {
        setVariableNameAndSubstitutions(variableNameIn, variableNameSubstitutionsIn, variableIndex);
    }

  signals:
    // Note, the following signals are common to many QE widgets,
    // if changing the doxygen comments, ensure relevent changes are migrated to all instances
    /// Sent when the widget is updated following a data change for the first variable
    /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
    /// For example a QList widget could log updates from this widget.
    void dbValueChanged1( const qlonglong& out );
    /// Sent when the widget is updated following a data change for the second variable
    /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
    /// For example a QList widget could log updates from this widget.
    void dbValueChanged2( const qlonglong& out );
    /// Sent when the widget is updated following a data change for the third variable
    /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
    /// For example a QList widget could log updates from this widget.
    void dbValueChanged3( const qlonglong& out );
    /// Sent when the widget is updated following a data change for the fourth variable
    /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
    /// For example a QList widget could log updates from this widget.
    void dbValueChanged4( const qlonglong& out );
    /// Sent when the widget is updated following a data change for the fifth variable
    /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
    /// For example a QList widget could log updates from this widget.
    void dbValueChanged5( const qlonglong& out );
    /// Sent when the widget is updated following a data change for the sixth variable
    /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
    /// For example a QList widget could log updates from this widget.
    void dbValueChanged6( const qlonglong& out );

  private:
    void setup();                                               // Initialisation common to all constructors
    qcaobject::QCaObject* createQcaItem( unsigned int variableIndex );
    void refreshData( const int index );

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
    void dragEnterEvent(QDragEnterEvent *event) { qcaDragEnterEvent( event ); }
    void dropEvent(QDropEvent *event)           { qcaDropEvent( event ); }
    void mousePressEvent(QMouseEvent *event)    { qcaMousePressEvent( event ); }
    void setDrop( QVariant drop );
    QVariant getDrop();

    // Copy paste
    QString copyVariable();
    QVariant copyData();
    void paste( QVariant s );

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
    void    setVariableName##VAR_INDEX##Property( QString variableName ){ variableNamePropertyManagers[VAR_INDEX].setVariableNameProperty( variableName ); } \
    QString getVariableName##VAR_INDEX##Property(){ return variableNamePropertyManagers[VAR_INDEX].getVariableNameProperty(); }

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
    void    setVariableNameSubstitutionsProperty( QString variableNameSubstitutions )
    {
        for( int i = 0; i < QESHAPE_NUM_VARIABLES; i++ )
        {
            variableNamePropertyManagers[i].setSubstitutionsProperty( variableNameSubstitutions );
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

    // Widget specific properties

    Q_ENUMS(animationOptions)
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

    Q_ENUMS(shapeOptions)

    /// Particular shape implemented by this widget.
    /// For example: line, rectangle, elipse. The coordinates used to draw the shape are taken from properties 'point1' to 'point10' as appropriate.
    Q_PROPERTY(shapeOptions shape READ getShapeProperty WRITE setShapeProperty)

    /// The number of points to use when drawing shapes that are defined by a variable number of points, such as polyline, polygon, path, and series of points.
    ///
    Q_PROPERTY(unsigned int numPoints READ getNumPoints WRITE setNumPoints )

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
    void setAnimation1Property( animationOptions animation ){ setAnimation( animation, 0 ); }
    animationOptions getAnimation1Property(){ return getAnimation( 0 ); }

    void setAnimation2Property( animationOptions animation ){ setAnimation( animation, 1 ); }
    animationOptions getAnimation2Property(){ return getAnimation( 1 ); }

    void setAnimation3Property( animationOptions animation ){ setAnimation( animation, 2 ); }
    animationOptions getAnimation3Property(){ return getAnimation( 2 ); }

    void setAnimation4Property( animationOptions animation ){ setAnimation( animation, 3 ); }
    animationOptions getAnimation4Property(){ return getAnimation( 3 ); }

    void setAnimation5Property( animationOptions animation ){ setAnimation( animation, 4 ); }
    animationOptions getAnimation5Property(){ return getAnimation( 4 ); }

    void setAnimation6Property( animationOptions animation ){ setAnimation( animation, 5 ); }
    animationOptions getAnimation6Property(){ return getAnimation( 5 ); }

    void setScale1Property( double scale ){ setScale( scale, 0 ); }
    double getScale1Property(){ return getScale( 0 ); }

    void setScale2Property( double scale ){ setScale( scale, 1 ); }
    double getScale2Property(){ return getScale( 1 ); }

    void setScale3Property( double scale ){ setScale( scale, 2 ); }
    double getScale3Property(){ return getScale( 2 ); }

    void setScale4Property( double scale ){ setScale( scale, 3 ); }
    double getScale4Property(){ return getScale( 3 ); }

    void setScale5Property( double scale ){ setScale( scale, 4 ); }
    double getScale5Property(){ return getScale( 4 ); }

    void setScale6Property( double scale ){ setScale( scale, 5 ); }
    double getScale6Property(){ return getScale( 5 ); }

    void setOffset1Property( double offset ){ setOffset( offset, 0 ); }
    double getOffset1Property(){ return getOffset( 0 ); }

    void setOffset2Property( double offset ){ setOffset( offset, 1 ); }
    double getOffset2Property(){ return getOffset( 1 ); }

    void setOffset3Property( double offset ){ setOffset( offset, 2 ); }
    double getOffset3Property(){ return getOffset( 2 ); }

    void setOffset4Property( double offset ){ setOffset( offset, 3 ); }
    double getOffset4Property(){ return getOffset( 3 ); }

    void setOffset5Property( double offset ){ setOffset( offset, 4 ); }
    double getOffset5Property(){ return getOffset( 4 ); }

    void setOffset6Property( double offset ){ setOffset( offset, 5 ); }
    double getOffset6Property(){ return getOffset( 5 ); }

    void setShapeProperty( shapeOptions shape ){ setShape(shape); }
    shapeOptions getShapeProperty(){ return getShape(); }

    void setPoint1Property( QPoint point ){ setPoint( point, 0 ); }
    QPoint getPoint1Property(){ return getPoint( 0 ); }

    void setPoint2Property( QPoint point ){ setPoint( point, 1 ); }
    QPoint getPoint2Property(){ return getPoint( 1 ); }

    void setPoint3Property( QPoint point ){ setPoint( point, 2 ); }
    QPoint getPoint3Property(){ return getPoint( 2 ); }

    void setPoint4Property( QPoint point ){ setPoint( point, 3 ); }
    QPoint getPoint4Property(){ return getPoint( 3 ); }

    void setPoint5Property( QPoint point ){ setPoint( point, 4 ); }
    QPoint getPoint5Property(){ return getPoint( 4 ); }

    void setPoint6Property( QPoint point ){ setPoint( point, 5 ); }
    QPoint getPoint6Property(){ return getPoint( 5 ); }

    void setPoint7Property( QPoint point ){ setPoint( point, 6 ); }
    QPoint getPoint7Property(){ return getPoint( 6 ); }

    void setPoint8Property( QPoint point ){ setPoint( point, 7 ); }
    QPoint getPoint8Property(){ return getPoint( 7 ); }

    void setPoint9Property( QPoint point ){ setPoint( point, 8 ); }
    QPoint getPoint9Property(){ return getPoint( 8 ); }

    void setPoint10Property( QPoint point ){ setPoint( point, 9 ); }
    QPoint getPoint10Property(){ return getPoint( 9 ); }

    void setColor1Property( QColor color ){ setColor( color, 0 ); }
    QColor getColor1Property(){ return getColor( 0 ); }

    void setColor2Property( QColor color ){ setColor( color, 1 ); }
    QColor getColor2Property(){ return getColor( 1 ); }

    void setColor3Property( QColor color ){ setColor( color, 2 ); }
    QColor getColor3Property(){ return getColor( 2 ); }

    void setColor4Property( QColor color ){ setColor( color, 3 ); }
    QColor getColor4Property(){ return getColor( 3 ); }

    void setColor5Property( QColor color ){ setColor( color, 4 ); }
    QColor getColor5Property(){ return getColor( 4 ); }

    void setColor6Property( QColor color ){ setColor( color, 5 ); }
    QColor getColor6Property(){ return getColor( 5 ); }

    void setColor7Property( QColor color ){ setColor( color, 6 ); }
    QColor getColor7Property(){ return getColor( 6 ); }

    void setColor8Property( QColor color ){ setColor( color, 7 ); }
    QColor getColor8Property(){ return getColor( 7 ); }

    void setColor9Property( QColor color ){ setColor( color, 8 ); }
    QColor getColor9Property(){ return getColor( 8 ); }

    void setColor10Property( QColor color ){ setColor( color, 9 ); }
    QColor getColor10Property(){ return getColor( 9 ); }
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEShape::UserLevels)
Q_DECLARE_METATYPE (QEShape::DisplayAlarmStateOptions)
Q_DECLARE_METATYPE (QEShape::animationOptions)
Q_DECLARE_METATYPE (QEShape::shapeOptions)
#endif

#endif // QE_SHAPE_H
