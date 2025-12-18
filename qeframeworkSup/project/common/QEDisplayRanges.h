/*  QEDisplayRanges.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_DISPLAY_RANGES_H
#define QE_DISPLAY_RANGES_H

#include <QEFrameworkLibraryGlobal.h>

// Refactor from QEStripChartUtilities TrackRanages with some
// functionalty from QEGraphic.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEDisplayRanges {
public:
   explicit QEDisplayRanges ();
   explicit QEDisplayRanges (const double min, const double max);

   // Equality operators.
   //
   bool operator == (const QEDisplayRanges& other) const;
   bool operator != (const QEDisplayRanges& other) const;

   // Provides a fuzzy equality function with a specified tolerance.
   // When both ranges are defined, the larger the tolerance value, the more
   // disimilar the ranges can be and still be deemed similar (equal-ish)
   // When tolerance set to 0.0, this function is effectively same as ==.
   //
   // Note: If both undefined then always returns true (equal).
   // If one defined and one undefined then always returns false (not equal).
   //
   bool isSimilar (const QEDisplayRanges& other, const double tolerance) const;

   void clear ();

   void setRange (const double min, const double max);
   void merge (const double d);               // defines/extends range to include d.
   void merge (const QEDisplayRanges& other); // defines/extends range to include other.

   bool getIsDefined () const;
   double getMinimum () const;
   double getMaximum () const;

   // returns true if range is defined together with min and max.
   //
   bool getMinMax (double & min, double& max) const;

   // Useful for setting up axies.
   //
   void adjustLogMinMax (double& minOut, double& maxOut, double& majorOut) const;
   void adjustMinMax    (const int number, const bool roundToMajor,
                         double& minOut, double& maxOut, double& majorOut) const;

private:
   double minimum;
   double maximum;
   bool isDefined;
};

#endif  // QE_DISPLAY_RANGES_H
