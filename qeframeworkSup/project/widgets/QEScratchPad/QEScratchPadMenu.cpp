/*  QEScratchPadMenu.cpp
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

#include <QDebug>
#include <QEScratchPadMenu.h>
#include <QECommon.h>

#define DEBUG  qDebug () << "QEScratchPadMenu::" <<  __FUNCTION__  << ":" << __LINE__


//------------------------------------------------------------------------------
//
QEScratchPadMenu::QEScratchPadMenu (const int slotIn, QWidget* parent) : QMenu (parent)
{
   this->slot = slotIn;

   // Ensure all actions are null unless otherwise defined.
   //
   for (int j = 0; j < ARRAY_LENGTH (this->actionList); j++) {
      this->actionList [j] = 0;
   }

   this->setTitle ("ScratchPad Item");

   this->make (this, "Add PV Name...",   false, SCRATCHPAD_ADD_PV_NAME);
   this->make (this, "Paste PV Name",    false, SCRATCHPAD_PASTE_PV_NAME);
   this->make (this, "Edit PV Name...",  false, SCRATCHPAD_EDIT_PV_NAME);
   this->make (this, "Clear",            false, SCRATCHPAD_DATA_CLEAR);

   QObject::connect (this, SIGNAL (triggered             (QAction* ) ),
                     this, SLOT   (contextMenuTriggered  (QAction* )));
}

//------------------------------------------------------------------------------
//
QEScratchPadMenu::~QEScratchPadMenu ()
{
}

//------------------------------------------------------------------------------
//
#define SET_ACTION(attribute)                                                  \
void QEScratchPadMenu::setAction##attribute (const ContextMenuOptions option,  \
                                             const bool value)                 \
{                                                                              \
   const int t =  option - ContextMenuItemFirst;                               \
   if (t >= 0 && t < ARRAY_LENGTH (this->actionList)) {                        \
      QAction* action = this->actionList [t];                                  \
      if (action) action->set##attribute (value);                              \
   }                                                                           \
}


SET_ACTION (Checked)
SET_ACTION (Enabled)
SET_ACTION (Visible)

#undef SET_ACTION

//------------------------------------------------------------------------------
//
void QEScratchPadMenu::setIsInUse (const bool isInUse)
{
   this->setActionVisible (SCRATCHPAD_ADD_PV_NAME,   !isInUse);
   this->setActionVisible (SCRATCHPAD_PASTE_PV_NAME, !isInUse);
   this->setActionVisible (SCRATCHPAD_EDIT_PV_NAME,  isInUse);
   this->setActionVisible (SCRATCHPAD_DATA_CLEAR,    isInUse);
}

//------------------------------------------------------------------------------
//
QAction* QEScratchPadMenu::make (QMenu* parent,
                                 const QString& caption,
                                 const bool checkable,
                                 const QEScratchPadMenu::ContextMenuOptions option)
{
   QAction* action;

   action = new QAction (caption, parent);
   action->setCheckable (checkable);
   action->setData (QVariant (int (option)));
   parent->addAction (action);

   this->actionList [option - ContextMenuItemFirst] = action;
   return action;
}

//------------------------------------------------------------------------------
//
void QEScratchPadMenu::contextMenuTriggered (QAction* selectedItem)
{
   bool okay;
   QEScratchPadMenu::ContextMenuOptions option;

   option = QEScratchPadMenu::ContextMenuOptions (selectedItem->data ().toInt (&okay));

   if (okay &&
       (option >= QEScratchPadMenu::ContextMenuItemFirst) &&
       (option <= QEScratchPadMenu::ContextMenuItemLast)) {
      emit this->contextMenuSelected (this->slot, option);
   }
}

// end
