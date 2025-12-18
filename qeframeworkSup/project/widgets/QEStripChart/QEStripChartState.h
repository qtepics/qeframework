/*  QEStripChartState.h
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

#ifndef QESTRIPCHARTSTATE_H
#define QESTRIPCHARTSTATE_H

#include <QDateTime>
#include <QList>

#include <QEFrameworkLibraryGlobal.h>
#include <persistanceManager.h>
#include <QEStripChartNames.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEStripChartState {
public:
   QEStripChartState ();
   void saveConfiguration (PMElement& parentElement);
   void restoreConfiguration (PMElement& parentElement);

   bool isNormalVideo;
   QEStripChartNames::ChartTimeModes chartTimeMode;
   QEStripChartNames::YScaleModes  yScaleMode;
   QEStripChartNames::ChartYRanges chartYScale;
   double yMinimum;
   double yMaximum;
   int duration;
   Qt::TimeSpec timeZoneSpec;
   QDateTime endDateTime;
};


// This class uses a QList in order to implement a stack.
//
// Note this class orginally extended QList<QEStripChartState>, but this way of
// specificying this class has issues with the Windows Visual Studio Compiler.
// It has now been modified to include a QList<QEStripChartState> member. The
// downside of this is that we must now provide list member access functions.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEStripChartStateList {
public:
   QEStripChartStateList ();

   void clear ();
   void push (const QEStripChartState& state);
   bool prev (QEStripChartState& state);
   bool next (QEStripChartState& state);

   bool prevAvailable ();
   bool nextAvailable ();

private:
   int chartStatePointer;
   QList<QEStripChartState> stateList;
};

#endif  // QESTRIPCHARTSTATE_H
