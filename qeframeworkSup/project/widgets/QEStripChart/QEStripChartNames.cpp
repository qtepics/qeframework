/*  QEStripChartNames.cpp
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
 *  Copyright (c) 2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QEStripChartNames.h>


//------------------------------------------------------------------------------
// static
QString QEStripChartNames::chartTimeModeStatus (const ChartTimeModes mode)
{
   QString result;

   switch (mode) {
      case tmRealTime:
         result = tr("Real Time");
         break;
      case tmPaused:
         result = tr("Paused");
         break;
      case tmHistorical:
         result = tr("Historical");
         break;
   }
   return result;
}

//------------------------------------------------------------------------------
// static
QString QEStripChartNames::chartYRangeStatus (const ChartYRanges yRange)
{
   QString result;

   switch (yRange) {
      case manual:
         result = tr("manual scale");
         break;
      case operatingRange:
         result = tr("operating range scale");
         break;
      case plotted:
         result = tr("plotted scale");
         break;
      case buffered:
         result = tr("buffered scale");
         break;
      case dynamic:
         result = tr("dynamic scale");
         break;
      case normalised:
         result = tr("normalised scale");
         break;
   }
   return result;
}

// end
