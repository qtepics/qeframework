/* QEStripChartState.h
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
 *  Copyright (c) 2013 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
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
