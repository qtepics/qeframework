/*  QEStripChartUtilities.cpp
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

#include <QDebug>
#include <QECommon.h>

#include "QEStripChartUtilities.h"

#define DEBUG qDebug () << "QEStripChartUtilities" << __LINE__ <<  __FUNCTION__  << "  "

//==============================================================================
//
QEValueScaling::QEValueScaling ()
{
   this->d = 0.0;
   this->m = 1.0;
   this->c = 0.0;
   this->t = 0.0;
}

//------------------------------------------------------------------------------
//
QEValueScaling::~QEValueScaling () {}

//------------------------------------------------------------------------------
//
void QEValueScaling::reset ()
{
   this->d = 0.0;
   this->m = 1.0;
   this->c = 0.0;
   this->t = 0.0;
}

//------------------------------------------------------------------------------
//
void QEValueScaling::assign (const QEValueScaling& source)
{
   this->d = source.d;
   this->m = source.m;
   this->c = source.c;
   this->t = source.t;
}

//------------------------------------------------------------------------------
//
void QEValueScaling::set (const double dIn, const double mIn,
                          const double cIn, const double tIn)
{
   this->d = dIn;
   this->m = mIn;
   this->c = cIn;
   this->t = tIn;
}

//------------------------------------------------------------------------------
//
void QEValueScaling::get (double &dOut, double &mOut,
                          double &cOut, double &tOut) const
{
   dOut = this->d;
   mOut = this->m;
   cOut = this->c;
   tOut = this->t;
}

//------------------------------------------------------------------------------
//
double QEValueScaling::getTimeOffset() const
{
   return this->t;
}

//------------------------------------------------------------------------------
//
void QEValueScaling::map (const double fromLower, const double fromUpper,
                        const double toLower,   const double toUpper)
{
   double delta;

   // Scaling is: y = (x - d)*m + c
   // We have three unknowns and two constraints, so have an extra
   // degree of freedom.
   //
   // Set origin to mid-point from value.
   //
   this->d = 0.5 * (fromLower + fromUpper);

   // Set Offset to mid-display value.
   //
   this->c = 0.5 * (toLower + toUpper);

   // Avoid the divide by zero.
   //
   delta = fromUpper - fromLower;
   if (delta >= 0.0) {
      delta = MAX (delta, +1.0E-12);
   } else {
      delta = MAX (delta, -1.0E-12);
   }

   // Set slope as ratio of to (display) span to form span.
   //
   this->m = (toUpper - toLower) / delta;
}

//------------------------------------------------------------------------------
//
bool QEValueScaling::isScaled () const
{
   return ((this->d != 0.0) || (this->m != 1.0) ||
           (this->c != 0.0) || (this->t != 0.0));
}

//------------------------------------------------------------------------------
//
QEDisplayRanges QEValueScaling::value (const QEDisplayRanges& displayRange) const
{
   QEDisplayRanges result;           // undefined
   bool okay;
   double min, max;

   okay = displayRange.getMinMax (min, max);
   if (okay) {
      // The range is defined - the extracted min and max are good, so scale
      // each limit and store into result.
      //
      double scaledMin = this->value (min);
      double scaledMax = this->value (max);
      if (scaledMax < scaledMin) {
         // m, the slope, is negative, we need to swap.
         //
         double temp = scaledMin;
         scaledMin = scaledMax;
         scaledMax = temp;
      }
      result.setRange (scaledMin, scaledMax);
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEValueScaling::saveConfiguration (PMElement& parentElement) const
{
   if (this->isScaled()) {
      PMElement scalingElement = parentElement.addElement ("Scaling");

      scalingElement.addAttribute ("Origin", this->d);
      scalingElement.addAttribute ("Slope",  this->m);
      scalingElement.addAttribute ("Offset", this->c);
      scalingElement.addAttribute ("TimeOffset", this->t);
   }
}

//------------------------------------------------------------------------------
//
void QEValueScaling::restoreConfiguration (PMElement& parentElement)
{
   double ad, am, ac, at;
   bool status;

   PMElement scalingElement = parentElement.getElement ("Scaling");

   status = scalingElement.getAttribute ("Origin", ad) &&
            scalingElement.getAttribute ("Slope",  am) &&
            scalingElement.getAttribute ("Offset", ac);

   if (!scalingElement.getAttribute ("TimeOffset", at)) {
      at = 0.0;
   }

   if (status) {
      this->set (ad, am, ac, at);
   }
}

// end
