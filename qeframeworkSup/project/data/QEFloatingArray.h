/*  QEFloatingArray.h
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

#ifndef QEFLOATING_ARRAY_H
#define QEFLOATING_ARRAY_H

#include <QVector>
#include <QEFrameworkLibraryGlobal.h>

/// This class provides short hand for QVector<double> together with some
/// basic double vector operations.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEFloatingArray : public QVector<double> {
public:
   // Various constructors that replicate those provided by QVector
   //
   explicit QEFloatingArray ();
   explicit QEFloatingArray (int size);
   explicit QEFloatingArray (int size, const double& t);
   explicit QEFloatingArray (const QVector<double>& other);

   // Allow assignment
   //
   QEFloatingArray& operator=(const QVector<double>& other);

   // Find min/max values of the array. If array has zero usable elements then
   // the returned value is the defaultValue.
   // NaN values are always ignored. By default +/-inf values are also ignored.
   //
   double minimumValue (const double& defaultValue = 0.0, const bool includeInf = false);
   double maximumValue (const double& defaultValue = 0.0, const bool includeInf = false);

   // Calculates dThis/dx for each point using a series of three-point
   // polynomials. First an last point based to a two-point polynomial.
   //
   QEFloatingArray calcDyByDx (const QVector<double>& x);

   // Calc median filter. window is median window size.
   // Should be > 0 and odd. 1 is essentuially no filter.
   //
   QEFloatingArray medianFilter (const int window);

private:
   static double derivative (const double xp1, const double yp1,
                             const double xp2, const double yp2);

   // This function calculates the derirative of the quadratic line through
   // the points (xp1, yp1), (xp2,yYp2) and (xp3, yp3) at the point xp2.
   //
   static double derivative (const double xp1, const double yp1,
                             const double xp2, const double yp2,
                             const double xp3, const double yp3);

};

#endif   // QEFLOATING_ARRAY_H
