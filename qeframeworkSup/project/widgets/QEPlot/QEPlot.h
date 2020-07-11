/*  QEPlot.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2020 Australian Synchrotron
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
 *  Original Author:  Glenn Jackson
 *  Maintained by:    Andrew Starritt
 *  Contact details:  andrews@ansto.gov.au
 */

#ifndef QE_PLOT_H
#define QE_PLOT_H

#include <QEWidget.h>
#include <QEFrame.h>
#include <QEFloating.h>
#include <QEFloatingFormatting.h>
#include <QEInteger.h>
#include <QEIntegerFormatting.h>
#include <QCaVariableNamePropertyManager.h>
#include <QCaDataPoint.h>
#include <QEvent>
#include <QPoint>
#include <QVector>
#include <QTimer>
#include <QHBoxLayout>
#include <QEFrameworkLibraryGlobal.h>

// Differed class declaration - no explicit dependency on Qwt header files.
//
class QEGraphic;

/// This class provides the means to display a number of PVs either as a
/// mini strip chart for scalar PVs or as a mini waveform plotter for array PVs.
///
/// NOTE: Mixing scalar and array PVs is not recomended.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPlot : public QEFrame {
   Q_OBJECT
public:
   enum Constants {
      QEPLOT_NUM_PLOTS = 8,      // Maximum number of data/size variables.
      QEPLOT_NUM_VARIABLES = 16  // Maximum number of variables.
   };

   // Synonyms for QwtPlotCurve::CurveStyle
   // This are converted dynamically as opposed to having an
   // explicit dependency on QwtPlotCurve
   //
   Q_ENUMS (TraceStyles)
   enum TraceStyles {
      Lines = 1,
      Sticks,
      Steps,
      Dots
   };

   // Consider adding a None value and removing the axisEnableY bool property.
   //
   Q_ENUMS (SelectedYAxis)
   enum SelectedYAxis {
      Left = 1,
      Right
   };

   /// EPICS variable name (CA PV).
   /// These variables is used to read updating values or waveforms.
   ///
   Q_PROPERTY (QString variable1 READ getVariableName1Property WRITE setVariableName1Property)
   Q_PROPERTY (QString variable2 READ getVariableName2Property WRITE setVariableName2Property)
   Q_PROPERTY (QString variable3 READ getVariableName3Property WRITE setVariableName3Property)
   Q_PROPERTY (QString variable4 READ getVariableName4Property WRITE setVariableName4Property)
   Q_PROPERTY (QString variable5 READ getVariableName5Property WRITE setVariableName5Property)
   Q_PROPERTY (QString variable6 READ getVariableName6Property WRITE setVariableName6Property)
   Q_PROPERTY (QString variable7 READ getVariableName7Property WRITE setVariableName7Property)
   Q_PROPERTY (QString variable8 READ getVariableName8Property WRITE setVariableName8Property)

   /// These variables is used to read effective waveforms sizes, e.g. wavefom.NORD
   /// If not specified/connected then the whole array is used for display purposes.
   /// Note: Only applicable to waveforms, still connects but otherwsie ignored for scalars.
   ///
   Q_PROPERTY (QString sizeVariable1 READ getSizeVariableName1Property WRITE setSizeVariableName1Property)
   Q_PROPERTY (QString sizeVariable2 READ getSizeVariableName2Property WRITE setSizeVariableName2Property)
   Q_PROPERTY (QString sizeVariable3 READ getSizeVariableName3Property WRITE setSizeVariableName3Property)
   Q_PROPERTY (QString sizeVariable4 READ getSizeVariableName4Property WRITE setSizeVariableName4Property)
   Q_PROPERTY (QString sizeVariable5 READ getSizeVariableName5Property WRITE setSizeVariableName5Property)
   Q_PROPERTY (QString sizeVariable6 READ getSizeVariableName6Property WRITE setSizeVariableName6Property)
   Q_PROPERTY (QString sizeVariable7 READ getSizeVariableName7Property WRITE setSizeVariableName7Property)
   Q_PROPERTY (QString sizeVariable8 READ getSizeVariableName8Property WRITE setSizeVariableName8Property)

   /// Macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2...
   /// Values may be quoted strings. For example, 'SAMPLE=SAM1, NAME = "Ref foil"'
   /// These substitutions are applied to all the variable names.
   ///
   Q_PROPERTY (QString variableSubstitutions READ  getVariableNameSubstitutionsProperty
                                             WRITE setVariableNameSubstitutionsProperty)

   /// Widget specific properties
   ///
   Q_PROPERTY (QColor traceColor1 READ getTraceColor1 WRITE setTraceColor1)
   Q_PROPERTY (QColor traceColor2 READ getTraceColor2 WRITE setTraceColor2)
   Q_PROPERTY (QColor traceColor3 READ getTraceColor3 WRITE setTraceColor3)
   Q_PROPERTY (QColor traceColor4 READ getTraceColor4 WRITE setTraceColor4)
   Q_PROPERTY (QColor traceColor5 READ getTraceColor5 WRITE setTraceColor5)
   Q_PROPERTY (QColor traceColor6 READ getTraceColor6 WRITE setTraceColor6)
   Q_PROPERTY (QColor traceColor7 READ getTraceColor7 WRITE setTraceColor7)
   Q_PROPERTY (QColor traceColor8 READ getTraceColor8 WRITE setTraceColor8)

   Q_PROPERTY (TraceStyles traceStyle1 READ getTraceStyle1 WRITE setTraceStyle1)
   Q_PROPERTY (TraceStyles traceStyle2 READ getTraceStyle2 WRITE setTraceStyle2)
   Q_PROPERTY (TraceStyles traceStyle3 READ getTraceStyle3 WRITE setTraceStyle3)
   Q_PROPERTY (TraceStyles traceStyle4 READ getTraceStyle4 WRITE setTraceStyle4)
   Q_PROPERTY (TraceStyles traceStyle5 READ getTraceStyle5 WRITE setTraceStyle5)
   Q_PROPERTY (TraceStyles traceStyle6 READ getTraceStyle6 WRITE setTraceStyle6)
   Q_PROPERTY (TraceStyles traceStyle7 READ getTraceStyle7 WRITE setTraceStyle7)
   Q_PROPERTY (TraceStyles traceStyle8 READ getTraceStyle8 WRITE setTraceStyle8)

   // Pen width or dot size.
   Q_PROPERTY (int traceWidth1 READ getTraceWidth1 WRITE setTraceWidth1)
   Q_PROPERTY (int traceWidth2 READ getTraceWidth2 WRITE setTraceWidth2)
   Q_PROPERTY (int traceWidth3 READ getTraceWidth3 WRITE setTraceWidth3)
   Q_PROPERTY (int traceWidth4 READ getTraceWidth4 WRITE setTraceWidth4)
   Q_PROPERTY (int traceWidth5 READ getTraceWidth5 WRITE setTraceWidth5)
   Q_PROPERTY (int traceWidth6 READ getTraceWidth6 WRITE setTraceWidth6)
   Q_PROPERTY (int traceWidth7 READ getTraceWidth7 WRITE setTraceWidth7)
   Q_PROPERTY (int traceWidth8 READ getTraceWidth8 WRITE setTraceWidth8)

   Q_PROPERTY (QString traceLegend1 READ getTraceLegend1 WRITE setTraceLegend1)
   Q_PROPERTY (QString traceLegend2 READ getTraceLegend2 WRITE setTraceLegend2)
   Q_PROPERTY (QString traceLegend3 READ getTraceLegend3 WRITE setTraceLegend3)
   Q_PROPERTY (QString traceLegend4 READ getTraceLegend4 WRITE setTraceLegend4)
   Q_PROPERTY (QString traceLegend5 READ getTraceLegend5 WRITE setTraceLegend5)
   Q_PROPERTY (QString traceLegend6 READ getTraceLegend6 WRITE setTraceLegend6)
   Q_PROPERTY (QString traceLegend7 READ getTraceLegend7 WRITE setTraceLegend7)
   Q_PROPERTY (QString traceLegend8 READ getTraceLegend8 WRITE setTraceLegend8)

   Q_PROPERTY (double yMin READ getYMin WRITE setYMin)
   Q_PROPERTY (double yMax READ getYMax WRITE setYMax)

   Q_PROPERTY (bool autoScale READ getAutoScale WRITE setAutoScale)
   Q_PROPERTY (bool archiveBackfill READ getArchiveBackfill WRITE setArchiveBackfill)

   Q_PROPERTY (bool axisEnableX READ getAxisEnableX WRITE setAxisEnableX)
   Q_PROPERTY (bool axisEnableY READ getAxisEnableY WRITE setAxisEnableY)
   Q_PROPERTY (SelectedYAxis selectedYAxis READ getSelectedYAxis WRITE setSelectedYAxis)

   Q_PROPERTY (bool gridEnableMajorX READ getGridEnableMajorX WRITE setGridEnableMajorX)
   Q_PROPERTY (bool gridEnableMajorY READ getGridEnableMajorY WRITE setGridEnableMajorY)

   Q_PROPERTY (bool gridEnableMinorX READ getGridEnableMinorX WRITE setGridEnableMinorX)
   Q_PROPERTY (bool gridEnableMinorY READ getGridEnableMinorY WRITE setGridEnableMinorY)

   Q_PROPERTY (QColor gridMajorColor READ getGridMajorColor WRITE setGridMajorColor)
   Q_PROPERTY (QColor gridMinorColor READ getGridMinorColor WRITE setGridMinorColor)

   Q_PROPERTY (QString title          READ getTitle           WRITE setTitle)
   Q_PROPERTY (QColor backgroundColor READ getBackgroundColor WRITE setBackgroundColor)
   Q_PROPERTY (QString xUnit          READ getXUnit           WRITE setXUnit)
   Q_PROPERTY (QString yUnit          READ getYUnit           WRITE setYUnit)

   /// Waveform related properties.
   Q_PROPERTY (double xStart      READ getXStart      WRITE setXStart)
   Q_PROPERTY (double xIncrement  READ getXIncrement  WRITE setXIncrement)
   Q_PROPERTY (double xFirst      READ getXFirst      WRITE setXFirst)
   Q_PROPERTY (double xLast       READ getXLast       WRITE setXLast)

   /// Plot time span/duration (specified in seconds), constrained to be >= 1.
   Q_PROPERTY (int timeSpan READ getTimeSpan WRITE setTimeSpan)

   /// Tick rate (specified in mSec), constrained to be >= 20 and <= 2000.
   /// Would be better called tick interval
   Q_PROPERTY (int tickRate READ getTickRate WRITE setTickRate)

   Q_PROPERTY (int margin READ getMargin WRITE setMargin)

public:
   typedef QEFrame ParentWidgetClass;

   // QEPlot Widget Context Menu values
   //
   enum OwnContextMenuOptions {
      PLOTCM_NONE = CM_SPECIFIC_WIDGETS_START_HERE,
      PLOTCM_ARCHIVE_BACKFILL,
      PLOTCM_SUB_CLASS_WIDGETS_START_HERE
   };

   explicit QEPlot (QWidget* parent = 0);
   explicit QEPlot (const QString& variable1Name, QWidget* parent = 0);
   ~QEPlot ();

   QSize sizeHint () const;

   void setVariableNameSubstitutionsProperty (const QString& variableNameSubstitutions);
   QString getVariableNameSubstitutionsProperty () const;

   void setYMin (const double yMin);
   double getYMin () const;

   void setYMax (double yMax);
   double getYMax () const;

   void setAutoScale (const bool yAxisAutoScale);
   bool getAutoScale () const;

   void setArchiveBackfill (const bool archiveBackfill);
   bool getArchiveBackfill () const;

   void setAxisEnableX (const bool axisEnableX);
   bool getAxisEnableX () const;

   void setAxisEnableY (const bool axisEnableY);
   bool getAxisEnableY () const;

   void setSelectedYAxis (const SelectedYAxis selectedYAxis);
   SelectedYAxis getSelectedYAxis () const;

   void setTitle (const QString& title);
   QString getTitle () const;

   void setBackgroundColor (const QColor backgroundColor);
   QColor getBackgroundColor () const;

   // Each property associated with one of the traces needs simple read and write functions.
   // Each simple function leverages off a relatively complex function that takes
   // a variableIndex parameter.
   //
   void setVariableNameIndexProperty (const QString& variableName,
                                      const unsigned int variableIndex);
   QString getVariableNameIndexProperty (const unsigned int variableIndex) const;

   void setSizeVariableNameIndexProperty (const QString& variableName,
                                          const unsigned int variableIndex);
   QString getSizeVariableNameIndexProperty (const unsigned int variableIndex) const;

   void setTraceStyle (const TraceStyles traceStyle, const unsigned int variableIndex);
   TraceStyles getTraceStyle (const unsigned int variableIndex) const;

   void setTraceWidth (const int traceWidth, const unsigned int variableIndex);
   int getTraceWidth (const unsigned int variableIndex) const;

   void setTraceColor (const QColor traceColor, const unsigned int variableIndex);
   QColor getTraceColor (const unsigned int variableIndex) const;

   void setTraceLegend (const QString& traceLegend, const unsigned int variableIndex);
   QString getTraceLegend (const unsigned int variableIndex) const;

   // moc does not allow us to use macros to define Q_PROPERTIES, but we can
   // use a macro do define the associated get/set functions.
   //
#define PROPERTY_ACCESS_FUNCTIONS(name)                                        \
   void setVariableName##name##Property (const QString& pvName);               \
   QString getVariableName##name##Property () const;                           \
                                                                               \
   void setSizeVariableName##name##Property (const QString& pvName);           \
   QString getSizeVariableName##name##Property () const;                       \
                                                                               \
   void setTraceStyle##name (const TraceStyles traceStyle);                    \
   TraceStyles getTraceStyle##name () const;                                   \
                                                                               \
   void setTraceWidth##name (const int traceWidth);                            \
   int getTraceWidth##name () const;                                           \
                                                                               \
   void   setTraceColor##name (const QColor traceColor);                       \
   QColor getTraceColor##name () const;                                        \
                                                                               \
   void    setTraceLegend##name (const QString& traceLegend);                  \
   QString getTraceLegend##name () const;


    PROPERTY_ACCESS_FUNCTIONS (1)
    PROPERTY_ACCESS_FUNCTIONS (2)
    PROPERTY_ACCESS_FUNCTIONS (3)
    PROPERTY_ACCESS_FUNCTIONS (4)
    PROPERTY_ACCESS_FUNCTIONS (5)
    PROPERTY_ACCESS_FUNCTIONS (6)
    PROPERTY_ACCESS_FUNCTIONS (7)
    PROPERTY_ACCESS_FUNCTIONS (8)

#undef PROPERTY_ACCESS_FUNCTIONS

   void setXUnit (const QString& xUnit);
   QString getXUnit () const;

   void setYUnit (const QString& yUnit);
   QString getYUnit () const;

   void setGridEnableMajorX (const bool gridEnableMajorX);
   bool getGridEnableMajorX () const;

   void setGridEnableMajorY (const bool gridEnableMajorY);
   bool getGridEnableMajorY () const;

   void setGridEnableMinorX (const bool gridEnableMinorX);
   bool getGridEnableMinorX () const;

   void setGridEnableMinorY (const bool gridEnableMinorY);
   bool getGridEnableMinorY () const;

   void setGridMajorColor (const QColor gridMajorColor);
   QColor getGridMajorColor () const;

   void setGridMinorColor (const QColor gridMinorColor);
   QColor getGridMinorColor () const;

   void setXStart (const double xStart);
   double getXStart () const;

   void setXIncrement (const double xIncrement);
   double getXIncrement () const;

   void setXFirst (const double xFirst);
   double getXFirst () const;

   void setXLast (const double xLast);
   double getXLast () const;

   void setTimeSpan (const int timeSpan);
   int getTimeSpan () const;

   void setTickRate (const int tickRate);
   int getTickRate () const;

   void setMargin (const int margin);
   int getMargin () const;

signals:
   // Note, the following signals are common to many QE widgets,
   // if changing the doxygen comments, ensure relevent changes are migrated to all instances
   /// Sent when the widget is updated following a data change
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
   //
   void dbValueChanged (const double& out);

   /// Sent when the widget is updated following a data change
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
   //
   void dbValueChanged (const QVector < double >&out);

   // Emit the mouse position - real world co-ordinatea
   //
   void mouseMove (const QPointF& posn);

protected:
   bool eventFilter (QObject* watched, QEvent* event);

   QMenu* buildContextMenu ();                        // Build the specific context menu
   void contextMenuTriggered (int selectedItemNum);   // An action was selected from the context menu

   // Functions common to most QE widgets
   //
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);
   void establishConnection (unsigned int variableIndex);

   // Drag and Drop
   void dragEnterEvent (QDragEnterEvent *event) { qcaDragEnterEvent (event);  }
   void dropEvent (QDropEvent *event)           { qcaDropEvent (event, true); }
   void mousePressEvent (QMouseEvent *event)    { qcaMousePressEvent (event); }

   // This widget uses the default setDrop/getDrop defined in QEWidget.

   // Copy paste
   QString copyVariable ();
   QVariant copyData ();
   void paste (QVariant s);

   QEFloatingFormatting floatingFormatting;
   QEIntegerFormatting integerFormatting;

private:
   void setup ();
   void setAlarmInfoCommon (QCaAlarmInfo& alarmInfo, const unsigned int variableIndex);
   void plotData ();
   void drawLegend ();
   void purgeOldData ();
   void updateGridSettings ();
   void setReadOut (const QString& text);
   void setPlotAreaYRange (const double min, const double max, const bool immediate);

   QHBoxLayout* layout;
   int layoutMargin;
   QEGraphic* plotArea;
   QWidget* legendArea;

   QTimer* tickTimer;           // Timer to keep strip chart scrolling and replotting in general
   int tickTimerCount;
   bool replotIsRequired;

   // General plot properties
   double yMin;
   double yMax;
   bool yAxisAutoScale;
   bool archiveBackfill;
   bool axisEnableX;
   bool axisEnableY;
   SelectedYAxis selectedYAxis;
   QColor backgroundColor;
   bool gridEnableMajorX;
   bool gridEnableMajorY;
   bool gridEnableMinorX;
   bool gridEnableMinorY;
   QColor gridMajorColor;
   QColor gridMinorColor;

   // Trace update and movement properties
   int tickRate;                // mS
   int timeSpan;                // Seconds

   // Waveform properties
   double xStart;
   double xIncrement;
   double xFirst;               // first and
   double xLast;                // last x display range.

   // Variables and functions to manage plot data
   class Trace;
   Trace* traces[QEPLOT_NUM_PLOTS];

private slots:
   void connectionChanged (QCaConnectionInfo& connectionInfo, const unsigned int&);
   void setPlotData (const QVector<double>& values, QCaAlarmInfo&,
                     QCaDateTime&, const unsigned int&);
   void setPlotData (const double value, QCaAlarmInfo&,
                     QCaDateTime&, const unsigned int&);
   void setSizeData (const long value, QCaAlarmInfo&,
                     QCaDateTime&, const unsigned int&);
   void tickTimeout ();

   void useNewVariableNameProperty (QString variableName,
                                    QString variableNameSubstitutions,
                                    unsigned int variableIndex);

   void setArchiveData (const QObject* userData, const bool okay,
                        const QCaDataPointList& archiveData,
                        const QString& pvName, const QString& supplementary);

   // From the QEGraphic plot object.
   //
   void plotMouseMove (const QPointF& posn);
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEPlot::TraceStyles)
Q_DECLARE_METATYPE (QEPlot::SelectedYAxis)
#endif

#endif                          // QE_PLOT_H
