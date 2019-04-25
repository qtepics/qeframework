/*  QEStripChartStatistics.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2019 Australian Synchrotron.
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
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#include "QEStripChartStatistics.h"
#include "QEStripChartItem.h"
#include <math.h>
#include <QColor>
#include <QDebug>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QECommon.h>
#include <QCaDateTime.h>
#include <QEDisplayRanges.h>
#include "ui_QEStripChartStatistics.h"

#define DEBUG qDebug () << "QEStripChartStatistics" << __LINE__ << __FUNCTION__ << "  "

// Much of this code is effectively the same as in the QEDistribution widget.
// Do keep these two items aligned as far as reasonably possible.
//
#define MAJOR_MINOR_RATIO     5

//------------------------------------------------------------------------------
//
QEStripChartStatistics::QEStripChartStatistics (const QString& pvNameIn,
                                                const QString& eguIn,
                                                const QCaDataPointList& dataList,
                                                QEStripChartItem* ownerIn,
                                                QWidget *parent) :
   QWidget (parent),
   ui (new Ui::QEStripChartStatistics)
{
   this->ui->setupUi (this);

   // Note: the axis objects are NOT parented.
   //
   this->xAxis = new QEAxisPainter (NULL);
   this->xAxis->setOrientation (QEAxisPainter::Left_To_Right);
   this->xAxis->setHasAxisLine (true);
   this->xAxis->setMajorMinorRatio (MAJOR_MINOR_RATIO);

   this->yAxis = new QEAxisPainter (NULL);
   this->yAxis->setOrientation (QEAxisPainter::Bottom_To_Top);
   this->yAxis->setHasAxisLine (true);
   this->yAxis->setMajorMinorRatio (MAJOR_MINOR_RATIO);
   this->yAxis->setPrecision (2);

   this->pvName = pvNameIn;
   this->egu = eguIn;
   this->owner = ownerIn;

   this->ui->plotFrame->installEventFilter (this);

   QObject::connect (this->ui->updateButton, SIGNAL (clicked       (bool)),
                     this,                   SLOT   (updateClicked (bool)));

   this->processDataList (dataList);
}

//------------------------------------------------------------------------------
//
QEStripChartStatistics::~QEStripChartStatistics()
{
   delete this->xAxis;
   delete this->yAxis;
   delete this->ui;
}

//------------------------------------------------------------------------------
//
void QEStripChartStatistics::updateClicked (bool)
{
   if (this->owner) {
      processDataList (this->owner->extractPlotPoints (false));
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartStatistics::processDataList (const QCaDataPointList& dataList)
{
   // Do stats - populate fields.
   // This form is not directly EPICS aware. Can use a basic form.
   //
   const int n = dataList.count ();

   this->clearLabels ();

   this->ui->pvNameLabel->setText (pvName);
   this->ui->numberOfPointsLabel->setText (QString ("%1").arg (n));

   if (n <= 0) return;

   QCaDateTime startTime;
   QCaDateTime endTime;
   QString format ("yyyy-MM-dd hh:mm:ss");

   startTime = dataList.value (0).datetime;
   this->ui->startTimeLabel->setText (startTime.toString (format) + "  " + QEUtilities::getTimeZoneTLA (startTime));

   endTime = dataList.value (n - 1).datetime;
   this->ui->endTimeLabel->setText (endTime.toString (format) + "  " + QEUtilities::getTimeZoneTLA (endTime));

   double duration = startTime.secondsTo (endTime);

   this->ui->durationLabel->setText (QEUtilities::intervalToString (duration, 0, true));

   this->valueCount = 0;
   this->valueMean = 0.0;
   this->valueStdDev = 0.0;

   double minimum = +9.99E+99;
   double maximum = -9.99E+99;
   double initialValue = 0.0;
   double finalValue = 0.0;

   double sumWeight = 0.0;
   double sumValue = 0.0;
   double sumValueSquared = 0.0;

   double sumX = 0.0;     // X here is time - relative to first time.
   double sumY = 0.0;
   double sumXX = 0.0;
   double sumXY = 0.0;

   for (int j = 0; j < n; j++) {
      const QCaDataPoint point = dataList.value (j);

      if (point.isDisplayable()) {
         this->valueCount++;

         minimum = MIN (minimum, point.value);
         maximum = MAX (maximum, point.value);

         if (this->valueCount == 1) {
            // Save first point value.
            //
            initialValue = point.value;
         }

         // This may be the last valid point - keep it.
         //
         finalValue = point.value;

         // Values are time weighed - determine weight.
         //
         double weight;
         if (j < (n - 1)) {
            // This is not the last point - use the duration from this
            // point to the next point as the weight.
            //
            QCaDateTime nextTime = dataList.value (j + 1).datetime;
            weight = point.datetime.secondsTo (nextTime);

         } else {
            // There is no next point - use an arbitary weight.
            //
            weight = 1.0;
         }

         sumWeight += weight;
         sumValue += point.value * weight;
         sumValueSquared += point.value * point.value * weight;

         // Least squares.
         // For x, use time from first point.
         //
         double x = startTime.secondsTo (point.datetime);

         sumX += x;
         sumY += point.value;
         sumXX += x * x;
         sumXY += x * point.value;
      }
   }

   this->ui->validPointsLabel->setText (QString ("%1").arg (this->valueCount));

   // Can we do any sensible stats?
   //
   if (this->valueCount <= 0) return;

   // Yes - let's do some sums.
   //
   // Avoid the divide by zero.
   //
   sumWeight = MAX (1.0E-8, sumWeight);

   this->valueMean = sumValue / sumWeight;

   // Variance:  mean (x^2) - mean (x)^2
   //
   double variance = (sumValueSquared / sumWeight) -
                     (this->valueMean * this->valueMean);

   // Rounding errors can lead to very small negative variance values (of the
   // order of -8.8e-16) which leads to NaN standard deviation values which then
   // causes a whole heap of issues: ensure the variance is non-negative.
   //
   variance = MAX (variance, 0.0);
   this->valueStdDev = sqrt (variance);

   // Least Squares
   //
   double slope = 0.0;
   if (this->valueCount >= 2) {
      double delta = (this->valueCount * sumXX) - (sumX * sumX);
      slope = ((this->valueCount * sumXY) - (sumX * sumY)) / delta;
   }

   // Recall sumValue += (value * weight);, and weight in seconds.
   //
   double integral = sumValue;

   // Populate form fields.
   //
   const QString units = egu.isEmpty() ? "" : " " + egu;

   this->ui->meanLabel->setText (QString ("%1%2").arg (this->valueMean).arg (units));
   this->ui->minimumLabel->setText (QString ("%1%2").arg (minimum).arg (units));
   this->ui->maximumLabel->setText (QString ("%1%2").arg (maximum).arg (units));
   this->ui->minMaxDiffLabel->setText (QString ("%1%2").arg (maximum - minimum).arg (units));

   this->ui->firstLastDiffLabel->setText (QString ("%1%2").arg (finalValue - initialValue).arg (units));
   this->ui->standardDeviationLabel->setText (QString ("%1%2").arg (this->valueStdDev).arg (units));
   this->ui->meanRateOfChangeLabel->setText (QString ("%1%2/sec").arg (slope).arg (units));
   this->ui->areaUnderCurveLabel->setText (QString ("%1%2-sec").arg (integral).arg (units));

   // Cribbed from QEDsitribution.
   // Can we refactor code??
   // For a distribution over +/-3 standard deviations.
   //
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

   const double x_plot_min = this->valueMean - 3.0 * this->valueStdDev;
   const double x_plot_max = this->valueMean + 3.0 * this->valueStdDev;
   const double plotDelta = 6.0 * this->valueStdDev / this->distributionCount;
   for (int j = 0; j < n; j++) {
      const QCaDataPoint point = dataList.value (j);
      if (point.isDisplayable()) {
         const double slot = (point.value - x_plot_min) / plotDelta;

         // Check for out of range values.
         //
         if (slot < 0.0 || slot >= this->distributionCount) continue;

         const int s = int (slot);
         this->distributionData [s] += 1;
      }
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
         this->valueCount > 0 ?
            double (distributionMax) / double (this->valueCount) :
            1.0;

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

   this->xAxis->setMinimum (plotMin);
   this->xAxis->setMaximum (plotMax);
   this->xAxis->setMinorInterval (plotMinor);

   // Ditto y
   //
   displayRange.setRange (0.0, fractionalMax);
   displayRange.adjustMinMax (5, true, plotMin, plotMax, plotMajor);
   plotMinor = plotMajor / MAJOR_MINOR_RATIO;

   this->yAxis->setMinimum (plotMin);
   this->yAxis->setMaximum (plotMax);
   this->yAxis->setMinorInterval (plotMinor);

   this->ui->plotFrame->update();
}

//------------------------------------------------------------------------------
//
void QEStripChartStatistics::clearLabels ()
{
   const QString nil ("");

   this->ui->pvNameLabel->setText (nil);
   this->ui->numberOfPointsLabel->setText (nil);
   this->ui->startTimeLabel->setText (nil);
   this->ui->endTimeLabel->setText (nil);
   this->ui->durationLabel->setText (nil);
   this->ui->validPointsLabel->setText (nil);

   this->ui->meanLabel->setText (nil);
   this->ui->minimumLabel->setText (nil);
   this->ui->maximumLabel->setText (nil);
   this->ui->minMaxDiffLabel->setText (nil);

   this->ui->firstLastDiffLabel->setText (nil);
   this->ui->standardDeviationLabel->setText (nil);
   this->ui->meanRateOfChangeLabel->setText (nil);
   this->ui->areaUnderCurveLabel->setText (nil);
}

//------------------------------------------------------------------------------
//
bool QEStripChartStatistics::eventFilter (QObject* watched, QEvent* event)
{
   const QEvent::Type type = event->type ();

   switch (type) {

      case QEvent::Paint:
         if (watched == this->ui->plotFrame) {
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
void QEStripChartStatistics::paintDistribution ()
{
   // Origin with respect to bottom-left corner
   //
   const int leftMargin  = 50;             // x origin
   const int rightMargin = 32;
   const int topMargin = 16;
   const int bottomMargin = 32;            // y origin

   const int width = this->ui->plotFrame->width();
   const int xmin = leftMargin;            // screen min
   const int xmax = width - rightMargin;   // screen max

   const int height = this->ui->plotFrame->height();
   const int ymin = topMargin;             // screen min
   const int ymax = height - bottomMargin; // screen max

   // Set up axis painters
   //
   this->xAxis->setGap (height - bottomMargin);
   this->xAxis->setIndent (leftMargin, rightMargin);

   this->yAxis->setGap (width - leftMargin);
   this->yAxis->setIndent (topMargin, bottomMargin);

   this->xAxis->draw (this->ui->plotFrame);
   this->yAxis->draw (this->ui->plotFrame);

   // Now draw distribution
   //
   if (this->valueCount < 1) return;  // sanity check

   QPainter painter (this->ui->plotFrame);
   QPen pen;
   QBrush brush;

   QPointF polygon [2 * ARRAY_LENGTH (this->distributionData) + 6];  // 128 points + loop back

   pen.setWidth (2);
   pen.setColor (QColor("#2060a0"));   // edge colour - dark blue
   pen.setStyle (Qt::SolidLine);
   painter.setPen (pen);

   brush.setColor (QColor("#80c0ff")); // fill colour - light blue
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
   const double x_plot_min = this->valueMean - 3.0 * this->valueStdDev;
   const double x_plot_max = this->valueMean + 3.0 * this->valueStdDev;

   const double ds = (x_plot_max - x_plot_min) / double (this->distributionCount);

   int number = 0;
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

   // Loop back to the start
   //
   polygon [number + 0] = QPointF (mx * x_plot_max + cx, ymax);
   polygon [number + 1] = QPointF (mx * x_plot_min + cx, ymax);
   polygon [number + 2] = polygon [0];

   painter.drawPolygon (polygon, number + 3);

   // Now draw draw the gaussian curve
   //
   QPointF gaussian [81];    // 81 a bit arbitary

   // width of each histogram bar - input units, not screen units.
   const double plotDelta = 6.0 * this->valueStdDev / this->distributionCount;

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

   pen.setWidth (2);
   pen.setStyle (Qt::SolidLine);
   pen.setColor (QColor ("#ff0000"));  // red
   painter.setPen (pen);
   painter.drawPolyline (gaussian, ARRAY_LENGTH (gaussian));
}

// end
