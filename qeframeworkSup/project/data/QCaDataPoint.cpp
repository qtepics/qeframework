/*  QCaDataPoint.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2024 Australian Synchrotron
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
 */

#include "QCaDataPoint.h"
#include <math.h>
#include <QDebug>
#include <QEArchiveInterface.h>
#include <QECommon.h>
#include <QEPlatform.h>

#define DEBUG  qDebug () << "QCaDataPoint" << __LINE__ <<  __FUNCTION__  << "  "

static const char* stdFormat = "dd/MMM/yyyy HH:mm:ss";

//==============================================================================
// QCaDataPoint methods
//==============================================================================
//
QCaDataPoint::QCaDataPoint ()
{
   this->value = 0.0;
}

//------------------------------------------------------------------------------
//
QCaDataPoint::QCaDataPoint (const QCaDataPoint& other)
{
   this->value = other.value;
   this->datetime = other.datetime;
   this->alarm = other.alarm;
}

//------------------------------------------------------------------------------
//
QCaDataPoint::~QCaDataPoint () {}

//------------------------------------------------------------------------------
//
QCaDataPoint& QCaDataPoint::operator=(const QCaDataPoint& other)
{
   this->value = other.value;
   this->datetime = other.datetime;
   this->alarm = other.alarm;
   return *this;
}

//------------------------------------------------------------------------------
//
bool QCaDataPoint::isDisplayable () const
{
   bool result;
   QEArchiveInterface::archiveAlarmSeverity severity;

   severity = (QEArchiveInterface::archiveAlarmSeverity) this->alarm.getSeverity ();

   switch (severity) {

      case QEArchiveInterface::archSevNone:
      case QEArchiveInterface::archSevMinor:
      case QEArchiveInterface::archSevMajor:
      case QEArchiveInterface::archSevEstRepeat:
      case QEArchiveInterface::archSevRepeat:
         // Infinites and NaNs are not displayable.
         //
         result = !(QEPlatform::isNaN (this->value) ||
                    QEPlatform::isInf (this->value));
         break;

      case QEArchiveInterface::archSevInvalid:
      case QEArchiveInterface::archSevDisconnect:
      case QEArchiveInterface::archSevStopped:
      case QEArchiveInterface::archSevDisabled:
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
QString QCaDataPoint::toString () const
{
   QString result;
   QString zone;
   QString valid = "?";
   QEArchiveInterface::archiveAlarmSeverity severity;
   QString severityText = "?";
   QString statusText = "?";

   zone = QEUtilities::getTimeZoneTLA (this->datetime);
   valid = this->isDisplayable () ? "True " : "False";
   severity = (QEArchiveInterface::archiveAlarmSeverity) this->alarm.getSeverity ();
   severityText = QEArchiveInterface::alarmSeverityName (severity);
   statusText = this->alarm.statusName();

   // At some timme the MMM (from the format string) changed from "Jan" to "Jan."
   // So much for backward compatibility. Lose the "." if it exists.
   //
   QString dateTimeImage = datetime.toString (stdFormat);
   if (dateTimeImage [6] == '.') {
      dateTimeImage = dateTimeImage.remove (6, 1);
   }

   result = QString ("%1  %2  %3  %4  %5  %6")
               .arg (dateTimeImage, 20)
               .arg (zone)
               .arg (this->value, 16, 'e', 8)
               .arg (valid, 10)
               .arg (severityText, 10)
               .arg (statusText, 10);

   return result;
}

//------------------------------------------------------------------------------
//
QString QCaDataPoint::toString (const QCaDateTime& originDateTime) const
{
   QString result;
   QString zone;
   double relative;
   QString valid = "?";
   QEArchiveInterface::archiveAlarmSeverity severity;
   QString severityText = "?";
   QString statusText = "?";

   zone = QEUtilities::getTimeZoneTLA (this->datetime);
   valid = this->isDisplayable () ? "True " : "False";
   severity = (QEArchiveInterface::archiveAlarmSeverity) this->alarm.getSeverity ();
   severityText = QEArchiveInterface::alarmSeverityName (severity);
   statusText = this->alarm.statusName();

   // Calculate the relative time from start.
   //
   relative = originDateTime.secondsTo (this->datetime);

   // At some timme the MMM (from the format string) changed from "Jan" to "Jan."
   // So much for backward compatibility. Lose the "." if it exists.
   //
   QString dateTimeImage = datetime.toString (stdFormat);
   if (dateTimeImage [6] == '.') {
      dateTimeImage = dateTimeImage.remove (6, 1);
   }

   result = QString ("%1  %2  %3  %4  %5  %6  %7")
               .arg (dateTimeImage, 20)
               .arg (zone)
               .arg (relative, 16, 'f', 3)
               .arg (this->value, 16, 'e', 8)
               .arg (valid, 10)
               .arg (severityText, 10)
               .arg (statusText, 10);

   return result;
}


//==============================================================================
// QCaDataPointList methods
//==============================================================================
//
QCaDataPointList::QCaDataPointList ()
{
}

//------------------------------------------------------------------------------
//
QCaDataPointList::~QCaDataPointList () {}  // place holder

//------------------------------------------------------------------------------
//
void QCaDataPointList::reserve (const int size)
{
   this->data.reserve (size);
}

//------------------------------------------------------------------------------
//
void QCaDataPointList::clear ()
{
   this->data.clear ();
}

//------------------------------------------------------------------------------
//
void QCaDataPointList::removeLast ()
{
   int c = this->data.count ();
   if (c > 0) this->data.remove (c - 1);
}

//------------------------------------------------------------------------------
//
void QCaDataPointList::removeFirst ()
{
   int c = this->data.count ();
   if (c > 0) this->data.remove (0);
}

//------------------------------------------------------------------------------
// removes the first n available items from the list
//
void QCaDataPointList::removeFirstItems (const int n)
{
   int c = this->data.count ();
   int r = MIN (c, n);
   if (r > 0) this->data.remove (0, r);
}

//------------------------------------------------------------------------------
//
void QCaDataPointList::append (const QCaDataPoint& other)
{
   this->data.append (other);
}

//------------------------------------------------------------------------------
//
void  QCaDataPointList::append (const QCaDataPointList& other)
{
   for (int j = 0; j < other.count(); j++) {
      this->data.append (other.data.value (j));
   }
}

//------------------------------------------------------------------------------
//
void QCaDataPointList::replace (const int i, const QCaDataPoint& t)
{
   this->data.replace (i, t);
}

//------------------------------------------------------------------------------
//
int QCaDataPointList::count () const
{
   return this->data.count ();
}

//------------------------------------------------------------------------------
//
QCaDataPoint QCaDataPointList::value (const int j) const
{
   return this->data.value (j);
}

//------------------------------------------------------------------------------
//
QCaDataPoint QCaDataPointList::last () const
{
   return this->data.last ();
}

//------------------------------------------------------------------------------
//
void QCaDataPointList::truncate (const int position)
{
   while (this->data.count () > position) {
      this->removeLast ();
   }
}

//------------------------------------------------------------------------------
//
int QCaDataPointList::indexBeforeTime (const QCaDateTime& searchTime,
                                       const int defaultIndex) const
{
   // Cover "corner-case" specific no answer cases.
   //
   if (this->data.count () <= 0) return defaultIndex;
   if (this->data.value (0).datetime > searchTime) return defaultIndex;

   // Cover no need to search case.
   //
   int first = 0;
   int last = this->data.count () - 1;
   if (this->data.value (last).datetime <= searchTime) return last;

   // We know first point <= searchTime, last point > searchTime
   // While first and last are not adjacent...
   while (last - first  > 1) {
      // Perform binary search to find point of iterest.
      //
      int midway = (first + last) / 2;
      if (this->data.value (midway).datetime <= searchTime) {
         first = midway;
      } else {
         last = midway;
      }
      //
      // It is still the case that first point <= searchTime, last point > searchTime
   }

   return first;
}

//------------------------------------------------------------------------------
//
const QCaDataPoint* QCaDataPointList::findNearestPoint (const QCaDateTime& searchTime) const
{
   const int number = this->data.count ();
   const int first = 0;
   const int last = number - 1;

   // Cover "corner-case" cases.
   //
   if (number <= 0) return NULL;
   if (searchTime <= this->data [first].datetime) return &this->data [first];
   if (searchTime >= this->data [last].datetime)  return &this->data [last];

   // number >= 2
   const int before = this->indexBeforeTime (searchTime, 0);
   const int after = before + 1;

   double bsdt = this->data.value (before).datetime.secondsTo (searchTime);
   double sadt = searchTime.secondsTo (this->data.value (after).datetime);

   const QCaDataPoint*  result = (bsdt < sadt) ? &this->data [before] : &this->data [after];
   return result;
}

//------------------------------------------------------------------------------
//
void QCaDataPointList::resample (const QCaDataPointList& source,
                                 const double interval,
                                 const QCaDateTime& endTime)
{
   QCaDateTime firstTime;
   int j;
   int next;
   QCaDateTime jthTime;
   QCaDataPoint point;

   this->clear ();
   if (source.count () <= 0) return;

   firstTime = source.value (0).datetime;
   jthTime = firstTime;
   next = 0;
   for (j = 0; jthTime < endTime; j++) {

      // Calculate to nearest mSec.
      //
      jthTime = firstTime.addMSecs ((qint64)( (double) j * 1000.0 * interval));

      while (next < source.count () && source.value (next).datetime <= jthTime) next++;
      point = source.value (next - 1);
      point.datetime = jthTime;
      this->append (point);
   }
}

//------------------------------------------------------------------------------
//
void QCaDataPointList::compact (const QCaDataPointList& source)
{
   int j;
   QCaDataPoint lastPoint;

   this->clear ();
   if (source.count () <= 0) return;

   // Copy first point.
   lastPoint = source.value (0);
   this->append (lastPoint);

   for (j = 1; j < source.data.count (); j++) {
      QCaDataPoint point = source.data.value (j);
      if ((point.value != lastPoint.value) ||
          (point.alarm != lastPoint.alarm)) {
         this->append (point);
         lastPoint = point;
      }
   }
}

//------------------------------------------------------------------------------
//
void QCaDataPointList::toStream (QTextStream& target,
                                 bool withIndex,
                                 bool withRelativeTime) const
{
   int number = this->count ();
   int j;
   QCaDateTime originDateTime;

   if (number > 0) {
      originDateTime = this->value (0).datetime;

      for (j = 0; j < number; j++) {
         QCaDataPoint point = this->value (j);
         QString item;

         item = "";
         if (withIndex) {
            item.append (QString ("%1  ").arg (j + 1, 6));
         }

         if (withRelativeTime) {
            item.append (point.toString (originDateTime));
         } else {
            item.append (point.toString ());
         }

         target << item << "\n";
      }
   }
   else {
      target << "(QCaDataPointList empty)" << "\n";
   }
}

//------------------------------------------------------------------------------
// Essentially relocated for QEStripChart statistics.
//
bool QCaDataPointList::calculateStatistics (Statistics& statistics,
                                            const bool extendToTimeNow) const
{
   // Ensure not erroneous.
   //
   statistics.isDefined = false;
   statistics.mean = 0.0;
   statistics.stdDeviation = 0.0;
   statistics.slope = 0.0;
   statistics.integral = 0.0;
   statistics.minimum = 0.0;
   statistics.maximum = 0.0;
   statistics.initialValue = 0.0;
   statistics.finalValue = 0.0;

   const int n = this->data.count ();
   if (n < 1) return false;

   double sumWeight = 0.0;          // i.e. time between points.
   double sumValue = 0.0;           // weighted sum
   double sumValueSquared = 0.0;    // weighted sum**2

   // Least squares calc. variables.
   // X here is time - relative to first time.
   // It's kind of arbitary - the slope works out the same.
   //
   QCaDateTime startTime = this->data.value (0).datetime;
   double sumX = 0.0;
   double sumY = 0.0;
   double sumXX = 0.0;
   double sumXY = 0.0;
   int count = 0;

   bool isFirst = true;
   for (int j = 0; j < n; j++) {
      const QCaDataPoint thisPoint = this->data.value (j+0);

      // Skip undisplayable points, e.g.alarm invalid or disconnected.
      //
      if (!thisPoint.isDisplayable()) continue;
      const double value = thisPoint.value;

      // Is there a following point?
      //
      if ((j + 1 < n) || extendToTimeNow) {
         // Yes - we can calculate the weight.
         //
         double weight;
         if (j + 1 < n) {
            const QCaDataPoint nextPoint = this->data.value (j+1);
            weight = thisPoint.datetime.secondsTo (nextPoint.datetime);
         } else {
            // Must be extendToTimeNow set true.
            //
            weight = thisPoint.datetime.secondsTo (QDateTime::currentDateTime().toUTC());
         }

         sumWeight += weight;
         sumValue += weight * value;
         sumValueSquared += weight * value * value;
      }

      if (isFirst) {
         isFirst = false;
         statistics.minimum = value;
         statistics.maximum = value;
         statistics.initialValue = value;
      } else {
         statistics.minimum = MIN (statistics.minimum, value);
         statistics.maximum = MAX (statistics.maximum, value);
      }

      statistics.finalValue = value;

      // Least squares.
      // For x, use time from first point.
      //
      const double x = startTime.secondsTo (thisPoint.datetime);

      sumX += x;
      sumY += value;
      sumXX += x * x;
      sumXY += x * value;

      count ++;
   }

   if (sumWeight <= 0.0) return false;

   statistics.mean = sumValue / sumWeight;

   // Variance:  mean (x^2) - mean (x)^2
   //
   double variance = (sumValueSquared / sumWeight) - (statistics.mean * statistics.mean);

   // Rounding errors can lead to very small negative variance values (of the
   // order of -8.8e-16) which leads to NaN standard deviation values which then
   // causes a whole heap of issues: ensure the variance is non-negative.
   //
   variance = MAX (variance, 0.0);
   statistics.stdDeviation = sqrt (variance);

   // Least Squares
   //
   if (count >= 2) {
      double delta = (count * sumXX) - (sumX * sumX);
      delta = MAX (delta, 1.0e-9);   // avoid the divide by zero
      statistics.slope = ((count * sumXY) - (sumX * sumY)) / delta;
   }

   // Recall sumValue += (value * weight), and weight in seconds.
   //
   statistics.integral = sumValue;

   statistics.isDefined = true;
   return true;
}

//------------------------------------------------------------------------------
//
void QCaDataPointList::distribute (double distribution [], const int size,
                                   const bool extendToTimeNow,
                                   const double first, const double increment) const
{
   // Initialise the distribution.
   //
   for (int j = 0; j < size; j++) {
      distribution [j] = 0.0;
   }

   const int n = this->data.count ();
   for (int j = 0; j < n; j++) {
      const QCaDataPoint thisPoint = this->data.value (j+0);

      // Skip undisplayable points, e.g.alarm invalid or disconnected.
      //
      if (!thisPoint.isDisplayable()) continue;
      const double value = thisPoint.value;

      // Is there a following point?
      //
      if ((j + 1 < n) || extendToTimeNow) {
         // Yes - we can calculate the weight.
         //
         double weight;
         if (j + 1 < n) {
            const QCaDataPoint nextPoint = this->data.value (j+1);
            weight = thisPoint.datetime.secondsTo (nextPoint.datetime);
         } else {
            // Must be extendToTimeNow set true.
            //
            weight = thisPoint.datetime.secondsTo (QDateTime::currentDateTime().toUTC());
         }

         // Avoid divide by zero, and the hence the creation of a NaN slot value
         //
         const double realSlot = (value - first) / MAX (increment, 1.0e-20);

         // Check for out of range values.
         //
         if (realSlot < 0.0 || realSlot >= size) continue;

         const int slot = int (realSlot);

         // Belts 'n' braces
         //
         if (slot < 0 || slot >= size) continue;

         distribution [slot] += weight;
      }
   }
}

//------------------------------------------------------------------------------
// Register own meta types.
// static
bool QCaDataPoint::registerMetaTypes ()
{
   qRegisterMetaType<QCaDataPoint> ("QCaDataPoint");
   qRegisterMetaType<QCaDataPointList> ("QCaDataPointList");
   return true;
}

// Elaborate on start up.
//
static const bool _elaborate = QCaDataPoint::registerMetaTypes ();

// end
