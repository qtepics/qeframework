/*  QEStripChartStatistics.cpp
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
 *  Copyright (c) 2013 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#include <math.h>
#include <QDebug>
#include <QECommon.h>
#include <QCaDateTime.h>
#include "QEStripChartItem.h"

#include "QEStripChartStatistics.h"
#include "ui_QEStripChartStatistics.h"

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

   this->pvName = pvNameIn;
   this->egu = eguIn;
   this->owner = ownerIn;

   QObject::connect (this->ui->updateButton, SIGNAL (clicked       (bool)),
                     this,                   SLOT   (updateClicked (bool)));

   this->processDataList (dataList);
}

//------------------------------------------------------------------------------
//
void QEStripChartStatistics::updateClicked (bool)
{
   if (this->owner) {
      processDataList (this->owner->determinePlotPoints ());
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

   int validCount = 0;
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
      QCaDataPoint point = dataList.value (j);

      if (point.isDisplayable()) {
         validCount++;

         minimum = MIN (minimum, point.value);
         maximum = MAX (maximum, point.value);

         if (validCount == 1) {
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

   this->ui->validPointsLabel->setText (QString ("%1").arg (validCount));

   // Can we do any sensible stats?
   //
   if (validCount > 0) {
      // Yes - let's do some sums.
      //
      // Avoid the divide by zero.
      //
      sumWeight = MAX (1.0E-8, sumWeight);

      double mean = sumValue / sumWeight;

      // Variance:  mean (x^2) - mean (x)^2
      //
      double variance = (sumValueSquared / sumWeight) - (mean * mean);
      double stdDev = sqrt (variance);

      // Least Squares
      //
      double slope = 0.0;
      if (validCount >= 2) {
         double delta = (validCount * sumXX) - (sumX * sumX);
         slope = ((validCount * sumXY) - (sumX * sumY)) / delta;
      }

      // Recall sumValue += (value * weight);, and weight in seconds.
      //
      double integral = sumValue;

      // Populate form fields.
      //
      const QString units = egu.isEmpty() ? "" : " " + egu;

      this->ui->meanLabel->setText (QString ("%1%2").arg (mean).arg (units));
      this->ui->minimumLabel->setText (QString ("%1%2").arg (minimum).arg (units));
      this->ui->maximumLabel->setText (QString ("%1%2").arg (maximum).arg (units));
      this->ui->minMaxDiffLabel->setText (QString ("%1%2").arg (maximum - minimum).arg (units));

      this->ui->firstLastDiffLabel->setText (QString ("%1%2").arg (finalValue - initialValue).arg (units));
      this->ui->standardDeviationLabel->setText (QString ("%1%2").arg (stdDev).arg (units));
      this->ui->meanRateOfChangeLabel->setText (QString ("%1%2/sec").arg (slope).arg (units));
      this->ui->areaUnderCurveLabel->setText (QString ("%1%2-sec").arg (integral).arg (units));
   }
}

//------------------------------------------------------------------------------
//
QEStripChartStatistics::~QEStripChartStatistics()
{
   delete this->ui;
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

// end
