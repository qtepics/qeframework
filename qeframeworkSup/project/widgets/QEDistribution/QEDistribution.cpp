/*  QEDistribution.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2019-2023 Australian Synchrotron.
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
#include <alarm.h>

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
enum Constants {
   PV_VARIABLE_INDEX   = 0,
   MAJOR_MINOR_RATIO   = 5,
   MAXIMUM_DATA_POINTS = 10000
};


// We use a shared timer for all QEDistribution instances.
// static
QTimer* QEDistribution::tickTimer = NULL;

//------------------------------------------------------------------------------
//
QEDistribution::QEDistribution (QWidget* parent) :
   QEAbstractDynamicWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->setup ();
}

//------------------------------------------------------------------------------
//
QEDistribution::QEDistribution (const QString& variableNameIn,
                                QWidget* parent) :
   QEAbstractDynamicWidget (parent),
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
   this->setFrameShadow (QFrame::Sunken);

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
   this->recalcIsRequired = true;
   this->replotIsRequired = true;

   // Ensure other values are at least valid/semi-sensible.
   //
   this->distributionCount = 0;
   this->distributionIncrement = 1.0;

   // Initate gathering of archive data - specifically the PV name list.
   //
   this->archiveAccess = new QEArchiveAccess (this);

   // Create forms/dialogs.
   //
   this->pvNameSelectDialog = new QEPVNameSelectDialog (this);

   this->isFirstUpdate = false;

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
   this->setDisplayAlarmStateOption (QE::Never);

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
   // The distribution sizing takes account of the indents.
   //
   this->xAxis->setIndent (yw - 2, 48);   // left, right
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

   // Now draw distribution
   //
   if (this->valueTotal <= 0.0) return;  // sanity check

   QPainter painter (this->plotArea);
   QPen pen;
   QBrush brush;
   QPointF polygon [2 * ARRAY_LENGTH (this->distributionData) + 6];  // num points + loop back

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
   const double x_plot_min = this->currentXPlotMin;
   const double x_plot_max = this->currentXPlotMax;
   const double ds = this->distributionIncrement;

   int number;
   if (this->mIsRectangular) {
      for (int j = 0; j < this->distributionCount; j++) {
         double u = ds * j + x_plot_min;
         double x = mx * u + cx;

         double p = double (this->distributionData [j]) / this->valueTotal; // proportion
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
         double p = double (this->distributionData [j]) / this->valueTotal; // proportion
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
   if (this->getShowGaussian() && (this->valueTotal > 0.0)) {

      QPointF gaussian [81];    // 81 a bit arbitary

      // width of each histogram bar - input units, not screen units.
      //
      const double plotDelta = this->distributionIncrement;

      // height of normal disribution density function.
      //
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
      const double x_plot_min = this->currentXPlotMin;
      const double plotDelta = this->distributionIncrement;  // size of bar

      const double slot = (x - x_plot_min) / plotDelta;
      if (slot >= 0.0 && slot < this->distributionCount) {
         int j = int (slot);
         double x1 = x_plot_min + (j + 0.0)*plotDelta;
         double x2 = x_plot_min + (j + 1.0)*plotDelta;

         double f = 100.0 * double (this->distributionData [j]) / this->valueTotal;

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
void QEDistribution::updatePlotLimits ()
{
   // Calculate plotted sample range - n standard deviations each side.
   // But ensure range is non-zero.
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

   // Do an update
   //
   this->currentXPlotMin = plotMin;
   this->currentXPlotMax = plotMax;
   this->currentXPlotMinor = plotMinor;

   this->xAxis->setMinimum (plotMin);
   this->xAxis->setMaximum (plotMax);
   this->xAxis->setMinorInterval (plotMinor);

   // Choose precision based of value of major interval.
   // The smaller plotMajor, the larger -log (plotMajor).
   //
   const int xp = int (1.0 - LOG10 (plotMajor));
   this->xAxis->setPrecision (xp);

   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEDistribution::updateDistribution ()
{
   const double span = this->currentXPlotMax - this->currentXPlotMin;
   double realNumberOfBin = span / MAX (1.0e-12, this->currentXPlotMinor);
   int numberOfBin = qRound (realNumberOfBin);
   numberOfBin = MAX (1, numberOfBin);

   // As we get more points, increase the number of bins to get a better
   // resolution.
   //
   const int count = this->pvData.count();
   if (count >= 400) numberOfBin *= 2;
   if (count >= 800) numberOfBin *= 2;

   // However ensure within range
   //
   this->distributionCount = LIMIT (numberOfBin, 1, ARRAY_LENGTH (this->distributionData));
   this->distributionIncrement = span / double (this->distributionCount);
   this->distributionIncrement = MAX (1.0e-9,  this->distributionIncrement);  // avoid divide by 0

   // Distribute values over the distribution data array.
   //
   this->pvData.distribute (this->distributionData, this->distributionCount,
                            true, this->currentXPlotMin, this->distributionIncrement);

   // Find the total and also find the max value so that we can calculate
   // a sensible y scale.
   //
   double distributionMax = 1.0;
   this->valueTotal = 0.0;
   for (int j = 0; j < this->distributionCount; j++) {
      distributionMax = MAX (distributionMax, this->distributionData [j]);
      this->valueTotal += this->distributionData [j];
   }

   // Now calclate the fractional max - this is in range  >0.0 to 1.0
   // We plot fractional values.
   //
   const double fractionalMax =
         this->valueTotal > 0.0 ? (distributionMax / this->valueTotal) :  1.0;

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

   // Do an update
   //
   this->currentYPlotMin = plotMin;
   this->currentYPlotMax = plotMax;
   this->currentYPlotMinor = plotMinor;

   this->yAxis->setMinimum (plotMin);
   this->yAxis->setMaximum (plotMax);
   this->yAxis->setMinorInterval (plotMinor);
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
      recalcIsRequired = true;
   }

   if (this->recalcIsRequired) {
      // Each second re-do the calculations
      // NOTE: Irrespective of PV update rate and/or other replot
      //       request, the stats are only calculated once per second.
      //
      // Recalc the stats, and check is calc okay.
      //
      QCaDataPointList::Statistics stats;
      if (this->pvData.calculateStatistics (stats, true)) {
         // Yes - the calc is okay.
         //
         this->countValueLabel->setNum (this->pvData.count ());
         this->meanValueLabel->setNum (stats.mean);
         this->minValueLabel->setNum (stats.minimum);
         this->maxValueLabel->setNum (stats.maximum);
         this->stdDevLabel->setNum (stats.stdDeviation);

         // Save the mean and standard devistion.
         //
         this->valueMean = stats.mean;
         this->valueStdDev = stats.stdDeviation;

         // This may update currentPlotMax, currentPlotMax etc.
         //
         this->updatePlotLimits ();

         // Update the distribution
         //
         this->updateDistribution ();

         this->replotIsRequired = true;
         this->recalcIsRequired = false;
      }
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
   this->recalcIsRequired = true;
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
int QEDistribution::addPvName (const QString& pvName)
{
   this->setPvName (pvName);
   return PV_VARIABLE_INDEX;
}

//------------------------------------------------------------------------------
//
void QEDistribution::clearAllPvNames ()
{
    this->setPvName ("");
}

//------------------------------------------------------------------------------
// programtic call
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

   if (!isConnected && (this->pvData.count() >= 1)) {
      // We have a channel disconnect.
      //
      // create a dummy point with last value and time now.
      //
      QCaDataPoint point = this->pvData.last ();
      point.datetime = QDateTime::currentDateTime ().toUTC ();
      this->pvData.append (point);

      // create a dummy point with same time but marked invalid to indicate a break.
      //
      point.alarm = QCaAlarmInfo (NO_ALARM, INVALID_ALARM);
      this->pvData.append (point);
   }

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
                                 QCaDateTime& timestamp, const unsigned int& pvi)
{
   if (pvi != PV_VARIABLE_INDEX) return;  // sanity check

   qcaobject::QCaObject* qca = this->getQcaItem (PV_VARIABLE_INDEX);
   if (!qca) return;  // sanity check

   if (this->isFirstUpdate) {
      this->stringFormatting.setArrayAction (QE::Index);
      this->stringFormatting.setDbEgu (qca->getEgu ());
      this->stringFormatting.setDbEnumerations (qca->getEnumerations ());
      this->stringFormatting.setDbPrecision (qca->getPrecision ());
   }

   QString text = this->stringFormatting.formatString (value, this->getArrayIndex ());
   this->valueLabel->setText (text);
   this->valueLabel->setStyleSheet (alarmInfo.style ());

   // Save the point - add to the PV data set.
   //
   QCaDataPoint point;

   point.value = value;
   point.datetime = timestamp;
   point.alarm = alarmInfo;
   this->pvData.append (point);

   // Don't let this data set tooo big.
   //
   if (this->pvData.count () >= MAXIMUM_DATA_POINTS) {
      this->pvData.removeFirst();
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
//
QVariant QEDistribution::copyData ()
{
   const double x_plot_min = this->currentXPlotMin;
   const double plotDelta = this->distributionIncrement;

   QString result;

   for (int j = 0; j < this->distributionCount; j++) {
      double x = x_plot_min + j*plotDelta;
      double y = this->distributionData [j];
      result.append (QString ("%1 %2\n").arg (x, -15).arg (y));
   }

   return QVariant (result);
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
void QEDistribution::saveConfiguration (PersistanceManager* pm)
{
   const QString formName = this->getPersistantName();

   PMElement formElement = pm->addNamedConfiguration (formName);

   // Note: we save the subsituted name (as opposed to template name and any macros).
   //
   QString pvName;
   pvName= this->getSubstitutedVariableName (PV_VARIABLE_INDEX);
   formElement.addValue ("PvName", pvName);

   formElement.addValue ("NumberStdDevs", this->getNumberStdDevs ());
   formElement.addValue ("IsRectangular", this->getIsRectangular ());
   formElement.addValue ("EdgeWidth", this->  getEdgeWidth ());
   formElement.addValue ("ShowGaussian", this-> getShowGaussian ());
   formElement.addValue ("GaussianWidth", this-> getGaussianWidth ());
   formElement.addValue ("BackgroundColour", this->getBackgroundColour ());
   formElement.addValue ("EdgeColour", this-> getEdgeColour ());
   formElement.addValue ("FillColour", this-> getFillColour ());
   formElement.addValue ("GaussianColour", this-> getGaussianColour ());
}

//------------------------------------------------------------------------------
//
void QEDistribution::restoreConfiguration (PersistanceManager* pm,
                                           restorePhases restorePhase)
{
   if (restorePhase != FRAMEWORK) return;

   const QString formName = this->getPersistantName ();
   PMElement formElement = pm->getNamedConfiguration (formName);
   if (formElement.isNull ()) return;   // sainity check

   bool status;
   QString pvName;
   double d;
   bool b;
   int i;
   QColor k;

   status = formElement.getValue ("PvName", pvName);
   if (status) {
      this->setPvName (pvName);
   }
   status = formElement.getValue ("NumberStdDevs", d);
   if (status) {
      this->setNumberStdDevs (d);
   }
   status = formElement.getValue ("IsRectangular", b);
   if (status) {
      this->setIsRectangular (b);
   }
   status = formElement.getValue ("EdgeWidth", i);
   if (status) {
      this->setEdgeWidth (i);
   }
   status = formElement.getValue ("ShowGaussian", b);
   if (status) {
      this->setShowGaussian (b);
   }
   status = formElement.getValue ("GaussianWidth", i);
   if (status) {
      this->setGaussianWidth (i);
   }
   status = formElement.getValue ("BackgroundColour", k);
   if (status) {
      this->setBackgroundColour (k);
   }
   status = formElement.getValue ("EdgeColour", k);
   if (status) {
      this->setEdgeColour (k);
   }
   status = formElement.getValue ("FillColour", k);
   if (status) {
      this->setFillColour (k);
   }
   status = formElement.getValue ("GaussianColour", k);
   if (status) {
      this->setGaussianColour (k);
   }
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

   // Ensure initial values are valid/semi-sensible.
   //
   this->currentXPlotMin = -3.0;
   this->currentXPlotMax = +3.0;
   this->currentXPlotMinor = 0.2;

   this->xAxis = new QEAxisPainter (this->plotArea);
   this->xAxis->setHasAxisLine (true);
   this->xAxis->setOrientation (Qt::Horizontal);
   this->xAxis->setInvertedAppearance (false);
   this->xAxis->setFixedHeight (32);
   this->xAxis->setMinimum (this->currentXPlotMin);
   this->xAxis->setMaximum (this->currentXPlotMax);
   this->xAxis->setMinorInterval (this->currentXPlotMinor);
   this->xAxis->setMajorMinorRatio (MAJOR_MINOR_RATIO);
   this->xAxis->setPrecision (2);

   // Ensure initial values are valid/semi-sensible.
   //
   this->currentYPlotMin = 0.0;
   this->currentYPlotMax = 1.0;
   this->currentYPlotMinor = 0.1;

   this->yAxis = new QEAxisPainter (this->plotArea);
   this->yAxis->setHasAxisLine (true);
   this->yAxis->setOrientation (Qt::Vertical);
   this->yAxis->setInvertedAppearance (false);
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
   const QString nil ("n/a");

   this->pvData.clear ();
   this->valueTotal = 0.0;
   this->valueMean = 0.0;
   this->valueStdDev = 0.0;

   this->distributionCount = 0;
   for (int j = 0; j < ARRAY_LENGTH (this->distributionData); j++) {
      this->distributionData [j] = 0.0;
   }

   this->countValueLabel->setText (nil);
   this->meanValueLabel->setText (nil);
   this->minValueLabel->setText (nil);
   this->maxValueLabel->setText (nil);
   this->stdDevLabel->setText (nil);
}

//------------------------------------------------------------------------------
//
void QEDistribution::setReadOut (const QString& text)
{
   message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STATUS);
   this->sendMessage (text, mt);
}

// end
