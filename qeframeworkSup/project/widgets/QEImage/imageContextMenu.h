/*  imageContextMenu.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2023 Australian Synchrotron
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
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 This class manages the context menu for the QEImage widget
 It includes a menu item to present the standard QEFramework context menu.
 Functions are available to add menu options, and sub menus.
 A function is available to present the menu to the user and return a user selection
 */

#ifndef QE_IMAGE_CONTEXT_MENU_H
#define QE_IMAGE_CONTEXT_MENU_H

#include <contextMenu.h>
#include <QMenu>

class imageContextMenu : public QMenu
{
   Q_OBJECT
public:

   explicit imageContextMenu( QWidget *parent = 0 );

   // IDs for all menu options
   // Each menu option has a unique ID across all menus
   // These IDs are in addition to standard context menu IDs and so start after contextMenu::CM_SPECIFIC_WIDGETS_START_HERE
   //
   enum imageContextMenuOptions {
      ICM_NONE = contextMenu::CM_SPECIFIC_WIDGETS_START_HERE,
      ICM_SAVE,
      ICM_PAUSE,
      ICM_ENABLE_TIME,
      ICM_ENABLE_FALSE_COLOUR,
      ICM_ENABLE_PROFILE_AXES,
      ICM_ENABLE_CURSOR_PIXEL,
      ICM_ABOUT_IMAGE,
      ICM_ENABLE_VERT1,
      ICM_ENABLE_VERT2,
      ICM_ENABLE_VERT3,
      ICM_ENABLE_VERT4,
      ICM_ENABLE_VERT5,
      ICM_ENABLE_HOZ1,
      ICM_ENABLE_HOZ2,
      ICM_ENABLE_HOZ3,
      ICM_ENABLE_HOZ4,
      ICM_ENABLE_HOZ5,
      ICM_ENABLE_AREA1,
      ICM_ENABLE_AREA2,
      ICM_ENABLE_AREA3,
      ICM_ENABLE_AREA4,
      ICM_ENABLE_LINE,
      ICM_ENABLE_TARGET,
      ICM_ENABLE_BEAM,
      ICM_DISPLAY_BUTTON_BAR,
      ICM_DISPLAY_IMAGE_DISPLAY_PROPERTIES,
      ICM_DISPLAY_RECORDER,
      ICM_ZOOM_SELECTED,
      ICM_ZOOM_FIT,
      ICM_ZOOM_PLUS,
      ICM_ZOOM_MINUS,
      ICM_ZOOM_10,
      ICM_ZOOM_25,
      ICM_ZOOM_50,
      ICM_ZOOM_75,
      ICM_ZOOM_100,
      ICM_ZOOM_150,
      ICM_ZOOM_200,
      ICM_ZOOM_300,
      ICM_ZOOM_400,
      ICM_ROTATE_NONE,
      ICM_ROTATE_RIGHT,
      ICM_ROTATE_LEFT,
      ICM_ROTATE_180,
      ICM_FLIP_HORIZONTAL,
      ICM_FLIP_VERTICAL,
      ICM_SELECT_PAN,
      ICM_SELECT_HSLICE1,
      ICM_SELECT_HSLICE2,
      ICM_SELECT_HSLICE3,
      ICM_SELECT_HSLICE4,
      ICM_SELECT_HSLICE5,
      ICM_SELECT_VSLICE1,
      ICM_SELECT_VSLICE2,
      ICM_SELECT_VSLICE3,
      ICM_SELECT_VSLICE4,
      ICM_SELECT_VSLICE5,
      ICM_SELECT_AREA1,
      ICM_SELECT_AREA2,
      ICM_SELECT_AREA3,
      ICM_SELECT_AREA4,
      ICM_SELECT_PROFILE,
      ICM_SELECT_TARGET,
      ICM_SELECT_BEAM,
      ICM_CLEAR_MARKUP,
      ICM_SET_LEGEND,
      ICM_THICKNESS_ONE_MARKUP,
      ICM_THICKNESS_SELECT_MARKUP,
      ICM_COPY_PLOT_DATA,
      ICM_FULL_SCREEN,
      ICM_DISPLAY_HSLICE1,
      ICM_DISPLAY_HSLICE2,
      ICM_DISPLAY_HSLICE3,
      ICM_DISPLAY_HSLICE4,
      ICM_DISPLAY_HSLICE5,
      ICM_DISPLAY_VSLICE1,
      ICM_DISPLAY_VSLICE2,
      ICM_DISPLAY_VSLICE3,
      ICM_DISPLAY_VSLICE4,
      ICM_DISPLAY_VSLICE5,
      ICM_DISPLAY_AREA1,
      ICM_DISPLAY_AREA2,
      ICM_DISPLAY_AREA3,
      ICM_DISPLAY_AREA4,
      ICM_DISPLAY_PROFILE,
      ICM_DISPLAY_TARGET,
      ICM_DISPLAY_BEAM,
      ICM_DISPLAY_TIMESTAMP,
      ICM_DISPLAY_ELLIPSE,
      ICM_OPTIONS,
      ICM_LOCAL_CONTROL
   };

   // Present the context menu, and return whatever the user selects
   void getContextMenuOption( const QPoint&, imageContextMenuOptions* option, bool* checked );

   // Add an item to the context menu
   void addMenuItem( const QString& title, const bool checkable, const bool checked, const imageContextMenuOptions option );

signals:

public slots:

private:
   // Add a menu item to the either the context menu, or one of its sub menus
   void addMenuItemGeneric( QMenu* menu, const QString& title, const bool checkable, const bool checked, const imageContextMenuOptions option );
};

#endif // QE_IMAGE_CONTEXT_MENU_H
