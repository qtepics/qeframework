/*  QEDistribution.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2019 Australian Synchrotron.
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
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrews@ansto.gov.au
 */

#include "QEDistribution.h"
#include <math.h>

#include <QApplication>
#include <QClipboard>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QDebug>

#include <QECommon.h>
#include <QCaObject.h>
#include <QEFloating.h>
#include <QEDisplayRanges.h>

#define DEBUG qDebug() << "QEDistribution" << __LINE__ << __FUNCTION__ << "  "

// Much of this code is effectively the same as in the QEStripChartStatistics widget.
// Do keep these two items aligned as far as reasonably possible.
//
#define PV_VARIABLE_INDEX     0
#define MAJOR_MINOR_RATIO     5

// We use a shared timer for all QEDistribution instances.
// static
QTimer* QEDistribution::tickTimer = NULL;

//------------------------------------------------------------------------------
//
QEDistribution::QEDistribution (QWidget* parent) :
   QEFrame (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->setup ();
}

//------------------------------------------------------------------------------
//
QEDistribution::QEDistribution (const QString& variableNameIn,
                                QWidget* parent) :
   QEFrame (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->setup ();
   this->setVariableName (variableNameIn, PV_VARIABLE_INDEX);
   this->activate ();
}

//------------------------------------------------------------------------------
//
QEDistribution::~QEDistribution () { }

//------------------------------------------------------------------------------
//
void QEDistribution::setup()
{
   // Create internal widget.
   //
   this->createWidgets ();

   // Set default property values.
   //
   this->setMinimumSize (760, 180);

   // Configure the panel and create contents
   //
   this->setFrameShape (QFrame::Panel);
   this->setFrameShadow (QFrame::Plain);

   this->mNumberStdDevs = 3;
   this->mIsRectangular = true;
   this->mEdgeWidth = 2;
   this->mShowGaussian = true;
   this->mGaussianWidth = 2;
   this->gaussianBox->setChecked (this->mShowGaussian);

   this->setBackgroundColour (QColor ("#e8e8e8"));  // light gray
   this->setEdgeColour (QColor ("#2060a0"));        // dark blue
   this->setGaussianColour (QColor ("#ff0000"));    // red
   this->setFillColour (QColor ("#80c0ff"));        // light blue

   this->tickTimerCount = 0;

   // Initate gathering of archive data - specifically the PV name list.
   //
   this->archiveAccess = new QEArchiveAccess (this);

   // Create forms/dialogs.
   //
   this->pvNameSelectDialog = new QEPVNameSelectDialog (this);

   this->isFirstUpdate = false;
   this->xChangePending = 0;
   this->yChangePending = 0;

   // Reset all the distribution related data
   //
   this->resetDistibution ();

   // Set up label context menu (as opposed to widget's overall context menu).
   //
   this->contextMenu = new QMenu (this);

   QAction* action;
   action = new QAction ("Add/Edit PV Name ", this->contextMenu);
   action->setData (QVariant (int (maAddEdit)));
   this->contextMenu->addAction (action);

   action = new QAction ("Paste PV Name ", this->contextMenu );
   action->setData (QVariant (int (maPaste)));
   this->contextMenu->addAction (action);
   this->pasteAction = action;  // save a reference.

   action = new QAction ("Clear ", this->contextMenu );
   action->setData (QVariant (int (maClear)));
   this->contextMenu->addAction (action);
   this->clearAction = action;  // save a reference.

   this->pvNameLabel->setContextMenuPolicy (Qt::CustomContextMenu);

   // Set up data
   // This control uses a single data source
   //
   this->setNumVariables (1);

   // Enable drag drop onto this widget by default.
   //
   this->setAllowDrop (true);

   // Use default context menu.
   //
   this->setupContextMenu ();

   // By default, the distribution widget does not display the alarm state.
   // The internal valueLabel widget does this on our behalf.
   //
   this->setDisplayAlarmStateOption (DISPLAY_ALARM_STATE_NEVER);

   // Set up a connection to receive variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   const char* slot = SLOT (newPvName (QString, QString, unsigned int));
   this->connectNewVariableNameProperty (slot);

   // Create the static QEDistribution timer if needs be.
   //
   if (QEDistribution::tickTimer == NULL) {
      QEDistribution::tickTimer = new QTimer (NULL);
      QEDistribution::tickTimer->start (50);  // mSec == 0.05s - refresh plot check at ~20Hz.
   }

   this->plotArea->setMouseTracking (true);
   this->xAxis->setMouseTracking (true);
   this->yAxis->setMouseTracking (true);

   this->plotArea->installEventFilter (this);
   this->xAxis->installEventFilter (this);
   this->yAxis->installEventFilter (this);

   this->connectSignalsToSlots ();
}

//------------------------------------------------------------------------------
// Provides quazi layout functionality
//
void QEDistribution::resizeDistribution ()
{
   const QSize size = this->plotArea->size();

   const int yw = this->yAxis->width();
   const int xh = this->xAxis->height();

   // The -2 is a tweak factor, a.k.a. QEAxisPainter "feature"
   //
   this->xAxis->setIndent (yw - 2, 20);   // left, right
   this->yAxis->setIndent (20, xh - 2);   // top, bottom

   QRect geo;

   geo = this->yAxis->geometry();
   geo.setTop (0);
   geo.setLeft (0);
   geo.setHeight (size.height());
   this->yAxis->setGeometry (geo);

   geo = this->xAxis->geometry();
   geo.setTop (size.height() - geo.height());
   geo.setLeft (0);
   geo.setWidth (size.width());
   this->xAxis->setGeometry (geo);
}

//------------------------------------------------------------------------------
//
void QEDistribution::paintDistribution ()
{
   const QSize size = this->plotArea->size();

   const int xmin = this->xAxis->getTopLeftIndent ();                    // screen min
   const int xmax = size.width() - this->xAxis->getRightBottomIndent();  // screen max

   const int ymin = this->yAxis->getTopLeftIndent ();                    // screen min
   const int ymax = size.height() - this->yAxis->getRightBottomIndent(); // screen max

   QPainter painter (this->plotArea);
   QPen pen;
   QBrush brush;
   QPointF polygon [2 * ARRAY_LENGTH (this->distributionData) + 6];  // 128 points + loop back

   int ew = this->getEdgeWidth ();
   if (ew == 0) {
      // We can't actually set a pen width of zero, so we go with 1 and use
      // same colour for both pen and brush.
      //
      ew = 1;
      pen.setColor (this->mFillColour);
   } else {
      pen.setColor (this->mEdgeColour);
   }

   pen.setWidth (ew);
   pen.setStyle (Qt::SolidLine);
   painter.setPen (pen);

   brush.setColor (this->mFillColour);
   brush.setStyle (Qt::SolidPattern);
   painter.setBrush (brush);

   // Calculate mapping from real world coordinates to screen coordinates.
   // This is of the form y = m.x + c
   // For y axis, screen y increases downwards, in my brain y increases upwards.
   //
   const double my = double (ymax - ymin) /
                     (this->yAxis->getMinimum() - this->yAxis->getMaximum());
   const double cy = ymax - my*this->yAxis->getMinimum();

   const double mx = double (xmax - xmin) /
                     (this->xAxis->getMaximum() - this->xAxis->getMinimum());
   const double cx = xmin - mx*this->xAxis->getMinimum();

   // The real world range of plotted values
   //
   const double x_plot_min = this->valueMean - this->mNumberStdDevs * this->valueStdDev;
   const double x_plot_max = this->valueMean + this->mNumberStdDevs * this->valueStdDev;

   const double ds = (x_plot_max - x_plot_min) / double (this->distributionCount);

   int number;
   if (this->mIsRectangular) {
      for (int j = 0; j < this->distributionCount; j++) {
         double u = ds * j + x_plot_min;
         double x = mx * u + cx;

         double p = double (this->distributionData [j]) / double (this->valueCount); // proportion
         double y = my * p + cy;
         polygon [2*j + 0] = QPointF (x, y);

         u = ds * (j+1.0) + x_plot_min;
         x = mx * u + cx;
         polygon [2*j + 1] = QPointF (x, y);
      }
      number = 2 * this->distributionCount;
   } else {
      for (int j = 0; j < this->distributionCount; j++) {
         double u = ds * (j + 0.5) + x_plot_min;
         double x = mx * u + cx;
         double p = double (this->distributionData [j]) / double (this->valueCount); // proportion
         double y = my * p + cy;
         polygon [j] = QPointF (x, y);
      }
      number = this->distributionCount;
   }

   // Loop back to the start
   //
   polygon [number + 0] = QPointF (mx * x_plot_max + cx, ymax);
   polygon [number + 1] = QPointF (mx * x_plot_min + cx, ymax);
   polygon [number + 2] = polygon [0];

   painter.drawPolygon (polygon, number + 3);

   // Now draw draw the gaussian curve
   // Need at least point to even think about trying to do this.
   //
   if (this->getShowGaussian() && (this->valueCount > 0)) {

      QPointF gaussian [81];    // 81 a bit arbitary

      // width of each histogram bar - input units, not screen units.
      const double plotDelta = this->calcPlotDelta ();

      // height of mormal disribution density function
      const double peakDensity = 1.0 / (this->valueStdDev * sqrt (TAU));

      const double peak = peakDensity * plotDelta;
      const double var = this->valueStdDev * this->valueStdDev;

      for (int j = 0; j < ARRAY_LENGTH (gaussian); j++) {
         const double x = xmin + (xmax - xmin) * j / (ARRAY_LENGTH (gaussian) - 1.0);
         const double u = (x - cx)/mx - this->valueMean;  // back to real world wrt mean
         const double p = peak * exp (-0.5*(u*u)/var);
         const double y = my * p + cy;
         gaussian [j] = QPointF (x, y);
      }

      pen.setWidth (this->getGaussianWidth ());
      pen.setStyle (Qt::SolidLine);
      pen.setColor (this->mGaussianColour);
      painter.setPen (pen);
      painter.drawPolyline (gaussian, ARRAY_LENGTH (gaussian));
   }

   // Ensure next timer tick only invokes plot if needs be.
   //
   this->replotIsRequired = false;
}

//------------------------------------------------------------------------------
//
void QEDistribution::mouseMoveDistribution (const QPoint& mousePosition)
{
   const QSize size = this->plotArea->size();

   const int xmin = this->xAxis->getTopLeftIndent ();
   const int xmax = size.width() - this->xAxis->getRightBottomIndent();
   const double spread = this->currentXPlotMax - this->currentXPlotMin;

   const double m = spread / (xmax - xmin);
   const double c = this->currentXPlotMin - m*xmin;

   const double px = mousePosition.x();
   const double x = m*px + c;   // convert x from pixel to real world coordinates

   QString message;

   if (this->distributionCount > 0) {
      const double x_plot_min = this->valueMean - this->mNumberStdDevs * this->valueStdDev;
      const double plotDelta = this->calcPlotDelta();  // size of bar

      const double slot = (x - x_plot_min) / plotDelta;
      if (slot >= 0.0 && slot < this->distributionCount) {
         int j = int (slot);
         double x1 = x_plot_min + (j + 0.0)*plotDelta;
         double x2 = x_plot_min + (j + 1.0)*plotDelta;

         double f = 100.0 * double (this->distributionData [j]) / double (this->valueCount);

         message = QString ("x: %1 .. %2  %3%").
                   arg (x1, 0, 'g', 5).
                   arg (x2, 0, 'g', 5).
                   arg (f,  0, 'f', 2);
      }
   }

   this->setReadOut (message);
}

//------------------------------------------------------------------------------
//
bool QEDistribution::updatePlotLimits ()
{
   bool result = false;

   // Calculate plotted sample range - n standard deviations each side.
   // But ensure range is non-zero
   //
   const double tstd = MAX (1.0e-9, this->valueStdDev);
   const double x_plot_min = this->valueMean - this->mNumberStdDevs * tstd;
   const double x_plot_max = this->valueMean + this->mNumberStdDevs * tstd;

   // Form "nice" rounded plot scale values.
   //
   QEDisplayRanges displayRange;
   double plotMin;
   double plotMax;
   double plotMajor;
   double plotMinor;

   displayRange.setRange (x_plot_min, x_plot_max);
   displayRange.adjustMinMax (5, true, plotMin, plotMax, plotMajor);
   plotMinor = plotMajor / MAJOR_MINOR_RATIO;

   // Have any of these changed ?
   // The changed function adds some hysteresis.
   // But also factor in number of outstanding change requests.
   //
   const double e = (this->xChangePending <= 20) ? 0.1 : 1.0 / double (-10.0 + this->xChangePending);
   if (QEDistribution::changed (this->currentXPlotMin, plotMin, e) ||
       QEDistribution::changed (this->currentXPlotMax, plotMax, e) ||
       QEDistribution::changed (this->currentXPlotMinor, plotMinor, e))
   {
      // Yes - do an update
      //
      this->currentXPlotMin = plotMin;
      this->currentXPlotMax = plotMax;
      this->currentXPlotMinor = plotMinor;

      this->xAxis->setMinimum (plotMin);
      this->xAxis->setMaximum (plotMax);
      this->xAxis->setMinorInterval (plotMinor);

      // Now redistribute historical data over the new plot range/resolution
      //
      this->updateDistribution ();

      this->replotIsRequired = true;
      result = true;

      this->xChangePending = 0;

   } else if ((this->currentXPlotMin != plotMin) ||
              (this->currentXPlotMax != plotMax) ||
              (this->currentXPlotMinor != plotMinor)) {

      this->xChangePending++;

   } else {
      this->xChangePending = 0;  // equal
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEDistribution::updateDistribution ()
{
   // As we get more points, increase the distribtion count to get a better resolution
   //
   this->distributionCount = ARRAY_LENGTH (this->distributionData);
   if (this->valueCount < 800) this->distributionCount = ARRAY_LENGTH (this->distributionData) / 2;
   if (this->valueCount < 400) this->distributionCount = ARRAY_LENGTH (this->distributionData) / 4;
   if (this->valueCount < 200) this->distributionCount = ARRAY_LENGTH (this->distributionData) / 8;
   if (this->valueCount < 100) this->distributionCount = ARRAY_LENGTH (this->distributionData) / 16;

   // However ensure within range
   //
   this->distributionCount = LIMIT (this->distributionCount, 1, ARRAY_LENGTH (this->distributionData));

   // Initialise the distribution data array.
   //
   for (int j = 0; j < this->distributionCount; j++) {
      this->distributionData [j] = 0;
   }

   const double x_plot_min = this->valueMean - this->mNumberStdDevs * this->valueStdDev;
   const double plotDelta = this->calcPlotDelta();
   const int n = this->historicalData.count();
   for (int i = 0; i < n; i++) {
      const double value = this->historicalData.value (i);
      const double slot = (value - x_plot_min) / plotDelta;

      // Check for out of range values.
      //
      if (slot < 0.0 || slot >= this->distributionCount) continue;

      const int s = int (slot);
      this->distributionData [s] += 1;
   }

   // Find the max value so that we can calculate a sensible y scale.
   //
   int distributionMax = 1;
   for (int j = 0; j < this->distributionCount; j++) {
      distributionMax = MAX (distributionMax, this->distributionData [j]);
   }

   // Now calclate the fractional max - this is in range  >0.0 to 1.0
   // We plot fractional values.
   //
   const double fractionalMax =
         n > 0 ?
            double (distributionMax) / double (n) :
            1.0;

   // Form "nice" rounded plot scale values.
   //
   QEDisplayRanges displayRange;
   double plotMin;
   double plotMax;
   double plotMajor;
   double plotMinor;

   displayRange.setRange (0.0, fractionalMax);
   displayRange.adjustMinMax (5, true, plotMin, plotMax, plotMajor);
   plotMinor = plotMajor / MAJOR_MINOR_RATIO;

   // Have any of these changed?
   // The changed function adds some hysteresis.
   // But also factor in number of outstanding change requests.
   //
   const double e = (this->yChangePending <= 20) ? 0.2 : 2.0 / double (-10.0 + this->yChangePending);
   if (QEDistribution::changed (this->currentYPlotMin, plotMin, e) ||
       QEDistribution::changed (this->currentYPlotMax, plotMax, e) ||
       QEDistribution::changed (this->currentYPlotMinor, plotMinor, e))
   {
      // Yes - do an update
      //
      this->currentYPlotMin = plotMin;
      this->currentYPlotMax = plotMax;
      this->currentYPlotMinor = plotMinor;

      this->yAxis->setMinimum (plotMin);
      this->yAxis->setMaximum (plotMax);
      this->yAxis->setMinorInterval (plotMinor);

      this->yChangePending = 0;

   } else if ((this->currentYPlotMin != plotMin) ||
              (this->currentYPlotMax != plotMax) ||
              (this->currentYPlotMinor != plotMinor)) {

      this->yChangePending++;

   } else {
      this->yChangePending = 0;  // equal
   }
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
bool QEDistribution::eventFilter (QObject* watched, QEvent* event)
{
   const QEvent::Type type = event->type ();
   QMouseEvent* mouseEvent = NULL;

   switch (type) {
      case QEvent::MouseMove:
         mouseEvent = static_cast<QMouseEvent *> (event);
         if (watched == this->plotArea) {
            this->mouseMoveDistribution (mouseEvent->pos ());
            return true;  // event handled.
         }

         if (watched == this->xAxis) {
            QPoint pos =  mouseEvent->pos () + this->xAxis->geometry().topLeft();
            this->mouseMoveDistribution (pos);
            return true;  // event handled.
         }

         if (watched == this->yAxis) {
            QPoint pos =  mouseEvent->pos () + this->yAxis->geometry().topLeft();
            this->mouseMoveDistribution (pos);
            return true;  // event handled.
         }

         break;

      case QEvent::Resize:
         if (watched == this->plotArea) {
            this->resizeDistribution ();
            return true;  // event handled.
         }
         break;

      case QEvent::Paint:
         if (watched == this->plotArea) {
            this->paintDistribution ();
            return true;  // event handled.
         }
         break;

      default:
         break;
   }

   return false;
}

//------------------------------------------------------------------------------
//
void QEDistribution::tickTimeout ()
{
   this->tickTimerCount = (this->tickTimerCount + 1) % 20;

   if (this->tickTimerCount == 0) {
      this->replotIsRequired = true;
   }

   // Check for replot required.
   //
   if (this->replotIsRequired) {
      this->plotArea->update ();   // clears replotIsRequired
   }
}

//------------------------------------------------------------------------------
//
void QEDistribution::setNumberStdDevs (const double numberStdDevsIn)
{
   this->mNumberStdDevs = LIMIT (numberStdDevsIn, 0.1, 9.0);
   this->updatePlotLimits ();
}

//------------------------------------------------------------------------------
//
double QEDistribution::getNumberStdDevs () const
{
   return this->mNumberStdDevs;
}

//------------------------------------------------------------------------------
//
void QEDistribution::setIsRectangular (const bool isRectangularIn)
{
   this->mIsRectangular = isRectangularIn;
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
bool QEDistribution::getIsRectangular () const
{
   return this->mIsRectangular;
}

//------------------------------------------------------------------------------
//
void QEDistribution::setShowGaussian (const bool showGaussianIn)
{
   this->mShowGaussian = showGaussianIn;
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
bool QEDistribution::getShowGaussian () const
{
   return  this->mShowGaussian;
}

//------------------------------------------------------------------------------
//
void QEDistribution::setEdgeWidth (const int edgeWidthIn)
{
   this->mEdgeWidth = LIMIT (edgeWidthIn, 0, 20);
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
int QEDistribution::getEdgeWidth () const
{
   return this->mEdgeWidth;
}

//------------------------------------------------------------------------------
//
void QEDistribution::setGaussianWidth (const int gaussianWidthIn)
{
   this->mGaussianWidth = LIMIT (gaussianWidthIn, 1, 20);
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
int QEDistribution::getGaussianWidth () const
{
   return this->mGaussianWidth;
}

//------------------------------------------------------------------------------
//
void QEDistribution::setBackgroundColour (const QColor colour)
{
   this->mBackgroundColour = colour;
   this->plotArea->setStyleSheet (QEUtilities::colourToStyle (this->mBackgroundColour));
}

//------------------------------------------------------------------------------
//
QColor QEDistribution::getBackgroundColour () const
{
   return this->mBackgroundColour;
}

//------------------------------------------------------------------------------
//
void QEDistribution::setEdgeColour (const QColor colour)
{
   this->mEdgeColour = colour;
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
QColor QEDistribution::getEdgeColour () const
{
   return this->mEdgeColour;
}

//------------------------------------------------------------------------------
//
void QEDistribution::setFillColour (const QColor colour)
{
   this->mFillColour = colour;
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
QColor QEDistribution::getFillColour () const
{
   return this->mFillColour;
}

//------------------------------------------------------------------------------
//
void QEDistribution::setGaussianColour (const QColor colour)
{
   this->mGaussianColour = colour;
   this->replotIsRequired = this->mShowGaussian;
}

//------------------------------------------------------------------------------
//
QColor QEDistribution::getGaussianColour () const
{
   return this->mGaussianColour;
}

//------------------------------------------------------------------------------
//
void QEDistribution::setPvName (const QString& pvName)
{
   this->resetDistibution ();
   this->pvNameLabel->setText (pvName);
   this->setVariableName (pvName, PV_VARIABLE_INDEX);
   this->establishConnection (PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
// slot
void QEDistribution::newPvName (QString pvName, QString subs, unsigned int pvi)
{
   if (pvi != PV_VARIABLE_INDEX) return;  // sanity check

   this->resetDistibution ();
   this->setVariableNameAndSubstitutions (pvName, subs, pvi);
   this->pvNameLabel->setText (this->getSubstitutedVariableName (pvi));
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. For a progress bar a QCaObject that streams integers is
// required.
//
qcaobject::QCaObject* QEDistribution::createQcaItem (unsigned int pvi)
{
   if (pvi != PV_VARIABLE_INDEX) return NULL;  // sanity check
   qcaobject::QCaObject* result = NULL;

   QString pvName = this->getSubstitutedVariableName (pvi);
   this->pvNameLabel->setText (pvName);

   result = new QEFloating (pvName, this, &this->floatingFormatting, pvi);

   // Apply currently defined array index and elements request values.
   //
   this->setSingleVariableQCaProperties (result);
   return result;
}

//--------------------------`----------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QEDistribution::establishConnection (unsigned int pvi)
{
   if (pvi != PV_VARIABLE_INDEX) return;  // sanity check

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject* qca = this->createConnection (pvi);

   // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots.
   //
   if (qca) {
      QObject::connect (qca,  SIGNAL (floatingChanged (const double&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                        this, SLOT   (setPvValue      (const double&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));

      QObject::connect (qca, SIGNAL (connectionChanged (QCaConnectionInfo&,const unsigned int&)),
                        this, SLOT  (connectionChanged (QCaConnectionInfo&,const unsigned int&)));
   }
}

//------------------------------------------------------------------------------
//
void QEDistribution::connectionChanged (QCaConnectionInfo& connectionInfo,
                                        const unsigned int& pvi)
{
   if (pvi != PV_VARIABLE_INDEX) return;  // sanity check

   // Note the connected state
   //
   const bool isConnected = connectionInfo.isChannelConnected ();

   // Display the connected state
   //
   this->updateToolTipConnection (isConnected, pvi);

   // Change style to reflect being connected/disconnected.
   //
   this->processConnectionInfo (isConnected, pvi);

   this->valueLabel->setEnabled (isConnected);

   this->isFirstUpdate = true;  // more trob. than it's worth to check if connect or disconnect.

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   this->emitDbConnectionChanged (pvi);
}

//------------------------------------------------------------------------------
//
void QEDistribution::setPvValue (const double& value, QCaAlarmInfo& alarmInfo,
                                 QCaDateTime&, const unsigned int& pvi)
{
   if (pvi != PV_VARIABLE_INDEX) return;  // sanity check

   qcaobject::QCaObject* qca = this->getQcaItem (PV_VARIABLE_INDEX);
   if (!qca) return;  // sanity check

   if (this->isFirstUpdate) {
      this->stringFormatting.setArrayAction (QEStringFormatting::INDEX);
      this->stringFormatting.setDbEgu (qca->getEgu ());
      this->stringFormatting.setDbEnumerations (qca->getEnumerations ());
      this->stringFormatting.setDbPrecision (qca->getPrecision ());
   }

   QString text = this->stringFormatting.formatString (value, this->getArrayIndex ());
   this->valueLabel->setText (text);
   this->valueLabel->setStyleSheet (alarmInfo.style ());

   // Do stats, provided the value is not invalid
   //
   if (!alarmInfo.isInvalid()) {

      // Do the PV statistics
      //
      this->valueCount += 1.0;

      if (this->valueCount <= 1.0) {
         this->valueMin = value;
         this->valueMax = value;
      } else {
         this->valueMin = MIN (this->valueMin, value);
         this->valueMax = MAX (this->valueMax, value);
      }

      this->valueSum += value;
      this->valueSquaredSum += value*value;

      this->valueMean = this->valueSum / this->valueCount;

      // Variance:  mean (x^2) - mean (x)^2
      //
      double variance = (this->valueSquaredSum / this->valueCount) -
                        (this->valueMean*this->valueMean);

      // Rounding errors can lead to very small negative variance values (of the
      // order of -8.8e-16) which leads to NaN standard deviation values which then
      // causes a whole heap of issues: ensure the variance is non-negative.
      //
      variance = MAX (variance, 0.0);
      this->valueStdDev = sqrt (variance);

      this->countValueLabel->setNum (int (this->valueCount));
      this->meanValueLabel->setNum (this->valueMean);
      this->minValueLabel->setNum (this->valueMin);
      this->maxValueLabel->setNum (this->valueMax);
      this->stdDevLabel->setNum (this->valueStdDev);

      this->historicalData.append (value);

      // This may update currentPlotMax, currentPlotMax etc.
      //
      bool updated = this->updatePlotLimits ();

      // Update if needs be
      //
      if (!updated) this->updateDistribution ();

      this->replotIsRequired = true;
   }

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (alarmInfo, pvi);

   // Lastly signal a database value change to any Link (or other) widgets
   // using one of the dbValueChanged signals declared in header file.
   //
   this->emitDbValueChanged (pvi);

   this->isFirstUpdate = false;
}

//------------------------------------------------------------------------------
//
void QEDistribution::runPVSelectDialog (bool)
{
   QString currentName = this->getSubstitutedVariableName (PV_VARIABLE_INDEX);
   this->pvNameSelectDialog->setPvName (currentName);
   const int n = this->pvNameSelectDialog->exec (this->pvNameLabel);
   if (n == 1) {
      // User has selected okay, has the PV name changed ?
      //
      const QString newName = this->pvNameSelectDialog->getPvName ();
      if (newName != currentName) {
         this->setPvName (newName);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEDistribution::resetButtonClicked (bool)
{
   this->resetDistibution ();
}

//------------------------------------------------------------------------------
//
void QEDistribution::contextMenuRequested (const QPoint& pos)
{
   QWidget* theSender = dynamic_cast <QWidget*> (this->sender ());
   if (!theSender) return;
   const QPoint golbalPos = theSender->mapToGlobal (pos);

   const bool inUse = !this->getSubstitutedVariableName (PV_VARIABLE_INDEX).isEmpty ();
   this->pasteAction->setEnabled (!inUse);
   this->clearAction->setEnabled (inUse);
   this->contextMenu->exec (golbalPos);
}

//------------------------------------------------------------------------------
//
void QEDistribution::contextMenuSelected (QAction* selectedItem)
{
   QClipboard *cb = NULL;
   QString pasteText;

   bool okay;
   const int iData = selectedItem->data ().toInt (&okay);

   const ContextMenuActions maData = ContextMenuActions (iData);
   switch (maData) {

      case maAddEdit:
         this->runPVSelectDialog (false);
         break;

      case maPaste:
         cb = QApplication::clipboard ();
         pasteText = cb->text().trimmed ();

         if (pasteText.isEmpty ()) break;
         this->setPvName (pasteText);
         break;

      case maClear:
         this->pvNameLabel->setText ("");

         this->setVariableNameAndSubstitutions ("", "", PV_VARIABLE_INDEX);
         this->deactivate ();

         this->valueLabel->setText ("");

         this->resetDistibution();
         break;

      default:
         DEBUG << "contextMenuTriggered - invalid action" << iData;
         break;
   }
}

//------------------------------------------------------------------------------
//
QString QEDistribution::copyVariable ()
{
   return this->getSubstitutedVariableName (PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
// What do we copy? Last value? The distribition?
//
QVariant QEDistribution::copyData ()
{
   QVariant result;
   /// return this->getQcaItem (PV_VARIABLE_INDEX)->getLastData();
   return result;
}

//------------------------------------------------------------------------------
//
void QEDistribution::paste (QVariant v)
{
   QString newName = v.toString ();
   this->setPvName (newName);
}

//------------------------------------------------------------------------------
//
void QEDistribution::createWidgets()
{
   static const int sg = 4;   // small gap - between name label and value label
   static const int bg = 12;  // big gap - between name/value label pairs.

   int left;

   this->verticalLayout = new QVBoxLayout (this);
   this->verticalLayout->setSpacing (2);
   this->verticalLayout->setContentsMargins (4, 4, 4, 4);

   this->resize_frame = new QEResizeableFrame (this);
   this->resize_frame->setMinimumSize (QSize (16, 60));
   this->resize_frame->setMaximumSize (QSize (QWIDGETSIZE_MAX, 60));
   this->resize_frame->setAllowedMinimum (8);
   this->resize_frame->setAllowedMaximum (60);

   // First row.
   //
   left = 8;

   this->pushButton = new QPushButton (this->resize_frame);
   this->pushButton->setGeometry (QRect (left, 7, 24, 18)); left += 24 + sg;
   this->pushButton->setText ("X");
   QFont font0;
   QFont font1;
   QFont font2;

   font0.setPointSize (8);
   font1.setFamily (QLatin1String ("Sans Serif"));
   font1.setPointSize (10);
   font2.setFamily (QLatin1String ("Sans Serif"));
   font2.setPointSize (8);

   this->pushButton->setFont (font0);
   this->pushButton->setFocusPolicy (Qt::NoFocus);

   this->pvNameLabel = new QLabel (this->resize_frame);
   this->pvNameLabel->setGeometry (QRect (left, 8, 420, 16)); left += 420 + sg;
   this->pvNameLabel->setText ("NAME...");
   this->pvNameLabel->setFont (font1);
   this->pvNameLabel->setStyleSheet (QEUtilities::colourToStyle (QColor (155, 205, 255)));
   this->pvNameLabel->setIndent (6);

   this->valueLabel = new QLabel (this->resize_frame);
   this->valueLabel->setGeometry (QRect (left, 8, 148, 16)); left += 148 + sg;
   this->valueLabel->setAlignment (Qt::AlignRight|Qt::AlignVCenter);
   this->valueLabel->setIndent (6);
   this->valueLabel->setStyleSheet (QEUtilities::offBackgroundStyle ());

   this->resetButton = new QPushButton (this->resize_frame);
   this->resetButton->setToolTip (" Reset/clear all data ");
   this->resetButton->setText ("Reset");
   this->resetButton->setGeometry (QRect (left, 7, 52, 20)); left += 52 + sg;
   this->resetButton->setFont (font0);
   this->resetButton->setFocusPolicy (Qt::NoFocus);
   this->resetButton->setStyleSheet (QEUtilities::colourToStyle (QColor (255, 255, 128)));

   this->gaussianBox = new QCheckBox (this->resize_frame);
   this->gaussianBox->setToolTip (" Show/hide gaussian curve ");
   this->gaussianBox->setText ("gaussian");
   this->gaussianBox->setGeometry (QRect (left, 7, 72, 20)); left += 72 + sg;
   this->gaussianBox->setFont (font0);
   this->gaussianBox->setFocusPolicy (Qt::NoFocus);

   // Next row
   //
   left = 8;

   this->label_1 = new QLabel (this->resize_frame);
   this->label_1->setText ("Number:");
   this->label_1->setGeometry (QRect (left, 32, 52, 16));
   left += 52 + sg;
   this->label_1->setFont (font2);

   this->countValueLabel = new QLabel (this->resize_frame);
   this->countValueLabel->setText ("0");
   this->countValueLabel->setGeometry (QRect (left, 32, 64, 16)); left += 64 + bg;
   this->countValueLabel->setStyleSheet (QEUtilities::colourToStyle (QColor (224,224,224)));
   this->countValueLabel->setAlignment (Qt::AlignRight|Qt::AlignVCenter);
   this->countValueLabel->setIndent (6);

   this->label_2 = new QLabel (this->resize_frame);
   this->label_2->setText ("Mean:");
   this->label_2->setGeometry (QRect (left, 32, 36, 16));
   left += 36 + sg;
   this->label_2->setFont (font2);

   this->meanValueLabel = new QLabel (this->resize_frame);
   this->meanValueLabel->setText ("0.0");
   this->meanValueLabel->setGeometry (QRect (left, 32, 100, 16)); left += 100 + bg;
   this->meanValueLabel->setStyleSheet (QEUtilities::colourToStyle (QColor (224,224,224)));
   this->meanValueLabel->setAlignment (Qt::AlignRight|Qt::AlignVCenter);
   this->meanValueLabel->setIndent (6);

   this->label_4 = new QLabel (this->resize_frame);
   this->label_4->setText ("Min:");
   this->label_4->setGeometry (QRect (left, 32, 28, 16)); left += 28 + sg;
   this->label_4->setFont (font2);

   this->minValueLabel = new QLabel (this->resize_frame);
   this->minValueLabel->setText ("0.0");
   this->minValueLabel->setGeometry (QRect (left, 32, 100, 16)); left += 100 + bg;
   this->minValueLabel->setStyleSheet (QEUtilities::colourToStyle (QColor (224,224,224)));
   this->minValueLabel->setAlignment (Qt::AlignRight|Qt::AlignVCenter);
   this->minValueLabel->setIndent (6);

   this->label_5 = new QLabel (this->resize_frame);
   this->label_5->setText ("Max:");
   this->label_5->setGeometry (QRect (left, 32, 28, 16)); left += 28 + sg;
   this->label_5->setFont (font2);

   this->maxValueLabel = new QLabel (this->resize_frame);
   this->maxValueLabel->setText ("0.0");
   this->maxValueLabel->setGeometry (QRect (left, 32, 100, 16)); left += 100 + bg;
   this->maxValueLabel->setStyleSheet (QEUtilities::colourToStyle (QColor (224,224,224)));
   this->maxValueLabel->setAlignment (Qt::AlignRight|Qt::AlignVCenter);
   this->maxValueLabel->setIndent (6);

   this->label_6 = new QLabel (this->resize_frame);
   this->label_6->setText ("Std Dev:");
   this->label_6->setGeometry (QRect (left, 32, 48, 16)); left += 48 + sg;
   this->label_6->setFont (font2);

   this->stdDevLabel = new QLabel (this->resize_frame);
   this->stdDevLabel->setText ("0.0");
   this->stdDevLabel->setGeometry (QRect (left, 32, 100, 16)); left += 100 + bg;
   this->stdDevLabel->setStyleSheet (QEUtilities::colourToStyle (QColor (224,224,224)));
   this->stdDevLabel->setAlignment (Qt::AlignRight|Qt::AlignVCenter);
   this->stdDevLabel->setIndent (6);

   this->plotArea = new QWidget (this);

   this->currentXPlotMin = -3.0;
   this->currentXPlotMax = +3.0;
   this->currentXPlotMinor = 0.2;

   this->xAxis = new QEAxisPainter (this->plotArea);
   this->xAxis->setHasAxisLine (true);
   this->xAxis->setOrientation (QEAxisPainter::Left_To_Right);
   this->xAxis->setFixedHeight (32);
   this->xAxis->setMinimum (this->currentXPlotMin);
   this->xAxis->setMaximum (this->currentXPlotMax);
   this->xAxis->setMinorInterval (this->currentXPlotMinor);
   this->xAxis->setMajorMinorRatio (MAJOR_MINOR_RATIO);
   this->xAxis->setPrecision (2);

   this->currentYPlotMin = 0.0;
   this->currentYPlotMax = 1.0;
   this->currentYPlotMinor = 0.1;

   this->yAxis = new QEAxisPainter (this->plotArea);
   this->yAxis->setHasAxisLine (true);
   this->yAxis->setOrientation (QEAxisPainter::Bottom_To_Top);
   this->yAxis->setFixedWidth (52);
   this->yAxis->setMinimum (this->currentYPlotMin);
   this->yAxis->setMaximum (this->currentYPlotMax);
   this->yAxis->setMinorInterval (this->currentYPlotMinor);
   this->yAxis->setMajorMinorRatio (MAJOR_MINOR_RATIO);
   this->yAxis->setPrecision (2);

   this->verticalLayout->addWidget (resize_frame);
   this->verticalLayout->addWidget (plotArea);
}

//------------------------------------------------------------------------------
//
void QEDistribution::connectSignalsToSlots ()
{
   QObject::connect (this->pushButton, SIGNAL (clicked (bool)),
                     this, SLOT (runPVSelectDialog (bool)));

   QObject::connect (this->resetButton, SIGNAL (clicked (bool)),
                     this, SLOT (resetButtonClicked (bool)));

   QObject::connect (this->gaussianBox, SIGNAL (clicked (bool)),
                     this, SLOT (setShowGaussian (const bool)));

   QObject::connect (QEDistribution::tickTimer, SIGNAL (timeout ()),
                     this, SLOT   (tickTimeout ()));

   QObject::connect (this->pvNameLabel, SIGNAL (customContextMenuRequested (const QPoint &)),
                     this,              SLOT   (contextMenuRequested       (const QPoint &)));

   QObject::connect (this->contextMenu, SIGNAL (triggered            (QAction*)),
                     this,              SLOT   (contextMenuSelected  (QAction*)));
}

//------------------------------------------------------------------------------
//
void QEDistribution::resetDistibution ()
{
   this->historicalData.clear ();
   this->valueCount = 0.0;
   this->valueSum = 0.0;
   this->valueSquaredSum = 0.0;
   this->valueMean = 0.0;
   this->valueStdDev = 0.0;
   this->valueMin = 0.0;
   this->valueMax = 0.0;

   this->distributionCount = 0;
   for (int j = 0; j < ARRAY_LENGTH (this->distributionData); j++) {
      this->distributionData [j] = 0;
   }
}

//------------------------------------------------------------------------------
//
void QEDistribution::setReadOut (const QString& text)
{
   message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STATUS);
   this->sendMessage (text, mt);
}

//------------------------------------------------------------------------------
//
double QEDistribution::calcPlotDelta () const
{
   const double spread = 2.0 * this->mNumberStdDevs * this->valueStdDev;

   // Calc plot delta - belts 'n' braces re divide by 0
   //
   double result = spread / double (this->distributionCount);
   if (result <= 1.0e-9) {
      result = 1.0e-9;
   }
   return result;
}

//------------------------------------------------------------------------------
// static
bool QEDistribution::changed (const double a, const double b, const double e)
{
    if (a == b) return false;
    double d = ABS (a-b);
    double s = 0.5*(ABS (a) + ABS (b));
    return d >= e*s;
}

// end
