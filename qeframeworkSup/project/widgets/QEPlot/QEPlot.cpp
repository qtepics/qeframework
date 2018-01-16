/*  QEPlot.cpp
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
 *  Copyright (c) 2009,2010,2016,2017,2018 Australian Synchrotron
 *
 *  Author:
 *    Glenn Jackson
 *  Contact details:
 *    glenn.jackson@synchrotron.org.au
*/

/*
  This class is a CA aware Plot widget and is based in part on the work of the Qwt project (http://qwt.sf.net).
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details.
 */

#include "QEPlot.h"
#include <QDebug>
#include <QECommon.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>

#define DEBUG qDebug () << "QEPlot" <<  __LINE__ << __FUNCTION__  << "  "


//-----------------------------------------------------------------------------
// Macro fuction to enure varable index in in expected range.
// Set defval to nil for void functions.
//
#define PV_INDEX_CHECK(vi, defval)   {                                 \
   if ((vi) >= QEPLOT_NUM_VARIABLES) {                                 \
      DEBUG << "unexpected variableIndex" << (vi);                     \
      return defval;                                                   \
   }                                                                   \
}

//------------------------------------------------------------------------------
// Convert between Qwt CurveStyle and own TraceStyles
//
static QEPlot::TraceStyles convertCurve2Trace (const QwtPlotCurve::CurveStyle style)
{
   QEPlot::TraceStyles result;
   switch (style) {
      case QwtPlotCurve::Lines:
         result = QEPlot::Lines;
         break;
      case QwtPlotCurve::Sticks:
         result = QEPlot::Sticks;
         break;
      case QwtPlotCurve::Steps:
         result = QEPlot::Steps;
         break;
      case QwtPlotCurve::Dots:
         result = QEPlot::Dots;
         break;
      default:
         result = QEPlot::Lines;
         break;
   }
   return result;
}

//------------------------------------------------------------------------------
//
static QwtPlotCurve::CurveStyle convertTrace2Curve (const QEPlot::TraceStyles style)
{
   QwtPlotCurve::CurveStyle result;
   switch (style) {
      case QEPlot::Lines:
         result = QwtPlotCurve::Lines;
         break;
      case QEPlot::Sticks:
         result = QwtPlotCurve::Sticks;
         break;
      case QEPlot::Steps:
         result = QwtPlotCurve::Steps;
         break;
      case QEPlot::Dots:
         result = QwtPlotCurve::Dots;
         break;
      default:
         result = QwtPlotCurve::Lines;
         break;
   }
   return result;
}

//==============================================================================
// Trace related data and properties
//
class QEPlot::Trace {
public:

   explicit Trace () {
      waveform = false;
      hasCurrentPoint = false;
      curve = NULL;
   }

   ~Trace () {}

   QVector <QCaDateTime> timeStamps;
   QVector <double> xdata;
   QVector <double> ydata;

   QwtPlotCurve* curve;
   QColor color;
   QString legend;

   // True if displaying a waveform (an array of values arriving in one update),
   // false if displaying a strip chart (individual values arriving over time)
   bool waveform;

   QwtPlotCurve::CurveStyle style;

   // If true this the last point is repeated at the current time.
   // This is done to ensure a trace is drawn all the way up to the current time.
   bool hasCurrentPoint;
};


//==============================================================================
// Constructor with no initialisation
//
QEPlot::QEPlot (QWidget* parent) : QEFrame (parent)
{
   this->setup ();
}

//------------------------------------------------------------------------------
// Constructor with known variable
//
QEPlot::QEPlot (const QString& variableNameIn,
                QWidget* parent) : QEFrame (parent)
{
   this->setup ();
   this->setVariableName (variableNameIn, 0);
   this->activate ();
}

//------------------------------------------------------------------------------
// Setup common to all constructors
//
void QEPlot::setup ()
{
   // Set plain shape and noframe - the internal QwtPlot widget does its
   // own shape/shadow
   this->setFrameShape(QFrame::NoFrame );
   this->setFrameShadow(QFrame::Plain );

   // Set up data
   // This control used a single data source
   this->setNumVariables( QEPLOT_NUM_VARIABLES );

   // Set up default properties
   this->setAllowDrop( false );

   // Set the initial state
   this->isConnected = false;

   // General plot properties
   this->yMin = 0.0;
   this->yMax = 0.0;
   this->autoScale = true;
   this->axisEnableX = true;
   this->axisEnableY = true;

   //setLabelOrientation (Qt::Orientation)Qt::Vertical

   // Default to one minute span
   this->tickRate = 50;
   this->timeSpan = 60;

   // Allocate Trace objects
   for (int i = 0; i < QEPLOT_NUM_VARIABLES; i++) {
      this->traces[i] = new Trace ();
   }

   // plot does the actual plotting
   this->plot = new QwtPlot (NULL);

   this->layout = new QVBoxLayout (this);
   this->layoutMargin = 0;
   this->layout->setMargin (layoutMargin);
   this->layout->addWidget (plot);

   this->tickTimer = new QTimer (this);
   QObject::connect (tickTimer, SIGNAL (timeout ()),
                     this, SLOT (tickTimeout ()));
   this->tickTimer->start (tickRate);

   // Waveform properties
   this->xStart = 0.0;
   this->xIncrement = 1.0;

   // Initially no curve or grid, and different trace colors
   for (int i = 0; i < QEPLOT_NUM_VARIABLES; i++) {
      Trace* tr = this->traces[i];
      tr->curve = NULL;
      switch (i)                // Note, this assumes 4 traces, but won't break with more or less
      {
         case 0:
            tr->color = Qt::black;
            break;
         case 1:
            tr->color = Qt::red;
            break;
         case 2:
            tr->color = Qt::green;
            break;
         case 3:
            tr->color = Qt::blue;
            break;
         default:
            tr->color = Qt::black;
            break;
      }
      tr->style = QwtPlotCurve::Lines;
   }

   this->grid = NULL;
   this->gridEnableMajorX = false;
   this->gridEnableMajorY = false;
   this->gridEnableMinorX = false;
   this->gridEnableMinorY = false;
   this->gridMajorColor = Qt::black;
   this->gridMinorColor = Qt::gray;


   // Assume we are plotting scalar (rather than array) data
   this->plottingArrayData = false;

   // Use standard context menu
   this->setupContextMenu ();

   // Use QwtPlot signals
   // !! move this functionality into QEWidget???
   // !! needs one for single variables and one for multiple variables, or just the multiple variable one for all
   // for each variable name property manager, set up an index to identify it when it signals and
   // set up a connection to recieve variable name property changes.
   // The variable name property manager class only delivers an updated variable name after the user has stopped typing
   for (int i = 0; i < QEPLOT_NUM_VARIABLES; i++) {
      this->variableNamePropertyManagers[i].setVariableIndex (i);
      QObject::connect (&this->variableNamePropertyManagers[i],
                        SIGNAL (newVariableNameProperty  (QString, QString, unsigned int)), this,
                        SLOT (useNewVariableNameProperty (QString, QString, unsigned int)));
   }

   this->plot->canvas ()->setMouseTracking (true);
   this->plot->canvas ()->installEventFilter (this);
   this->setBackgroundColor (QColor (200,200,200));
}

//------------------------------------------------------------------------------
//
QEPlot::~QEPlot ()
{
   if (this->tickTimer) {
      this->tickTimer->stop ();
      delete this->tickTimer;
   }

   for (int i = 0; i < QEPLOT_NUM_VARIABLES; i++) {
      Trace* tr = this->traces[i];
      if (tr->curve) {
         delete tr->curve;
         tr->curve = NULL;
      }
   }

   if (this->grid) {
      delete this->grid;
   }

   // Dellocate Trace objects
   for (int i = 0; i < QEPLOT_NUM_VARIABLES; i++) {
      if (this->traces[i])
         delete this->traces[i];
   }
}

//------------------------------------------------------------------------------
// Provides size hint in designer - in not a constraint
//
QSize QEPlot::sizeHint () const
{
   return QSize (240, 100);
}

//------------------------------------------------------------------------------
// Convert canvas position into real world co-ordinates.
//
void QEPlot::canvasMouseMove (QMouseEvent* mouseEvent)
{
   QPoint pos = mouseEvent->pos ();
   double x = plot->invTransform (QwtPlot::xBottom, pos.x ());
   double y = plot->invTransform (QwtPlot::yLeft, pos.y ());
   QPointF posn = QPointF (x, y);
   emit mouseMove (posn);
}

//------------------------------------------------------------------------------
// Handle events, specifically the mouse move events
//
bool QEPlot::eventFilter (QObject* obj, QEvent* event)
{
   const QEvent::Type type = event->type ();
   QMouseEvent* mouseEvent = NULL;

   switch (type) {
      case QEvent::MouseMove:
         mouseEvent = static_cast <QMouseEvent*>(event);
         if (obj == plot->canvas ()) {
            canvasMouseMove (mouseEvent);
            return true;        // we have handled move mouse event
         }
         break;

      default:
         break;
   }

   return false;
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of QCaObject required.
// For a strip chart a QCaObject that streams floating point data is required.
//
qcaobject::QCaObject* QEPlot::createQcaItem (unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex, NULL);

   // Create the item as a QEFloating
   return new QEFloating (this->getSubstitutedVariableName (variableIndex), this,
                          &this->floatingFormatting, variableIndex);
}

//------------------------------------------------------------------------------
// Start updating.  Implementation of VariableNameManager's virtual funtion to
// establish a connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QEPlot::establishConnection (unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex, );

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   qcaobject::QCaObject* qca = this->createConnection (variableIndex);

   if (!qca) return;

   // If a QCaObject object is now available to supply data update signals,
   // connect it to the appropriate slots
   //
   QObject::connect (qca, SIGNAL (floatingArrayChanged (const QVector <double>&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                     this,  SLOT (setPlotData          (const QVector <double>&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));
   QObject::connect (qca, SIGNAL (floatingChanged      (const double, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                     this,  SLOT (setPlotData          (const double, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));
   QObject::connect (qca, SIGNAL (connectionChanged    (QCaConnectionInfo&, const unsigned int&)),
                     this,  SLOT (connectionChanged    (QCaConnectionInfo&, const unsigned int&)));
}


//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the strip chart looks and change the tool tip
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QEPlot::connectionChanged (QCaConnectionInfo& connectionInfo,
                                const unsigned int& variableIndex)
{
   PV_INDEX_CHECK (variableIndex, );

   // Note the connected state
   this->isConnected = connectionInfo.isChannelConnected ();

   // Display the connected state
   this->updateToolTipConnection (isConnected, variableIndex);
   this->processConnectionInfo (isConnected, variableIndex);
}

//------------------------------------------------------------------------------
// Update the plotted data with a new single value
// This is a slot used to recieve data updates from a QCaObject based class.
//
void QEPlot::setPlotData (const double value, QCaAlarmInfo& alarmInfo,
                          QCaDateTime& timestamp,
                          const unsigned int& variableIndex)
{
   PV_INDEX_CHECK (variableIndex, );

   // A seperate data connection (QEPlot::setPlotData( const QVector<double>& values, ... ) manages
   // array data (it also determines if we are getting array data), so do nothing more here if plotting array data data
   if (this->plottingArrayData) {
      return;
   }

   // Signal a database value change to any Link widgets
   emit dbValueChanged (value);

   // If the date is more than a wisker into the future, limit it.
   // This will happen if the source is on another machine with an incorrect time.
   // Allow a little bit of time (100mS) as machines will not be synchronised perfectly.
   // This will help if updates get bunched.
   // If this is not done and we are adding a last point at the current time, this last point will be before this actual data point
   QCaDateTime ct = QCaDateTime::currentDateTime ();
   double tsDiff = ct.secondsTo (timestamp);
   if (tsDiff > 0.1) {
      timestamp = ct.addMSecs (100);
   }
   // Else, If the date is a long way in the past, limit to a small amount.
   // This will happen if the source is on another machine with an incorrect time.
   // Allow a bit of time (500mS) as machines will not be synchronised perfectly and for network latency hichups.
   // If this is not done and we are adding a last point at the current time, there will always be a flat bit of line at the end of the plot.
   else if (tsDiff < -0.5) {
      timestamp = ct.addMSecs (-500);
   }

   // Select the curve information for this variable
   Trace* tr = this->traces[variableIndex];

   // Flag this trace is displaying a strip chart
   tr->waveform = false;

   // If we are currently holding array data, get rid of it as we are switching to scalar data.
   // (This is very unlikely, but couild happen if the IOC has rebooted)
   // Note, array data does not have timestamps.
   if (tr->timeStamps.count () != tr->ydata.count ()) {
      tr->timeStamps.clear ();
      tr->ydata.clear ();
      tr->xdata.clear ();
      tr->hasCurrentPoint = false;
   }
   // If the last point was repeated at the current time to ensure the trace is
   // displayed up to the current time, remove it
   if (tr->hasCurrentPoint) {
      int size = tr->xdata.size ();
      tr->timeStamps.remove (size - 1);
      tr->ydata.remove (size - 1);
      tr->xdata.remove (size - 1);
      tr->hasCurrentPoint = false;
   }
   // Add the new data point
   tr->timeStamps.append (timestamp);
   tr->ydata.append (value);
   tr->xdata.append (0.0);      // keep x and y arrays the same size
   regenerateTickXData (variableIndex);

   // Remove any old data
   QDateTime oldest = QDateTime::currentDateTime ();
   oldest = oldest.addSecs (-(int) (timeSpan));
   while (tr->timeStamps.count () > 1) {
      if (tr->timeStamps[1] < oldest) {
         tr->timeStamps.remove (0);
         tr->xdata.remove (0);
         tr->ydata.remove (0);
      } else {
         break;
      }
   }

   // Fix the X for a strip chart
   plot->setAxisScale (QwtPlot::xBottom, -(double) timeSpan, 0.0);

   // The data is now ready to plot
   this->setPlotDataCommon (variableIndex);
   this->setalarmInfoCommon (alarmInfo, variableIndex);
}

//------------------------------------------------------------------------------
// Update the plotted data with a new array of values
// This is a slot used to recieve data updates from a QCaObject based class.
//
void QEPlot::setPlotData (const QVector <double>& values,
                          QCaAlarmInfo& alarmInfo, QCaDateTime &,
                          const unsigned int& variableIndex)
{
   PV_INDEX_CHECK (variableIndex, );

   // A seperate data connection (QEPlot::setPlotData( const double value, ... ) manages scalar data,
   // so decide if we are plotting scalar or array data and do nothing more here if plotting scalar data
   this->plottingArrayData = (values.count () > 1);
   if (!this->plottingArrayData) {
      return;
   }

   // Signal a database value change to any Link widgets
   emit dbValueChanged (values);

   // Select the curve information for this variable
   Trace* tr = this->traces[variableIndex];

   // Flag this trace is displaying a waveform
   tr->waveform = true;

   // Clear any previous data
   tr->xdata.clear ();
   tr->ydata.clear ();
   tr->timeStamps.clear ();
   tr->hasCurrentPoint = false;

   // If no increment was supplied, use 1 by default
   double inc;
   inc = this->xIncrement == 0.0 ? 1.0 : this->xIncrement;

   for (int i = 0; i < values.count (); i++) {
      tr->xdata.append (xStart + ((double) i * inc));
      tr->ydata.append (values[i]);
   }

   // Autoscale X for a waveform
   this->plot->setAxisAutoScale (QwtPlot::xBottom);

   // The data is now ready to plot
   this->setPlotDataCommon (variableIndex);
   this->setalarmInfoCommon (alarmInfo, variableIndex);
}

//------------------------------------------------------------------------------
// Update the plot with new data.
// The new data may be due to a new value being added to the current values (stripchart)
// or the new data may be due to a new waveform
//
void QEPlot::setPlotDataCommon (const unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex, );

   Trace* tr = this->traces[variableIndex];

   // Create the curve if it does not exist
   if (!tr->curve) {
      tr->curve = new QwtPlotCurve (tr->legend);

      this->setCurveColor (tr->color, variableIndex);
      tr->curve->setRenderHint (QwtPlotItem::RenderAntialiased);
      tr->curve->setStyle (tr->style);
      tr->curve->attach (plot);
   }

   // Set the curve data
#if QWT_VERSION >= 0x060000
   tr->curve->setSamples (tr->xdata, tr->ydata);
#else
   tr->curve->setData (tr->xdata, tr->ydata);
#endif

   // Update the plot
   this->plot->replot ();
}

//------------------------------------------------------------------------------
//
void QEPlot::setalarmInfoCommon (QCaAlarmInfo & alarmInfo,
                                 const unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex, );

   // Invoke common alarm handling processing.
   // TODO: Aggregate all channel severities into a single alarm state.
   this->processAlarmInfo (alarmInfo, variableIndex);
}

//------------------------------------------------------------------------------
// For strip chart functionality
// Recalculate the x value as time goes by
//
void QEPlot::regenerateTickXData (const unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex, );

   Trace* tr = this->traces[variableIndex];

   QCaDateTime now = QDateTime::currentDateTime ();
   for (int i = 0; i < tr->xdata.count (); i++) {
      tr->xdata[i] = now.secondsTo (tr->timeStamps[i]);
   }
}

//------------------------------------------------------------------------------
// Update the chart if it is a strip chart
//
void QEPlot::tickTimeout ()
{
   for (int i = 0; i < QEPLOT_NUM_VARIABLES; i++) {
      Trace* tr = this->traces[i];
      if (tr->curve && !tr->waveform) {
         // Ensure the trace continues all the way up to the current time
         // regardless of when the last point appeared

         // If there is any data...
         int size = tr->ydata.size ();
         if (size) {
            // If there is a simulated 'current' point...
            if (tr->hasCurrentPoint) {
               // ...update it to the current time
               tr->timeStamps[size - 1] = QDateTime::currentDateTime ();
            }
            // If there is no simulated 'current' point...
            else {
               // ...duplicate the last point at the current time
               tr->timeStamps.append (QDateTime::currentDateTime ());
               tr->ydata.append (tr->ydata[size - 1]);
               tr->xdata.append (tr->xdata[size - 1]);
               tr->hasCurrentPoint = true;
            }
         }

         // Recalculate where the points now are and display them
         this->regenerateTickXData (i);
         this->setPlotDataCommon (i);
      }
   }
}

//------------------------------------------------------------------------------
// Set variable name
//
void QEPlot::useNewVariableNameProperty (QString variableNameIn,
                                         QString variableNameSubstitutionsIn,
                                         unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex, );

   this->setVariableNameAndSubstitutions (variableNameIn,
                                          variableNameSubstitutionsIn,
                                          variableIndex);
}

//------------------------------------------------------------------------------
// Update the color of the trace
//
void QEPlot::setCurveColor (const QColor color,
                            const unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex, );

   Trace* tr = this->traces[variableIndex];
   if (tr->curve) {
      tr->curve->setPen (color);
   }
}

//------------------------------------------------------------------------------
// Variable name proprty access access
//
void QEPlot::setVariableNameIndexProperty (const QString& variableName,
                                           const unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex, );
   this->variableNamePropertyManagers[variableIndex].setVariableNameProperty (variableName);
}

//------------------------------------------------------------------------------
//
QString QEPlot::getVariableNameIndexProperty (const unsigned int variableIndex) const
{
   PV_INDEX_CHECK (variableIndex, "");
   return this->variableNamePropertyManagers[variableIndex].getVariableNameProperty ();
}

//------------------------------------------------------------------------------
// Variable substitutions access
//
void QEPlot::setVariableNameSubstitutionsProperty (const QString& variableNameSubstitutions)
{
   // Same substitutions apply to all variables.
   for (int i = 0; i < QEPLOT_NUM_VARIABLES; i++) {
      this->variableNamePropertyManagers[i].
          setSubstitutionsProperty (variableNameSubstitutions);
   }
}

QString QEPlot::getVariableNameSubstitutionsProperty () const
{
   // All the same - any variable's substitutions will do.
   return this->variableNamePropertyManagers[0].getSubstitutionsProperty ();
}


//==============================================================================
// Copy / Paste
QString QEPlot::copyVariable ()
{
   QString text;
   for (int i = 0; i < QEPLOT_NUM_VARIABLES; i++) {
      QString pv = this->getSubstitutedVariableName (i);
      if (!pv.isEmpty ()) {
         if (!text.isEmpty ()) text.append (" ");
         text.append (pv);
      }
   }

   return text;
}

//------------------------------------------------------------------------------
//
QVariant QEPlot::copyData ()
{
   QString text;
   for (int i = 0; i < QEPLOT_NUM_VARIABLES; i++) {
      Trace* tr = this->traces[i];
      // Use i + 1 (as opposed to just i) as variable propety names are 1 to 4, not 0 to 3.
      QString tl = tr->legend.isEmpty () ? QString ("Variable %1").arg (i + 1) : tr->legend;
      text.append (QString ("\n%1\nx\ty\n").arg (tl));
      for (int j = 0; j < tr->xdata.count (); j++) {
         text.append (QString ("%1\t%2\n").arg (tr->xdata[j]).arg (tr->ydata[j]));
      }
   }

   return QVariant (text);
}

//------------------------------------------------------------------------------
//
void QEPlot::paste (QVariant v)
{
   QStringList PVs;

   // v.toString is a bit limiting when v is a StringList or a List of String, so
   // use common variantToStringList function which handles these options.
   //
   PVs = QEUtilities::variantToStringList (v);

   for (int i = 0; (i < PVs.size ()) && (i < QEPLOT_NUM_VARIABLES); i++) {
      this->setVariableName (PVs[i], i);
      this->establishConnection (i);
   }
}

//==============================================================================
// Property functions
//
// Access functions for YMin
void QEPlot::setYMin (double yMinIn)
{
   yMin = yMinIn;
   if (!this->autoScale) {
      this->plot->setAxisScale (QwtPlot::yLeft, this->yMin, this->yMax);
   }
}

double QEPlot::getYMin () const
{
   return this->yMin;
}

//------------------------------------------------------------------------------
// Access functions for yMax
void QEPlot::setYMax (double yMaxIn)
{
   this->yMax = yMaxIn;
   if (!this->autoScale) {
      plot->setAxisScale (QwtPlot::yLeft, this->yMin, this->yMax);
   }
}

double QEPlot::getYMax () const
{
   return this->yMax;
}

//------------------------------------------------------------------------------
// Access functions for autoScale
void QEPlot::setAutoScale (bool autoScaleIn)
{
   this->autoScale = autoScaleIn;

   // Set auto scale if requested, or if manual scale values are invalid
   if (this->autoScale || this->yMin == this->yMax) {
      this->plot->setAxisAutoScale (QwtPlot::yLeft);
   } else {
      this->plot->setAxisScale (QwtPlot::yLeft, yMin, yMax);
   }
}

bool QEPlot::getAutoScale () const
{
   return this->autoScale;
}

//------------------------------------------------------------------------------
// Access functions for X axis visibility
void QEPlot::setAxisEnableX (bool axisEnableXIn)
{
   this->axisEnableX = axisEnableXIn;
   this->plot->enableAxis (QwtPlot::xBottom, this->axisEnableX);
}

bool QEPlot::getAxisEnableX () const
{
   return this->axisEnableX;
}

//------------------------------------------------------------------------------
// Access functions for Y axis visibility
void QEPlot::setAxisEnableY (bool axisEnableYIn)
{
   this->axisEnableY = axisEnableYIn;
   this->plot->enableAxis (QwtPlot::yLeft, this->axisEnableY);
}

bool QEPlot::getAxisEnableY () const
{
   return this->axisEnableY;
}

//------------------------------------------------------------------------------
// Access functions for grid enable
void QEPlot::setGridEnableMajorX (bool gridEnableMajorXIn)
{
   this->gridEnableMajorX = gridEnableMajorXIn;
   this->setGridEnable ();
}

void QEPlot::setGridEnableMajorY (bool gridEnableMajorYIn)
{
   this->gridEnableMajorY = gridEnableMajorYIn;
   this->setGridEnable ();
}

void QEPlot::setGridEnableMinorX (bool gridEnableMinorXIn)
{
   this->gridEnableMinorX = gridEnableMinorXIn;
   this->setGridEnable ();
}

void QEPlot::setGridEnableMinorY (bool gridEnableMinorYIn)
{
   this->gridEnableMinorY = gridEnableMinorYIn;
   this->setGridEnable ();
}

//------------------------------------------------------------------------------
void QEPlot::setGridEnable ()
{
   // If any grid is required, create a grid and set it up
   // Note, Qwt will ignore minor enable if major is not enabled
   if (this->gridEnableMajorX || this->gridEnableMajorY ||
       this->gridEnableMinorX || this->gridEnableMinorY) {
      if (!this->grid) {
         this->grid = new QwtPlotGrid;
#if QWT_VERSION >= 0x060100
         this->grid->setMajorPen (QPen (gridMajorColor, 0, Qt::DotLine));
         this->grid->setMinorPen (QPen (gridMinorColor, 0, Qt::DotLine));
#else
         this->grid->setMajPen (QPen (gridMajorColor, 0, Qt::DotLine));
         this->grid->setMinPen (QPen (gridMinorColor, 0, Qt::DotLine));
#endif
         this->grid->attach (plot);
      }
      this->grid->enableX (gridEnableMajorX);
      this->grid->enableY (gridEnableMajorY);
      this->grid->enableXMin (gridEnableMinorX);
      this->grid->enableYMin (gridEnableMinorY);
   }
   // No grid required, get rid of any grid
   else {
      if (this->grid) {
         this->grid->detach ();
         delete this->grid;
         this->grid = NULL;
      }
   }
}

bool QEPlot::getGridEnableMajorX () const
{
   return this->gridEnableMajorX;
}

bool QEPlot::getGridEnableMajorY () const
{
   return this->gridEnableMajorY;
}

bool QEPlot::getGridEnableMinorX () const
{
   return this->gridEnableMinorX;
}

bool QEPlot::getGridEnableMinorY () const
{
   return this->gridEnableMinorY;
}


//------------------------------------------------------------------------------
// Access functions for grid major colour
void QEPlot::setGridMajorColor (QColor gridMajorColorIn)
{
   this->gridMajorColor = gridMajorColorIn;
   if (this->grid) {
#if QWT_VERSION >= 0x060100
      this->grid->setMajorPen (QPen (this->gridMajorColor, 0, Qt::DotLine));
#else
      this->grid->setMajPen   (QPen (this->gridMajorColor, 0, Qt::DotLine));
#endif
   }
}

QColor QEPlot::getGridMajorColor () const
{
   return this->gridMajorColor;
}

//------------------------------------------------------------------------------
// Access functions for grid minor colour
void QEPlot::setGridMinorColor (QColor gridMinorColorIn)
{
   this->gridMinorColor = gridMinorColorIn;
   if (grid) {
#if QWT_VERSION >= 0x060100
      this->grid->setMinorPen (QPen (this->gridMinorColor, 0, Qt::DotLine));
#else
      this->grid->setMinPen   (QPen (this->gridMinorColor, 0, Qt::DotLine));
#endif
   }
}

QColor QEPlot::getGridMinorColor () const
{
   return this->gridMinorColor;
}


//------------------------------------------------------------------------------
// Access functions for title
void QEPlot::setTitle (const QString & title)
{
   this->plot->setTitle (title);
}

QString QEPlot::getTitle () const
{
   return this->plot->title ().text ();
}

//------------------------------------------------------------------------------
// Access functions for backgroundColor
void QEPlot::setBackgroundColor (const QColor backgroundColorIn)
{
   // cache in widget for proper behaviour.
   //
   this->backgroundColor = backgroundColorIn;

#if QWT_VERSION >= 0x060000
   QBrush brush = plot->canvasBackground ();
   brush.setColor (this->backgroundColor);
   plot->setCanvasBackground (brush);
#else
   plot->setCanvasBackground (this->backgroundColor);
#endif
}

QColor QEPlot::getBackgroundColor () const
{
   return this->backgroundColor;
}

//------------------------------------------------------------------------------
// Access functions for traceStyle
void QEPlot::setTraceStyle (const TraceStyles traceStyle,
                            const unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex, );

   Trace* tr = this->traces[variableIndex];
   tr->style = convertTrace2Curve (traceStyle);
   if (tr->curve) {
      tr->curve->setStyle (tr->style);
   }
}

//------------------------------------------------------------------------------
//
QEPlot::TraceStyles QEPlot::getTraceStyle (const unsigned int variableIndex) const
{
   PV_INDEX_CHECK (variableIndex, Lines);
   return convertCurve2Trace (this->traces[variableIndex]->style);
}

//------------------------------------------------------------------------------
// Access functions for traceColor
void QEPlot::setTraceColor (const QColor traceColor,
                            const unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex, );
   this->traces[variableIndex]->color = traceColor;
   this->setCurveColor (traceColor, variableIndex);
}

//------------------------------------------------------------------------------
//
QColor QEPlot::getTraceColor (const unsigned int variableIndex) const
{
   PV_INDEX_CHECK (variableIndex, Qt::black);
   return this->traces[variableIndex]->color;
}


//------------------------------------------------------------------------------
// Access functions for traceLegend
void QEPlot::setTraceLegend (const QString & traceLegend,
                             const unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex, );

   Trace* tr = this->traces[variableIndex];

   tr->legend = traceLegend;
   if (traceLegend.count ()) {
      this->plot->insertLegend (new QwtLegend (), QwtPlot::RightLegend);
   } else {
      this->plot->insertLegend (NULL, QwtPlot::RightLegend);
   }

   if (tr->curve) {
      tr->curve->setTitle (traceLegend);
   }
}

//------------------------------------------------------------------------------
//
QString QEPlot::getTraceLegend (const unsigned int variableIndex) const
{
   PV_INDEX_CHECK (variableIndex, "");
   return this->traces[variableIndex]->legend;
}

//------------------------------------------------------------------------------
// Access functions for xUnit
void QEPlot::setXUnit (const QString & xUnit)
{
   this->plot->setAxisTitle (QwtPlot::xBottom, xUnit);
}

QString QEPlot::getXUnit () const
{
   return this->plot->axisTitle (QwtPlot::xBottom).text ();
}

//------------------------------------------------------------------------------
// Access functions for yUnit
void QEPlot::setYUnit (const QString & yUnit)
{
   this->plot->setAxisTitle (QwtPlot::yLeft, yUnit);
}

QString QEPlot::getYUnit () const
{
   return this->plot->axisTitle (QwtPlot::yLeft).text ();
}

//------------------------------------------------------------------------------
// Access functions for xStart
void QEPlot::setXStart (double xStartIn)
{
   this->xStart = xStartIn;
}

double QEPlot::getXStart () const
{
   return this->xStart;
}

//------------------------------------------------------------------------------
// Access functions for xIncrement
void QEPlot::setXIncrement (double xIncrementIn)
{
   this->xIncrement = xIncrementIn;
}

double QEPlot::getXIncrement () const
{
   return this->xIncrement;
}

//------------------------------------------------------------------------------
// Access functions for timeSpan
void QEPlot::setTimeSpan (int timeSpanIn)
{
   this->timeSpan = MAX (1, timeSpanIn);
}

int QEPlot::getTimeSpan () const
{
   return this->timeSpan;
}

//------------------------------------------------------------------------------
// Access functions for tickRate
void QEPlot::setTickRate (int tickRateIn)
{
   this->tickRate = MAX (20, tickRateIn);     // Limit to >= 20, i.e. <= 50 Hz.
   if (this->tickTimer) {
      this->tickTimer->stop ();
      this->tickTimer->start (tickRate);
   }
}

int QEPlot::getTickRate () const
{
   return this->tickRate;
}

//------------------------------------------------------------------------------
// Access functions for margin
void QEPlot::setMargin (const int marginIn)
{
   this->layoutMargin = LIMIT (marginIn, 0, 100);
   this->layout->setMargin (layoutMargin);
}

int QEPlot::getMargin () const
{
   return this->layoutMargin;
}

//------------------------------------------------------------------------------
// There are foure PVs and assocates fours sets of trace properties, which
// are all essentially identical, save for the function name and trace index.
// names go 1 to 4, indicies go 0 to 3
//
#define ACCESS_FUNCTIONS(name, index)                                       \
void QEPlot::setVariableName##name##Property (const QString & pvName)       \
{                                                                           \
   this->setVariableNameIndexProperty (pvName, index);                      \
}                                                                           \
                                                                            \
QString QEPlot::getVariableName##name##Property () const                    \
{                                                                           \
   return this->getVariableNameIndexProperty (index);                       \
}                                                                           \
                                                                            \
void QEPlot::setTraceStyle##name (const TraceStyles traceStyle)             \
{                                                                           \
   this->setTraceStyle (traceStyle, index);                                 \
}                                                                           \
                                                                            \
QEPlot::TraceStyles QEPlot::getTraceStyle##name () const                    \
{                                                                           \
   return this->getTraceStyle (index);                                      \
}                                                                           \
                                                                            \
void QEPlot::setTraceColor##name (const QColor traceColor)                  \
{                                                                           \
   this->setTraceColor (traceColor, index);                                 \
}                                                                           \
                                                                            \
QColor QEPlot::getTraceColor##name () const                                 \
{                                                                           \
   return this->getTraceColor (index);                                      \
}                                                                           \
                                                                            \
void QEPlot::setTraceLegend##name (const QString& traceLegend)              \
{                                                                           \
   this->setTraceLegend (traceLegend, index);                               \
}                                                                           \
                                                                            \
QString QEPlot::getTraceLegend##name () const                               \
{                                                                           \
   return this->getTraceLegend (index);                                     \
}


ACCESS_FUNCTIONS (1, 0)
ACCESS_FUNCTIONS (2, 1)
ACCESS_FUNCTIONS (3, 2)
ACCESS_FUNCTIONS (4, 3)

#undef ACCESS_FUNCTIONS

// end
