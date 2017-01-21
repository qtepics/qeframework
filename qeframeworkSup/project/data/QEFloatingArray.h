/*  QEFloatingArray.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
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
 *  Copyright (c) 2013 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QEFLOATING_ARRAY_H
#define QEFLOATING_ARRAY_H

#include <QVector>
#include <QEPluginLibrary_global.h>

/// This class provides short hand for QVector<double> together with some
/// basic double vector operations.
///
class QEPLUGINLIBRARYSHARED_EXPORT QEFloatingArray : public QVector<double> {
public:
   // Various constructors that replicate those provided by QVector
   //
   explicit QEFloatingArray ();
   explicit QEFloatingArray (int size);
   explicit QEFloatingArray (int size, const double& t);
   explicit QEFloatingArray (const QVector<double>& other);

   // Find min/max values of the array. If array has zero elements then
   // the returned value is the defaultValue .
   //
   double minimumValue (const double& defaultValue = 0.0);
   double maximumValue (const double& defaultValue = 0.0);

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
