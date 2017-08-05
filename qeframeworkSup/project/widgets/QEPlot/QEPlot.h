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
 *  Copyright (c) 2009,2010,2017 Australian Synchrotron
 *
 *  Author:
 *    Glenn Jackson
 *  Contact details:
 *    glenn.jackson@synchrotron.org.au
 */

#ifndef QE_PLOT_H
#define QE_PLOT_H

#include <QEWidget.h>
#include <QEFrame.h>
#include <QEFloating.h>
#include <QEFloatingFormatting.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEvent>
#include <QPoint>
#include <QVector>
#include <QTimer>
#include <QVBoxLayout>
#include <QEFrameworkLibraryGlobal.h>

// Maximum number of variables.
#define QEPLOT_NUM_VARIABLES 4

// Differed class declaration - no user dependency on Qwt header files.
//
class QwtPlot;
class QwtPlotGrid;

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPlot : public QEFrame {
    Q_OBJECT

public:
    // Synonyms for QwtPlotCurve::CurveStyle
    // This are converted dynamically as opposed to having an
    // explicit dependency on QwtPlotCurve
    //
    Q_ENUMS(TraceStyles)
    enum TraceStyles { Lines = 1,
                       Sticks,
                       Steps,
                       Dots };

    explicit QEPlot( QWidget* parent = 0 );
    explicit QEPlot( const QString &variableName, QWidget* parent = 0 );

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

    void setTitle (const QString& title);
    QString getTitle() const;

    void    setBackgroundColor( const QColor backgroundColor );
    QColor getBackgroundColor() const;

    // Each property associated with one of the four traces needs simple read and write functions.
    // Each simple function leverages off a relatively complex function that takes a variableIndex parameter.
    // For historical reasons, variable name access functions are named 0 to 3, while other attribues are 1 to 4.
    //
    void setVariableNameIndexProperty ( const QString& variableName, const unsigned int variableIndex);
    void setVariableName0Property( const QString& pvName );
    void setVariableName1Property( const QString& pvName );
    void setVariableName2Property( const QString& pvName );
    void setVariableName3Property( const QString& pvName );

    QString getVariableNameIndexProperty( const unsigned int variableIndex ) const;
    QString getVariableName0Property () const;
    QString getVariableName1Property () const;
    QString getVariableName2Property () const;
    QString getVariableName3Property () const;

    void setTraceStyle(  const TraceStyles traceStyle, const unsigned int variableIndex );
    void setTraceStyle1( const TraceStyles traceStyle );
    void setTraceStyle2( const TraceStyles traceStyle );
    void setTraceStyle3( const TraceStyles traceStyle );
    void setTraceStyle4( const TraceStyles traceStyle );

    TraceStyles getTraceStyle( const unsigned int variableIndex ) const;
    TraceStyles getTraceStyle1() const;
    TraceStyles getTraceStyle2() const;
    TraceStyles getTraceStyle3() const;
    TraceStyles getTraceStyle4() const;

    void    setTraceColor(  const QColor traceColor, const unsigned int variableIndex );
    void    setTraceColor1( const QColor traceColor );
    void    setTraceColor2( const QColor traceColor );
    void    setTraceColor3( const QColor traceColor );
    void    setTraceColor4( const QColor traceColor );

    QColor getTraceColor( const unsigned int variableIndex ) const;
    QColor getTraceColor1() const;
    QColor getTraceColor2() const;
    QColor getTraceColor3() const;
    QColor getTraceColor4() const;

    void    setTraceLegend(  const QString& traceLegend, const unsigned int variableIndex );
    void    setTraceLegend1( const QString& traceLegend );
    void    setTraceLegend2( const QString& traceLegend );
    void    setTraceLegend3( const QString& traceLegend );
    void    setTraceLegend4( const QString& traceLegend );

    QString getTraceLegend( const unsigned int variableIndex ) const;
    QString getTraceLegend1() const;
    QString getTraceLegend2() const;
    QString getTraceLegend3() const;
    QString getTraceLegend4() const;

    void    setXUnit( const QString& xUnit );
    QString getXUnit() const;

    void    setYUnit( const QString& yUnit );
    QString getYUnit() const;

    void setGridEnableMajorX( bool gridEnableMajorXIn );
    bool getGridEnableMajorX() const;

    void setGridEnableMajorY( bool gridEnableMajorYIn );
    bool getGridEnableMajorY() const;

    void setGridEnableMinorX( bool gridEnableMinorXIn );
    bool getGridEnableMinorX() const;

    void setGridEnableMinorY( bool gridEnableMinorYIn );
    bool getGridEnableMinorY() const;

    void setGridMajorColor( QColor gridMajorColorIn );
    QColor getGridMajorColor() const;

    void setGridMinorColor( QColor gridMinorColorIn );
    QColor getGridMinorColor() const;

    void setXStart( double xStart );
    double getXStart() const;

    void setXIncrement( double xIncrement );
    double getXIncrement() const;

    void setTimeSpan( int timeSpan );
    int getTimeSpan() const;

    void setTickRate( int tickRate );
    int getTickRate() const;

    void setMargin( const int margin );
    int getMargin() const;

signals:
    void mouseMove( const QPointF& posn );

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
    // !! needs one for single variables and one for multiple variables, or
    //    just the multiple variable one for all
    void useNewVariableNameProperty( QString variableName,
                                     QString variableNameSubstitutions,
                                     unsigned int variableIndex );

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

    QVBoxLayout* layout;
    int layoutMargin;
    QwtPlot* plot;
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
    int tickRate; // mS
    int timeSpan; // Seconds

    // Waveform properties
    double xStart;
    double xIncrement;

    bool plottingArrayData;    // True if plotting scalar (rather than array) data. Used to ensure only one plot mechanism is used.

    // Functions common to most QE widgets
    qcaobject::QCaObject* createQcaItem( unsigned int variableIndex );

    bool isConnected;

    // Variables and functions to manage plot data
    class Trace;
    Trace* traces[QEPLOT_NUM_VARIABLES];

    void regenerateTickXData( const unsigned int variableIndex );

    void setCurveColor( const QColor color, const unsigned int variableIndex );
    void setGridEnable();

    // Drag and Drop
protected:
    void dragEnterEvent(QDragEnterEvent *event) { qcaDragEnterEvent( event ); }
    void dropEvent(QDropEvent *event)           { qcaDropEvent( event ); }
    void mousePressEvent(QMouseEvent *event)    { qcaMousePressEvent( event ); }
    // This widget uses the setDrop/getDrop defined in QEWidget.

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

    /// EPICS variable name (CA PV).
    /// This variable is used to read updating values or waveforms for plotting in the first trace.
    Q_PROPERTY(QString variable1 READ getVariableName0Property WRITE setVariableName0Property)

    /// EPICS variable name (CA PV).
    /// This variable is used to read updating values or waveforms for plotting in the second trace.
    Q_PROPERTY(QString variable2 READ getVariableName1Property WRITE setVariableName1Property)

    /// EPICS variable name (CA PV).
    /// This variable is used to read updating values or waveforms for plotting in the third trace.
    Q_PROPERTY(QString variable3 READ getVariableName2Property WRITE setVariableName2Property)

    /// EPICS variable name (CA PV).
    /// This variable is used to read updating values or waveforms for plotting in the fourth trace.
    Q_PROPERTY(QString variable4 READ getVariableName3Property WRITE setVariableName3Property)

    /// Macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2... Values may be quoted strings. For example, 'SAMPLE=SAM1, NAME = "Ref foil"'
    /// These substitutions are applied to all the variable names.
    Q_PROPERTY(QString variableSubstitutions READ getVariableNameSubstitutionsProperty WRITE setVariableNameSubstitutionsProperty)

    /// Property access function for #variableSubstitutions property. This has special behaviour to work well within designer.
    void    setVariableNameSubstitutionsProperty( const QString& variableNameSubstitutions );

    /// Property access function for #variableSubstitutions property. This has special behaviour to work well within designer.
    QString getVariableNameSubstitutionsProperty() const;

public:
    //=================================================================================

    // Widget specific properties
    //
    Q_PROPERTY(QColor traceColor1 READ getTraceColor1 WRITE setTraceColor1)
    Q_PROPERTY(QColor traceColor2 READ getTraceColor2 WRITE setTraceColor2)
    Q_PROPERTY(QColor traceColor3 READ getTraceColor3 WRITE setTraceColor3)
    Q_PROPERTY(QColor traceColor4 READ getTraceColor4 WRITE setTraceColor4)

    Q_PROPERTY(TraceStyles traceStyle1 READ getTraceStyle1 WRITE setTraceStyle1)
    Q_PROPERTY(TraceStyles traceStyle2 READ getTraceStyle2 WRITE setTraceStyle2)
    Q_PROPERTY(TraceStyles traceStyle3 READ getTraceStyle3 WRITE setTraceStyle3)
    Q_PROPERTY(TraceStyles traceStyle4 READ getTraceStyle4 WRITE setTraceStyle4)

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
    Q_PROPERTY(int timeSpan READ getTimeSpan WRITE setTimeSpan)
    Q_PROPERTY(int tickRate READ getTickRate WRITE setTickRate)
    Q_PROPERTY(int margin READ getMargin WRITE setMargin)
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEPlot::TraceStyles)
#endif

#endif // QE_PLOT_H
