/*  QEPlotterNames.h
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
 *  Copyright (c) 2013,2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

// Provides common definitions of both conext menu and tool bar actions.
//
#ifndef QE_PLOTTER_NAMES_H
#define QE_PLOTTER_NAMES_H

#include <QObject>
#include <contextMenu.h>

// Define plotter specific names (enumerations)
// We use a QObject as opposed to a namespace as this allows us leverage off the
// meta object compiler output, specifically allows us to use the enumToString
// and stringToEnum functions in QEUtilities.
//
class QEPlotterNames : public QObject {
   Q_OBJECT
public:
    QEPlotterNames (QObject* parent = 0) : QObject (parent) {}

   // IDs for all menu options
   // Each menu option has a unique ID across all plotter menus and toolbars
   // These IDs are in addition to standard context menu IDs and so start after
   // contextMenu::CM_SPECIFIC_WIDGETS_START_HERE
   //
   enum MenuActions {
      PLOTTER_FIRST = contextMenu::CM_SPECIFIC_WIDGETS_START_HERE,  // Must be first

      // General context menu items, many also available in the tool bar.
      //
      PLOTTER_SHOW_HIDE_CROSSHAIRS, //
      PLOTTER_SHOW_HIDE_TOOLBAR,   //
      PLOTTER_SHOW_HIDE_PV_ITEMS,  //
      PLOTTER_SHOW_HIDE_STATUS,    //
      PLOTTER_EMIT_COORDINATES,    //

      PLOTTER_PREV,                // Previous state
      PLOTTER_NEXT,                // Previous state

      PLOTTER_NORMAL_VIDEO,        //
      PLOTTER_REVERSE_VIDEO,       //

      PLOTTER_LINEAR_Y_SCALE,      //
      PLOTTER_LOG_Y_SCALE,         //

      PLOTTER_MANUAL_Y_RANGE,      // User selected YMin YMax
      PLOTTER_CURRENT_Y_RANGE,     // YMin/YMax based on overal min/max of current data set
      PLOTTER_DYNAMIC_Y_RANGE,     // As PLOTTER_CURRENT_Y_RANGE, but dynamic per update
      PLOTTER_NORAMLISED_Y_RANGE,  // Range 0 to 1: Data mapped Min => 0, Max => 1
      PLOTTER_FRACTIONAL_Y_RANGE,  // Range 0 to 1: Data mapped (value / Max)

      PLOTTER_LINEAR_X_SCALE,      //
      PLOTTER_LOG_X_SCALE,         //

      PLOTTER_MANUAL_X_RANGE,      // User selected XMin XMax
      PLOTTER_CURRENT_X_RANGE,     // XMin/XMax based on overal min/max of current data set
      PLOTTER_DYNAMIC_X_RANGE,     // As PLOTTER_CURRENT_X_RANGE, but dynamic per update

      PLOTTER_PLAY,                //
      PLOTTER_PAUSE,               //

      PLOTTER_LOAD_CONFIG,         // Load plotter widget configuration file.
      PLOTTER_SAVE_CONFIG,         // Save plotter widget configuration file.

      PLOTTER_COPY_VARIABLE,       // Rebadge of CM_COPY_VARIABLE
      PLOTTER_COPY_DATA,           // Rebadge of CM_COPY_DATA
      PLOTTER_PASTE,               // Rebadge of CM_PASTE
      PLOTTER_DRAG_VARIABLE,       // Rebadge of CM_DRAG_VARIABLE
      PLOTTER_DRAG_DATA,           // Rebadge of CM_DRAG_DATA

      // PV item context menu items.
      //
      PLOTTER_LINE_BOLD,           //
      PLOTTER_LINE_DOTS,           //
      PLOTTER_LINE_VISIBLE,        //
      PLOTTER_LINE_NO_MEDIAN_FILTER,  //
      PLOTTER_LINE_MEDIAN_3_FILTER,   //
      PLOTTER_LINE_MEDIAN_5_FILTER,   //
      PLOTTER_LINE_COLOUR,         //

      PLOTTER_DATA_SELECT,         //
      PLOTTER_DATA_DIALOG,         //
      PLOTTER_DATA_CLEAR,          //

      PLOTTER_SCALE_TO_MIN_MAX,    //
      PLOTTER_SCALE_TO_ZERO_MAX,   //

      // Data PV related menu items - keep more or less in line with standard QE contextMenu
      //
      PLOTTER_COPY_DATA_VARIABLE,
      PLOTTER_COPY_DATA_DATA,
      PLOTTER_PASTE_DATA_PV,
//    PLOTTER_DRAG_DATA_VARIABLE,
//    PLOTTER_DRAG_DATA_DATA,
      PLOTTER_SHOW_DATA_PV_PROPERTIES,
      PLOTTER_ADD_DATA_PV_TO_STRIPCHART,
      PLOTTER_ADD_DATA_PV_TO_SCRATCH_PAD,
      PLOTTER_GENERAL_DATA_PV_EDIT,

      // Size PV related menu items.
      //
      PLOTTER_COPY_SIZE_VARIABLE,
      PLOTTER_COPY_SIZE_DATA,
      PLOTTER_PASTE_SIZE_PV,
//    PLOTTER_DRAG_SIZE_VARIABLE,
//    PLOTTER_DRAG_SIZE_DATA,
      PLOTTER_SHOW_SIZE_PV_PROPERTIES,
      PLOTTER_ADD_SIZE_PV_TO_STRIPCHART,
      PLOTTER_ADD_SIZE_PV_TO_SCRATCH_PAD,
      PLOTTER_GENERAL_SIZE_PV_EDIT,

      PLOTTER_LAST                     // Must be last
   };

   enum ScaleModes { smFixed,          // Fixed scale in x and y
                     smNormalised,     // y plots scales such that { min to max } map to { 0 to 1 }
                     smFractional,     // y plots scales such that { min to max } map to { 0 to 1 }
                     smDynamic };      // x and y scales continually adjuxsted.

};

#endif  // QE_PLOTTER_ACTIONS_H
