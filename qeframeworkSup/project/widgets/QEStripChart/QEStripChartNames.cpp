/*  QEStripChartNames.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2016-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include <QEStripChartNames.h>


//------------------------------------------------------------------------------
// static
QString QEStripChartNames::chartTimeModeStatus (const ChartTimeModes mode)
{
   QString result;

   switch (mode) {
      case tmRealTime:
         result = "Real Time";
         break;
      case tmPaused:
         result = "Paused";
         break;
      case tmHistorical:
         result = "Historical";
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
         result = "manual scale";
         break;
      case operatingRange:
         result = "operating range scale";
         break;
      case plotted:
         result = "plotted scale";
         break;
      case buffered:
         result = "buffered scale";
         break;
      case dynamic:
         result = "dynamic scale";
         break;
      case normalised:
         result = "normalised scale";
         break;
   }
   return result;
}

// end
