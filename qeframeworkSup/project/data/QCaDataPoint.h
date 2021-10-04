/* QCaDataPoint.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2021 Australian Synchrotron
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

#ifndef QE_DATA_POINT_H
#define QE_DATA_POINT_H

#include <QVector>
#include <QMetaType>
#include <QString>
#include <QTextStream>

#include <QCaAlarmInfo.h>
#include <QCaDateTime.h>
#include <QEFrameworkLibraryGlobal.h>

/// This class used to hold a single data point. Objects of this type are
/// intended for use QCaStripChart in particular, but also for the interface
/// to the Channel Access and Archive Appliance archives.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QCaDataPoint {
public:
   explicit QCaDataPoint ();
   bool isDisplayable () const;     // i.e. is okay, not invalid and not disconnected.

   // Generate image of point.
   //
   QString toString () const;                                   // basic
   QString toString (const QCaDateTime& originDateTime) const;  // ... plus a relative time

   // We don't bother with a variant but just use a double.  A double can be
   // used to hold all CA data types except strings (which is are not plotable).
   //
   double value;
   QCaDateTime datetime;      // datetime + nSec
   QCaAlarmInfo alarm;
};


/// Defines a list of data points.
///
/// Note this class orginally extended QList<QCaDataPoint>, but this way of
/// specifying this class has issues with the Windows Visual Studio Compiler.
/// It has now been modified to include a QList<QCaDataPoint> member. The
/// downside of this is that we must now provide list member access functions.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QCaDataPointList  {
public:
   explicit QCaDataPointList ();
   ~QCaDataPointList ();

   // Provide access to the inner vector functions.
   //
   void reserve (const int size);
   void clear ();
   void removeLast ();
   void removeFirst ();
   void append (const QCaDataPointList& other);
   void append (const QCaDataPoint& other);
   void replace (const int i, const QCaDataPoint& t);
   int count () const;

   // Removes the first n items from the list.
   //
   void removeFirstItems (const int n);

   QCaDataPoint value (const int j) const;
   QCaDataPoint last () const;

   // Truncates the list at the given position index.
   // If the specified position index is beyond the end of the list, nothing happens.
   //
   void truncate (const int position);

   // Returns index of the last point with a time <= searchTime;
   // or returns default index value if no point satifies the criteria
   // Uses a binary search to find point of iterest.
   // Note: assumes that the data point list is in increasing time order.
   //
   int indexBeforeTime (const QCaDateTime& searchTime,
                        const int defaultIndex) const;

   // Return a reference to the point nearest to the specified time or NULL.
   // WARNING - do not store this reference. To be consider valid during the
   // processing of a single event only.
   //
   const QCaDataPoint* findNearestPoint (const QCaDateTime& searchTime) const;

   // Resamples the source list of points into the current list.
   // Items are resampled into data points at fixed time intervals.
   // No interpolation - the "current" value is carried forward to the next sample point(s).
   // Note: any previous data is lost.
   //
   void resample (const QCaDataPointList& source,
                  const double interval,
                  const QCaDateTime& endTime);

   // Removes duplicate sample points.
   // Note: any previous data is lost.
   //
   void compact (const QCaDataPointList& source);

   // Write whole list to target stream.
   //
   void toStream (QTextStream& target, bool withIndex, bool withRelativeTime) const;

   // Used by QEStripChart statistics and QEDistribution.
   //
   struct Statistics {
      bool isDefined;
      double mean;
      double stdDeviation;
      double slope;
      double integral;
      double minimum;
      double maximum;
      double initialValue;
      double finalValue;
   };

   // Calculates the statistics data related to the current data set.
   // If extendToTimeNow set true, a virtual data point with last known value
   // and the current time is effectively added to the data set.
   // Ratiobnale: many PVs only sends updated on change, so not doing this can
   // skew the contribution to the stats of the last point.
   //
   bool calculateStatistics (Statistics& statistics,
                             const bool extendToTimeNow) const;

   // Distributes the values into the distribution array.
   // The distribution array MUST have at least size elements.
   // Values less than first or greater than (first + size*increment) are ignored.
   // If extendToTimeNow set true, a virtual data point with last known value
   // and the current time is effectively added to the data set.
   //
   void distribute (double distribution [], const int size,
                    const bool extendToTimeNow,
                    const double first, const double increment) const;

private:
   QVector<QCaDataPoint> data;
};

// These types are used in inter thread signals - must be registered.
//
Q_DECLARE_METATYPE (QCaDataPoint)
Q_DECLARE_METATYPE (QCaDataPointList)

#endif  // QE_DATA_POINT_H
