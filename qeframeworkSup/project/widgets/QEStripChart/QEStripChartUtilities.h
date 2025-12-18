/*  QEStripChartUtilities.h
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
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT ValueScaling {
public:
   ValueScaling ();

   void reset ();
   void assign (const ValueScaling & s);
   void set (const double dIn, const double mIn, const double cIn);
   void get (double &dOut, double &mOut, double &cOut) const;

   // Find annd set d, m and c such that the from values map to the to values,
   // e.g a PVs HOPR/LOPR values map to current chart range values.
   //
   void map (const double fromLower, const double fromUpper,
             const double toLower,   const double toUpper);

   bool isScaled () const;

   inline double value (const double x) const {
      return (x - d) * m + c;
   }

   QEDisplayRanges value (const QEDisplayRanges& x) const;

   void saveConfiguration (PMElement & parentElement) const;
   void restoreConfiguration (PMElement & parentElement);

private:
   double d;   // origin
   double m;   // slope
   double c;   // offset
};

#endif  // QE_STRIP_CHART_UTILITIES_H
