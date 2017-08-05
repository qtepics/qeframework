/*  QEStripChartItem.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2012,2016,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <alarm.h>

#include <QApplication>
#include <QColor>
#include <QColorDialog>
#include <QFileDialog>
#include <QVariantList>
#include <QClipboard>

#include <QCaObject.h>
#include <QEArchiveInterface.h>
#include <QECommon.h>
#include <QEGraphic.h>
#include <QEScaling.h>
#include "QEStripChartItem.h"
#include "QEStripChartContextMenu.h"
#include "QEStripChartStatistics.h"

#define DEBUG  qDebug () <<  "QEStripChartItem" << __LINE__ <<  __FUNCTION__  << "  "

// Defines the maximum number of points requested to be extracted from the
// archiver per PV. The Channel Access archiver interface itself supports upto
// 10K points, but on a typical sized screen, we cannot sensibly use more points.
//
#define MAXIMUM_HISTORY_POINTS   5000

// Defines the number of live points to be accumulated before dropping
// older points.
//
#define DEFAULT_MAXIMUM_LIVE_POINTS      40000

// Can't declare black as QColor (0x000000)
//
static const QColor clWhite (0xFF, 0xFF, 0xFF, 0xFF);
static const QColor clBlack (0x00, 0x00, 0x00, 0xFF);

// Define colours: essentially RGB byte triplets
//
static const QColor item_colours [QEStripChart::NUMBER_OF_PVS] = {
    QColor (0xFF0000), QColor (0x0000FF), QColor (0x008000), QColor (0xFF8000),
    QColor (0x4080FF), QColor (0x800000), QColor (0x008080), QColor (0x808000),
    QColor (0x800080), QColor (0x00FF00), QColor (0x00FFFF), QColor (0xE0E000),
    QColor (0x8F00C0), QColor (0x008FC0), QColor (0xB040B0), clBlack
};


static const QString letterStyle ("QWidget { background-color: #e8e8e8; }");
static const QString inuseStyle  ("QWidget { background-color: #e0e0e0; }");
static const QString unusedStyle ("QWidget { background-color: #c0c0c0; }");

static const QString scaledTip  (" Note: this PV has been re-scaled ");
static const QString regularTip (" Use context menu to modify PV attributes or double click here. ");


//==============================================================================
//
QEStripChartItem::QEStripChartItem (QEStripChart* chartIn,
                                    const int slotIn,
                                    QWidget* parent) : QWidget (parent)
{
   QColor defaultColour;

   // Save and set input parameters.
   //
   this->chart = chartIn;
   this->slot = slotIn;

   // Construct internal widgets for this chart item.
   //
   this->createInternalWidgets ();

   this->maxRealTimePoints = DEFAULT_MAXIMUM_LIVE_POINTS;
   this->previousQcaItem = NULL;

   this->dataKind = NotInUse;
   this->calculator = new QEExpressionEvaluation ();
   this->expression = "";
   this->expressionIsValid = false;

   // Set up other properties.
   //
   this->pvSlotLetter->setStyleSheet (letterStyle);

   this->pvName->setIndent (6);
   this->pvName->setToolTip (regularTip);
   this->pvName->installEventFilter (this);   // for double click

   // Set the event filter for drag/dropping.
   //
   this->setAcceptDrops (true);
   this->installEventFilter (this);

   // Set up context menus.
   //
   this->pvName->setContextMenuPolicy (Qt::CustomContextMenu);

   // Setup QELabel properties.
   //
   this->caLabel->setAlignment (Qt::AlignLeft);

   // We have to be general here.
   //
   this->caLabel->setPrecision (6);
   this->caLabel->setForceSign (true);
   this->caLabel->setUseDbPrecision (false);
   this->caLabel->setNotation (QEStringFormatting::NOTATION_AUTOMATIC);
   this->caLabel->setTrailingZeros (false);
   this->caLabel->setArrayAction (QEStringFormatting::INDEX);
   this->caLabel->setArrayIndex (0);

   QFont font = this->caLabel->font ();
   font.setFamily ("Monospace");
   this->caLabel->setFont (font);

   if (slot < QEStripChart::NUMBER_OF_PVS) {
      defaultColour = item_colours [this->slot];
   } else {
      defaultColour = clBlack;
   }
   this->setColour (defaultColour);

   // Clear/initialise.
   //
   this->clear ();

   // Assign the chart widget message source id the the associated archive access object.
   //
   this->archiveAccess.setMessageSourceId (chartIn->getMessageSourceId ());

   // Set up a connection to recieve variable name property changes.  The variable
   // name property manager class only delivers an updated variable name after the
   // user has stopped typing.
   //
   this->pvNameProperyManager.setVariableIndex (0);
   QObject::connect (&this->pvNameProperyManager, SIGNAL (newVariableNameProperty (QString, QString, unsigned int)),
                     this,                        SLOT   (newVariableNameProperty (QString, QString, unsigned int)));


   // Set up connection to archive access mamanger.
   //
   QObject::connect (&this->archiveAccess, SIGNAL (setArchiveData (const QObject*, const bool, const QCaDataPointList&,
                                                                   const QString&, const QString&)),
                     this,                 SLOT   (setArchiveData (const QObject*, const bool, const QCaDataPointList&,
                                                                   const QString&, const QString&)));


   this->connect (this->pvName, SIGNAL (customContextMenuRequested (const QPoint &)),
                  this,         SLOT   (contextMenuRequested (const QPoint &)));

   // Connect the context menus
   //
   this->connect (this->inUseMenu, SIGNAL (contextMenuSelected (const QEStripChartNames::ContextMenuOptions)),
                  this,            SLOT   (contextMenuSelected (const QEStripChartNames::ContextMenuOptions)));

   this->connect (this->emptyMenu, SIGNAL (contextMenuSelected (const QEStripChartNames::ContextMenuOptions)),
                  this,            SLOT   (contextMenuSelected (const QEStripChartNames::ContextMenuOptions)));

   // Connect letter button
   //
   QObject::connect (this->pvSlotLetter, SIGNAL ( clicked (bool)),
                     this,   SLOT   ( letterButtonClicked (bool)));

   this->hostSlotAvailable = false;

   // Prepare to interact with whatever application is hosting this widget.
   // For example, the QEGui application can host docks and toolbars for QE widgets
   // Needed to lauch the PV Statistics window.
   //
   if (this->chart && this->chart->isProfileDefined ()) {
      // Setup a signal to request component hosting.
      //
      QObject* launcher = this->chart->getGuiLaunchConsumer ();
      if (launcher) {
         this->hostSlotAvailable =
            QObject::connect (this,     SIGNAL (requestAction (const QEActionRequests& )),
                              launcher, SLOT   (requestAction (const QEActionRequests& )));
      }
   }
}

//------------------------------------------------------------------------------
//
QEStripChartItem::~QEStripChartItem ()
{
   // Place holder
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::createInternalWidgets ()
{
   QString letter;

   letter.clear ();
   letter.append (char (int ('A') + this->slot));

   this->layout = new QHBoxLayout (this);
   this->layout->setSpacing (4);
   this->layout->setContentsMargins (1, 1, 1, 1);

   this->pvSlotLetter = new QPushButton (letter, this);
   this->pvSlotLetter->setFixedWidth (20);
   this->pvSlotLetter->setFixedHeight (16);
   layout->addWidget (this->pvSlotLetter);

   this->pvName = new QLabel (this);
   this->pvName->setMinimumSize (QSize (328, 15));
   this->pvName->setMaximumSize (QSize (1200, 15));
   this->pvName->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Preferred);

   layout->addWidget (this->pvName);

   this->caLabel = new QELabel (this);
   this->caLabel->setMinimumSize (QSize (88, 15));
   this->caLabel->setMaximumSize (QSize (200, 15));
   this->layout->addWidget (this->caLabel);

   // Set up the stretchh ratios.
   this->layout->setStretch (0, 0);
   this->layout->setStretch (1, 3);
   this->layout->setStretch (2, 1);

   this->colourDialog = new QColorDialog (this);
   this->inUseMenu = new QEStripChartContextMenu (true, this);
   this->emptyMenu = new QEStripChartContextMenu (false, this);
   this->adjustPVDialog = new QEStripChartAdjustPVDialog (this);
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::clear ()
{
   this->dataKind = NotInUse;
   this->caLabel->deactivate ();
   this->caLabel->setVariableNameAndSubstitutions ("", "", 0);
   this->caLabel->setText ("-");
   this->caLabel->setStyleSheet (unusedStyle);
   this->previousQcaItem = NULL;

   this->displayedMinMax.clear ();
   this->historicalMinMax.clear ();
   this->realTimeMinMax.clear ();
   this->historicalTimeDataPoints.clear ();
   this->dashExists = false;
   this->realTimeDataPoints.clear ();
   this->maxRealTimePoints = DEFAULT_MAXIMUM_LIVE_POINTS;

   this->useReceiveTime = false;
   this->archiveReadHow = QEArchiveInterface::Linear;
   this->lineDrawMode = QEStripChartNames::ldmRegular;
   this->linePlotMode = QEStripChartNames::lpmRectangular;

   // Reset identity sclaing
   //
   this->scaling.reset();

   this->setCaption ();
}

//------------------------------------------------------------------------------
//
qcaobject::QCaObject* QEStripChartItem::getQcaItem ()
{
   // We "know" that a QELabel has only one PV.
   //
   return this->caLabel->getQcaItem (0);
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::connectQcaSignals ()
{
   qcaobject::QCaObject* qca;

   // Set up connection if we can/if we need to.
   //
   qca = this->getQcaItem ();

   if (qca && (qca != this->previousQcaItem)) {
      this->previousQcaItem = qca;

      QObject::connect (qca, SIGNAL (connectionChanged (QCaConnectionInfo&, const unsigned int& ) ),
                        this,  SLOT (setDataConnection (QCaConnectionInfo&, const unsigned int& ) ) );

      QObject::connect (qca, SIGNAL (dataChanged  (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                        this,  SLOT (setDataValue (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::setPvName (const QString& pvName, const QString& substitutions)
{
   QString substitutedPVName;

   // Clear any existing data and reset defaults.
   //
   this->clear ();
   this->chart->evaluateAllowDrop ();   // move to strip chart proper??

   // We "know" that a QELabel has only one PV (index = 0).
   //
   this->caLabel->deactivate();
   this->caLabel->setVariableNameAndSubstitutions (pvName.trimmed (), substitutions, 0);

   // Verify caller attempting add a potentially sensible PV?
   //
   substitutedPVName = caLabel->getSubstitutedVariableName (0);
   if (substitutedPVName.isEmpty ()) return;

   // Ensure we always active irrespective of the profile DontActivateYet state.
   //
   this->caLabel->activate();

   this->caLabel->setStyleSheet (inuseStyle);
   this->dataKind = PVData;
   this->setCaption ();

   // Set up connections.
   //
   this->connectQcaSignals ();
}

//------------------------------------------------------------------------------
//
QString QEStripChartItem::getPvName () const
{
   return this->isInUse () ? this->caLabel->getSubstitutedVariableName (0) : "";
}

//------------------------------------------------------------------------------
//
QString QEStripChartItem::getEgu () const
{
   QString result = "";
   if (this->isInUse ()) {
      qcaobject::QCaObject* qca = this->caLabel->getQcaItem (0);
      if (qca) result = qca->getEgu ();
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::setCaption ()
{
   QString caption;
   QString substitutedPVName;

   caption.clear ();

   if (this->isInUse ()) {
      if (this->scaling.isScaled ()) {
         caption.append ("*");
      } else {
         caption.append (" ");
      }

      substitutedPVName = this->caLabel->getSubstitutedVariableName (0);
      caption.append (substitutedPVName);
   }

   this->pvName->setText (caption);
}

//------------------------------------------------------------------------------
//
bool QEStripChartItem::isInUse () const
{
   return ((this->dataKind == PVData) || (this->dataKind == CalculationData));
}

//------------------------------------------------------------------------------
//
bool QEStripChartItem::isCalculation () const
{
   return (this->dataKind == CalculationData);
}

//------------------------------------------------------------------------------
//
QEDisplayRanges QEStripChartItem::getLoprHopr (bool doScale)
{
   QEDisplayRanges result;
   qcaobject::QCaObject *qca;
   double lopr;
   double hopr;

   result.clear ();

   if (this->isInUse ()) {
      qca = this->getQcaItem ();
      if (qca) {
         lopr = qca->getDisplayLimitLower ();
         hopr = qca->getDisplayLimitUpper ();
      } else {
         lopr = hopr = 0.0;
      }

      // If either HOPR or LOPR are non zero - then range is deemed defined.
      //
      if ((lopr != 0.0) || (hopr != 0.0)) {
         result.merge (lopr);
         result.merge (hopr);
      }
   }
   if (doScale) {
       result = this->scaling.value (result);
   }
   return result;
}


//------------------------------------------------------------------------------
//
QEDisplayRanges QEStripChartItem::getDisplayedMinMax (bool doScale)
{
   QEDisplayRanges result;

   result = this->displayedMinMax;
   if (doScale) {
       result = this->scaling.value (result);
   }
   return result;
}


//------------------------------------------------------------------------------
//
QEDisplayRanges QEStripChartItem::getBufferedMinMax (bool doScale)
{
   QEDisplayRanges result;

   result = this->historicalMinMax;
   result.merge (this->realTimeMinMax);

   if (doScale) {
       result = this->scaling.value (result);
   }
   return result;
}

//------------------------------------------------------------------------------
// macro functions to convert real-world values to a plot values, doing safe log conversion if required.
//
#define PLOT_T(t) (t)
#define PLOT_Y(y) (this->scaling.value (y))

//------------------------------------------------------------------------------
//
QPointF QEStripChartItem::dataPointToReal (const QCaDataPoint& point) const
{

   const QCaDateTime end_time = this->chart->getEndDateTime ();
   const double t = end_time.secondsTo (point.datetime);
   QPointF result = QPointF (PLOT_T (t), PLOT_Y (point.value));
   return result;
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::plotDataPoints (const QCaDataPointList& dataPoints,
                                       const bool isRealTime,
                                       const Qt::PenStyle penStyle,
                                       QEDisplayRanges& plottedTrackRange)
{
   const QCaDateTime start_time = this->chart->getStartDateTime ();
   const QCaDateTime end_time = this->chart->getEndDateTime ();
   const double duration = this->chart->getDuration ();
   QEGraphic* graphic = this->chart->plotArea;

   QVector<double> tdata;
   QVector<double> ydata;
   QCaDataPoint point;
   QCaDataPoint previous;
   bool doesPreviousExist;
   bool isFirstPoint;
   double t;
   bool extendToEnd = false;

   if (!graphic) return;   // sanity check

   graphic->setCurveRenderHint (QwtPlotItem::RenderAntialiased, false);
   graphic->setCurveStyle (QwtPlotCurve::Lines);

   QPen pen = this->getPen ();
   pen.setStyle (penStyle);
   graphic->setCurvePen (pen);

   // Both values zero is deemed to be undefined.
   //
   plottedTrackRange.clear ();
   isFirstPoint = true;
   doesPreviousExist = false;

   // Determine number of points that can be plotted.
   //
   const int count = dataPoints.count ();
   const int first = dataPoints.indexBeforeTime (start_time, 0);
   const int last  = dataPoints.indexBeforeTime (end_time, count);
   const int number = last - first + 1;

   // The maximum width of the chart is typically of the order of
   // 1200 pixels. No point over-plotting if we have lots of data. If
   // more that 3*chart width then start decimating.
   //
   const int width = this->chart->plotArea->geometry ().width ();

   // Calculate decimation factor
   //
   const int decimation = 1 + number/(3 * width);

   // Also if we are decimatinging - don't bother rectangularising the plot.
   //
   QEStripChartNames::LinePlotModes workingPlotMode = this->linePlotMode;
   if (decimation > 1) workingPlotMode = QEStripChartNames::lpmSmooth;

   // Reserve required number of draw points up front.
   //
   int drawPoints = (number / decimation) + 1;
   if (workingPlotMode == QEStripChartNames::lpmRectangular) {
     drawPoints = 2*drawPoints;
   }
   tdata.reserve (drawPoints);
   ydata.reserve (drawPoints);

   for (int j = first; j < count; j += decimation) {
      point = dataPoints.value (j);

      // Calculate the time of this point (in seconds) relative to the end of the chart.
      //
      t = end_time.secondsTo (point.datetime);

      if (t < -duration) {
         // Point time is before current time range of the chart.
         //
         // Just save this point. Last time it is saved it will be the
         // pen-ultimate point before the chart start time.
         //
         previous = point;

         // Only "exists" if plottable.
         //
         doesPreviousExist = point.isDisplayable ();  // (previous.alarm.isInvalid () == false);

      }
      else if ((t >= -duration) && (t <= 0.0)) {
         // Point time is within current time range of the chart.
         //
         // Is it a valid point - can we sensible plot it?
         //
         if (point.isDisplayable ()) {
            // Yes we can.
            //
            if (!this->firstPointIsDefined) {
               this->firstPointIsDefined = true;
               this->firstPoint = point;
            }

            // start edge effect required?
            //
            if (isFirstPoint && doesPreviousExist) {
                tdata.append (PLOT_T (-duration));
                ydata.append (PLOT_Y (previous.value));
                plottedTrackRange.merge (previous.value);
            }

            if (workingPlotMode == QEStripChartNames::lpmRectangular) {
               // Do steps - do it like this as using qwt Step mode is not what I want.
               //
               if (ydata.count () >= 1) {
                  tdata.append (PLOT_T (t));
                  ydata.append (ydata.last ());   // copy - don't need PLOT_Y
               }
            }

            tdata.append (PLOT_T (t));
            ydata.append (PLOT_Y (point.value));
            plottedTrackRange.merge (point.value);

         } else {
            // plot what we have so far (need at least 2 points).
            //
            if (tdata.count () >= 1) {
               // The current pont is unplotable (invalid/disconneted).
               // Create  a valid stopper point consisting of prev. point value and this point time.
               //
               tdata.append (PLOT_T (t));
               ydata.append (ydata.last ());   // is a copy - no PLOT_Y required.

               graphic->plotCurveData (tdata, ydata);

               tdata.clear ();
               ydata.clear ();
            }
         }

         // We have processed at least one point now.
         //
         isFirstPoint = false;

      } else {
         // Point time is after current plot time of the chart.
         // This this point is dispalyable, then plot upto the edge of the chart.
         //
         extendToEnd = point.isDisplayable ();;
         break;
      }
   }

   // Start edge special required?
   //
   if (isFirstPoint && doesPreviousExist) {
       tdata.append (PLOT_T (-duration));
       ydata.append (PLOT_Y (previous.value));
       plottedTrackRange.merge (previous.value);
   }

   // Plot what we have accumulated.
   //
   if (ydata.count () >= 1) {
      // Extention to time now required?
      //
      if (isRealTime || extendToEnd) {
         // Replicate last value upto end of chart.
         //
         tdata.append (PLOT_T (0.0));
         ydata.append (ydata.last ());   // is a copy - no PLOT_Y required.
      }
      graphic->plotCurveData (tdata, ydata);
   }
}

#undef PLOT_T
#undef PLOT_Y

//------------------------------------------------------------------------------
//
QCaDataPointList QEStripChartItem::determinePlotPoints ()
{
   const QCaDateTime end_time = this->chart->getEndDateTime ();
   const double duration = this->chart->getDuration ();

   QCaDataPointList result;

   int count;
   QCaDataPoint point;
   double t;
   bool isFirst;
   QCaDataPointList* listArray [2];

   // Create an array so that we loop over both lists.
   //
   listArray [0] = &this->historicalTimeDataPoints;
   listArray [1] = &this->realTimeDataPoints;

   for (int i = 0; i < 2; i++) {
      QCaDataPointList* list = listArray [i];
      isFirst = true;
      count = list->count ();
      for (int j = 0; j < count; j++) {
         point = list->value (j);

         // Calculate the time of this point (in seconds) relative to the end of the chart.
         //
         t = end_time.secondsTo (point.datetime);

         if ((t >= -duration) && (t <= 0.0)) {
            // Point time is within current time range of the chart.
            //
            if (isFirst && (j > 0)) {
               // do one previous point.
               //
               result.append (list->value (j - 1));
            }
            isFirst = false;
            result.append (point);
         } else if (t > 0.0) {
            // do one follwing point, then  skip the rest.
            result.append (point);
            break;
         }
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::plotData ()
{
   QEDisplayRanges temp;

   this->displayedMinMax.clear ();
   this->firstPointIsDefined = false;

   if (this->lineDrawMode != QEStripChartNames::ldmHide) {

      this->plotDataPoints (this->historicalTimeDataPoints, false, Qt::SolidLine, temp);
      this->displayedMinMax.merge (temp);

      this->plotDataPoints (this->realTimeDataPoints, true, Qt::SolidLine, temp);
      this->displayedMinMax.merge (temp);

      // Do historical dash special if required.
      //
      if (this->dashExists) {
         QCaDataPointList dashList;
         dashList.append (this->dashStart);
         dashList.append (this->dashEnd);
         this->plotDataPoints (dashList, false, Qt::DashLine, temp);
      }
   }

   // Sometimes the qca Item first used is not the qca Item we end up with, due the
   // vagaries of loading ui files and the framework start up. As plot data called
   // on a regular basis this is a convient place to recall connectQca.
   // Note: connectQcaSignals only does anything if underlying qca item has changed.
   //
   this->connectQcaSignals ();
}

//------------------------------------------------------------------------------
//
const QCaDataPoint* QEStripChartItem::findNearestPoint (const QCaDateTime& searchTime) const
{
   const QCaDataPoint* result = NULL;

   const QCaDataPoint* historicalNearest = this->historicalTimeDataPoints.findNearestPoint (searchTime);
   const QCaDataPoint* realTimeNearest = this->realTimeDataPoints.findNearestPoint (searchTime);

   if (!historicalNearest) {
      result = realTimeNearest;
   } else if (!realTimeNearest) {
      result = historicalNearest;
   } else {
      // Both points found.
      //
      double hdt = historicalNearest->datetime.secondsTo (searchTime);
      double rdt = realTimeNearest->datetime.secondsTo (searchTime);

      result = ABS (hdt) >= ABS (rdt) ? realTimeNearest : historicalNearest;
   }

   return result;
}


//------------------------------------------------------------------------------
//
void QEStripChartItem::newVariableNameProperty (QString pvName, QString substitutions, unsigned int)
{
   this->setPvName (pvName, substitutions);

   // Re evaluate the chart drag drop allowed status.
   //
   this->chart->evaluateAllowDrop ();
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::addRealTimeDataPoint (const QCaDataPoint& point)
{
   // Do any decimation and/or dead-banding here.
   //
   this->realTimeDataPoints.append (point);
   if (this->realTimeDataPoints.count () > this->maxRealTimePoints) {
      this->realTimeDataPoints.removeFirst ();
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::setDataConnection (QCaConnectionInfo& connectionInfo, const unsigned int& )
{
   QCaDataPoint point;

   this->isConnected = connectionInfo.isChannelConnected ();
   if ((this->isConnected == false) && (this->realTimeDataPoints.count () >= 1)) {
      // We have a channel disconnect.
      //
      // create a dummy point with last value and time now.
      //
      point = this->realTimeDataPoints.last ();
      point.datetime = QDateTime::currentDateTime ().toUTC ();
      this->addRealTimeDataPoint (point);

      // create a dummy point with same time but marked invalid to indicate a break.
      //
      point.alarm = QCaAlarmInfo (NO_ALARM, INVALID_ALARM);
      this->addRealTimeDataPoint (point);

      this->chart->setRecalcIsRequired ();
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::setDataValue (const QVariant& value, QCaAlarmInfo& alarm,
                                     QCaDateTime& datetime, const unsigned int& )
{
   QVariant input;
   double y;
   bool okay;
   QCaDataPoint point;

   // Do something sensible with array PVs.
   //
   if (value.type () == QVariant::List) {
      QVariantList list = value.toList ();
      // Use first element. Consdider some mechanism to all the element to
      // be selected buy the user.
      //
      input = list.value (0);
   } else {
      input = value;  // use as is
   }

   y = input.toDouble (&okay);
   if (okay) {
      // Conversion went okay - use this point.
      //
      point.value = y;
      point.alarm = alarm;
   } else {
      // Could not convert to a double - mark as an invalid point.
      //
      point.value = 0.0;
      point.alarm = QCaAlarmInfo (NO_ALARM, INVALID_ALARM);
   }

   // Some records, e.g. the motor record, post RBV updated without updating
   // the the process time until the end of the move. Sometimes the server and/or
   // client time is just wrong. In these cases it is better to plot using the
   // receive time.
   //
   if (this->useReceiveTime) {
      point.datetime = QDateTime::currentDateTime ().toUTC ();
   } else {
      point.datetime = datetime;
   }

   if (point.isDisplayable ()) {
      this->realTimeMinMax.merge (point.value);
   }

   this->addRealTimeDataPoint (point);
   this->chart->setRecalcIsRequired ();
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::setArchiveData (const QObject* userData, const bool okay,
                                       const QCaDataPointList& archiveData,
                                       const QString& pvName, const QString& supplementary)
{
   QCaDateTime firstRealTime;
   int count;
   QCaDataPoint point;

   if ((userData == this) && (okay)) {

      this->dashExists = false;

      // Clear any existing data and save new data
      // Maybe would could/should do some stiching together
      //
      this->historicalTimeDataPoints.clear ();
      this->historicalTimeDataPoints = archiveData;

      // Determine number of valid points, and generate user information message.
      //
      count = this->historicalTimeDataPoints.count ();
      int validCount = 0;
      for (int j = 0; j < count; j++) {
         QCaDataPoint p = this->historicalTimeDataPoints.value (j);
         if (p.isDisplayable ()) {
            validCount++;
         }
      }

      QString message = QString ("%1: %2 out of %3 points valid")
            .arg (pvName).arg (validCount).arg(count);
      this->chart->setReadOut (message);

      // Have any data points been returned?
      //
      if (count > 0) {

         // Now throw away any historical data that overlaps with the real time data,
         // there is no need for two copies. We keep the real time data as it is of
         // a better quality.
         //
         // Find trucate time
         //
         if (this->realTimeDataPoints.count () > 0) {
            firstRealTime = this->realTimeDataPoints.value (0).datetime;
         } else {
            firstRealTime = QDateTime::currentDateTime ().toUTC ();
         }

         // Look at first historical data point.
         //
         point = this->historicalTimeDataPoints.value(0);
         if (point.datetime >= firstRealTime) {
            // Historical data adds nothing here.
            return;
         }

         // Purge all points with a time >= firstRealTime, except for the
         // the very first point after first time.
         //
         while (this->historicalTimeDataPoints.count () >= 2) {
            int penUltimate = this->historicalTimeDataPoints.count () - 2;
            point = this->historicalTimeDataPoints.value(penUltimate);
            if (point.datetime >= firstRealTime) {
               this->historicalTimeDataPoints.removeLast ();
            } else {
               // purge complete
               break;
            }
         }

         // Truncate last historical point so that there is no time overlap.
         //
         QCaDataPoint lastPoint = this->historicalTimeDataPoints.last ();
         if (lastPoint.datetime > firstRealTime) {
            lastPoint.datetime = firstRealTime;
            int last = this->historicalTimeDataPoints.count () - 1;
            this->historicalTimeDataPoints.replace (last, point);
         }


         // Because the archiver is a few minutes out of date, there may be
         // a gap between the end of the received historical data and the start
         // of the buffered real time data - therefore we create a virtual
         // data points in order to 'terminate' the historical data.
         // We also define the Dash parameters.
         //
         if ((lastPoint.datetime < firstRealTime) && lastPoint.isDisplayable()) {

            // Create virtual invalid point at end of historical data.
            // Limit Time to be no more than live data or or 10 seconds.
            //
            QCaDataPoint virtualPoint = lastPoint;
            QCaDateTime plus10 = lastPoint.datetime.addSeconds (10.0);
            virtualPoint.datetime = MIN (firstRealTime, plus10);

            // Append virtual historical point.
            //
            this->historicalTimeDataPoints.append (virtualPoint);

            // Set up historical to live dash parameters.
            //
            this->dashStart = virtualPoint;
            this->dashEnd = virtualPoint;
            this->dashEnd.datetime = firstRealTime;
            this->dashExists = true;
         }

         // Now determine the min and max values of the remaining data points.
         //
         this->historicalMinMax.clear ();
         count = this->historicalTimeDataPoints.count ();
         for (int j = 0; j < count; j++) {
            point = this->historicalTimeDataPoints.value (j);
            if (point.isDisplayable ()) {
               this->historicalMinMax.merge (point.value);
            }
         }
      } else {
         this->chart->setReadOut (supplementary);
      }

      // and replot the data
      //
      this->chart->setReplotIsRequired ();

   } else {
      this->chart->setReadOut (supplementary);
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::readArchive ()
{
   if (!this->isInUse ()) return;  // sainity check

   const double chartDuration =  this->chart->getDuration();  // in seconds

   // For longer time frames use selected data extractions.
   // For short time frames, we can accomodate raw data extraction.
   //
   const double rawLimit = 10.0 * 60.0;
   QEArchiveInterface::How how =
         (chartDuration >= rawLimit) ? this->archiveReadHow : QEArchiveInterface::Raw;

   // Depending on the mode, we actually request a bit more  before
   // and/or after the displayed window in order to cashe data for when
   // the operator pages forward or backwards.
   //
   // However we limit any extra size to at most one day.
   //
   const double aday = 24.0 * 60.0 * 60.0;
   double extra;
   switch (how) {
      case QEArchiveInterface::Raw:          extra = 0.0;                        break;
      case QEArchiveInterface::SpreadSheet:  extra = 0.0;                        break;
      case QEArchiveInterface::Averaged:     extra = MIN (aday, chartDuration);  break;
      case QEArchiveInterface::PlotBinning:  extra = 0.0;                        break;
      case QEArchiveInterface::Linear:       extra = MIN (aday, chartDuration);  break;
      default:                               extra = 0.0;                        break;
   }

   const QDateTime archiveStartDateTime = this->chart->getStartDateTime ().addSecs (-extra);
   const QDateTime archiveEndDateTime   = this->chart->getEndDateTime ().addSecs (+extra);

   // Doesn't apply to Plot_Binning which return up to the maximum
   // supported by archiver (currently 10K).
   //
   int numberPoints = MAXIMUM_HISTORY_POINTS;

   // Extract the array element index used to display this PV.
   // Go with zero for now.
   //
   int arrayIndex = 0;

   // Assign the chart widget message source id the the associated archive access object.
   // We re-assign just before each read in case it has changed.
   //
   this->archiveAccess.setMessageSourceId (this->chart->getMessageSourceId ());

   this->archiveAccess.readArchive
         (this, this->getPvName (), archiveStartDateTime, archiveEndDateTime,
          numberPoints, how, arrayIndex);
}

//------------------------------------------------------------------------------
//
void QEStripChartItem:: normalise () {
   // Just leverage off the context menu handler.
   //
   this->contextMenuSelected (QEStripChartNames::SCCM_SCALE_PV_AUTO);
}

//------------------------------------------------------------------------------
//
QColor QEStripChartItem::getColour ()
{
   return this->colour;
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::setColour (const QColor & colourIn)
{
   QString styleSheet;

   this->colour = colourIn;
   styleSheet =  QEUtilities::colourToStyle (this->colour);
   this->pvName->setStyleSheet (styleSheet);
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::highLight (bool isHigh)
{
   QString styleSheet;

   if (isHigh) {
      styleSheet =  QEUtilities::colourToStyle (clWhite);
   } else {
      styleSheet =  QEUtilities::colourToStyle (this->colour);
   }

   this->pvName->setStyleSheet (styleSheet);
}

//------------------------------------------------------------------------------
//
QPen QEStripChartItem::getPen ()
{
   QPen result (this->getColour ());

   switch (this->lineDrawMode) {
      case QEStripChartNames::ldmHide:
         result.setWidth (0);
         break;

      case QEStripChartNames::ldmRegular:
         result.setWidth (1);
         break;

      case QEStripChartNames::ldmBold:
         result.setWidth (2);
         break;

      default:
         result.setWidth (1);
         break;
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::pvNameDropEvent (QDropEvent *event)
{
   // If no text available, do nothing
   //
   if (!event->mimeData()->hasText ()){
      event->ignore ();
      return;
   }

   // Get the drop data
   //
   const QMimeData *mime = event->mimeData ();

   // If there is any text, drop the text

   if (!mime->text().isEmpty ()) {
      // Get the component textual parts
      //
      QStringList pieces = mime->text ().split (QRegExp ("\\s+"),
                                                QString::SkipEmptyParts);

      // Carry out the drop action
      // Assume only the first text part is of interest
      //
      this->setPvName (pieces [0], "");
   }

   // Tell the dropee that the drop has been acted on
   //
   if (event->source() == this) {
      event->setDropAction(Qt::CopyAction);
      event->accept();
   } else {
      event->acceptProposedAction ();
   }
}

//------------------------------------------------------------------------------
//
bool QEStripChartItem::eventFilter (QObject *obj, QEvent *event)
{
   const QEvent::Type type = event->type ();
   QMouseEvent* mouseEvent = NULL;
   bool selfDrop;

   switch (type) {

      case QEvent::MouseButtonDblClick:
         mouseEvent = static_cast<QMouseEvent *> (event);
         if (obj == this->pvName && (mouseEvent->button () == Qt::LeftButton)) {
            this->runSelectNameDialog (this->pvName);
            return true;  // we have handled double click
         }
         break;

      case QEvent::DragEnter:
         if (obj == this) {
            QDragEnterEvent* dragEnterEvent = static_cast<QDragEnterEvent*> (event);

            // Avoid self drops. Only allow drop if not own caLabel, not this QEStripChartItem
            // and not own chart.
            //
            selfDrop =
                  (dragEnterEvent->source() == this->caLabel) ||
                  (dragEnterEvent->source() == this) ||
                  (dragEnterEvent->source() == this->chart);

            // Can only drop if text and not in use.
            //
            if (dragEnterEvent->mimeData()->hasText () && !this->isInUse() && !selfDrop) {
               dragEnterEvent->setDropAction (Qt::CopyAction);
               dragEnterEvent->accept ();
               this->highLight (true);
            } else {
               dragEnterEvent->ignore ();
               this->chart->setAcceptDrops (false); // stop chart accepting this
               this->highLight (false);
            }
            return true;   // we have handled drag enter event
         }
         break;

      case QEvent::DragLeave:
         if (obj == this) {
            this->highLight (false);
            this->chart->evaluateAllowDrop ();   // allow drops if applicable
            return true;                         // we have handled drag leave event
         }
         break;


      case QEvent::Drop:
         if (obj == this) {
            QDropEvent* dropEvent = static_cast<QDropEvent*> (event);
            this->pvNameDropEvent (dropEvent);
            this->highLight (false);
            this->chart->evaluateAllowDrop ();   // allow drops if applicable
            return true;                         // we have handled drag drop event
         }
         break;

      default:
         // Just fall through
         break;
   }

   // we have not handled this event, pass to parent
   //
   return QWidget::eventFilter (obj, event);
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::writeTraceToFile ()
{
   QString filename;

   filename = QFileDialog::getSaveFileName
         (this, "Select output trace file", "./", "Text files(*.txt);;All files(*.*)");

   if (filename.isEmpty ()) {
      return;
   }

   QFile file (filename);
   if (!file.open (QIODevice::WriteOnly)) {
      qDebug() << "Could not open file " << filename;
      return;
   }

   QTextStream ts (&file);

   ts << "#   No  TimeStamp                     Relative Time    Value                Okay     Severity    Status\n";

   QCaDataPointList dataPoints = this->determinePlotPoints ();

   dataPoints.toStream (ts, true, true);
   file.close ();
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::generateStatistics ()
{
   qcaobject::QCaObject* qca = this->getQcaItem ();
   QString egu = qca ? qca->getEgu() : "";
   QCaDataPointList dataPoints = this->determinePlotPoints ();
   QEStripChartStatistics* pvStatistics;

   // Create new statistic widget.
   //
   pvStatistics = new QEStripChartStatistics (this->getPvName (), egu, dataPoints, this, NULL);

   // Scale statistics widget to current application scaling.
   //
   QEScaling::applyToWidget (pvStatistics);

   if (this->hostSlotAvailable) {
      // Create component item and associated request.
      //
      componentHostListItem item (pvStatistics, QEActionRequests::OptionFloatingDockWindow , false, this->getPvName () + " Statistics");

      // ... and request this hosted by the support application.
      //
      emit requestAction (QEActionRequests (item));

   } else {
      // Just show it.
      //
      pvStatistics->setWindowTitle (this->getPvName () + " Statistics");
      pvStatistics->show ();
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::runSelectNameDialog (QWidget* control)
{
   int n;

   this->chart->pvNameSelectDialog->setPvName (this->getPvName ());
   n = this->chart->pvNameSelectDialog->exec (control ? control : this);
   if (n == 1) {
      // User has selected okay.
      //
      if (this->getPvName () != this->chart->pvNameSelectDialog->getPvName ()) {
         this->setPvName (this->chart->pvNameSelectDialog->getPvName (), "");
      }
      // and replot the data
      //
      this->chart->setReplotIsRequired ();
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::letterButtonClicked (bool)
{
   QWidget* from = dynamic_cast <QWidget*> (sender ());
   this->runSelectNameDialog (from);
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::contextMenuRequested (const QPoint & pos)
{
   QPoint tempPos;
   QPoint golbalPos;

   tempPos = pos;
   tempPos.setY (-2);   // always align same wrt top of label
   golbalPos = this->mapToGlobal (tempPos);

   if (this->isInUse()) {
      // Ensure menu status reflects the current state.
      //
      this->inUseMenu->setUseReceiveTime (this->getUseReceiveTime ());
      this->inUseMenu->setArchiveReadHow (this->getArchiveReadHow ());
      this->inUseMenu->setLineDrawMode (this->getLineDrawMode ());
      this->inUseMenu->setLinePlotMode (this->getLinePlotMode ());
      this->inUseMenu->exec (golbalPos, 0);
   } else {
      this->emptyMenu->setPredefinedNames (chart->getPredefinedPVNameList ());
      this->emptyMenu->exec (golbalPos, 0);
   }
}


//------------------------------------------------------------------------------
//
void QEStripChartItem::contextMenuSelected (const QEStripChartNames::ContextMenuOptions option)
{
   QEDisplayRanges range;
   double min, max;
   double midway;
   bool status;
   int n;

   switch (option) {

      case QEStripChartNames::SCCM_READ_ARCHIVE:
         this->readArchive();
         break;

      case  QEStripChartNames::SCCM_SCALE_CHART_AUTO:
         range = this->getLoprHopr (true);
         status = range.getMinMax(min, max);
         if (status) {
            this->chart->setYRange (min, max);
         }
         break;

      case QEStripChartNames::SCCM_SCALE_CHART_PLOTTED:
         range = this->getDisplayedMinMax (true);
         status = range.getMinMax(min, max);
         if (status) {
            this->chart->setYRange (min, max);
         }
         break;

      case QEStripChartNames::SCCM_SCALE_CHART_BUFFERED:
         range = this->getBufferedMinMax (true);
         status = range.getMinMax(min, max);
         if (status) {
            this->chart->setYRange (min, max);
         }
         break;


      case QEStripChartNames::SCCM_SCALE_PV_RESET:
         this->scaling.reset ();
         this->setCaption ();
         this->chart->setReplotIsRequired ();
         break;


      case QEStripChartNames::SCCM_SCALE_PV_GENERAL:
         this->adjustPVDialog->setSupport (this->chart->getYMinimum (),
                                           this->chart->getYMaximum (),
                                           this->getLoprHopr(false),
                                           this->getDisplayedMinMax(false),
                                           this->getBufferedMinMax(false));

         this->adjustPVDialog->setValueScaling (this->scaling);
         n = this->adjustPVDialog->exec (this);
         if (n == 1) {
            // User has selected okay.
            this->scaling.assign (this->adjustPVDialog->getValueScaling ());
            this->setCaption ();
            this->chart->setReplotIsRequired ();
         }
         break;

      case QEStripChartNames::SCCM_SCALE_PV_AUTO:
         range = this->getLoprHopr (false);
         status = range.getMinMax (min, max);
         if (status) {
            this->scaling.map (min, max, this->chart->getYMinimum (), this->chart->getYMaximum ());
            this->setCaption ();
            this->chart->setReplotIsRequired ();
         }
         break;

      case QEStripChartNames::SCCM_SCALE_PV_PLOTTED:
         range = this->getDisplayedMinMax (false);
         status = range.getMinMax (min, max);
         if (status) {
            this->scaling.map (min, max, this->chart->getYMinimum (), this->chart->getYMaximum ());
            this->setCaption ();
            this->chart->setReplotIsRequired ();
         }
         break;

      case QEStripChartNames::SCCM_SCALE_PV_BUFFERED:
         range = this->getBufferedMinMax (false);
         status = range.getMinMax (min, max);
         if (status) {
            this->scaling.map (min, max, this->chart->getYMinimum (), this->chart->getYMaximum ());
            this->setCaption ();
            this->chart->setReplotIsRequired ();
         }
         break;

      case QEStripChartNames::SCCM_SCALE_PV_CENTRE:
         if (this->firstPointIsDefined) {
            midway = (chart->getYMinimum () + this->chart->getYMaximum () ) / 2.0;
            this->scaling.set (this->firstPoint.value, 1.0, midway);
            this->setCaption ();
            this->chart->setReplotIsRequired ();
         }
         break;

      case QEStripChartNames::SCCM_LINE_COLOUR:
         this->colourDialog->setCurrentColor (this->getColour ());
         this->colourDialog->open (this, SLOT (setColour (const QColor &)));
         break;

      case QEStripChartNames::SCCM_PV_ADD_NAME:
      case QEStripChartNames::SCCM_PV_EDIT_NAME:
         this->runSelectNameDialog (this->pvName);
         break;

      case QEStripChartNames::SCCM_PV_PASTE_NAME:
         {
            QClipboard *cb = QApplication::clipboard ();
            QString pasteText = cb->text().trimmed();

            if (! pasteText.isEmpty()) {
               this->setPvName (pasteText, "");
            }
         }
         break;

      case QEStripChartNames::SCCM_PV_WRITE_TRACE:
         this->writeTraceToFile ();
         break;

      case QEStripChartNames::SCCM_PV_STATS:
         this->generateStatistics ();
         break;

      case QEStripChartNames::SCCM_ADD_TO_PREDEFINED:
         this->chart->addToPredefinedList (this->getPvName ());
         break;

      case QEStripChartNames::SCCM_PREDEFINED_01:
      case QEStripChartNames::SCCM_PREDEFINED_02:
      case QEStripChartNames::SCCM_PREDEFINED_03:
      case QEStripChartNames::SCCM_PREDEFINED_04:
      case QEStripChartNames::SCCM_PREDEFINED_05:
      case QEStripChartNames::SCCM_PREDEFINED_06:
      case QEStripChartNames::SCCM_PREDEFINED_07:
      case QEStripChartNames::SCCM_PREDEFINED_08:
      case QEStripChartNames::SCCM_PREDEFINED_09:
      case QEStripChartNames::SCCM_PREDEFINED_10:
         n = option - QEStripChartNames::SCCM_PREDEFINED_01;
         this->setPvName (chart->getPredefinedItem (n), "");
         break;

      case QEStripChartNames::SCCM_PV_CLEAR:
         this->clear ();
         this->chart->evaluateAllowDrop ();   // move to strip chart proper??
         break;

      case QEStripChartNames::SCCM_PLOT_SERVER_TIME:
         this->useReceiveTime = false;
         break;

      case QEStripChartNames::SCCM_PLOT_CLIENT_TIME:
         this->useReceiveTime = true;
         break;

      case QEStripChartNames::SCCM_PLOT_RECTANGULAR:
         this->linePlotMode = QEStripChartNames::lpmRectangular;
         this->chart->setReplotIsRequired ();
         break;

      case QEStripChartNames::SCCM_PLOT_SMOOTH:
         this->linePlotMode = QEStripChartNames::lpmSmooth;
         this->chart->setReplotIsRequired ();
         break;

      case QEStripChartNames::SCCM_ARCH_LINEAR:
         this->archiveReadHow = QEArchiveInterface::Linear;
         break;

      case QEStripChartNames::SCCM_ARCH_PLOTBIN:
         this->archiveReadHow = QEArchiveInterface::PlotBinning;
         break;

      case QEStripChartNames::SCCM_ARCH_RAW:
         this->archiveReadHow = QEArchiveInterface::Raw;
         break;

      case QEStripChartNames::SCCM_ARCH_SHEET:
         this->archiveReadHow = QEArchiveInterface::SpreadSheet;
         break;

      case QEStripChartNames::SCCM_ARCH_AVERAGED:
         this->archiveReadHow = QEArchiveInterface::Averaged;
         break;

      case QEStripChartNames::SCCM_LINE_HIDE:
         this->lineDrawMode = QEStripChartNames::ldmHide;
         this->chart->setReplotIsRequired ();
         break;

      case QEStripChartNames::SCCM_LINE_REGULAR:
         this->lineDrawMode = QEStripChartNames::ldmRegular;
         this->chart->setReplotIsRequired ();
         break;

      case QEStripChartNames::SCCM_LINE_BOLD:
         this->lineDrawMode = QEStripChartNames::ldmBold;
         this->chart->setReplotIsRequired ();
         break;


      default:
         DEBUG << int (option) << this->pvName->text () << "tbd";
   }

   // Set tool top depending on current scaling.
   //
   if (this->scaling.isScaled()) {
      this->pvName->setToolTip (scaledTip);
   } else {
      this->pvName->setToolTip (regularTip);
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::saveConfiguration (PMElement & parentElement)
{
   // Any config data to save?
   //
   if (this->isInUse ()) {
      PMElement pvElement = parentElement.addElement ("PV");
      pvElement.addAttribute ("slot", (int) this->slot);

      // Note: we save the actual, i.e. substituted, PV name.
      //
      pvElement.addValue ("Name", this->getPvName ());

      // Save any scaling.
      //
      this->scaling.saveConfiguration (pvElement);
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::restoreConfiguration (PMElement& parentElement)
{
   QString pvName;
   bool status;

   this->clear();

   PMElement pvElement = parentElement.getElement ("PV", "slot", (int) this->slot);
   if (pvElement.isNull ()) return;

   // Attempt to extract a PV name
   //
   status = pvElement.getValue ("Name", pvName);
   if (status) {
      this->setPvName (pvName, "");
      this->scaling.restoreConfiguration (pvElement);
      this->setCaption ();
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartItem::updateMenu (QAction* action, const bool inUseMenu)
{
   if(!action) return;

   if (inUseMenu) {
      action->setParent (this->inUseMenu);
      this->inUseMenu->addAction (action);
   } else {
      action->setParent (this->emptyMenu);
      this->emptyMenu->addAction (action);
   }
}

// end
