/*  QEStripChartNames.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2021 Australian Synchrotron
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
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_STRIPCHART_NAMES_H
#define QE_STRIPCHART_NAMES_H

#include <QObject>
#include <QString>
#include <contextMenu.h>
#include <QEAbstractDynamicWidget.h>
#include <QEFrameworkLibraryGlobal.h>

// Define strip chart specific names (enumerations).
// We use a QObject as opposed to a namespace as this allows us leverage off the
// meta object compiler output, specifically allows us to use the enumToString
// and stringToEnum functions in QEUtilities.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEStripChartNames : public QObject {
   Q_OBJECT
public:

   // Chart time mode options.
   //
   enum ChartTimeModes {
      tmRealTime,      //
      tmPaused,        //
      tmHistorical     //
   };

   Q_ENUMS (ChartTimeModes)

   // A more user friendly version than enumToString.
   static QString chartTimeModeStatus (const ChartTimeModes mode);

   enum ChartYRanges {
      manual,          // chart set manually via dialog
      operatingRange,  // i.e. HOPR/LOPR values
      plotted,         // what is currently displayed
      buffered,        // what is buffered - include waht is displayed and what of "off screen".
      dynamic,         // chart range adhjuested dynamically (based on plotted values)
      normalised       // chart range set to 0 .. 100, PVs scaled
   };

   Q_ENUMS (ChartYRanges)

   // A more user friendly version than enumToString.
   static QString chartYRangeStatus (const ChartYRanges yRange);

   enum PlayModes {
      play,            // run chart real time
      pause,           // pause chart updates
      forward,         // move start/end time forards
      backward,        // move start/end time backwards
      selectTimes      // set start/end time via dialog
   };

   Q_ENUMS (PlayModes)

   enum StateModes {
      previous,         // previous state
      next              // next state
   };

   Q_ENUMS (StateModes)

   enum VideoModes {
      normal,          // white background
      reverse          // black backgound
   };

   Q_ENUMS (VideoModes)

   enum YScaleModes {
      linear,          // lineary Y scale
      log              // log Y scale - limited to -20
   };

   Q_ENUMS (YScaleModes)


   enum LineDrawModes {
      ldmHide,         // do not draw
      ldmRegular,      // normal draw
      ldmBold          // bold draw
   };

   Q_ENUMS (LineDrawModes)

   enum LinePlotModes {
      lpmRectangular,  // plot current value until next value with step
      lpmSmooth        // plot linear interpolation from current to next value.
   };

   Q_ENUMS (LinePlotModes)

   // IDs for all menu options
   // Each menu option has a unique ID across all menus
   // These IDs are in addition to standard context menu IDs and so start after
   // contextMenu::CM_SPECIFIC_WIDGETS_START_HERE
   //
   enum ContextMenuOptions {
      SCCM_NONE = QEAbstractDynamicWidget::ADWCM_SUB_CLASS_WIDGETS_START_HERE,

      // General context menu items.
      //
      SCCM_SHOW_HIDE_TOOLBAR,     //
      SCCM_SHOW_HIDE_PV_ITEMS,    //

      // Item specific menu options - when item is defined
      //
      SCCM_READ_ARCHIVE,
      SCCM_RECALCULATE,
      //
      SCCM_SCALE_CHART_AUTO,
      SCCM_SCALE_CHART_PLOTTED,
      SCCM_SCALE_CHART_BUFFERED,
      //
      SCCM_SCALE_PV_RESET,
      SCCM_SCALE_PV_GENERAL,
      SCCM_SCALE_PV_AUTO,
      SCCM_SCALE_PV_PLOTTED,
      SCCM_SCALE_PV_PLOTTED_UPPER,
      SCCM_SCALE_PV_PLOTTED_CENTRE,
      SCCM_SCALE_PV_PLOTTED_LOWER,
      SCCM_SCALE_PV_BUFFERED,
      SCCM_SCALE_PV_FIRST_CENTRE,
      //
      SCCM_PLOT_RECTANGULAR,
      SCCM_PLOT_SMOOTH,
      SCCM_PLOT_SERVER_TIME,
      SCCM_PLOT_CLIENT_TIME,
      //
      SCCM_ARCH_LINEAR,
      SCCM_ARCH_PLOTBIN,
      SCCM_ARCH_RAW,
      SCCM_ARCH_SHEET,
      SCCM_ARCH_AVERAGED,
      //
      SCCM_LINE_HIDE,
      SCCM_LINE_REGULAR,
      SCCM_LINE_BOLD,
      SCCM_LINE_COLOUR,
      //
      SCCM_PV_EDIT_NAME,
      SCCM_ADD_TO_PREDEFINED,
      SCCM_PV_WRITE_TRACE,
      SCCM_PV_STATS,
      SCCM_PV_CLEAR,

      // Item specific menu options - when item is not defined
      //
      SCCM_PV_ADD_NAME,
      SCCM_PV_PASTE_NAME,
      //
      SCCM_PREDEFINED_01,
      SCCM_PREDEFINED_02,
      SCCM_PREDEFINED_03,
      SCCM_PREDEFINED_04,
      SCCM_PREDEFINED_05,
      SCCM_PREDEFINED_06,
      SCCM_PREDEFINED_07,
      SCCM_PREDEFINED_08,
      SCCM_PREDEFINED_09,
      SCCM_PREDEFINED_10
   };

   // These MUST be consistant with above declaration.
   //
   static const ContextMenuOptions ContextMenuItemFirst = SCCM_SHOW_HIDE_TOOLBAR;
   static const ContextMenuOptions ContextMenuItemLast  = SCCM_PREDEFINED_10;

   static const int NumberMenuItems = (ContextMenuItemLast - ContextMenuItemFirst + 1);
   static const int NumberPrefefinedItems = (SCCM_PREDEFINED_10 - SCCM_PREDEFINED_01 + 1);
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEStripChartNames::ChartTimeModes)
Q_DECLARE_METATYPE (QEStripChartNames::ChartYRanges)
Q_DECLARE_METATYPE (QEStripChartNames::PlayModes)
Q_DECLARE_METATYPE (QEStripChartNames::StateModes)
Q_DECLARE_METATYPE (QEStripChartNames::VideoModes)
Q_DECLARE_METATYPE (QEStripChartNames::YScaleModes)
Q_DECLARE_METATYPE (QEStripChartNames::LineDrawModes)
Q_DECLARE_METATYPE (QEStripChartNames::LinePlotModes)
#endif

#endif   // QE_STRIPCHART_NAMES_H
