/*  QCaDataPoint.cpp
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
 *  Copyright (c) 2012,2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>
#include <QEArchiveInterface.h>
#include <QCaDataPoint.h>
#include <QECommon.h>

#define DEBUG  qDebug () << "QCaDataPoint" << __LINE__ <<  __FUNCTION__  << "  "


static const QString stdFormat = "dd/MMM/yyyy HH:mm:ss";

//==============================================================================
// QCaDataPoint methods
//==============================================================================
//
QCaDataPoint::QCaDataPoint ()
{
   this->value = 0.0;

   // Register type.
   //
   qRegisterMetaType<QCaDataPoint> ("QCaDataPoint");
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
         result = true;
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

   result = QString ("%1  %2  %3  %4  %5  %6")
               .arg (this->datetime.toString (stdFormat), 20)
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

   result = QString ("%1  %2  %3  %4  %5  %6  %7")
               .arg (this->datetime.toString (stdFormat), 20)
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
   // Register type.
   //
   qRegisterMetaType<QCaDataPointList> ("QCaDataPointList");
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

// end
