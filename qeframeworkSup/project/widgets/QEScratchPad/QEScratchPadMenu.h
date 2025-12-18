/*  QEScratchPadMenu.h
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

#ifndef QE_SCRATCH_PAD_MENU_H
#define QE_SCRATCH_PAD_MENU_H

#include <QAction>
#include <QMenu>
#include <QObject>
#include <QWidget>
#include <contextMenu.h>
#include <QEAbstractDynamicWidget.h>

class QEScratchPadMenu : public QMenu {
Q_OBJECT
public:
   explicit QEScratchPadMenu (const int slot, QWidget* parent = 0);
   virtual ~QEScratchPadMenu ();

   // IDs for all menu options
   // Each menu option has a unique ID across all menus
   // These IDs are in addition to standard context menu IDs and so start after
   // contextMenu::CM_SPECIFIC_WIDGETS_START_HERE
   //
   enum ContextMenuOptions {
      SCRATCHPAD_NONE = QEAbstractDynamicWidget::ADWCM_SUB_CLASS_WIDGETS_START_HERE,

      // Menu items for global QEWidget context menu
      //
      SCRATCHPAD_SORT_PV_NAMES,
      SCRATCHPAD_CLEAR_ALL,

      // Menu items for QEScratchPadMenu context menu.
      //
      SCRATCHPAD_ADD_PV_NAME,
      SCRATCHPAD_PASTE_PV_NAME,
      SCRATCHPAD_EDIT_PV_NAME,
      SCRATCHPAD_DATA_CLEAR

   };

   // These MUST be consistant with above declaration.
   //
   static const ContextMenuOptions ContextMenuItemFirst = SCRATCHPAD_ADD_PV_NAME;
   static const ContextMenuOptions ContextMenuItemLast  = SCRATCHPAD_DATA_CLEAR;
   static const int NumberContextMenuItems = ContextMenuItemLast - ContextMenuItemFirst + 1;


   // Allow setting of specific action attributes using option as index.
   //
   void setActionChecked (const ContextMenuOptions option, const bool visible);
   void setActionEnabled (const ContextMenuOptions option, const bool visible);
   void setActionVisible (const ContextMenuOptions option, const bool visible);

   void setIsInUse (const bool isInUse);

signals:
   // All the triggered actions from the various sub-menu items are
   // converted to an ContextMenuOptions value.
   //
   void contextMenuSelected (const int, const QEScratchPadMenu::ContextMenuOptions);

private:
   int slot;
   QAction* actionList [NumberContextMenuItems];

   // Utility function to create and set up an action.
   //
   QAction* make (QMenu *parent,
                  const QString &caption,
                  const bool checkable,
                  const QEScratchPadMenu::ContextMenuOptions option);

private slots:
   void contextMenuTriggered (QAction* selectedItem);
};

#endif  // QE_SCRATCH_PAD_MENU_H
