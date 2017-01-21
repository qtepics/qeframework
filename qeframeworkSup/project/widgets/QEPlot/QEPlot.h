/*  QEPlot.h
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
 *    Glenn Jackson
 *  Contact details:
 *    glenn.jackson@synchrotron.org.au
 */

#ifndef QE_PLOT_H
#define QE_PLOT_H

#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <QEWidget.h>
#include <QEFloating.h>
#include <QEFloatingFormatting.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEvent>
#include <QPoint>
#include <QVector>
#include <QTimer>
#include <QEPluginLibrary_global.h>

// Maximum number of variables.
#define QEPLOT_NUM_VARIABLES 4

// Trace related data and properties
class trace {
    public:

    trace(){ waveform = false; hasCurrentPoint = false; }
    QVector<QCaDateTime> timeStamps;
    QVector<double> xdata;
    QVector<double> ydata;

    QwtPlotCurve* curve;
    QColor color;
    QString legend;
    bool waveform;  // True if displaying a waveform (an array of values arriving in one update), false if displaying a strip chart (individual values arriving over time)
    QwtPlotCurve::CurveStyle style;

    bool hasCurrentPoint;   // If true this the last point is repeated at the current time. this is done to ensure a trace is drawn all the way up to the current time.
};

class QEPLUGINLIBRARYSHARED_EXPORT QEPlot : public QwtPlot, public QEWidget {
    Q_OBJECT

  public:
    QEPlot( QWidget *parent = 0 );
    QEPlot( const QString &variableName, QWidget *parent = 0 );

    ~QEPlot();

    QSize sizeHint() const;

    // Property convenience functions

    void setYMin( double yMin );
    double getYMin() const;

    void setYMax( double yMax );
    double getYMax() const;

    void setAutoScale( bool autoScale );
    bool getAutoScale() const;

    void setAxisEnableX( bool axisEnableXIn );
    bool getAxisEnableX() const;

    void setAxisEnableY( bool axisEnableYIn );
    bool getAxisEnableY() const;

    // No QEPlot::setTitle() needed. Uses QwtPlot::setTitle()
    QString getTitle() const;

    void    setBackgroundColor( QColor backgroundColor );
    QColor getBackgroundColor() const;

    void    setTraceStyle( QwtPlotCurve::CurveStyle traceStyle, const unsigned int variableIndex );
    QwtPlotCurve::CurveStyle getTraceStyle( const unsigned int variableIndex ) const;

    void    setTraceColor( QColor traceColor, const unsigned int variableIndex );
    void    setTraceColor1( QColor traceColor );
    void    setTraceColor2( QColor traceColor );
    void    setTraceColor3( QColor traceColor );
    void    setTraceColor4( QColor traceColor );
    QColor getTraceColor( const unsigned int variableIndex ) const;
    QColor getTraceColor1() const;
    QColor getTraceColor2() const;
    QColor getTraceColor3() const;
    QColor getTraceColor4() const;

    void    setTraceLegend1( QString traceLegend );
    void    setTraceLegend2( QString traceLegend );
    void    setTraceLegend3( QString traceLegend );
    void    setTraceLegend4( QString traceLegend );

    QString getTraceLegend1() const;
    QString getTraceLegend2() const;
    QString getTraceLegend3() const;
    QString getTraceLegend4() const;

    void    setXUnit( QString xUnit );
    QString getXUnit() const;

    void    setYUnit( QString yUnit );
    QString getYUnit() const;

    void setGridEnableMajorX( bool gridEnableMajorXIn );
    void setGridEnableMajorY( bool gridEnableMajorYIn );
    void setGridEnableMinorX( bool gridEnableMinorXIn );
    void setGridEnableMinorY( bool gridEnableMinorYIn );
    bool getGridEnableMajorX() const;
    bool getGridEnableMajorY() const;
    bool getGridEnableMinorX() const;
    bool getGridEnableMinorY() const;

    void setGridMajorColor( QColor gridMajorColorIn );
    void setGridMinorColor( QColor gridMinorColorIn );
    QColor getGridMajorColor() const;
    QColor getGridMinorColor() const;

    void setXStart( double xStart );
    double getXStart() const;

    void setXIncrement( double xIncrement );
    double getXIncrement() const;

    void setTimeSpan( unsigned int timeSpan );
    unsigned int getTimeSpan() const;

    void setTickRate( unsigned int tickRate );
    unsigned int getTickRate() const;

  signals:
    void mouseMove     (const QPointF& posn);

  protected:
    QEFloatingFormatting floatingFormatting;
    bool localEnabled;
    bool allowDrop;

    void canvasMouseMove( QMouseEvent* mouseEvent );
    bool eventFilter( QObject *obj, QEvent *event );
    void establishConnection( unsigned int variableIndex );

  private slots:
    void connectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int & );
    void setPlotData( const QVector<double>& values, QCaAlarmInfo&, QCaDateTime&, const unsigned int& );
    void setPlotData( const double value, QCaAlarmInfo&, QCaDateTime&, const unsigned int& );
    void tickTimeout();
    // !! move this functionality into QEWidget???
    // !! needs one for single variables and one for multiple variables, or just the multiple variable one for all
    void useNewVariableNameProperty( QString variableNameIn, QString variableNameSubstitutionsIn, unsigned int variableIndex )// !! move into Standard Properties section??
    {
        setVariableNameAndSubstitutions(variableNameIn, variableNameSubstitutionsIn, variableIndex);
    }

  signals:
    // Note, the following signals are common to many QE widgets,
    // if changing the doxygen comments, ensure relevent changes are migrated to all instances
    /// Sent when the widget is updated following a data change
    /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
    /// For example a QList widget could log updates from this widget.
    void dbValueChanged( const double& out );
    /// Sent when the widget is updated following a data change
    /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
    /// For example a QList widget could log updates from this widget.
    void dbValueChanged( const QVector<double>& out );

  private:

    void setup();

    QTimer* tickTimer;          // Timer to keep strip chart scrolling
    void setPlotDataCommon( const unsigned int variableIndex );
    void setalarmInfoCommon( QCaAlarmInfo& alarmInfo, const unsigned int variableIndex );


    // General plot properties
    double yMin;
    double yMax;
    bool autoScale;
    bool axisEnableX;
    bool axisEnableY;
    QwtPlotGrid* grid;
    bool gridEnableMajorX;
    bool gridEnableMajorY;
    bool gridEnableMinorX;
    bool gridEnableMinorY;
    QColor gridMajorColor;
    QColor gridMinorColor;

    // Trace update and movement properties
    unsigned int tickRate; //mS
    unsigned int timeSpan; // Seconds

    // Waveform properties
    double xStart;
    double xIncrement;

    bool plottingArrayData;    // True if plotting scalar (rather than array) data. Used to ensure only one plot mechanism is used.

    // Functions common to most QE widgets
    qcaobject::QCaObject* createQcaItem( unsigned int variableIndex );

    bool isConnected;
    
    // Variables and functions to manage plot data

    trace traces[QEPLOT_NUM_VARIABLES];

    void regenerateTickXData( const unsigned int variableIndex );

    void setCurveColor( const QColor color, const unsigned int variableIndex );
    void    setTraceLegend( QString traceLegend, const unsigned int variableIndex );
    QString getTraceLegend( const unsigned int variableIndex ) const;
    void setGridEnable();

    // Drag and Drop
protected:
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
        QCaVariableNamePropertyManager variableNamePropertyManagers[QEPLOT_NUM_VARIABLES];
    public:

    // Define a variable
    // Note, the QPROPERTY declaration itself can't be in this macro
#define VARIABLE_PROPERTY_ACCESS(VAR_INDEX) \
    void    setVariableName##VAR_INDEX##Property( QString variableName ){ variableNamePropertyManagers[VAR_INDEX].setVariableNameProperty( variableName ); } \
    QString getVariableName##VAR_INDEX##Property(){ return variableNamePropertyManagers[VAR_INDEX].getVariableNameProperty(); }

    VARIABLE_PROPERTY_ACCESS(0)
    /// EPICS variable name (CA PV).
    /// This variable is used to read updating values or waveforms for plotting in the first trace.
    Q_PROPERTY(QString variable1 READ getVariableName0Property WRITE setVariableName0Property)

    VARIABLE_PROPERTY_ACCESS(1)
    /// EPICS variable name (CA PV).
    /// This variable is used to read updating values or waveforms for plotting in the second trace.
    Q_PROPERTY(QString variable2 READ getVariableName1Property WRITE setVariableName1Property)

    VARIABLE_PROPERTY_ACCESS(2)
    /// EPICS variable name (CA PV).
    /// This variable is used to read updating values or waveforms for plotting in the third trace.
    Q_PROPERTY(QString variable3 READ getVariableName2Property WRITE setVariableName2Property)

    VARIABLE_PROPERTY_ACCESS(3)
    /// EPICS variable name (CA PV).
    /// This variable is used to read updating values or waveforms for plotting in the fourth trace.
    Q_PROPERTY(QString variable4 READ getVariableName3Property WRITE setVariableName3Property)

#undef VARIABLE_PROPERTY_ACCESS

    /// Macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2... Values may be quoted strings. For example, 'SAMPLE=SAM1, NAME = "Ref foil"'
    /// These substitutions are applied to all the variable names.
    Q_PROPERTY(QString variableSubstitutions READ getVariableNameSubstitutionsProperty WRITE setVariableNameSubstitutionsProperty)

    /// Property access function for #variableSubstitutions property. This has special behaviour to work well within designer.
    void    setVariableNameSubstitutionsProperty( QString variableNameSubstitutions )
    {
        for( int i = 0; i < QEPLOT_NUM_VARIABLES; i++ )
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

    Q_PROPERTY(QColor traceColor1 READ getTraceColor1 WRITE setTraceColor1)
    Q_PROPERTY(QColor traceColor2 READ getTraceColor2 WRITE setTraceColor2)
    Q_PROPERTY(QColor traceColor3 READ getTraceColor3 WRITE setTraceColor3)
    Q_PROPERTY(QColor traceColor4 READ getTraceColor4 WRITE setTraceColor4)

    Q_ENUMS(TraceStyles)
    Q_PROPERTY(TraceStyles traceStyle1 READ getTraceStyle1 WRITE setTraceStyle1)
    Q_PROPERTY(TraceStyles traceStyle2 READ getTraceStyle2 WRITE setTraceStyle2)
    Q_PROPERTY(TraceStyles traceStyle3 READ getTraceStyle3 WRITE setTraceStyle3)
    Q_PROPERTY(TraceStyles traceStyle4 READ getTraceStyle4 WRITE setTraceStyle4)
    enum TraceStyles { Lines  = QwtPlotCurve::Lines,
                       Sticks = QwtPlotCurve::Sticks,
                       Steps  = QwtPlotCurve::Steps,
                       Dots   = QwtPlotCurve::Dots };
    void setTraceStyle1( TraceStyles traceStyle ){ setTraceStyle( (QwtPlotCurve::CurveStyle)traceStyle, 0 ); }
    void setTraceStyle2( TraceStyles traceStyle ){ setTraceStyle( (QwtPlotCurve::CurveStyle)traceStyle, 1 ); }
    void setTraceStyle3( TraceStyles traceStyle ){ setTraceStyle( (QwtPlotCurve::CurveStyle)traceStyle, 2 ); }
    void setTraceStyle4( TraceStyles traceStyle ){ setTraceStyle( (QwtPlotCurve::CurveStyle)traceStyle, 3 ); }

    TraceStyles getTraceStyle1(){ return (TraceStyles)(getTraceStyle( 0 )); }
    TraceStyles getTraceStyle2(){ return (TraceStyles)(getTraceStyle( 1 )); }
    TraceStyles getTraceStyle3(){ return (TraceStyles)(getTraceStyle( 2 )); }
    TraceStyles getTraceStyle4(){ return (TraceStyles)(getTraceStyle( 3 )); }


    Q_PROPERTY(QString traceLegend1 READ getTraceLegend1 WRITE setTraceLegend1)
    Q_PROPERTY(QString traceLegend2 READ getTraceLegend2 WRITE setTraceLegend2)
    Q_PROPERTY(QString traceLegend3 READ getTraceLegend3 WRITE setTraceLegend3)
    Q_PROPERTY(QString traceLegend4 READ getTraceLegend4 WRITE setTraceLegend4)

    Q_PROPERTY(double yMin READ getYMin WRITE setYMin)
    Q_PROPERTY(double yMax READ getYMax WRITE setYMax)

    Q_PROPERTY(bool autoScale READ getAutoScale WRITE setAutoScale)

    Q_PROPERTY(bool axisEnableX READ getAxisEnableX WRITE setAxisEnableX)
    Q_PROPERTY(bool axisEnableY READ getAxisEnableY WRITE setAxisEnableY)

    Q_PROPERTY(bool gridEnableMajorX READ getGridEnableMajorX WRITE setGridEnableMajorX)
    Q_PROPERTY(bool gridEnableMajorY READ getGridEnableMajorY WRITE setGridEnableMajorY)

    Q_PROPERTY(bool gridEnableMinorX READ getGridEnableMinorX WRITE setGridEnableMinorX)
    Q_PROPERTY(bool gridEnableMinorY READ getGridEnableMinorY WRITE setGridEnableMinorY)

    Q_PROPERTY(QColor gridMajorColor READ getGridMajorColor WRITE setGridMajorColor)
    Q_PROPERTY(QColor gridMinorColor READ getGridMinorColor WRITE setGridMinorColor)

    Q_PROPERTY(QString title READ getTitle WRITE setTitle)
    Q_PROPERTY(QColor backgroundColor READ getBackgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QString xUnit READ getXUnit WRITE setXUnit)
    Q_PROPERTY(QString yUnit READ getYUnit WRITE setYUnit)
    Q_PROPERTY(double xStart READ getXStart WRITE setXStart)
    Q_PROPERTY(double xIncrement READ getXIncrement WRITE setXIncrement)
    Q_PROPERTY(unsigned int timeSpan READ getTimeSpan WRITE setTimeSpan)
    Q_PROPERTY(unsigned int tickRate READ getTickRate WRITE setTickRate)
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEPlot::UserLevels)
Q_DECLARE_METATYPE (QEPlot::DisplayAlarmStateOptions)
Q_DECLARE_METATYPE (QEPlot::TraceStyles)
#endif

#endif // QE_PLOT_H
