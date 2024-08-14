/*  QEPlot.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2023 Australian Synchrotron
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

/*
  This class is a CA aware Plot widget and is based in part on the work of the
  Qwt project (http://qwt.sf.net).  It is tighly integrated with the base class
  QEWidget. Refer to QEWidget.cpp for details.
 */

#include "QEPlot.h"

#include <QDebug>
#include <QFontMetrics>
#include <QPainter>
#include <QPen>
#include <QBrush>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#include <alarm.h>

#include <QECommon.h>
#include <QEPlatform.h>
#include <QEScaling.h>
#include <QEGraphicNames.h>
#include <QEGraphic.h>
#include <QEArchiveAccess.h>
#include <QEDisplayRanges.h>

#define DEBUG qDebug () << "QEPlot" <<  __LINE__ << __FUNCTION__  << "  "


//-----------------------------------------------------------------------------
// Macro fuction to ensure varable index is in the expected range.
// Set defval to nil for void functions.
//
// General variable index  0 .. 15
// Data variable index     0 .. 7
// Size variable index     8 .. 15
//
#define PV_INDEX_CHECK(vi, defval)   {                                 \
   if ((vi) >= QEPLOT_NUM_VARIABLES) {                                 \
      DEBUG << "unexpected variableIndex" << (vi);                     \
      return defval;                                                   \
   }                                                                   \
}


#define DATA_INDEX_CHECK(vi, defval)   {                               \
   if ((vi) >= QEPLOT_NUM_PLOTS) {                                     \
      DEBUG << "unexpected data variableIndex" << (vi);                \
      return defval;                                                   \
   }                                                                   \
}


#define SIZE_INDEX_CHECK(vi, defval)   {                               \
   if (((vi) < QEPLOT_NUM_PLOTS) || ((vi) >= QEPLOT_NUM_VARIABLES)) {  \
      DEBUG << "unexpected size variableIndex" << (vi);                \
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
class QEPlot::Trace : public QObject {
public:
   explicit Trace (const int instanceIn, QEPlot* parent);
   ~Trace ();

   void reset ();
   void requestArchiveData ();
   void setArchiveData (const QCaDataPointList& archiveData);

   const QEPlot* owner;
   const int instance;
   int width;
   bool isVisible;

   QEArchiveAccess archiveAccess;  // Allow access to archivers

   // Holds plot data
   //
   QCaDataPointList scalarData;

   // Holds waveform data
   QVector <double> ydata;

   QColor color;
   QwtPlotCurve::CurveStyle style;
   QString legend;

   bool isInUse;                // Essential indicates if PV name is set or not.
   bool isConnected;
   bool isFirstUpdate;

   // True if displaying a waveform (an array of values arriving in one update),
   // false if displaying a strip chart (individual values arriving over time).
   // Used to ensure only one plot mechanism is used.
   //
   bool isWaveform;             // otherwise is scalar
   bool dataSizeDefined;
   int  dataSize;

   QCaVariableNamePropertyManager dnpm;   // data name
   QCaVariableNamePropertyManager snpm;   // size name
};


//------------------------------------------------------------------------------
//
QEPlot::Trace::Trace (const int instanceIn, QEPlot* parent) :
   QObject (parent),
   owner (parent),
   instance (instanceIn)
{
   static const QColor defaultColors [] = {
      Qt::black, Qt::red, Qt::green, Qt::blue,
      Qt::cyan, Qt::magenta, Qt::yellow, Qt::gray
   };

   this->reset ();   // resets dynamic infomation
   this->dataSizeDefined = false;
   this->dataSize = 0;

   if (this->instance >= 0 && this->instance < ARRAY_LENGTH (defaultColors)) {
      this->color = defaultColors [this->instance];
   } else {
      this->color = Qt::black;
   }
   this->style = QwtPlotCurve::Lines;
   this->width = 1;
   this->isVisible = true;

   // Set up connection to archive access.
   // Note: we go via the owner which has class defined in the header file,
   // and therefore is processed by moc allowing slot setup.
   //
   QObject::connect
         (&this->archiveAccess,
          SIGNAL (setArchiveData (const QObject*, const bool, const QCaDataPointList&, const QString&, const QString&)),
          this->owner,
          SLOT   (setArchiveData (const QObject*, const bool, const QCaDataPointList&, const QString&, const QString&)));
}

//------------------------------------------------------------------------------
//
QEPlot::Trace::~Trace ()
{
   this->scalarData.clear();
}

//------------------------------------------------------------------------------
//
void QEPlot::Trace::reset ()
{
   this->isInUse = false;
   this->isConnected = false;
   this->isFirstUpdate = false;
   this->isWaveform = false;
   this->scalarData.clear();
   this->ydata.clear();
}

//------------------------------------------------------------------------------
//
void QEPlot::Trace::requestArchiveData ()
{
   if (!this->isInUse || this->isWaveform) return;  // sanity check

   static const qint64 rawLimit = 600;  // seconds

   // Calculate the time span - add 10% wiggle room.
   //
   const qint64 span = (this->owner->timeSpan * 11) / 10;
   const QDateTime archiveEndDateTime = QDateTime::currentDateTime().toUTC();
   const QDateTime archiveStartDateTime = archiveEndDateTime.addSecs (-span);
   const QEArchiveInterface::How how =
         (span >= rawLimit) ? QEArchiveInterface::Linear : QEArchiveInterface::Raw;

   QString pvName = this->owner->getSubstitutedVariableName (this->instance);
   this->archiveAccess.readArchive
         (this, pvName, archiveStartDateTime, archiveEndDateTime, 1000, how, 0);
}

//------------------------------------------------------------------------------
//
void QEPlot::Trace::setArchiveData (const QCaDataPointList& archiveData)
{
   if (!this->isInUse || this->isWaveform) return;  // sanity check

   // Merge archiveData and (current) scalarData into new scalarData
   //
   QCaDataPointList mergedData = archiveData;

   if (this->scalarData.count() >= 1) {
      // We have at least one one of live/real time data.
      // There may be overlap between live/real time data and histrotical
      // archive data, so purge duplicate arhioive data
      //
      const QCaDateTime firstLiveTime = this->scalarData.value(0).datetime;
      const int posn = mergedData.indexBeforeTime (firstLiveTime, mergedData.count());
      mergedData.truncate (posn);
   }

   // Do actual merge and re-assign.
   //
   mergedData.append (this->scalarData);
   this->scalarData = mergedData;
}

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
QEPlot::QEPlot (const QString& variableNameIn, QWidget* parent) : QEFrame (parent)
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
   // First allocate internal widgets and other objects.
   //
   // Allocate plotArea, which does the actual plotting, and layout.
   //
   this->plotArea = new QEGraphic (this);
   this->legendArea = new QWidget (this);
   this->legendArea->setFixedWidth (2);    // effectivetly invisible.
   this->layout = new QHBoxLayout (this);
   this->layoutMargin = 0;
   this->layout->setContentsMargins (this->layoutMargin, this->layoutMargin,
                                     this->layoutMargin, this->layoutMargin);
   this->layout->setSpacing (0);
   this->layout->addWidget (this->plotArea);
   this->layout->addWidget (this->legendArea);

   // Allocate Trace objects
   //
   for (int i = 0; i < QEPLOT_NUM_PLOTS; i++) {
      this->traces[i] = new Trace (i, this);
   }

   // Set default inherited property values.
   //
   // Set plain shape and noframe.
   //
   this->setFrameShape (QFrame::NoFrame);
   this->setFrameShadow (QFrame::Plain);
   this->setVariableAsToolTip (true);
   this->setDisplayAlarmStateOption (QE::Always);
   this->setAllowDrop (false);

   // General plot properties
   //
   this->yMin = 0.0;
   this->yMax = 1.0;
   this->yAxisAutoScale = true;
   this->archiveBackfill = false;
   this->axisEnableX = true;
   this->axisEnableY = true;
   this->selectedYAxis = Left;
   this->useFullLengthArraySubscriptions = false;  // go with modern behaviour by default.

   this->mMouseMoveSignals = QE::signalData;

   // Default to one minute span
   //
   this->tickRate = 50;   // millSec
   this->timeSpan = 60;   // seconds

   this->setPlotAreaYRange (0.0, 1000.0, true);
   this->plotArea->setXRange (0.0, 1000.0, QEGraphicNames::SelectByValue, 5, false);

   // Tracking on by default - connect mouse move signal.
   //
   QObject::connect (this->plotArea, SIGNAL (mouseMove     (const QPointF&)),
                     this,           SLOT   (plotMouseMove (const QPointF&)));

   this->replotIsRequired = true;       // ensure we re plot on the first tick.
   this->tickTimerCount = 0;

   this->tickTimer = new QTimer (this);
   QObject::connect (this->tickTimer, SIGNAL (timeout     ()),
                     this,            SLOT   (tickTimeout ()));
   this->tickTimer->start (20);

   // Waveform properties
   //
   this->xStart = 0.0;
   this->xIncrement = 1.0;

   // Nominal window size should encompass most sensible PVs
   //
   this->xFirst = -1000000.0;
   this->xLast  = +1000000.0;

   this->gridEnableMajorX = false;
   this->gridEnableMajorY = false;
   this->gridEnableMinorX = false;
   this->gridEnableMinorY = false;
   this->gridMajorColor = Qt::black;
   this->gridMinorColor = Qt::gray;
   this->setBackgroundColor (QColor (220, 220, 220));
   this->updateGridSettings ();

   this->plotArea->installCanvasEventFilter (this);
   this->legendArea->installEventFilter (this);

   // Set up QEWidget data
   // This control used a single data source
   //
   this->setNumVariables (QEPLOT_NUM_VARIABLES);

   // Use standard context menu
   //
   this->setupContextMenu ();

   // For each variable name property manager, set up an index to identify it when
   // it signals and set up a connection to recieve variable name property changes.
   // The variable name property manager class only delivers an updated variable
   // name after the user has stopped typing.
   //
   for (int i = 0; i < QEPLOT_NUM_PLOTS; i++) {
      QCaVariableNamePropertyManager* vnpm;

      vnpm = &this->traces[i]->dnpm;
      vnpm->setVariableIndex (i);
      QObject::connect (vnpm, SIGNAL (newVariableNameProperty    (QString, QString, unsigned int)),
                        this, SLOT   (useNewVariableNameProperty (QString, QString, unsigned int)));

      vnpm = &this->traces[i]->snpm;
      vnpm->setVariableIndex (i + QEPLOT_NUM_PLOTS);
      QObject::connect (vnpm, SIGNAL (newVariableNameProperty    (QString, QString, unsigned int)),
                        this, SLOT   (useNewVariableNameProperty (QString, QString, unsigned int)));
   }
}

//------------------------------------------------------------------------------
//
QEPlot::~QEPlot ()
{
   if (this->tickTimer) {
      this->tickTimer->stop ();
   }

   // Trace objects are QObjects and auto deleted.
}

//------------------------------------------------------------------------------
// Provides size hint in designer - it is not a constraint
//
QSize QEPlot::sizeHint () const
{
   return QSize (240, 100);
}

//------------------------------------------------------------------------------
//
QEGraphic* QEPlot::getGraphic () const
{
   return this->plotArea;
}

//------------------------------------------------------------------------------
// Note: posn is from QEGraphic and is the real world co-ordinates
// as opposed to the pixel co-ordinates.
//
void QEPlot::plotMouseMove (const QPointF& posn)
{
   QString message;

   if (this->mMouseMoveSignals & (QE::signalStatus|QE::signalText)) {
      // Prepare the text
      //
      QString f;   // temp

      // Do we use the x- co-ordonate of the time stamp.
      // For mini strip chart, use time stamp.
      // For waveforms, use x co-ordinate value.
      // For mixed mode (unexpected), use the time stamp.
      //
      bool allDataIsWaveform = true;
      for (int i = 0; i < QEPLOT_NUM_PLOTS; i++) {
         Trace* tr = this->traces[i];
         if (tr->isInUse && !tr->isWaveform) {
             allDataIsWaveform  = false;
             break;
         }
      }

      if (allDataIsWaveform) {
         f = QString::asprintf ("X: %+.10g", posn.x ());
         message.append (f);
         f = QString::asprintf ("    Y: %+.10g", posn.y ());
         message.append (f);
      } else {
         // Note: Cribbed from the StripChart - keep alligned.
         //
         // Convert cursor x to absolute cursor time.
         // x is the time (in seconds) relative to end of the plot.
         //
         static const QString format = "ddd yyyy-MM-dd hh:mm:ss.zzz";
         const QCaDateTime now = QDateTime::currentDateTime ().toLocalTime();
         const qint64 mSec = qint64 (1000.0 * posn.x ());  // convert to mSec
         const QDateTime t = now.addMSecs (mSec);

         message = "Time: ";

         // Keep only most significant digit of the milli-seconds,
         // i.e. tenths of a second.
         //
         f = t.toString (format).left (format.length() - 2);
         message.append (f);

         QString zoneTLA = QEUtilities::getTimeZoneTLA (Qt::LocalTime, t);
         message.append (" ").append (zoneTLA);

         // Show y value associated with current cursor position.
         //
         f = QString::asprintf ("    Value: %+.10g", posn.y ());
         message.append (f);
      }

      // Do we send a status message?
      // Appears on the status bar of containing form.
      //
      if (this->mMouseMoveSignals & QE::signalStatus) {
         this->setReadOut (message);
      }

      // Do we emit time/value as string signal ?
      // Can go to QLabel or any other widget that accepts a string.
      //
      if (this->mMouseMoveSignals & QE::signalText) {
         emit mouseMove (message);
      }
   }

   // Do we send a data signal message ?
   //
   if (this->mMouseMoveSignals & QE::signalData) {
      emit mouseMove (posn);
   }
}

//------------------------------------------------------------------------------
//
bool QEPlot::eventFilter (QObject* watched, QEvent* event)
{
   const QEvent::Type type = event->type ();
   QMouseEvent* mouseEvent = NULL;

   bool result = false;

   switch (type) {

      case QEvent::Paint:
         if (watched == this->legendArea) {
            this->drawLegend ();
            result = true;  // event handled.
         }
         break;

      case QEvent::MouseButtonPress:
         if (this->plotArea->isCanvasObject (watched)) {
            mouseEvent = static_cast<QMouseEvent *> (event);
            if (mouseEvent->buttons() & (Qt::LeftButton | Qt::MiddleButton)) {
               // The left or middle button has been pressed.
               // Initiate dragging or middle click.
               this->qcaMousePressEvent (mouseEvent);
               result = true;  // event handled.
            }
         }
         break;

      default:
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
QMenu* QEPlot::buildContextMenu ()
{
   QMenu* menu = ParentWidgetClass::buildContextMenu ();
   QAction* action;

   menu->addSeparator ();

   action = new QAction ("Archive backfill", menu);
   action->setCheckable (false);
   action->setData (PLOTCM_ARCHIVE_BACKFILL);
   menu->addAction (action);

   return menu;
}

//------------------------------------------------------------------------------
//
void QEPlot::contextMenuTriggered (int selectedItemNum)
{
   switch (selectedItemNum) {

      case PLOTCM_ARCHIVE_BACKFILL:
         for (int j = 0; j < ARRAY_LENGTH(this->traces); j++) {
            Trace* tr = this->traces[j];
            if (tr && tr->isInUse) {
               tr->requestArchiveData();
            }
         }
         break;

      default:
         // Call parent class function.
         //
         ParentWidgetClass::contextMenuTriggered (selectedItemNum);
         break;
   }
}


//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. For a strip chart a QCaObject that streams floating point
// data is required.
//
qcaobject::QCaObject* QEPlot::createQcaItem (unsigned int vi)
{
   PV_INDEX_CHECK (vi, NULL);

   const QString pvName = this->getSubstitutedVariableName (vi);

   qcaobject::QCaObject* result;

   if (vi < QEPLOT_NUM_PLOTS) {
      // Create the item as a QEFloating
      //
      result = new QEFloating (pvName, this, &this->floatingFormatting, vi);

      if (result && !this->useFullLengthArraySubscriptions) {
         // Only read effective number, e.g. as defied by .NORD for a waveform record.
         //
         result->setRequestedElementCount (0);
      }

   } else {
      // Must be the size variable
      //
      result = new QEInteger (pvName, this, &this->integerFormatting, vi);
   }
   return result;
}

//------------------------------------------------------------------------------
// Start updating.  Implementation of VariableNameManager's virtual funtion to
// establish a connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QEPlot::establishConnection (unsigned int vi)
{
   PV_INDEX_CHECK (vi,);

   // Create a connection. If successfull, the QCaObject object that will
   // supply data update signals will be returned.
   //
   qcaobject::QCaObject* qca = this->createConnection (vi);
   if (!qca) return;         // sanity check.

   if (vi < QEPLOT_NUM_PLOTS) {
      // Find the curve/trace information for this variable.
      //
      Trace* tr = this->traces[vi];
      if (!tr) return;           // sanity check.
      tr->reset();               // clears any old data when re-activated.
      tr->isInUse = true;        // Controlled soley by the data variable.
   }

   // If a QCaObject object is now available to supply data update signals,
   // connect it to the appropriate slots
   //
   if (vi < QEPLOT_NUM_PLOTS) {
      // Must be the data variable - capture both scalars and arrays
      //
      QObject::connect (qca, SIGNAL (floatingArrayChanged (const QVector <double>&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                        this,  SLOT (setPlotData          (const QVector <double>&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));
      QObject::connect (qca, SIGNAL (floatingChanged      (const double, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                        this,  SLOT (setPlotData          (const double, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));
   } else {
      // Must be the size variable.
      //
      QObject::connect (qca, SIGNAL (integerChanged       (const long, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                        this,  SLOT (setSizeData          (const long, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));
   }

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
   PV_INDEX_CHECK (variableIndex,);

   // Select the curve/trace information for this variable
   //
   Trace* tr = this->traces[variableIndex % QEPLOT_NUM_PLOTS];
   if (!tr) return;             // sanity check.

   // Note the connected state
   //
   bool isConnected = connectionInfo.isChannelConnected ();

   // Display the connected state.
   //
   this->updateToolTipConnection (isConnected, variableIndex);

   if (variableIndex >= QEPLOT_NUM_PLOTS) {
      // Must be the size PV
      // Either way (a connect or a disconnet), reset the data size
      //
      tr->dataSizeDefined = false;
      this->replotIsRequired = true;
      return;
   }

   tr->isConnected = isConnected;
   tr->isFirstUpdate = true; // no need to check if connect/disconnect

   if (!tr->isWaveform && !tr->isConnected && (tr->scalarData.count () >= 1)) {
      // We have a channel disconnect.
      //
      // create a dummy point with last value and time now.
      //
      QCaDataPoint point = tr->scalarData.last ();
      point.datetime = QDateTime::currentDateTime ().toUTC ();
      tr->scalarData.append (point);

      // create a dummy point with same time but marked invalid to indicate a break.
      //
      point.alarm = QCaAlarmInfo (NO_ALARM, INVALID_ALARM);
      tr->scalarData.append (point);
   }

   // This updates the style.  We want to be disabled/greyed-out only
   // if all in use varaibles are not conncted.
   //
   isConnected = false;
   bool isNone = true;
   for (int i = 0; i < QEPLOT_NUM_PLOTS; i++) {
      Trace* tr = this->traces[i];
      if (tr && tr->isInUse) {
         isNone = false;
         if (tr->isConnected) {
            isConnected = true;
         }
      }
   }

   this->replotIsRequired = true;

   this->processConnectionInfo (isNone || isConnected, variableIndex);
}

//------------------------------------------------------------------------------
// Update the plotted data with a new single value
// This is a slot used to recieve data updates from a QCaObject based class.
//
void QEPlot::setPlotData (const double value, QCaAlarmInfo& alarmInfo,
                          QCaDateTime& timestamp, const unsigned int& variableIndex)
{
   DATA_INDEX_CHECK (variableIndex,);

   // Select the curve/trace information for this variable
   //
   Trace* tr = this->traces[variableIndex];
   if (!tr) return;      // sanity check.


   // A seperate data connection (QEPlot::setPlotData( const QVector<double>& values, ... )
   // manages array data (it also determines if we are getting array data), so do
   // nothing more here if plotting array data data.
   //
   if (tr->isWaveform) {
      return;
   }

   // Do any specials for first update
   //
   if (tr->isFirstUpdate) {
      tr->isFirstUpdate = false;   // clear
      // Is auto-backfill enabled?
      if (this->archiveBackfill) {
         // Yes - issue archive data request.
         //
         tr->requestArchiveData();
      }
   }

   // Just save the point - add to the current data set.
   //
   QCaDataPoint point;

   point.value = value;
   point.alarm = alarmInfo;

   // If the date is more than a wisker into the future, limit it.
   // This will happen if the source is on another machine with an incorrect time.
   // Allow a little bit of time (100mS) as machines will not be synchronised perfectly.
   // This will help if updates get bunched.
   // If this is not done and we are adding a last point at the current time,
   // this last point will be before this actual data point.
   //
   QCaDateTime ct = QCaDateTime::currentDateTime ().toUTC();
   double tsDiff = ct.secondsTo (timestamp);
   if (tsDiff > 0.1) {
      timestamp = ct.addMSecs (100);
   }

   // Else, If the date is a long way in the past, limit to a small amount.
   // This will happen if the source is on another machine with an incorrect time.
   // Allow a bit of time (500mS) as machines will not be synchronised perfectly
   // and for network latency hichups. If this is not done and we are adding a
   // last point at the current time, there will always be a flat bit of line at
   // the end of the plot.
   //
   else if (tsDiff < -0.5) {
      timestamp = ct.addMSecs (-500);
   }

   // Finalise point and append to scalar data set.
   //
   point.datetime = timestamp;
   tr->scalarData.append (point);

   // The data is now ready to plot
   //
   this->setAlarmInfoCommon (alarmInfo, variableIndex);

   // Signal a database value change to any Link widgets.
   //
   emit dbValueChanged (value);
}

//------------------------------------------------------------------------------
// Update the plotted data with a new array of values
// This is a slot used to recieve data updates from a QCaObject based class.
//
void QEPlot::setPlotData (const QVector <double>& values,
                          QCaAlarmInfo& alarmInfo, QCaDateTime&,
                          const unsigned int& variableIndex)
{
   DATA_INDEX_CHECK (variableIndex,);

   // Select the curve/trace information for this variable
   //
   Trace* tr = this->traces[variableIndex];

   if (!tr) return;     // sanity check.

   // A separate data connection (QEPlot::setPlotData (const double value, ...)
   // manages scalar data, so decide if we are plotting scalar or array data and
   // do nothing more here if plotting scalar data.
   //
   tr->isWaveform = (values.count () > 1);
   if (!tr->isWaveform) {
      return;
   }

   // Do any specials for first update
   //
   if (tr->isFirstUpdate) {
      // No special action required.
      tr->isFirstUpdate = false;
   }

   // Clear any previous data
   //
   tr->scalarData.clear();
   tr->ydata.clear ();

   for (int i = 0; i < values.count (); i++) {
      tr->ydata.append (values[i]);
   }

   // The data is now ready to plot.
   //
   this->replotIsRequired = true;

   this->setAlarmInfoCommon (alarmInfo, variableIndex);

   // Signal a database value change to any Link or similar widgets
   //
   emit dbValueChanged (values);
}

//------------------------------------------------------------------------------
//
void QEPlot::setSizeData (const long value, QCaAlarmInfo& alarmInfo,
                          QCaDateTime&, const unsigned int& vi)
{
   SIZE_INDEX_CHECK (vi,);

   // Select the curve/trace information for this variable
   //
   Trace* tr = this->traces[vi % QEPLOT_NUM_PLOTS];
   if (!tr) return;     // sanity check.

   bool valueDefined = !alarmInfo.isInvalid();

   // Has it effectively changed state?
   //
   if ((tr->dataSizeDefined != valueDefined) ||
       (valueDefined && (tr->dataSize != value))) {
      tr->dataSize = value;
      tr->dataSizeDefined = valueDefined;
      this->replotIsRequired = true;
   }

   // Update the tool tip for this PV, but not general widget alarm state.
   //
   this->updateToolTipAlarm (alarmInfo, vi);
}

//------------------------------------------------------------------------------
//
void QEPlot::setAlarmInfoCommon (QCaAlarmInfo& alarmInfo,
                                 const unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex,);

   // Invoke common alarm handling processing.
   // TODO: Aggregate all channel severities into a single alarm state.
   //
   this->processAlarmInfo (alarmInfo, variableIndex);
}

//------------------------------------------------------------------------------
//
void QEPlot::purgeOldData () {
   // Remove any old data
   // Find chart start time.
   //
   QDateTime startTime = QDateTime::currentDateTime ().toUTC();
   startTime = startTime.addSecs (-this->timeSpan);

   for (int i = 0; i < QEPLOT_NUM_PLOTS; i++) {
      Trace* tr = this->traces[i];
      while (tr->scalarData.count () >= 2) {
         // Check the time of the oldest but one.
         // We need to keep at least one prior to the start time.
         //
         QCaDateTime datetime = tr->scalarData.value(1).datetime;
         if (datetime < startTime) {
            tr->scalarData.removeFirst ();
         } else {
            break;
         }
      }
   }
}

//------------------------------------------------------------------------------
//
void QEPlot::plotData ()
{
   const QCaDateTime now = QDateTime::currentDateTimeUtc();

   // First release any/all allocated curves.
   //
   this->plotArea->releaseCurves ();

   QEDisplayRanges xRange;
   QEDisplayRanges yRange;

   // If no increment was supplied, use 1.0 by default
   //
   const double inc = this->xIncrement == 0.0 ? 1.0 : this->xIncrement;

   // Now plot each curve.
   //
   for (int i = 0; i < QEPLOT_NUM_PLOTS; i++) {
      Trace* tr = this->traces[i];
      if (!tr) continue;    // sanity check
      if (!tr->isInUse) continue;
      if (!tr->isVisible) continue;
      if (tr->width == 0) continue;

      QVector <double> xdata;
      QVector <double> ydata;
      QPen pen;

      pen.setColor (tr->color);
      pen.setWidth (tr->width);
      pen.setStyle (Qt::SolidLine);

      this->plotArea->setCurvePen (pen);
      this->plotArea->setCurveRenderHint (QwtPlotItem::RenderAntialiased, false);
      this->plotArea->setCurveStyle (tr->style);

      // Are we dealing with a waveform or a scalar.
      //
      if (tr->isWaveform) {
         // It is a wavform.
         //
         int n = tr->ydata.count();

         if (tr->dataSizeDefined) {
            // A data size has been set - truncate plotting accordingly.
            //
            n = MIN (n, tr->dataSize);
         }

         // We cannot plot less than one point.
         //
         if (n < 1) continue;

         xdata.reserve (n);
         ydata.reserve (n);

         for (int i = 0; i < n; i++) {
            double x = this->xStart + (double (i) * inc);
            double y = tr->ydata [i];

            // Only display that portion of the data that is needed.
            //
            if ((x >= this->xFirst) && (x <= this->xLast)) {

               // Can't plot NaN or Inf, and also can cause a freeze if we try.
               //
               const bool plotable = (!QEPlatform::isNaN (y) &&
                                      !QEPlatform::isInf (y));

               if (plotable) {
                  // Just append to the x/y data
                  //
                  xRange.merge (x);
                  yRange.merge (y);
                  xdata.append (x);
                  ydata.append (y);
               } else {
                  // This point is not plotable.
                  // plot what we have so far (need at least 1 point).
                  //
                  if (xdata.count () >= 1) {
                     // Plot it, and clear the data in order to start again.
                     //
                     this->plotArea->plotCurveData (xdata, ydata, QwtPlot::yLeft);
                     xdata.clear ();
                     ydata.clear ();
                  }
               }
            }
         }
         // Plot what, if anything,  we have accumulated.
         //
         if (xdata.count () >= 1) {
            this->plotArea->plotCurveData (xdata, ydata, QwtPlot::yLeft);
         }

      } else {
         // It is a scalar.
         //
         const int n = tr->scalarData.count();
         if (n < 1) continue;

         // Fixed range irrespective of the data.
         //
         xRange.merge (double(-this->timeSpan));
         xRange.merge (0.0);

         xdata.reserve (n + 1);
         ydata.reserve (n + 1);

         for (int i = 0; i < n; i++) {
            const QCaDataPoint point = tr->scalarData.value(i);

            // Can't plot NaN or Inf, and also can cause a freeze if we try.
            //
            const bool plotable = (!QEPlatform::isNaN (point.value) &&
                                   !QEPlatform::isInf (point.value));

            if (plotable && point.isDisplayable()) {
               // Just append to the x/y data
               //
               xdata.append (now.secondsTo (point.datetime));
               ydata.append (point.value);
               yRange.merge (point.value);

            } else {
               // This point is not plotable/displayable.
               // plot what we have so far (need at least 1 point).
               //
               if (xdata.count () >= 1) {
                  // The current point is unplotable (invalid/disconneted/NaN/Inf).
                  // Create  a valid stopper point consisting of prev. point
                  // value and this point's time.
                  //
                  xdata.append (now.secondsTo (point.datetime));
                  ydata.append (ydata.last ());

                  // Plot it, and clear the data in order to start again.
                  //
                  this->plotArea->plotCurveData (xdata, ydata, QwtPlot::yLeft);
                  xdata.clear ();
                  ydata.clear ();
               }
            }
         }

         // Plot what, if anything,  we have accumulated.
         //
         if (xdata.count () >= 1) {
            // Replicate last know value as a current point.
            //
            const QCaDataPoint point = tr->scalarData.last();
            xdata.append (0.0);        // relative time now.
            ydata.append (point.value);

            this->plotArea->plotCurveData (xdata, ydata, QwtPlot::yLeft);
         }
      }
   }

   if (this->yAxisAutoScale && yRange.getIsDefined()) {
      double min, max;
      yRange.getMinMax (min, max);
      this->setPlotAreaYRange (min, max, false);
   }

   if (xRange.getIsDefined()) {
      // This is a pesudo auto scale
      //
      double min, max;
      xRange.getMinMax (min, max);
      this->plotArea->setXRange (min, max, QEGraphicNames::SelectByValue, 10, false);
   }

   // Trigger an actual replot.
   //
   this->plotArea->replot ();

   // Last - clear teh replot required flag.
   //
   this->replotIsRequired = false;

#undef UPDATE_X_MIN_MAX
}

//------------------------------------------------------------------------------
// Paints the legend. We do this ourselves, as opposed to using Qwt's in built
// legend as that is curved based, not PV based. As such, no legend displayed
// until data is available, and multiple legends when disconnect-reconnect occurs
// due to separate curve used for each section.
//
void QEPlot::drawLegend ()
{
   bool legendIsRequired = false;

   // Extract the current scaling applied to this widget.
   //
   int m, d;
   QEScaling::getWidgetScaling (this, m, d);

#define SCALE(x) (int(m*x)/d)

   for (int i = 0; i < QEPLOT_NUM_PLOTS; i++) {
      Trace* tr = this->traces[i];
      if (tr && tr->legend.length() > 0) {
         // We have atleast one legend.
         //
         legendIsRequired = true;
         break;
      }
   }

   if (!legendIsRequired) {
      // Essentially do not display. Note: if we set to zero, or set non-visible,
      // this masks paint update events, and never displayed again.
      //
      this->legendArea->setFixedWidth (2);
      return;
   }

   // Ensure draw behaves well when scaling applied.
   //
   const int topOffset   = SCALE (8);  //  QEScaling::scale (8);
   const int topDelta    = SCALE (27); //  QEScaling::scale (27);
   const int leftOffset  = SCALE (4);  //  QEScaling::scale (4);
   const int boxSize     = SCALE (7);  //  QEScaling::scale (7);
   const int leftText    = SCALE (18); //  QEScaling::scale (18);
   const int rightOffset = SCALE (4);  //  QEScaling::scale (4);

   QPainter painter (this->legendArea);
   QFontMetrics fm = painter.fontMetrics ();
   int maxTextWidth = 0;

   for (int i = 0, row = 0; i < QEPLOT_NUM_PLOTS; i++) {
      const Trace* tr = this->traces[i];

      if (tr->legend.isEmpty()) continue;   // skip this one.

      const int top = topOffset + topDelta*row;

      QRect box = QRect (leftOffset, top, boxSize, boxSize);
      QPen pen;
      QBrush brush;

      pen.setStyle (Qt::SolidLine);
      pen.setColor (tr->color);
      pen.setWidth (1);
      painter.setPen (pen);

      brush.setStyle (Qt::SolidPattern);
      brush.setColor (tr->color);
      painter.setBrush (brush);

      painter.drawRect (box);

      pen.setColor (Qt::black);
      pen.setWidth (1);
      painter.setPen (pen);
      painter.drawText (leftText, top + boxSize, tr->legend);
      maxTextWidth = MAX (maxTextWidth, fm.horizontalAdvance (tr->legend));

      row++;
   }

   int requiredLegendWidth = leftText + maxTextWidth + rightOffset;

   // Allow no more than 25% of the width of the widget
   //
   requiredLegendWidth = MIN (requiredLegendWidth, this->width()/4);
   this->legendArea->setFixedWidth (requiredLegendWidth);

#undef SCALE
}

//------------------------------------------------------------------------------
// Update the chart if it is a strip chart
//
void QEPlot::tickTimeout ()
{
   bool tickIsRequired = false;

   // The base/fixed timer rate is 20mS/50Hz
   //
   this->tickTimerCount += 20;

   if (this->tickTimerCount >= this->tickRate) {
      this->tickTimerCount -= this->tickRate;
      tickIsRequired = true;
   }

   // Shuffle up date for non-waveforms.
   //
   if (tickIsRequired) {
      for (int i = 0; i < QEPLOT_NUM_PLOTS; i++) {
         Trace* tr = this->traces[i];

         if (tr->isInUse && !tr->isWaveform) {
            this->replotIsRequired = true;
         }
      }
   }

   if (this->replotIsRequired) {
      this->purgeOldData ();
      this->plotData ();        // this clears replotIsRequired
   }
}

//------------------------------------------------------------------------------
// Set variable name
//
void QEPlot::useNewVariableNameProperty (QString pvName, QString subs, unsigned int vi)
{
   PV_INDEX_CHECK (vi,);

   if (vi < QEPLOT_NUM_PLOTS) {
      // The non-size name pv name has been changed or cleared.
      //
      Trace* tr = this->traces[vi];
      tr->reset();
   }

   this->setVariableNameAndSubstitutions (pvName, subs, vi);
}

//------------------------------------------------------------------------------
// Variable name property access access
//
void QEPlot::setVariableNameIndexProperty (const QString& variableName,
                                           const unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex,);
   this->traces[variableIndex]->dnpm.setVariableNameProperty (variableName);
}

//------------------------------------------------------------------------------
//
QString QEPlot::getVariableNameIndexProperty (const unsigned int variableIndex) const
{
   PV_INDEX_CHECK (variableIndex, "");
   return this->traces[variableIndex]->dnpm.getVariableNameProperty ();
}

//------------------------------------------------------------------------------
// Size variable name property access access
//
void QEPlot::setSizeVariableNameIndexProperty (const QString& variableName,
                                               const unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex,);
   const int index = variableIndex % QEPLOT_NUM_PLOTS;
   this->traces[index]->snpm.setVariableNameProperty (variableName);
}

//------------------------------------------------------------------------------
//
QString QEPlot::getSizeVariableNameIndexProperty (const unsigned int variableIndex) const
{
   PV_INDEX_CHECK (variableIndex, "");
   const int index = variableIndex % QEPLOT_NUM_PLOTS;
   return this->traces[index]->snpm.getVariableNameProperty ();
}


//------------------------------------------------------------------------------
// Variable substitutions access
//
void QEPlot::setVariableNameSubstitutionsProperty (const QString& variableNameSubstitutions)
{
   // Same substitutions apply to all variables.
   //
   for (int i = 0; i < QEPLOT_NUM_PLOTS; i++) {
      this->traces[i]->dnpm.setSubstitutionsProperty (variableNameSubstitutions);
      this->traces[i]->snpm.setSubstitutionsProperty (variableNameSubstitutions);
   }
}

//------------------------------------------------------------------------------
//
QString QEPlot::getVariableNameSubstitutionsProperty () const
{
   // All the same - any variable's substitutions will do.
   //
   return this->traces[0]->dnpm.getSubstitutionsProperty ();
}

//------------------------------------------------------------------------------
// slot
void QEPlot::setArchiveData (const QObject* userData, const bool okay,
                             const QCaDataPointList& archiveData,
                             const QString& pvName,
                             const QString& supplementary)
{
   bool foundSlot = false;
   int trace = -1;

   for (int j = 0; j < ARRAY_LENGTH(this->traces); j++) {
      Trace* tr = this->traces[j];
      if (tr && (userData == tr)) {
         foundSlot = true;
         trace = j;
         break;
      }
   }

   if (foundSlot && okay) {
      Trace* tr = this->traces[trace];
      tr->setArchiveData (archiveData);
   } else {
      QString message = QString ("trace: %1, pv: %3, status: %2, info: %4")
            .arg (trace)
            .arg (pvName)
            .arg (okay ? "okay" : "fail")
            .arg (supplementary);
      this->setReadOut (message);
   }
}

//==============================================================================
// Copy / Paste
QString QEPlot::copyVariable ()
{
   // Form space separates list of PV names.
   //
   QString text;
   for (int i = 0; i < QEPLOT_NUM_PLOTS; i++) {
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

   for (int i = 0; i < QEPLOT_NUM_PLOTS; i++) {
      Trace* tr = this->traces[i];
      if (!tr || !tr->isInUse) continue;

      // Use i + 1 (as opposed to just i) as variable property names are 1 to 4, not 0 to 3.
      //
      QString tl = tr->legend.isEmpty ()? QString ("Variable %1").arg (i + 1) : tr->legend;
      text.append (QString ("\n%1\nx\ty\n").arg (tl));

      for (int j = 0; j < tr->ydata.count (); j++) {
         double x = this->xStart + j* this->xIncrement;
         text.append (QString ("%1\t%2\n").arg (x).arg (tr->ydata[j]));
      }
   }

   return QVariant (text);
}

//------------------------------------------------------------------------------
// Paste to next empty, i.e. not in use, trace.
//
void QEPlot::paste (QVariant v)
{
   // v.toString is a bit limiting when v is a StringList or a List of String, so
   // use common variantToStringList function which handles these options.
   //
   const QStringList pvNames = QEUtilities::variantToStringList (v);
   const int numPVs = pvNames.size ();
   for (int i = 0, p = 0; (i < QEPLOT_NUM_PLOTS) && (p < numPVs); i++) {
      Trace* tr = this->traces[i];
      if (tr && !tr->isInUse) {
         this->setVariableName (pvNames[p], i);
         this->establishConnection (i);
         p++;
      }
   }
}

//------------------------------------------------------------------------------
// Whatever Y range is selected - apply to both left and right y axis.
//
void QEPlot::setPlotAreaYRange (const double min, const double max, const bool immediate)
{
   this->plotArea->setYRange (min, max, QEGraphicNames::SelectByValue, 5, immediate, QwtPlot::yRight);
   this->plotArea->setYRange (min, max, QEGraphicNames::SelectByValue, 5, immediate, QwtPlot::yLeft);
}

//==============================================================================
// Property functions
//
// Access functions for YMin
void QEPlot::setYMin (const double yMinIn)
{
   this->yMin = yMinIn;
   if (!this->yAxisAutoScale) {
      this->setPlotAreaYRange (this->yMin, this->yMax, false);
      this->replotIsRequired = true;
   }
}

//------------------------------------------------------------------------------
//
double QEPlot::getYMin () const
{
   return this->yMin;
}

//------------------------------------------------------------------------------
// Access functions for yMax
void QEPlot::setYMax (const double yMaxIn)
{
   this->yMax = yMaxIn;
   if (!this->yAxisAutoScale) {
      this->setPlotAreaYRange (this->yMin, this->yMax, false);
      this->replotIsRequired = true;
   }
}

//------------------------------------------------------------------------------
//
double QEPlot::getYMax () const
{
   return this->yMax;
}

//------------------------------------------------------------------------------
// Access functions for autoScale
void QEPlot::setAutoScale (const bool autoScaleIn)
{
   this->yAxisAutoScale = autoScaleIn;

   // Set auto scale if requested, or if manual scale values are invalid.
   //
   if (this->yAxisAutoScale || this->yMin >= this->yMax) {
      this->plotArea->setAxisAutoScale (QwtPlot::yLeft, true);
   } else {
      // Just re-applying the range does not cut-the-mustard, even if we turn auto scale off.
      // We need to set a different range, and then reset to the original.
      //
      this->setPlotAreaYRange (this->yMin, this->yMax + 1.0, false);
      this->setPlotAreaYRange (this->yMin, this->yMax, false);
   }

   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
bool QEPlot::getAutoScale () const
{
   return this->yAxisAutoScale;
}

//------------------------------------------------------------------------------
//
void QEPlot::setArchiveBackfill (const bool archiveBackfillIn)
{
   this->archiveBackfill = archiveBackfillIn;
}

//------------------------------------------------------------------------------
//
void QEPlot::setLogScale (const bool logScale)
{
   this->plotArea->setYLogarithmic (logScale);
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
bool QEPlot::getLogScale () const
{
   return this->plotArea->getYLogarithmic ();
}

//------------------------------------------------------------------------------
//
bool QEPlot::getArchiveBackfill () const
{
   return this->archiveBackfill;
}

//------------------------------------------------------------------------------
// Access functions for X axis visibility
void QEPlot::setAxisEnableX (const bool axisEnableXIn)
{
   this->axisEnableX = axisEnableXIn;
   this->plotArea->enableAxis (QwtPlot::xBottom, this->axisEnableX);
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
bool QEPlot::getAxisEnableX () const
{
   return this->axisEnableX;
}

//------------------------------------------------------------------------------
// Access functions for Y axis visibility
void QEPlot::setAxisEnableY (const bool axisEnableYIn)
{
   this->axisEnableY = axisEnableYIn;
   this->plotArea->enableAxis (QwtPlot::yLeft,  this->axisEnableY && (this->selectedYAxis == Left));
   this->plotArea->enableAxis (QwtPlot::yRight, this->axisEnableY && (this->selectedYAxis == Right));
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
bool QEPlot::getAxisEnableY () const
{
   return this->axisEnableY;
}

//------------------------------------------------------------------------------
//
void QEPlot::setSelectedYAxis (const SelectedYAxis selectedYAxisIn)
{
   this->selectedYAxis = selectedYAxisIn;
   this->plotArea->enableAxis (QwtPlot::yLeft,  this->axisEnableY && (this->selectedYAxis == Left));
   this->plotArea->enableAxis (QwtPlot::yRight, this->axisEnableY && (this->selectedYAxis == Right));
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
QEPlot::SelectedYAxis QEPlot::getSelectedYAxis () const
{
   return this->selectedYAxis;
}

//------------------------------------------------------------------------------
//
void QEPlot::updateGridSettings ()
{
   // If any grid is required, create a grid and set it up
   // Note, Qwt will ignore minor enable if major is not enabled
   //
   QPen majorPen;
   QPen minorPen;

   majorPen.setColor (this->gridMajorColor);
   majorPen.setStyle (Qt::DotLine);

   minorPen.setColor (this->gridMinorColor);
   minorPen.setStyle (Qt::DotLine);

   this->plotArea->setGridPens (majorPen, minorPen,
                                this->gridEnableMajorX, this->gridEnableMajorY,
                                this->gridEnableMinorX, this->gridEnableMinorY);

   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEPlot::setReadOut (const QString& text)
{
   message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STATUS);
   this->sendMessage (text, mt);
}

//------------------------------------------------------------------------------
// Access functions for grid enable
void QEPlot::setGridEnableMajorX (const bool gridEnableMajorXIn)
{
   this->gridEnableMajorX = gridEnableMajorXIn;
   this->updateGridSettings ();
}

//------------------------------------------------------------------------------
//
void QEPlot::setGridEnableMajorY (const bool gridEnableMajorYIn)
{
   this->gridEnableMajorY = gridEnableMajorYIn;
   this->updateGridSettings ();
}

//------------------------------------------------------------------------------
//
void QEPlot::setGridEnableMinorX (const bool gridEnableMinorXIn)
{
   this->gridEnableMinorX = gridEnableMinorXIn;
   this->updateGridSettings ();
}

//------------------------------------------------------------------------------
//
void QEPlot::setGridEnableMinorY (const bool gridEnableMinorYIn)
{
   this->gridEnableMinorY = gridEnableMinorYIn;
   this->updateGridSettings ();
}

//------------------------------------------------------------------------------
//
bool QEPlot::getGridEnableMajorX () const
{
   return this->gridEnableMajorX;
}

//------------------------------------------------------------------------------
//
bool QEPlot::getGridEnableMajorY () const
{
   return this->gridEnableMajorY;
}

//------------------------------------------------------------------------------
//
bool QEPlot::getGridEnableMinorX () const
{
   return this->gridEnableMinorX;
}

//------------------------------------------------------------------------------
//
bool QEPlot::getGridEnableMinorY () const
{
   return this->gridEnableMinorY;
}

//------------------------------------------------------------------------------
// Access functions for grid major colour
void QEPlot::setGridMajorColor (const QColor gridMajorColorIn)
{
   this->gridMajorColor = gridMajorColorIn;
   this->updateGridSettings ();
}

//------------------------------------------------------------------------------
//
QColor QEPlot::getGridMajorColor () const
{
   return this->gridMajorColor;
}

//------------------------------------------------------------------------------
// Access functions for grid minor colour
void QEPlot::setGridMinorColor (const QColor gridMinorColorIn)
{
   this->gridMinorColor = gridMinorColorIn;
   this->updateGridSettings ();
}

//------------------------------------------------------------------------------
//
QColor QEPlot::getGridMinorColor () const
{
   return this->gridMinorColor;
}

//------------------------------------------------------------------------------
// Access functions for title
void QEPlot::setTitle (const QString& title)
{
   this->plotArea->setTitle (title);
}

//------------------------------------------------------------------------------
//
QString QEPlot::getTitle () const
{
   return this->plotArea->getTitle ();
}

//------------------------------------------------------------------------------
// Access functions for backgroundColor
void QEPlot::setBackgroundColor (const QColor backgroundColorIn)
{
   // cache in widget for proper behaviour.
   //
   this->backgroundColor = backgroundColorIn;
   this->plotArea->setBackgroundColour (this->backgroundColor);
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
QColor QEPlot::getBackgroundColor () const
{
   return this->backgroundColor;
}

//------------------------------------------------------------------------------
// Access functions for traceStyle
void QEPlot::setTraceStyle (const TraceStyles traceStyle, const unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex,);

   Trace* tr = this->traces[variableIndex];
   tr->style = convertTrace2Curve (traceStyle);
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
QEPlot::TraceStyles QEPlot::getTraceStyle (const unsigned int variableIndex) const
{
   PV_INDEX_CHECK (variableIndex, Lines);
   return convertCurve2Trace (this->traces[variableIndex]->style);
}

//------------------------------------------------------------------------------
// Access functions for traceWidth
void QEPlot::setTraceWidth (const int traceWidth, const unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex,);
   Trace* tr = this->traces[variableIndex];
   tr->width = LIMIT (traceWidth, 0, 20);  // 20 arbitrary but sufficient
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
int QEPlot::getTraceWidth (const unsigned int variableIndex) const
{
   PV_INDEX_CHECK (variableIndex, 1);
   Trace* tr = this->traces[variableIndex];
   return tr->width;
}

//------------------------------------------------------------------------------
// Access functions for traceColor
void QEPlot::setTraceColor (const QColor traceColor, const unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex,);
   this->traces[variableIndex]->color = traceColor;
   this->legendArea->update ();
   this->replotIsRequired = true;
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
void QEPlot::setTraceLegend (const QString& traceLegend, const unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex,);

   Trace* tr = this->traces[variableIndex];
   if (!tr) return;    // sanity check

   tr->legend = traceLegend;
   this->legendArea->update();
}

//------------------------------------------------------------------------------
//
QString QEPlot::getTraceLegend (const unsigned int variableIndex) const
{
   PV_INDEX_CHECK (variableIndex, "");

   Trace* tr = this->traces[variableIndex];
   if (!tr) return "";    // sanity check

   return tr->legend;
}

//------------------------------------------------------------------------------
//
void QEPlot::setTraceVisible (const bool traceVisible, const unsigned int variableIndex)
{
   PV_INDEX_CHECK (variableIndex,);

   Trace* tr = this->traces[variableIndex];
   if (!tr) return;    // sanity check

   tr->isVisible = traceVisible;
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
bool QEPlot::getTraceVisible (const unsigned int variableIndex) const
{
   PV_INDEX_CHECK (variableIndex, false);

   Trace* tr = this->traces[variableIndex];
   if (!tr) return false;    // sanity check

   return tr->isVisible;
}


//------------------------------------------------------------------------------
// Access functions for xUnit
void QEPlot::setXUnit (const QString& xUnit)
{
   this->plotArea->setAxisTitle (QwtPlot::xBottom, xUnit);
}

//------------------------------------------------------------------------------
//
QString QEPlot::getXUnit () const
{
   return this->plotArea->getAxisTitle (QwtPlot::xBottom);
}

//------------------------------------------------------------------------------
// Access functions for yUnit
void QEPlot::setYUnit (const QString& yUnit)
{
   this->plotArea->setAxisTitle (QwtPlot::yLeft, yUnit);
}

//------------------------------------------------------------------------------
//
QString QEPlot::getYUnit () const
{
   return this->plotArea->getAxisTitle (QwtPlot::yLeft);
}

//------------------------------------------------------------------------------
// Access functions for xStart
void QEPlot::setXStart (const double xStartIn)
{
   this->xStart = xStartIn;
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
double QEPlot::getXStart () const
{
   return this->xStart;
}

//------------------------------------------------------------------------------
// Access functions for xIncrement
void QEPlot::setXIncrement (const double xIncrementIn)
{
   this->xIncrement = xIncrementIn;
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
double QEPlot::getXIncrement () const
{
   return this->xIncrement;
}

//------------------------------------------------------------------------------
// Access functions for xFirst
void QEPlot::setXFirst (const double xFirstIn)
{
   this->xFirst = xFirstIn;
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
double QEPlot::getXFirst () const
{
   return this->xFirst;
}

//------------------------------------------------------------------------------
// Access functions for xLast
void QEPlot::setXLast (const double xLastIn)
{
   this->xLast = xLastIn;
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
double QEPlot::getXLast () const
{
   return this->xLast;
}

//------------------------------------------------------------------------------
// Access functions for timeSpan
void QEPlot::setTimeSpan (const int timeSpanIn)
{
   this->timeSpan = MAX (1, timeSpanIn);
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
int QEPlot::getTimeSpan () const
{
   return this->timeSpan;
}

//------------------------------------------------------------------------------
// Access functions for tickRate
void QEPlot::setTickRate (const int tickRateIn)
{
   this->tickRate = LIMIT (tickRateIn, 20, 2000);       // Limit to >= 20, i.e. <= 50 Hz.
}

//------------------------------------------------------------------------------
//
int QEPlot::getTickRate () const
{
   return this->tickRate;
}

//------------------------------------------------------------------------------
// Access functions for margin
void QEPlot::setMargin (const int marginIn)
{
   this->layoutMargin = LIMIT (marginIn, 0, 100);
   this->layout->setContentsMargins (layoutMargin, layoutMargin, layoutMargin, layoutMargin);
}

//------------------------------------------------------------------------------
//
int QEPlot::getMargin () const
{
   return this->layoutMargin;
}

//------------------------------------------------------------------------------
//
void QEPlot::setMouseMoveSignals (const QE::MouseMoveSignalFlags flags)
{
   this->mMouseMoveSignals = flags;
}

//------------------------------------------------------------------------------
//
QE::MouseMoveSignalFlags QEPlot::getMouseMoveSignals () const
{
   return this->mMouseMoveSignals;
}

//------------------------------------------------------------------------------
//
void QEPlot::setFullLengthArraySubscriptions (const bool useFullLengthArraySubscriptionsIn)
{
   this->useFullLengthArraySubscriptions = useFullLengthArraySubscriptionsIn;
}

//------------------------------------------------------------------------------
//
bool QEPlot::getFullLengthArraySubscriptions() const
{
   return this->useFullLengthArraySubscriptions;
}

//------------------------------------------------------------------------------
// There are foure PVs and assocates fours sets of trace properties, which
// are all essentially identical, save for the function name and trace index.
// names go 1 to 4, indicies go 0 to 3
//
#define ACCESS_FUNCTIONS(name, index)                                       \
void QEPlot::setVariableName##name##Property (const QString& pvName)        \
{                                                                           \
   this->setVariableNameIndexProperty (pvName, index);                      \
}                                                                           \
/*  */                                                                      \
QString QEPlot::getVariableName##name##Property () const                    \
{                                                                           \
   return this->getVariableNameIndexProperty (index);                       \
}                                                                           \
/*  */                                                                      \
void QEPlot::setSizeVariableName##name##Property (const QString& pvName)    \
{                                                                           \
   this->setSizeVariableNameIndexProperty (pvName, index);                  \
}                                                                           \
/*  */                                                                      \
QString QEPlot::getSizeVariableName##name##Property () const                \
{                                                                           \
   return this->getSizeVariableNameIndexProperty (index);                   \
}                                                                           \
   /*  */                                                                   \
void QEPlot::setTraceStyle##name (const TraceStyles traceStyle)             \
{                                                                           \
   this->setTraceStyle (traceStyle, index);                                 \
}                                                                           \
/*  */                                                                      \
QEPlot::TraceStyles QEPlot::getTraceStyle##name () const                    \
{                                                                           \
   return this->getTraceStyle (index);                                      \
}                                                                           \
/*  */                                                                      \
void QEPlot::setTraceWidth##name (const int traceWidth)                     \
{                                                                           \
   this->setTraceWidth (traceWidth, index);                                 \
}                                                                           \
/*  */                                                                      \
int QEPlot::getTraceWidth##name () const                                    \
{                                                                           \
   return this->getTraceWidth (index);                                      \
}                                                                           \
/*  */                                                                      \
void QEPlot::setTraceColor##name (const QColor traceColor)                  \
{                                                                           \
   this->setTraceColor (traceColor, index);                                 \
}                                                                           \
/*  */                                                                      \
QColor QEPlot::getTraceColor##name () const                                 \
{                                                                           \
   return this->getTraceColor (index);                                      \
}                                                                           \
/*  */                                                                      \
void QEPlot::setTraceLegend##name (const QString& traceLegend)              \
{                                                                           \
   this->setTraceLegend (traceLegend, index);                               \
}                                                                           \
/*  */                                                                      \
QString QEPlot::getTraceLegend##name () const                               \
{                                                                           \
   return this->getTraceLegend (index);                                     \
}                                                                           \
/*  */                                                                      \
void QEPlot::setTraceVisible##name (const bool visible)                     \
{                                                                           \
   this->setTraceVisible (visible, index);                                  \
}                                                                           \
/*  */                                                                      \
bool QEPlot::getTraceVisible##name () const                                 \
{                                                                           \
  return this->getTraceVisible (index);                                     \
}


ACCESS_FUNCTIONS (1, 0)
ACCESS_FUNCTIONS (2, 1)
ACCESS_FUNCTIONS (3, 2)
ACCESS_FUNCTIONS (4, 3)
ACCESS_FUNCTIONS (5, 4)
ACCESS_FUNCTIONS (6, 5)
ACCESS_FUNCTIONS (7, 6)
ACCESS_FUNCTIONS (8, 7)

#undef ACCESS_FUNCTIONS

// end
