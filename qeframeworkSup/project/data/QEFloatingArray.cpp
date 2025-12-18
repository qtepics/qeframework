/*  QEFloatingArray.cpp
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

#include "QEFloatingArray.h"
#include <algorithm>
#include <QDebug>
#include <QtAlgorithms>
#include <QECommon.h>
#include <QEPlatform.h>

//=================================================================================
// QEFloatingArray
//=================================================================================
//
QEFloatingArray::QEFloatingArray () : QVector<double> () { }

QEFloatingArray::QEFloatingArray (int size) : QVector<double> (size) { }

QEFloatingArray::QEFloatingArray (int size, const double& t) : QVector<double> (size, t) { }

QEFloatingArray::QEFloatingArray (const QVector<double>& other) : QVector<double> (other) { }

//---------------------------------------------------------------------------------
// Copy a vertor from another and return value
//
QEFloatingArray& QEFloatingArray::operator=( const  QVector<double>& other )
{
    // Do parent class stuff assignment.
    *(QVector<double>*) this = QVector<double> (other);

    // return value as well.
    return *this;
}

//---------------------------------------------------------------------------------
//
double QEFloatingArray::minimumValue (const double& defaultValue, const bool includeInf)
{
   const int n = this->count ();

   double result = defaultValue;
   bool isFirst = true;
   for (int j = 0; j < n; j++) {
      const double v = this->value (j);

      // Ignore nan values, and only include inf if requested
      //
      if (QEPlatform::isNaN (v)) continue;
      if (!includeInf && QEPlatform::isInf (v)) continue;

      if (isFirst) {
         result = v;
         isFirst = false;
      } else {
         result = MIN (result, v);   // not first, so must merge
      }
   }
   return result;
}

//---------------------------------------------------------------------------------
//
double QEFloatingArray::maximumValue (const double& defaultValue, const bool includeInf)
{
   const int n = this->count ();

   double result = defaultValue;
   bool isFirst = true;
   for (int j = 0; j < n; j++) {
      const double v = this->value (j);

      // Ignore nan values, and only include inf if requested
      //
      if (QEPlatform::isNaN (v)) continue;
      if (!includeInf && QEPlatform::isInf (v)) continue;

      if (isFirst) {
         result = v;
         isFirst = false;
      } else {
         result = MAX (result, v);   // not first, so must merge
      }
   }
   return result;
}

//---------------------------------------------------------------------------------
//
QEFloatingArray QEFloatingArray::calcDyByDx (const QVector<double>& x)
{
   const int size = MIN (this->size(), x.size());
   QEFloatingArray result;
   double s;
   int j;

   result.clear ();

   if (size == 1) {
      result.append (0.0);

   } else if (size == 2) {

      s = derivative (x.value (0), this->value (0),
                      x.value (1), this->value (1));

      result.append (s);
      result.append (s);

   } else if (size >= 3) {

      // First point.
      //
      s = derivative (x.value (0), this->value (0),
                      x.value (1), this->value (1));
      result.append (s);

      // Middle points.
      //
      for (j = 1 ; j < size - 1; j++) {
         s = derivative (x.value(j - 1), this->value(j - 1),
                         x.value(j    ), this->value(j    ),
                         x.value(j + 1), this->value(j + 1));
         result.append (s);
      }

      // Last point.
      //
      s = derivative (x.value (size - 2), this->value (size - 2),
                      x.value (size - 1), this->value (size - 1));
      result.append (s);
   }

   return result;
}

//---------------------------------------------------------------------------------
//
QEFloatingArray QEFloatingArray::medianFilter (const int window)
{
   const int size = this->size ();
   QEFloatingArray result;
   result.reserve (size);

   if (window > 1) {
      const int offset = window / 2;
      for (int j = 0; j < size; j++) {
         // Must cater for edge effects
         //
         int pos = MAX (j - offset, 0);
         int end = MIN (j + offset, size - 1);

         // Extract vector slice, sort and extract middle, i.e. median, value.
         //
         QVector<double> qtemp = this->mid (pos, end - pos + 1);
         std::vector <double> temp;

         // Create a std vector form the QVector. Is there a better way?
         //
         for (int j = 0; j < qtemp.size(); j++) {
            temp.push_back(qtemp[j]);
         }

         std::sort (temp.begin(), temp.end());
         result.append (temp [qtemp.size ()/2]);
      }
   } else {
      // Window size is 1 (identity) or invalid - just return this vector.
      //
      result = *this;
   }
   return result;
}


#define MIN_DELTA_X  (1.0E-20)

//---------------------------------------------------------------------------------
// static
double QEFloatingArray::derivative (const double xp1, const double yp1,
                                    const double xp2, const double yp2)
{
   double result;
   double dx, dy;

   dx = xp2 - xp1;
   dy = yp2 - yp1;

   // Must avoid the divide by zero.
   //
   result = (ABS(dx) >= MIN_DELTA_X) ?  (dy / dx ) : 0.0;

   return result;
}


//---------------------------------------------------------------------------------
// static
double QEFloatingArray::derivative (const double xp1, const double yp1,
                                    const double xp2, const double yp2,
                                    const double xp3, const double yp3)
{
   double result;
   double x1, x3;
   double y1, y3;
   double divisor;

   // Form quadratic:    y  = a.x^2 + b.x + c
   // then:              y' = 2.a.x + b
   // and specifically:  y'(X2) = 2.a.(X2) + b
   //
   // First perform linear co-ordinate mapping (xpi, ypi) to (xi, yi)
   // such that:  xi = xpi - xp2,  and  yi = ypi - yp2.
   //
   x1 = xp1 - xp2;
   y1 = yp1 - yp2;
   x3 = xp3 - xp2;
   y3 = yp3 - yp2;

   // x2 = y2 = 0 by defn, which implies c = 0.
   // Therefore need to solve for a, b using:
   //
   //   y1 = a.x1.x1 + b.x1
   //   y3 = a.x3.x3 + b.x3
   //
   // Note y'(x2) = y'(0) = b,  which implies we only need to find b.
   //
   // Avoid the divide by zero.
   //
   divisor = x1*x3*(x3 - x1);
   if (ABS (divisor) >= MIN_DELTA_X) {
      // Now calculate b
      //
      result = (y1*x3*x3 - y3*x1*x1) / divisor;
   } else {
      result = 0.0;
   }

   return result;
}

// end
