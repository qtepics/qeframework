/*  QEStripChart.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEStripChart.h"
#include "QEStripChartToolBar.h"
#include "QEStripChartItem.h"

#include <math.h>

#include <QBrush>
#include <QCursor>
#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QFont>
#include <QIcon>
#include <QLabel>
#include <QList>
#include <QMutex>
#include <QPen>
#include <QPushButton>
#include <QScrollArea>
#include <QStringList>
#include <QToolButton>
#include <QApplication>
#include <QClipboard>

#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <QEGraphicNames.h>
#include <QEGraphic.h>

#include <alarm.h>

#include <QEAdaptationParameters.h>
#include <QECommon.h>
#include <QCaObject.h>
#include <QELabel.h>
#include <QCaVariableNamePropertyManager.h>

#define DEBUG  qDebug () << "QEStripChart" << __LINE__ << __FUNCTION__ << "  "

static const QColor clWhite (0xFF, 0xFF, 0xFF, 0xFF);
static const QColor clBlack (0x00, 0x00, 0x00, 0xFF);

#define PV_DELTA_HEIGHT    18
#define PV_FRAME_HEIGHT    (8 + ((NUMBER_OF_PVS + 1) / 2) * PV_DELTA_HEIGHT)
#define PV_SCROLL_HEIGHT   (PV_FRAME_HEIGHT + 6)

// required height for n PVs.
//
#define PV_ITEM_HEIGHT(n)  (8 + ((int(n) + 1)/ 2) * 19 + 6)

// default height is for ten (as opposed to 16) PVs.
//
#define PV_DEFAULT_HEIGHT  PV_ITEM_HEIGHT(10)

#define MINIMUM_SPAN   1.0E-12    // Absolute min y range
#define MINIMUM_RATIO  1.0E-6     // Min relative range, e.g. 1000000 to 1000001

// We use a shared time for all QEStripCharts.
//
QTimer* QEStripChart::tickTimer = NULL;


//==============================================================================
// Local support classes.
//==============================================================================
//
//==============================================================================
//
class QEPVNameLists : public QStringList {
public:
   static void constructor ();   // idempotent - constucts a singleton class

   void prependOrMoveToFirst (const QString & item);
   void saveConfiguration (PMElement & parentElement);
   void restoreConfiguration (PMElement & parentElement);
private:
   QMutex *mutex;
   int predefinedCount;
   explicit QEPVNameLists ();
   virtual ~QEPVNameLists ();
};

//------------------------------------------------------------------------------
//
QEPVNameLists::QEPVNameLists ()
{
   this->mutex = new QMutex ();
   this->predefinedCount = 0;
}

//------------------------------------------------------------------------------
//
QEPVNameLists::~QEPVNameLists ()
{
   delete this->mutex;
}

//------------------------------------------------------------------------------
//
void QEPVNameLists::prependOrMoveToFirst (const QString& item)
{
   QMutexLocker locker (this->mutex);

   // Is item already in the list?
   //
   const int posn = this->indexOf (item, 0);
   if (posn < 0) {
      // Not in list
      // Ensure insert postiton is not out of bounds - Qt6 can't cope.
      //
      const int insertHere = MIN (this->predefinedCount, this->count());
      this->insert (insertHere, item);

   } else if (posn > this->predefinedCount) {
      // item in list - move to front if not predefined.
#if QT_VERSION < 0x060000
      this->swap (this->predefinedCount, posn);
#else
      this->swapItemsAt (this->predefinedCount, posn);
#endif
   }
   // else posn in range >=0 to <=predefined - nothing to do.
   // Either predefined or already in top undefined slot.

   if (this->count () > QEStripChartNames::NumberPrefefinedItems) {
      this->removeLast ();
   }
}

//------------------------------------------------------------------------------
//
void QEPVNameLists::saveConfiguration (PMElement& parentElement)
{
   PMElement predefinedElement = parentElement.addElement ("Predefined");

   const int number = this->count ();
   predefinedElement.addAttribute ("Number", number);
   for (int j = 0; j < number; j++) {
      PMElement pvElement = predefinedElement.addElement ("PV");
      pvElement.addAttribute ("id", j);
      pvElement.addValue ("Name", this->value (j));
   }

}

//------------------------------------------------------------------------------
//
void QEPVNameLists::restoreConfiguration (PMElement& parentElement)
{
   PMElement predefinedElement = parentElement.getElement ("Predefined");
   int number;

   if (predefinedElement.isNull ()) return;

   const bool status = predefinedElement.getAttribute ("Number", number);
   if (status) {
      this->clear ();
      this->predefinedCount = 0;   // must be <= the number in the list

      // Read in reverse order (as use insert into list with prependOrMoveToFirst).
      //
      for (int j = number - 1; j >= 0; j--) {
         PMElement pvElement = predefinedElement.getElement ("PV", "id", j);

         if (pvElement.isNull ()) continue;

         QString pvName;
         const bool status = pvElement.getValue ("Name", pvName);
         if (status) {
            this->prependOrMoveToFirst (pvName);
         }
      }
   }
}

//------------------------------------------------------------------------------
// This is a static list shared amongst all instances of the strip chart widget.
//
static QEPVNameLists* predefinedPVNameList = NULL;

void QEPVNameLists::constructor () {
   // Construct common object if needs be.
   //
   if (!predefinedPVNameList) {
      predefinedPVNameList = new QEPVNameLists ();

      QEAdaptationParameters ap ("QE_");
      QString predefined = ap.getString ("stripchart_predefined_pvs", "");

      // Split input string using space as delimiter.
      // Could extend to use regular expression and split on any white space character.
      //
      QStringList pvNameList = QEUtilities::split (predefined);

      // Processin reverse order (as use insert into list with prependOrMoveToFirst).
      // We don't use append as this do not check for duplicates.
      //
      int number = pvNameList.count();
      for (int j = number - 1; j >= 0; j--) {
         QString pvName = pvNameList.value(j);
         if (!pvName.isEmpty()) {
            predefinedPVNameList->prependOrMoveToFirst (pvName);
         }
      }
      predefinedPVNameList->predefinedCount = predefinedPVNameList->count ();
   }
}


//==============================================================================
// QEStripChart class functions
//==============================================================================
//
void QEStripChart::createInternalWidgets ()
{
   // Create dialog.
   // We have one dialog per strip chart (as opposed to per pv item) as this not only saves
   // resources, but a single dialog will remember filter and other state information.
   //
   this->pvNameSelectDialog = new QEPVNameSelectDialog (this);

   // Create tool bar frame and tool buttons.
   //
   this->toolBar = new QEStripChartToolBar (); // this will become parented by toolBarResize

   // Connect various tool bar signals to the chart.
   //
   QObject::connect (this->toolBar, SIGNAL (stateSelected  (const QEStripChartNames::StateModes)),
                     this,          SLOT   (stateSelected  (const QEStripChartNames::StateModes)));

   QObject::connect (this->toolBar, SIGNAL (videoModeSelected  (const QE::VideoModes)),
                     this,          SLOT   (videoModeSelected  (const QE::VideoModes)));

   QObject::connect (this->toolBar, SIGNAL (yScaleModeSelected  (const QEStripChartNames::YScaleModes)),
                     this,          SLOT   (yScaleModeSelected  (const QEStripChartNames::YScaleModes)));

   QObject::connect (this->toolBar, SIGNAL (yRangeSelected  (const QEStripChartNames::ChartYRanges)),
                     this,          SLOT   (yRangeSelected  (const QEStripChartNames::ChartYRanges)));

   QObject::connect (this->toolBar, SIGNAL (durationSelected  (const int)),
                     this,          SLOT   (durationSelected  (const int)));

   QObject::connect (this->toolBar, SIGNAL (selectDuration ()),
                     this,          SLOT   (selectDuration ()));

   QObject::connect (this->toolBar, SIGNAL (playModeSelected  (const QEStripChartNames::PlayModes)),
                     this,          SLOT   (playModeSelected  (const QEStripChartNames::PlayModes)));

   QObject::connect (this->toolBar, SIGNAL (timeZoneSelected (const Qt::TimeSpec)),
                     this,          SLOT   (timeZoneSelected (const Qt::TimeSpec)));

   QObject::connect (this->toolBar, SIGNAL (writeAllSelected  ()),
                     this,          SLOT   (writeAllSelected  ()));

   QObject::connect (this->toolBar, SIGNAL (readArchiveSelected  ()),
                     this,          SLOT   (readArchiveSelected  ()));

   QObject::connect (this->toolBar, SIGNAL (loadSelectedFile (const QString&)),
                     this,          SLOT   (loadNamedWidetConfiguration (const QString&)));

   QObject::connect (this->toolBar, SIGNAL (loadSelected ()),
                     this,          SLOT   (loadWidgetConfiguration ()));

   QObject::connect (this->toolBar, SIGNAL (saveAsSelected ()),
                     this,          SLOT   (saveWidgetConfiguration ()));


   // Create user controllable resize area
   //
   this->toolBarResize = new QEResizeableFrame (QEResizeableFrame::BottomEdge, 8, 8 + this->toolBar->designHeight (), this);
   this->toolBarResize->setFixedHeight (8 + this->toolBar->designHeight ());
   this->toolBarResize->setFrameShape (QFrame::Panel);
   this->toolBarResize->setGrabberToolTip ("Re size tool bar display area");
   this->toolBarResize->setWidget (this->toolBar);

   // Create PV frame and PV name labels and associated CA labels.
   //
   this->pvFrame = new QFrame ();  // this will become parented by pvScrollArea
   this->pvFrame->setFixedHeight (PV_FRAME_HEIGHT);

   this->pvGridLayout = new QGridLayout (this->pvFrame);
   this->pvGridLayout->setContentsMargins (2, 2, 2, 2);
   this->pvGridLayout->setHorizontalSpacing (8);
   this->pvGridLayout->setVerticalSpacing (2);

   // Create widgets (parented by chart) and chart item that manages these.
   //
   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem* chartItem  = new QEStripChartItem (this, slot, this->pvFrame);

      // Add to grid.
      //
      this->pvGridLayout->addWidget (chartItem, slot / 2, slot %2);
      this->items [slot] = chartItem;
   }

   // Create scrolling area and add pv frame.
   //
   this->pvScrollArea = new QScrollArea ();          // this will become parented by pvResizeFrame
   this->pvScrollArea->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
   this->pvScrollArea->setWidgetResizable (true);    // MOST IMPORTANT
   this->pvScrollArea->setWidget (this->pvFrame);

   // Create user controllable resize area
   //
   this->pvResizeFrame = new QEResizeableFrame (QEResizeableFrame::BottomEdge, 18, PV_SCROLL_HEIGHT + 8, this);
   this->pvResizeFrame->setFixedHeight (PV_DEFAULT_HEIGHT + 8);
   this->pvResizeFrame->setFrameShape (QFrame::Panel);
   this->pvResizeFrame->setGrabberToolTip ("Re size PV display area");
   this->pvResizeFrame->setWidget (this->pvScrollArea);

   // Create plotting frame and plot area.
   //
   this->plotFrame = new QFrame (this);
   this->plotFrame->setFrameShape (QFrame::Panel);

   this->plotArea = new QEGraphic (this->plotFrame);
   this->plotArea->installCanvasEventFilter (this);

   // Select the markups available on the strip chart.
   //
   this->plotArea->setAvailableMarkups
         (QEGraphicNames::Area | QEGraphicNames::Line | QEGraphicNames::Box |
          QEGraphicNames::VerticalLine_1 | QEGraphicNames::VerticalLine_2 |
          QEGraphicNames::HorizontalLine_1 | QEGraphicNames::HorizontalLine_2 |
          QEGraphicNames::HorizontalLine_3 | QEGraphicNames::HorizontalLine_4);

   // Set up the initial markup positions.
   //
   this->plotArea->setMarkupPosition (QEGraphicNames::HorizontalLine_1, QPointF (0.0,  5.0));
   this->plotArea->setMarkupPosition (QEGraphicNames::HorizontalLine_2, QPointF (0.0, 10.0));
   this->plotArea->setMarkupPosition (QEGraphicNames::HorizontalLine_3, QPointF (0.0, 15.0));
   this->plotArea->setMarkupPosition (QEGraphicNames::HorizontalLine_4, QPointF (0.0, 20.0));

   this->plotArea->setMarkupPosition (QEGraphicNames::VerticalLine_1, QPointF (-10.0, 0));
   this->plotArea->setMarkupPosition (QEGraphicNames::VerticalLine_2, QPointF ( -5.0, 0));

   QObject::connect (this->plotArea, SIGNAL (mouseMove     (const QPointF&)),
                     this,           SLOT   (plotMouseMove (const QPointF&)));

   QObject::connect (this->plotArea, SIGNAL (markupMove     (const QEGraphicNames::Markups, const QPointF&)),
                     this,           SLOT   (markupMove     (const QEGraphicNames::Markups, const QPointF&)));

   QObject::connect (this->plotArea, SIGNAL (wheelRotate   (const QPointF&, const int)),
                     this,           SLOT   (zoomInOut     (const QPointF&, const int)));

   QObject::connect (this->plotArea, SIGNAL (areaDefinition (const QPointF&, const QPointF&)),
                     this,           SLOT   (scaleSelect    (const QPointF&, const QPointF&)));

   QObject::connect (this->plotArea, SIGNAL (lineDefinition (const QPointF&, const QPointF&)),
                     this,           SLOT   (lineSelected   (const QPointF&, const QPointF&)));

   // Create layouts.
   //
   this->layout1 = new QVBoxLayout (this);
   this->layout1->setContentsMargins (4, 4, 4, 4);
   this->layout1->setSpacing (4);
   this->layout1->addWidget (this->toolBarResize);
   this->layout1->addWidget (this->pvResizeFrame);
   this->layout1->addWidget (this->plotFrame);

   this->layout2 = new QVBoxLayout (this->plotFrame);
   this->layout2->setContentsMargins (4, 4, 4, 4);
   this->layout2->setSpacing (4);
   this->layout2->addWidget (this->plotArea);

   // Use default context menu.
   //
   this->setupContextMenu ();

   // Clear / initialise plot.
   //
   this->chartYScale = QEStripChartNames::dynamic;
   this->yScaleMode = QEStripChartNames::linear;
   this->chartTimeMode = QEStripChartNames::tmRealTime;
   this->timeScale = 1.0;
   this->timeUnits = "secs";

   this->setNormalBackground (true);
}

//------------------------------------------------------------------------------
//
QEStripChartItem* QEStripChart::getItem (const int slot) const
{
   return ((slot >= 0) && (slot < NUMBER_OF_PVS)) ? this->items [slot] : NULL;
}

//------------------------------------------------------------------------------
//
void QEStripChart::setNormalBackground (const bool isNormalVideoIn)
{
   QColor background;
   QRgb gridColour;
   QPen pen;

   this->isNormalVideo = isNormalVideoIn;

   background = this->isNormalVideo ? clWhite : clBlack;
   this->plotArea->setBackgroundColour (background);

   gridColour =  this->isNormalVideo ? 0x00c0c0c0 : 0x00404040;
   pen.setColor(QColor (gridColour));
   pen.setStyle (Qt::DashLine);
   this->plotArea->setGridPen (pen);

   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEStripChart::calcDisplayMinMax ()
{
   QEDisplayRanges tr;
   double min;
   double max;

   if (this->chartYScale == QEStripChartNames::manual) return;

   tr.clear ();

   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem* item = this->getItem (slot);
      if (item && (item->isInUse() == true)) {
         switch (this->chartYScale) {
            case QEStripChartNames::operatingRange:  tr.merge (item->getLoprHopr (true));         break;
            case QEStripChartNames::plotted:         tr.merge (item->getDisplayedMinMax (true));  break;
            case QEStripChartNames::buffered:        tr.merge (item->getBufferedMinMax (true));   break;
            case QEStripChartNames::dynamic:         tr.merge (item->getDisplayedMinMax (true));  break;
            default:       DEBUG << "Well this is unexpected"; return; break;
         }
      }
   }

   if (tr.getMinMax (min, max) == true) {
      this->yMinimum = min;

      const double temp1 = min + MINIMUM_SPAN;
      const double temp2 = min + ABS (min) * MINIMUM_RATIO;
      const double minMax = MAX (temp1, temp2);

      this->yMaximum = MAX (max, minMax);
   } // else do not change.
}


//------------------------------------------------------------------------------
//
const QCaDataPoint* QEStripChart::findNearestPoint (const QPointF& posn,
                                                    int& slotOut) const
{
   const QCaDateTime end_time = this->getEndDateTime ();
   const QCaDataPoint* result = NULL;

   slotOut = -1;

   // Convert cursor x to absolute cursor time.
   // x is the time (in seconds) relative to the chart end time.
   //
   const QCaDateTime searchTime = this->timeAt (posn.x ());

   int closest = 0x7FFFFFFF;
   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem* item = this->getItem (slot);
      if (item && (item->isInUse () == true)) {
         const QCaDataPoint* nearest = item->findNearestPoint (searchTime);
         if (nearest) {
            // write a functions (t, y) <==>  QCaDataPoint
            const QPointF nearestPoint = item->dataPointToReal (*nearest);
            const QPoint difference = this->plotArea->pixelDistance (posn, nearestPoint);

            // Close enough to even be considered.
            // Note: 4 is the box half size when plotted.
            //
            if (ABS (difference.x ()) > 4)  continue;
            if (ABS (difference.y ()) > 4)  continue;

            // Closer than any previous found point?
            //
            int distance = difference.x ()*difference.x ()  +
                           difference.y ()*difference.y ();
            if (distance < closest) {
               closest = distance;
               slotOut = slot;
               result = nearest;
            }
         }
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEStripChart::doCurrentValueCalculations ()
{
   const QCaDateTime datetime = QCaDateTime::currentDateTime ().toUTC ();

   QEStripChartItem::CalcInputs values;
   bool okay;

   // First initialise all values - undefined artefacts yield zero.
   //
   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      values [slot] = 0.0;
   }

   // Extract non-calculated values. Don't allow use of previous calculated values.
   //
   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem* item = this->getItem (slot);
      if (item && item->isPvData ()) {
         double t = item->getCurrentValue (okay);
         if (okay) values [slot] = t;
      }
   }

   // Do calculations and back fill calculated values.
   // Note: Calculations can only use PV values and already calculated values.
   //
   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem* item = this->getItem (slot);
      if (item && item->isCalculation ()) {
         item->calculateAndUpdate (datetime, values);
         double t = item->getCurrentValue (okay);
         if (okay) values [slot] = t;
      }
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::plotData ()
{
   const double oneDay = 86400.0;  // in seconds

   double d;
   QPen pen;
   QDateTime dt;
   QString zoneTLA;

   // First release any/all allocated curves.
   //
   this->plotArea->releaseCurves ();

   d = this->getDuration ();
   if (d <= 1.0) {
      this->timeScale = 0.001;
      this->timeUnits = "mSec";
   } else if (d <= 60.0) {           // <= a minute
      this->timeScale = 1.0;
      this->timeUnits = "secs";
   } else if (d <= 3600.0) {         // <= an hour
      this->timeScale = 60.0;
      this->timeUnits = "mins";
   } else if (d <= oneDay) {         // <= a day
      this->timeScale = 3600.0;
      this->timeUnits = "hrs";
   } else  if (d <= 100.0*oneDay) {  // <= a 100 days
      this->timeScale = oneDay;
      this->timeUnits = "days";
   } else {
      this->timeScale = 7.0*oneDay;
      this->timeUnits = "weeks";
   }

   // Get embedded canvas geometry and draw in time units.
   // Maybe we could draw "on top of" axis.
   //
   QRect canGeo = this->plotArea->getEmbeddedQwtPlot ()->canvas()->geometry();
   QPoint pixpos = QPoint (canGeo.width()/2, canGeo.height () - 10);

   pen.setColor (this->isNormalVideo ? clBlack : clWhite);
   pen.setStyle (Qt::SolidLine);
   pen.setWidth (1);

   this->plotArea->setCurvePen (pen); // current curev pen used for text.
   this->plotArea->setTextPointSize (8);
   this->plotArea->drawText (pixpos, this->timeUnits, QEGraphicNames::PixelPosition, true);

   this->plotArea->setXScale (1.0 / this->timeScale);
   this->plotArea->setXLogarithmic (false);
   this->plotArea->setYLogarithmic (this->yScaleMode == QEStripChartNames::log);

   // Update the plot for each PV.
   // Allocate curve and call curve-setSample/setData.
   //
   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      if (this->getItem (slot)->isInUse ()) {
         this->getItem (slot)->plotData ();
      }
   }

   if (this->chartYScale == QEStripChartNames::dynamic) {
      // Re-calculate chart range.
      //
      this->calcDisplayMinMax ();
   }

   this->plotArea->setYRange (this->getYMinimum (), this->getYMaximum (),
                              QEGraphicNames::SelectBySize, 40, false);
   this->plotArea->setXRange (-d/this->timeScale, 0.0,
                              QEGraphicNames::SelectByValue, 5, false);

   if (this->plotArea->getMarkupEnabled (QEGraphicNames::Box)) {
      QEStripChartItem* item = this->getItem (this->selectedPointSlot);
      if (item) {
         QCaDataPoint nearest;
         nearest.datetime = this->selectedPointDateTime;
         nearest.value = this->selectedPointValue;
         this->plotArea->setMarkupPosition (QEGraphicNames::Box, item->dataPointToReal(nearest));
      }
   }

   this->plotArea->replot ();

   QString format = "yyyy-MM-dd hh:mm:ss";
   QString times = " ";

   dt = this->getStartDateTime ().toTimeSpec (this->timeZoneSpec);
   zoneTLA = QEUtilities::getTimeZoneTLA (this->timeZoneSpec, dt);

   times.append (dt.toString (format)).append (" ").append (zoneTLA);
   times.append (" to ");

   dt = this->getEndDateTime ().toTimeSpec (this->timeZoneSpec);
   zoneTLA = QEUtilities::getTimeZoneTLA (this->timeZoneSpec, dt);

   times.append (dt.toString (format)).append (" ").append (zoneTLA);

   // update tool bar status fields
   //
   this->toolBar->setTimeStatus (times);

   QString durationImage = QEUtilities::intervalToString ((double)this->getDuration (), 0, true);
   this->toolBar->setDurationStatus (durationImage);

   this->toolBar->setYRangeStatus (this->chartYScale);
   this->toolBar->setTimeModeStatus (this->chartTimeMode);

   this->markupMove (QEGraphicNames::VerticalLine_1);  // force update (for real time)
   this->markupMove (QEGraphicNames::HorizontalLine_1);
   this->markupMove (QEGraphicNames::HorizontalLine_3);

   // Last - clear flag.
   //
   this->replotIsRequired = false;
}

//------------------------------------------------------------------------------
//
void QEStripChart::scaleSelect (const QPointF& start, const QPointF& finish)
{
   QPoint distance = this->plotArea->pixelDistance (start, finish);

   // The QEGraphic validates the selection, i.e. that user has un-ambiguously
   // selected x (time) scaling or y scaling. Need only figure out which one.
   //
   if (ABS (distance.y ()) >=  ABS (distance.x ())) {
      // Makeing a Y scale adjustment.
      //
      this->setYRange (finish.y (), start.y ());
      this->pushState ();

   } else  {
      // Makeing a time scale adjustment.
      //
      double dt;
      int duration;
      QDateTime et;
      QDateTime now;

      dt = finish.x () - start.x ();
      duration = MAX (1, int (dt));

      et = this->getEndDateTime ().addSecs (finish.x ());

      now = QDateTime::currentDateTime ();
      if (et >= now) {
         // constrain
         et = now;
      } else {
         this->chartTimeMode = QEStripChartNames::tmHistorical;
      }

      this->setDuration (duration);
      this->setEndDateTime (et);
      this->pushState ();

   }
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEStripChart::lineSelected (const QPointF&, const QPointF&)
{
   // no action per se - just request a replot (without the line).
   //
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEStripChart::setReadOut (const QString & text)
{
   message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STATUS);
   this->sendMessage (text, mt);
}

//------------------------------------------------------------------------------
//
QDateTime QEStripChart::timeAt (const double x) const
{
   const qint64 mSec = qint64 (1000.0 * x);
   QDateTime result = this->getEndDateTime ().toTimeSpec (this->timeZoneSpec);
   result = result.addMSecs (mSec);
   return result;
}

//----------------------------------------------------------------------------
//
bool QEStripChart::eventFilter (QObject* watched, QEvent* event)
{
   const QEvent::Type type = event->type ();
   QMouseEvent* mouseEvent = NULL;

   bool result = false;

   switch (type) {
      case QEvent::MouseButtonPress:
         if (this->plotArea->isCanvasObject (watched)) {
            mouseEvent = static_cast<QMouseEvent *> (event);
            if (mouseEvent->buttons() & Qt::RightButton) {
               // The right (alternate) button has been pressed - are we currently
               // hovering over a data point?
               //
               if (this->plotArea->getMarkupVisible (QEGraphicNames::Box)) {
                  this->plotArea->setMarkupEnabled (QEGraphicNames::Box, true);
                  this->replotIsRequired = true;
               }
            }
         }
         result = false;
         break;

      case QEvent::MouseButtonRelease:
         if (this->plotArea->isCanvasObject (watched)) {
            mouseEvent = static_cast<QMouseEvent *> (event);
            if (!(mouseEvent->buttons() & Qt::RightButton)) {
               // Button released, right no lonlger pressed.
               //
               if (this->plotArea->getMarkupEnabled (QEGraphicNames::Box)) {
                  this->plotArea->setMarkupEnabled (QEGraphicNames::Box, false);
                  this->replotIsRequired = true;
               }
            }
         }
         result = false;
         break;

      default:
         result = false;
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEStripChart::plotMouseMove  (const QPointF& position)
{
   static const QString format = "ddd yyyy-MM-dd hh:mm:ss.zzz";

   QString zoneTLA;
   QString mouseReadOut;
   QString f;   // temp string
   QPointF slope;

   // Convert cursor x to absolute cursor time.
   // x is the time (in seconds) relative to the chart end time.
   //
   const QDateTime t = this->timeAt (position.x ());

   mouseReadOut = "Time: ";

   // Keep only most significant digit of the milli-seconds,
   // i.e. tenths of a second.
   //
   f = t.toString (format).left (format.length() - 2);
   mouseReadOut.append (f);

   zoneTLA = QEUtilities::getTimeZoneTLA (this->timeZoneSpec, t);
   mouseReadOut.append (" ").append (zoneTLA);

   // Show relative time form end of chart in days hours, mins and seconds.
   //
   mouseReadOut.append ("    ");
   f = QEUtilities::intervalToString (position.x(), 1, true);
   mouseReadOut.append (f);

   // Show y value associated with current cursor position.
   //
   f = QString::asprintf ("    Value: %+.10g", position.y ());
   mouseReadOut.append (f);

   // Is the line markup "on show"?
   //
   if (this->plotArea->getSlopeIsDefined (slope)) {
      const double dt = slope.x ();
      const double dy = slope.y ();

      // Calc delta time precision
      //
      int prec;
      if (dt < 1.0) {
         prec = 3;
      } else if (dt < 10.0) {
         prec = 2;
      } else if (dt < 300.0) {   // 5 minutes
         prec = 1;
      } else {
         prec = 0;
      }

      f = QEUtilities::intervalToString (dt, prec, false);
      mouseReadOut.append (QString  ("    dt: %1 ").arg (f));

      f = QString::asprintf ("  dy: %+.6g", dy);
      mouseReadOut.append (f);

      // Calculate slope, but avoid the divide by 0.
      //
      mouseReadOut.append ("  dy/dt: ");
      if (dt != 0.0) {
         f = QString::asprintf ("%+.6g", dy/dt);
      } else {
         if (dy != 0.0) {
            f = QString::asprintf ("%sinf", (dy >= 0.0) ? "+" : "-");
         } else {
            f = QString::asprintf ("n/a");
         }
      }
      mouseReadOut.append (f);
   }

   // If the box markup is enabled, thean just leave the selected data point alone,
   // otherwise check to see if we are hovering over a data point.
   //
   if (!this->plotArea->getMarkupEnabled (QEGraphicNames::Box)) {

      const QCaDataPoint* nearest = NULL;

      const bool boxWasVisible = this->plotArea->getMarkupVisible (QEGraphicNames::Box);

      // Find neaerst point that is also near enough.
      //
      nearest = this->findNearestPoint (position, this->selectedPointSlot);
      if (nearest) {
         QEStripChartItem* item = this->getItem (this->selectedPointSlot);
         if (item) {
            this->selectedPointDateTime = nearest->datetime;
            this->selectedPointValue = nearest->value;

            this->plotArea->setMarkupVisible (QEGraphicNames::Box, true);
            this->plotArea->setMarkupPosition (QEGraphicNames::Box, item->dataPointToReal(*nearest));

            // Form the string/image of the value.
            //
            const QString svalue = QString::number(nearest->value, 'e', 5);

            mouseReadOut.append (QString (" [%1  %2]").arg (item->getCaptionLabel()).arg (svalue));

            QStringList info;
            info.append (item->getPvName());
            const QString desc = item->getDescription();
            if (!desc.isEmpty()) info.append (desc);
            info.append (QString ("%1 %2").arg (svalue).arg (item->getEgu ()));
            info.append (nearest->datetime.toString (format).left (format.length() - 2));

            this->plotArea->setMarkupData (QEGraphicNames::Box, QVariant (info));
            this->setContextMenuPolicy (Qt::NoContextMenu);
         }

      } else {
         this->plotArea->setMarkupVisible (QEGraphicNames::Box, false);
         this->setContextMenuPolicy (Qt::CustomContextMenu);
      }

      const bool boxIsVisible = this->plotArea->getMarkupVisible (QEGraphicNames::Box);

      if (boxIsVisible != boxWasVisible) {
         // Change of stae - force replot
         this->replotIsRequired = true;
      }
   }

   this->setReadOut (mouseReadOut);
}

//------------------------------------------------------------------------------
//
void QEStripChart::markupMove (const QEGraphicNames::Markups markup,
                               const QPointF& /* position */)
{
   double from;
   double to;
   QCaDateTime t1;
   QCaDateTime t2;

   switch (markup) {
      case QEGraphicNames::HorizontalLine_1:
      case QEGraphicNames::HorizontalLine_2:
         from = this->plotArea->getMarkupPosition (QEGraphicNames::HorizontalLine_1).y();
         to   = this->plotArea->getMarkupPosition (QEGraphicNames::HorizontalLine_2).y();
         this->toolBar->setValue1Refs (from, to);
         break;

      case QEGraphicNames::HorizontalLine_3:
      case QEGraphicNames::HorizontalLine_4:
         from = this->plotArea->getMarkupPosition (QEGraphicNames::HorizontalLine_3).y();
         to   = this->plotArea->getMarkupPosition (QEGraphicNames::HorizontalLine_4).y();
         this->toolBar->setValue2Refs (from, to);
         break;

      case QEGraphicNames::VerticalLine_1:
      case QEGraphicNames::VerticalLine_2:
         t1 = this->timeAt (this->plotArea->getMarkupPosition (QEGraphicNames::VerticalLine_1).x());
         t2 = this->timeAt (this->plotArea->getMarkupPosition (QEGraphicNames::VerticalLine_2).x());
         this->toolBar->setTimeRefs (t1, t2);
         break;

      default:
         break;
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::archiveStatus (const QEArchiveAccess::StatusList& statusList)
{
   int total = 0;
   for (int j = 0; j < statusList.count (); j++) {
      QEArchiveAccess::Status item = statusList.value (j);
      total += item.pending;
   }
   this->toolBar->setNOARStatus (total);
}

//------------------------------------------------------------------------------
//
void QEStripChart::captureState (QEStripChartState& chartState)
{
   // Capture current state.
   //
   chartState.isNormalVideo = this->isNormalVideo;
   chartState.yScaleMode = this->yScaleMode;
   chartState.chartYScale = this->chartYScale;
   chartState.yMinimum = this->getYMinimum ();
   chartState.yMaximum = this->getYMaximum ();
   chartState.chartTimeMode = this->chartTimeMode;
   chartState.duration = this->getDuration ();
   chartState.timeZoneSpec = this->timeZoneSpec;
   chartState.endDateTime = this->getEndDateTime ();
}

//------------------------------------------------------------------------------
//
void QEStripChart::applyState (const QEStripChartState& chartState)
{
   this->setNormalBackground (chartState.isNormalVideo);
   this->yScaleMode = chartState.yScaleMode;
   this->chartYScale = chartState.chartYScale;
   this->setYRange (chartState.yMinimum, chartState.yMaximum);
   this->chartTimeMode =  chartState.chartTimeMode;
   this->setEndDateTime (chartState.endDateTime);
   this->setDuration (chartState.duration);
   this->timeZoneSpec = chartState.timeZoneSpec;
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEStripChart::pushState ()
{
   QEStripChartState chartState;

   // Capture current state.
   //
   this->captureState (chartState);
   this->chartStateList.push (chartState);

   // Enable/disble buttons according to availability.
   //
   this->toolBar->setStateSelectionEnabled (QEStripChartNames::previous, (this->chartStateList.prevAvailable ()));
   this->toolBar->setStateSelectionEnabled (QEStripChartNames::next,     (this->chartStateList.nextAvailable ()));
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEStripChart::prevState ()
{
   QEStripChartState chartState;

   if (this->chartStateList.prev (chartState)) {
      this->applyState (chartState);
      this->toolBar->setStateSelectionEnabled (QEStripChartNames::previous, (this->chartStateList.prevAvailable ()));
      this->toolBar->setStateSelectionEnabled (QEStripChartNames::next,     (this->chartStateList.nextAvailable ()));
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::nextState ()
{
   QEStripChartState chartState;

   if (this->chartStateList.next (chartState)) {
      this->applyState (chartState);
      this->toolBar->setStateSelectionEnabled (QEStripChartNames::previous, (this->chartStateList.prevAvailable ()));
      this->toolBar->setStateSelectionEnabled (QEStripChartNames::next,     (this->chartStateList.nextAvailable ()));
   }
}

//------------------------------------------------------------------------------
// Constructor
//
QEStripChart::QEStripChart (QWidget * parent) : QEAbstractDynamicWidget (parent)
{
   // Configure the panel and create contents
   //
   this->setFrameShape (QFrame::Panel);
   this->setFrameShadow (QFrame::Plain);

   this->setMinimumSize (1080, 400);   // keep this and sizeHint consistant

   // Construct common object if needs be.
   //
   QEPVNameLists::constructor ();

   // Construct internal widgets for this chart.
   //
   this->createInternalWidgets ();

   this->timeZoneSpec = Qt::LocalTime;
   this->duration = 600;     // ten minutes.
   this->timeScale = 60.0;   // minutes
   this->timeUnits = "mins";

   this->enableConextMenu = true;
   this->toolBarIsVisible = true;
   this->pvItemsIsVisible = true;
   this->setNumberPvsVisible (10);

   // We always use UTC (EPICS) time within the strip chart.
   // Set directly here as using setEndTime has side effects.
   //
   this->endDateTime = QDateTime::currentDateTime ().toUTC ();

   this->yMinimum = 0.0;
   this->yMaximum = 100.0;

   // Initialise selected point related variables.
   //
   this->selectedPointSlot = -1;
   this->selectedPointValue = 0.0;
   this->selectedPointDateTime = this->endDateTime;

   this->plotArea->setXScale (1.0 / this->timeScale);
   this->plotArea->setXRange (-this->duration / this->timeScale, 0.0,
                              QEGraphicNames::SelectByValue, 5, true);
   this->plotArea->setYRange (this->yMinimum, this->yMaximum,
                              QEGraphicNames::SelectBySize, 40, true);

   // Variables are managed by the strip chart item widgets.
   //
   this->variableNameSubstitutions = "";
   this->setNumVariables (0);

   this->setNumberOfContextMenuItems (ARRAY_LENGTH (this->items));

   // Construct dialogs.
   //
   this->durationDialog = new QEStripChartDurationDialog (this);
   this->timeDialog = new QEStripChartTimeDialog (this);
   this->yRangeDialog = new QEStripChartRangeDialog (this);

   // Construct access - needed for status, specifically number of outstanding requests.
   //
   this->archiveAccess = new QEArchiveAccess (this);
   QObject::connect (this->archiveAccess,
                     SIGNAL     (archiveStatus (const QEArchiveAccess::StatusList&)),
                     this, SLOT (archiveStatus (const QEArchiveAccess::StatusList&)));

   // This info re-emitted on change, but we need to stimulate an initial update.
   //
   this->archiveAccess->resendStatus ();

   this->replotIsRequired = true; // ensure process on first tick.
   this->tickTimerCount = 0;

   // Create QEStripChart timer if needs be.
   //
   if (QEStripChart::tickTimer == NULL) {
      QEStripChart::tickTimer = new QTimer (NULL);
      QEStripChart::tickTimer->start (50);  // mSec == 0.05s - refresh plot check at ~20Hz.
   }
   QObject::connect (this->tickTimer, SIGNAL (timeout ()), this, SLOT (tickTimeout ()));

   // Enable drag drop onto this widget.
   //
   this->evaluateAllowDrop ();

   this->chartStateList.clear ();
   this->pushState ();  // baseline state - there is always at least one.
}

//------------------------------------------------------------------------------
//
QEStripChart::~QEStripChart () { }

//------------------------------------------------------------------------------
//
QSize QEStripChart::sizeHint () const
{
   return QSize (1080, 400);
}

//------------------------------------------------------------------------------
//
void QEStripChart::setVariableNameProperty (const int slot, const QString& pvName)
{
   QEStripChartItem * item = this->getItem (slot);
   if (item) {
      item->pvNameProperyManager.setVariableNameProperty (pvName);
   } else {
      DEBUG << "slot out of range " << slot;
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::setEnableConextMenu (bool enableIn)
{
   this->enableConextMenu = enableIn;
}

//------------------------------------------------------------------------------
//
bool QEStripChart::getEnableConextMenu () const
{
    return this->enableConextMenu;
}

//------------------------------------------------------------------------------
//
void QEStripChart::setToolBarVisible (bool visibleIn)
{
   this->toolBarIsVisible = visibleIn;
   this->toolBarResize->setVisible (visibleIn);
}

//------------------------------------------------------------------------------
//
bool QEStripChart::getToolBarVisible () const
{
   return this->toolBarIsVisible;
}

//------------------------------------------------------------------------------
//
void QEStripChart::setPvItemsVisible (bool visibleIn)
{
   this->pvItemsIsVisible = visibleIn;
   this->pvResizeFrame->setVisible (visibleIn);
}

//------------------------------------------------------------------------------
//
bool QEStripChart::getPvItemsVisible () const
{
   return this->pvItemsIsVisible;
}

//------------------------------------------------------------------------------
//
void QEStripChart::setNumberPvsVisible (int number)
{
   this->numberPvsVisible = LIMIT (number, 0, NUMBER_OF_PVS);
   this->pvResizeFrame->setFixedHeight (PV_ITEM_HEIGHT (this->numberPvsVisible));
}

//------------------------------------------------------------------------------
//
int QEStripChart::getNumberPvsVisible () const
{
   return this->numberPvsVisible;
}

//------------------------------------------------------------------------------
//
QString QEStripChart::getVariableNameProperty (const int slot) const
{
   QString result;
   QEStripChartItem * item = this->getItem (slot);
   if (item) {
      result = item->pvNameProperyManager.getVariableNameProperty ();
   } else {
      DEBUG << "slot out of range " << slot;
      result = "";
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEStripChart::setVariableNameSubstitutionsProperty (const QString& variableNameSubstitutionsIn)
{
   // Save local copy - just for getVariableNameSubstitutionsProperty.
   //
   this->variableNameSubstitutions = variableNameSubstitutionsIn;

   // The same subtitutions apply to all PVs.
   //
   for (int j = 0; j < NUMBER_OF_PVS; j++ ) {
      QEStripChartItem* item = this->getItem (j);
      item->pvNameProperyManager.setSubstitutionsProperty (variableNameSubstitutionsIn);
   }
}

//------------------------------------------------------------------------------
//
QString QEStripChart::getVariableNameSubstitutionsProperty () const
{
   return this->variableNameSubstitutions;
}

//------------------------------------------------------------------------------
//
void QEStripChart::setAliasName (const int slot, const QString& aliasName)
{
   QEStripChartItem* item = this->getItem (slot);
   if (item) {
      item->setAliasName (aliasName);
   } else {
      DEBUG << "slot out of range " << slot;
   }
}

//------------------------------------------------------------------------------
//
QString QEStripChart::getAliasName (const int slot) const
{
   QEStripChartItem* item = this->getItem (slot);
   if (item) {
      return item->getAliasName ();
   } else {
      DEBUG << "slot out of range " << slot;
      return "";
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::setColourProperty (const int slot, const QColor& colour)
{
   QEStripChartItem* item = this->getItem (slot);
   if (item) {
      item->setColour (colour);
   } else {
      DEBUG << "slot out of range " << slot;
   }
}

//------------------------------------------------------------------------------
//
QColor QEStripChart::getColourProperty (const int slot) const
{
   QEStripChartItem* item = this->getItem (slot);
   if (item) {
      return item->getColour ();
   } else {
      DEBUG << "slot out of range " << slot;
      return QColor (0x00, 0x00, 0x00, 0xFF);
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::setPvName (const int slot, const QString& pvName)
{
   QEStripChartItem* item = this->getItem (slot);
   if (item) {
      item->setPvName (pvName, "");
   }
}

//------------------------------------------------------------------------------
//
QString QEStripChart::getPvName (const int slot) const
{
   QString result = "";
   QEStripChartItem* item = this->getItem (slot);
   if (item) {
      result = item->getPvName ();
   }
   return result;
}

//------------------------------------------------------------------------------
//
int QEStripChart::addPvName (const QString& pvName)
{
   int result = -1;

   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem * item = this->getItem (slot);
      if (item->isInUse() == false) {
         // Found an empty slot.
         //
         item->setPvName (pvName, "");
         result = (int) slot;
         break;
      }
   }

   // Determine if we are now full.
   //
   this->evaluateAllowDrop ();
   return result;
}

//------------------------------------------------------------------------------
//
void QEStripChart::clearAllPvNames ()
{
   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      this->setPvName (slot, "");
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::updateItemMenu (const int slot, QAction* action, const bool inUseMenu)
{
   QEStripChartItem* item = this->getItem (slot);
   if (item) {
      item->updateMenu (action, inUseMenu);
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::writeTraceToFile (const int slot)
{
   QEStripChartItem* item = this->getItem (slot);
   if (item) {
      item->writeTraceToFile ();
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::tickTimeout ()
{
   this->tickTimerCount = (this->tickTimerCount + 1) % 20;

   // Evaluate at (approx) 10 Hz.
   if ((this->tickTimerCount % 2) == 0) {
      this->doCurrentValueCalculations ();
   }

   if ((this->tickTimerCount % 20) == 0) {
      // 20th update, i.e. 1 second has passed - must replot.
      this->replotIsRequired = true;
   }

   if (this->replotIsRequired) {
      if (this->chartTimeMode == QEStripChartNames::tmRealTime) {
         this->setEndDateTime (QDateTime::currentDateTime ());
      }
      this->plotData ();  // clears replotIsRequired
   }
}


//=============================================================================
// Handle toolbar signals
//
void QEStripChart::stateSelected (const QEStripChartNames::StateModes mode)
{
   if (mode == QEStripChartNames::previous) {
      this->prevState ();
   } else {
      this->nextState ();
   }

   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEStripChart::videoModeSelected (const QE::VideoModes mode)
{
   this->setNormalBackground (mode == QE::normal);
   this->pushState ();
}

//------------------------------------------------------------------------------
//
QE::VideoModes QEStripChart::getVideoMode () const
{
   return this->isNormalVideo ? QE::normal : QE::reverse;
}

//------------------------------------------------------------------------------
//
void QEStripChart::yScaleModeSelected (const QEStripChartNames::YScaleModes mode)
{
   this->yScaleMode = mode;
   this->pushState ();
}

//------------------------------------------------------------------------------
//
QEStripChartNames::YScaleModes QEStripChart::getYScaleMode () const
{
   return this->yScaleMode;
}

//------------------------------------------------------------------------------
//
void QEStripChart::yRangeSelected (const QEStripChartNames::ChartYRanges scale)
{
   int n;

   switch (scale) {
      case QEStripChartNames::manual:
         this->yRangeDialog->setRange (this->getYMinimum (), this->getYMaximum ());
         n = this->yRangeDialog->exec (this);
         if (n == 1) {
            this->chartYScale = scale;
            // User has selected okay.
            //
            this->setYRange (this->yRangeDialog->getMinimum (),
                             this->yRangeDialog->getMaximum ());
         }
         this->pushState ();
         break;

      case QEStripChartNames::operatingRange:
      case QEStripChartNames::plotted:
      case QEStripChartNames::buffered:
      case QEStripChartNames::dynamic:
         this->chartYScale = scale;
         this->calcDisplayMinMax ();
         this->pushState ();
         break;

      case QEStripChartNames::normalised:
         this->setYRange (0.0, 100.0);
         this->chartYScale = scale;

         for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
            QEStripChartItem * item = this->getItem (slot);
            if (item->isInUse ()) {
               item->normalise ();
            }
         }
         this->pushState ();
         break;

      default:
         DEBUG << "Well this is unexpected:" << (int) scale;
         break;
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::setYRangeMode (const PropertyChartYRanges scale)
{
   // Just set the mode. Intended for designer propery set. Must avoid dialog.
   //
   this->chartYScale = (QEStripChartNames::ChartYRanges) scale;
}

//------------------------------------------------------------------------------
//
QEStripChart::PropertyChartYRanges QEStripChart::getYRangeMode () const
{
   return (PropertyChartYRanges) this->chartYScale;
}

//------------------------------------------------------------------------------
//
void QEStripChart::durationSelected (const int seconds)
{
   this->setDuration (seconds);
   this->pushState ();
}

//------------------------------------------------------------------------------
//
void QEStripChart::selectDuration ()
{
   this->durationDialog->setDuration (this->getDuration ());
   const int n = this->durationDialog->exec (this->toolBar);
   if (n == 1) {
      const int d = this->durationDialog->getDuration ();
      this->setDuration (d);
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::timeZoneSelected (const Qt::TimeSpec timeZoneSpecIn)
{
   this->timeZoneSpec = timeZoneSpecIn;
   this->pushState ();
}

//------------------------------------------------------------------------------
//
void QEStripChart::playModeSelected (const QEStripChartNames::PlayModes mode)
{
   int n;
   int d;

   switch (mode) {

      case QEStripChartNames::play:
         this->chartTimeMode = QEStripChartNames::tmRealTime;
         // Note: using setEndTime causes a replot.
         this->setEndDateTime (QDateTime::currentDateTime ());
         this->pushState ();
         break;

      case QEStripChartNames::pause:
         this->chartTimeMode = QEStripChartNames::tmPaused;
         this->pushState ();
         break;

      case QEStripChartNames::forward:
         this->chartTimeMode = QEStripChartNames::tmHistorical;
         this->setEndDateTime (this->getEndDateTime ().addSecs (+this->duration));
         this->pushState ();
         break;

      case QEStripChartNames::backward:
         this->chartTimeMode = QEStripChartNames::tmHistorical;
         this->setEndDateTime (this->getEndDateTime ().addSecs (-this->duration));
         this->pushState ();
         break;

      case QEStripChartNames::selectTimes:
         this->timeDialog->setMaximumDateTime (QDateTime::currentDateTime ().toTimeSpec (this->timeZoneSpec));
         this->timeDialog->setStartDateTime (this->getStartDateTime().toTimeSpec (this->timeZoneSpec));
         this->timeDialog->setEndDateTime (this->getEndDateTime().toTimeSpec (this->timeZoneSpec));
         n = this->timeDialog->exec (this);
         if (n == 1) {
            // User has selected okay.
            //
            this->chartTimeMode = QEStripChartNames::tmHistorical;
            this->setEndDateTime (this->timeDialog->getEndDateTime ());

            // We use the possibly limited chart end time in order to calculate the
            // duration.
            //
            d = this->timeDialog->getStartDateTime ().secsTo (this->getEndDateTime());
            this->setDuration (d);
            this->pushState ();
         }
         break;
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::zoomInOut (const QPointF& about, const int zoomAmount)
{
   if (zoomAmount) {
      // We really only need the sign of the zoomAmount.
      //
      const double factor = (zoomAmount >= 0) ? 0.95 : (1.0 / 0.95);

      double newMin;
      double newMax;

      if (this->yScaleMode == QEStripChartNames::log) {
         const double logAboutY = LOG10 (about.y ());

         newMin = EXP10 (logAboutY + (LOG10 (this->yMinimum) - logAboutY) * factor);
         newMax = EXP10 (logAboutY + (LOG10 (this->yMaximum) - logAboutY) * factor);
      } else {
         newMin = about.y () + (this->yMinimum - about.y ()) * factor;
         newMax = about.y () + (this->yMaximum - about.y ()) * factor;
      }

      this->setYRange (newMin, newMax);
      this->pushState ();
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::writeAllSelected ()
{
   QString defaultPath = this->getDefaultDir ();

   // Launch the dialog
   //
   QString filename = QFileDialog::getSaveFileName
         (this, "Select output all trace file", defaultPath,
          "CSV files(*.csv);;Text files(*.txt);;All files(*.*)");

   if (filename.isEmpty ()) {
      return;
   }

   bool isCsv = filename.endsWith(".csv");

   QVector<QEStripChartItem*> itemList;
   itemList.reserve (NUMBER_OF_PVS);

   // Create a set of QEStripChartItem items.
   //
   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem* item = this->getItem (slot);
      if (item && item->isInUse()) {
         itemList.append (item);
      }
   }

   QEStripChartItem::writeListToFile (this, itemList, filename, 1.0, isCsv);
}

//------------------------------------------------------------------------------
//
void QEStripChart::readArchiveSelected ()
{
   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem* item = this->getItem (slot);
      if (item->isPvData ()) {
         item->readArchive ();
      }
   }
}

//
// end of tool bar handlers ====================================================


//------------------------------------------------------------------------------
//
void QEStripChart::addToPredefinedList (const QString & pvName)
{
   predefinedPVNameList->prependOrMoveToFirst (pvName);
}

//------------------------------------------------------------------------------
//
QStringList QEStripChart::getPredefinedPVNameList () const
{
   return QStringList (*predefinedPVNameList);
}

//------------------------------------------------------------------------------
//
QString QEStripChart::getPredefinedItem (int i) const
{
   return predefinedPVNameList->value (i, "");
}

//------------------------------------------------------------------------------
// Start/end time
//
QDateTime QEStripChart::getStartDateTime () const
{
   return this->getEndDateTime().addSecs (-this->duration);
}

//------------------------------------------------------------------------------
//
QDateTime QEStripChart::getEndDateTime () const
{
   return this->endDateTime;
}

//------------------------------------------------------------------------------
//
void QEStripChart::setEndDateTime (QDateTime endDateTimeIn)
{
   QDateTime useUTC = endDateTimeIn.toUTC ();
   QDateTime nowUTC = QDateTime::currentDateTime ().toUTC ();

   // No peeking into the future.
   //
   if (useUTC > nowUTC) {
      useUTC = nowUTC;
   }

   if (this->endDateTime != useUTC) {
      this->endDateTime = useUTC;
      this->replotIsRequired = true;
   }
}

//------------------------------------------------------------------------------
//
int QEStripChart::getDuration () const
{
   return this->duration;
}

//------------------------------------------------------------------------------
//
void QEStripChart::setDuration (int durationIn)
{
   // A duration of less than 1 second is not allowed.
   //
   if (durationIn < 1) {
      durationIn = 1;
   }

   if (this->duration != durationIn) {
      this->duration = durationIn;
      this->replotIsRequired = true;
   }
}

//----------------------------------------------------------------------------
//
double QEStripChart::getYMinimum () const
{
   return this->yMinimum;
}

//----------------------------------------------------------------------------
//
void QEStripChart::setYMinimum (const double yMinimumIn)
{
   this->yMinimum = yMinimumIn;

   const double temp1 = this->yMinimum + MINIMUM_SPAN;
   const double temp2 = this->yMinimum + ABS (this->yMinimum) * MINIMUM_RATIO;
   const double minMax = MAX (temp1, temp2);

   this->yMaximum = MAX (this->yMaximum, minMax);
   this->chartYScale = QEStripChartNames::manual;
   this->replotIsRequired = true;
}

//----------------------------------------------------------------------------
//
double QEStripChart::getYMaximum () const
{
   return this->yMaximum;
}

//----------------------------------------------------------------------------
//
void QEStripChart::setYMaximum (const double yMaximumIn)
{
   this->yMaximum = yMaximumIn;

   const double temp1 = this->yMaximum - MINIMUM_SPAN;
   const double temp2 = this->yMaximum - ABS (this->yMaximum) * MINIMUM_RATIO;
   const double maxMin = MIN (temp1, temp2);

   this->yMinimum = MIN (this->yMinimum, maxMin);
   this->chartYScale = QEStripChartNames::manual;
   this->replotIsRequired = true;
}

//----------------------------------------------------------------------------
//
void QEStripChart::setYRange (const double yMinimumIn, const double yMaximumIn)
{
   this->setYMaximum (yMaximumIn);
   this->setYMinimum (yMinimumIn);
}

//----------------------------------------------------------------------------
//
QMenu* QEStripChart::buildContextMenu ()
{
   QMenu* menu = QEAbstractDynamicWidget::buildContextMenu ();  // build parent context menu
   QAction* action;

   menu->addSeparator ();

   action = new QAction ("Show/Hide Tool Bar", menu);
   action->setCheckable (true);
   action->setChecked(this->toolBarIsVisible);
   action->setEnabled (this->enableConextMenu);
   action->setData (QEStripChartNames::SCCM_SHOW_HIDE_TOOLBAR);
   menu->addAction (action);

   action = new QAction ("Show/Hide PV Items", menu);
   action->setCheckable (true);
   action->setChecked(this->pvItemsIsVisible);
   action->setEnabled (this->enableConextMenu);
   action->setData (QEStripChartNames::SCCM_SHOW_HIDE_PV_ITEMS);
   menu->addAction (action);

   this->addPVLabelModeContextMenu (menu);

   return menu;
}

//----------------------------------------------------------------------------
//
void QEStripChart::contextMenuTriggered (int selectedItemNum)
{
   switch (selectedItemNum) {
      case QEStripChartNames::SCCM_SHOW_HIDE_TOOLBAR:
         this->setToolBarVisible (!this->getToolBarVisible ());
         break;

      case QEStripChartNames::SCCM_SHOW_HIDE_PV_ITEMS:
         this->setPvItemsVisible (!this->getPvItemsVisible ());
         break;

      default:
         // process parent context menu
         //
         QEAbstractDynamicWidget::contextMenuTriggered (selectedItemNum);
         break;
   }
}

//----------------------------------------------------------------------------
//
void QEStripChart::pvLabelModeChanged ()
{
   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem* item = this->getItem (slot);

      if (item) {
         item->setCaption ();
      }
   }
}

//----------------------------------------------------------------------------
//
QString QEStripChart::copyVariable ()
{
   QString result;

   // Create space delimited set of PV names.
   //
   result = "";
   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem* item = this->getItem (slot);

      if ((item) && (item->isPvData () == true)) {
         if (!result.isEmpty()) {
            result.append (" ");
         };
         result.append (item->getPvName ());
      }
   }
   return result;
}

//----------------------------------------------------------------------------
//
QVariant QEStripChart::copyData ()
{
   return  QVariant ();  // place holder.
}

//----------------------------------------------------------------------------
// Determine if user allowed to drop new PVs into this widget.
//
void QEStripChart::evaluateAllowDrop ()
{
   bool allowDrop;

   // Hypoyhesize that the strip chart is full.
   //
   allowDrop = false;
   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem* item = this->getItem (slot);

      if ((item) && (item->isInUse () == false)) {
         // Found an empty slot.
         //
         allowDrop = true;
         break;
      }
   }

   this->setAllowDrop (allowDrop);
}

//------------------------------------------------------------------------------
//
qcaobject::QCaObject* QEStripChart::createQcaItem (unsigned int variableIndex)
{
   DEBUG << "unexpected call, variableIndex = " << variableIndex;
   return NULL;
}

//------------------------------------------------------------------------------
//
void QEStripChart::establishConnection (unsigned int /* variableIndex */ )
{
   // A framework feature is that there is always at least one variable.
   // DEBUG << "unexpected call, variableIndex = " << variableIndex;
}

//------------------------------------------------------------------------------
//
void QEStripChart::saveConfiguration (PersistanceManager* pm)
{
   if (!pm) return;   // sanity check
   if (!predefinedPVNameList) return;   // sanity check

   const QString formName = this->getPersistantName();

   // Do common stuff first.
   // How can we avoid doing this mutiple times??
   //
   PMElement commonElement = pm->addNamedConfiguration ("QEStripChart_Common");
   predefinedPVNameList->saveConfiguration (commonElement);

   // Now do form instance specific stuff.
   //
   PMElement formElement = pm->addNamedConfiguration (formName);

   // Capture current state.
   //
   QEStripChartState chartState;
   this->captureState (chartState);
   chartState.saveConfiguration (formElement);

   // Capture markup states.
   //
   this->plotArea->saveConfiguration (formElement);

   // Save each active PV.
   //
   PMElement pvListElement = formElement.addElement ("PV_List");
   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem* item = this->getItem (slot);
      if (item) {
         item->saveConfiguration (pvListElement);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEStripChart::restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase)
{
   if (!pm) return;   // sanity check
   if (!predefinedPVNameList) return;   // sanity check

   if (restorePhase != FRAMEWORK) return;

   const QString formName = this->getPersistantName();

   // Do common stuff first.
   // How can we avoid doing this mutiple times??
   //
   PMElement commonElement = pm->getNamedConfiguration ("QEStripChart_Common");
   predefinedPVNameList->restoreConfiguration (commonElement);

   // Now do form instance specific stuff.
   //
   PMElement formElement = pm->getNamedConfiguration (formName);

   // Restore chart state.
   //
   QEStripChartState chartState;
   chartState.restoreConfiguration (formElement);
   this->applyState (chartState);

   // Restore markup states.
   //
   this->plotArea->restoreConfiguration (formElement);

   // Restore each PV.
   //
   PMElement pvListElement = formElement.getElement ("PV_List");
   for (int slot = 0; slot < NUMBER_OF_PVS; slot++) {
      QEStripChartItem* item = this->getItem (slot);
      if (item) {
         item->restoreConfiguration (pvListElement);
      }
   }
}

// end
