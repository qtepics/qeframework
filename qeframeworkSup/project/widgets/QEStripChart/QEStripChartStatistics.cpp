/*  QEStripChartStatistics.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
   this->xAxis->setOrientation (Qt::Horizontal);
   this->xAxis->setInvertedAppearance (false);
   this->xAxis->setHasAxisLine (true);
   this->xAxis->setMajorMinorRatio (MAJOR_MINOR_RATIO);

   this->yAxis = new QEAxisPainter (NULL);
   this->yAxis->setOrientation (Qt::Vertical);
   this->yAxis->setInvertedAppearance (false);
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
   this->ui->validPointsLabel->setText (QString ("%1").arg (n));

   QCaDataPointList::Statistics stats;
   // Can we do any sensible stats?
   //
   if (!dataList.calculateStatistics (stats, false)) {
      return;
   }

   this->valueMean = stats.mean;
   this->valueStdDev = stats.stdDeviation;

   // Set up the unit strings qualifier.
   // Cribbed directly from the Delphi strip_chart_pv_stats.pas unit.
   //
   QString units;
   QString slopeUnits;
   QString integralUnits;

   if (!this->egu.isEmpty ()) {
      // The PV has explicity units.
      //
      units = " " + this->egu;
      slopeUnits = " " + this->egu + "/sec";

      // Do specials for when the base units are xxx/s, xxx/sec, xxx/min or xxx/Hr.
      //
      const int eguLen = this->egu.length();
      if (this->egu.endsWith ("/sec")) {
         // xxx/sec-secs is just xxx
         integralUnits = " " + this->egu.mid (0, eguLen - 4);
      } else if (this->egu.endsWith ("/s")) {
         //  /sec and /s  both used
         integralUnits = " " + this->egu.mid (0, eguLen - 2);
      } else if (this->egu.endsWith ("/min")) {
         // Need to convert from xxx/min-secs to xxx/min-mins, i.e. xxx.
         //
         stats.integral /= 60.0;
         integralUnits = " " + this->egu.mid (0, eguLen - 4);
      } else if (this->egu.endsWith ("/Hr")) {
         // Need to convert from xxx/Hr-secs to xxx/Hr-Hrs, i.e. xxx.
         //
         stats.integral /= 3600.0;
         integralUnits = " " + this->egu.mid (0, eguLen - 3);
      } else {
         integralUnits = " " + this->egu + "-secs";
      }
   } else {
      // No units per se - easy.
      //
      units = "";
      slopeUnits = " /sec";
      integralUnits = " secs";
   }

   // Populate form fields.
   //
   this->ui->meanLabel->setText (QString ("%1%2").arg (stats.mean).arg (units));
   this->ui->minimumLabel->setText (QString ("%1%2").arg (stats.minimum).arg (units));
   this->ui->maximumLabel->setText (QString ("%1%2").arg (stats.maximum).arg (units));
   this->ui->minMaxDiffLabel->setText (QString ("%1%2").arg (stats.maximum - stats.minimum).arg (units));

   this->ui->firstLastDiffLabel->setText (QString ("%1%2").arg (stats.finalValue - stats.initialValue).arg (units));
   this->ui->standardDeviationLabel->setText (QString ("%1%2").arg (stats.stdDeviation).arg (units));
   this->ui->meanRateOfChangeLabel->setText (QString ("%1%2").arg (stats.slope).arg (slopeUnits));
   this->ui->areaUnderCurveLabel->setText (QString ("%1%2").arg (stats.integral).arg (integralUnits));

   // Data min/max
   //
   const double x_plot_min = this->valueMean - 3.0 * this->valueStdDev;
   const double x_plot_max = this->valueMean + 3.0 * this->valueStdDev;

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

   // Choose precision based of value of major interval.
   // The smaller plotMajor, the larger -log (plotMajor).
   //
   const int xp = int (1.0 - LOG10 (plotMajor));
   this->xAxis->setPrecision (xp);

   // Cribbed from QEDistribution.
   // Can we refactor code?
   // For a distribution over +/-3 standard deviations.
   //
   const double span = plotMax - plotMin;
   double realNumberOfBin = span / MAX (1.0e-12, plotMinor);
   int numberOfBin = qRound (realNumberOfBin);
   numberOfBin = MAX (1, numberOfBin);

   // As we get more points, increase the number of bins to get a better
   // resolution.
   //
   const int count = dataList.count();
   if (count >= 400) numberOfBin *= 2;
   if (count >= 800) numberOfBin *= 2;

   // However ensure within range
   //
   this->distributionCount = LIMIT (numberOfBin, 1, ARRAY_LENGTH (this->distributionData));
   this->distributionIncrement = span / double (this->distributionCount);
   this->distributionIncrement = MAX (1.0e-9,  this->distributionIncrement);  // avoid divide by 0

   // Distribute weighted values over the distribution data array.
   //
   dataList.distribute (this->distributionData, this->distributionCount,
                        false, plotMin, this->distributionIncrement);

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
         this->valueTotal > 0 ? (distributionMax / this->valueTotal) :  1.0;

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

   this->valueTotal = 0.0;
   this->valueMean = 0.0;
   this->valueStdDev = 0.0;

   this->distributionCount = 0;
   for (int j = 0; j < ARRAY_LENGTH (this->distributionData); j++) {
      this->distributionData [j] = 0.0;
   }

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
   if (this->valueTotal <= 0.0) return;  // sanity check

   QPainter painter (this->ui->plotFrame);
   QPen pen;
   QBrush brush;

   QPointF polygon [2 * ARRAY_LENGTH (this->distributionData) + 6];  // num points + loop back

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
   const double x_plot_min = this->xAxis->getMinimum();
   const double x_plot_max = this->xAxis->getMaximum();
   const double ds = this->distributionIncrement;

   int number = 0;
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

   pen.setWidth (2);
   pen.setStyle (Qt::SolidLine);
   pen.setColor (QColor ("#ff0000"));  // red
   painter.setPen (pen);
   painter.drawPolyline (gaussian, ARRAY_LENGTH (gaussian));
}

// end
