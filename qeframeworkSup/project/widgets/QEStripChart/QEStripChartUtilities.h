/*  QEStripChartUtilities.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_STRIP_CHART_UTILITIES_H
#define QE_STRIP_CHART_UTILITIES_H

#include <QDateTime>
#include <persistanceManager.h>
#include <QEFrameworkLibraryGlobal.h>

#include <QEDisplayRanges.h>

//==============================================================================
// Utility classes
//==============================================================================
// Allows PV points to be scaled as:  y' = (y - d)*m + c
// This is useful whem comparing values with disparate ranages.
// It is a simple linear scaling. While d and c are not independent, from a
// user point of view is it often easier to specify a 'd' and/or a 'c' value.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEValueScaling {
public:
   QEValueScaling ();
   ~QEValueScaling ();

   void reset ();
   void assign (const QEValueScaling& s);
   void set (const double d, const double m, const double c, const double t);
   void get (double &d, double &m, double &c, double &t) const;

   double getTimeOffset() const;

   // Find and set d, m and c such that the from values map to the to values,
   // e.g a PVs HOPR/LOPR values map to current chart range values.
   //
   void map (const double fromLower, const double fromUpper,
             const double toLower,   const double toUpper);

   bool isScaled () const;

   inline double value (const double x) const {
      return (x - d) * m + c;
   }

   // Re-scale display range.
   //
   QEDisplayRanges value (const QEDisplayRanges& displayRange) const;

   void saveConfiguration (PMElement& parentElement) const;
   void restoreConfiguration (PMElement& parentElement);

private:
   double d;   // origin
   double m;   // slope
   double c;   // offset
   double t;   // time offset
};

#endif  // QE_STRIP_CHART_UTILITIES_H
