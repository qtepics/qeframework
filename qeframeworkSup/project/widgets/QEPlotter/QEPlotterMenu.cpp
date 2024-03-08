/*  QEPlotterMenu.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2022 Australian Synchrotron.
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>

#include <QECommon.h>
#include <QEWidget.h>
#include <QEPlotterMenu.h>

#define DEBUG  qDebug () << "QEPlotterMenu::" <<  __FUNCTION__  << ":" << __LINE__


//------------------------------------------------------------------------------
//
QEPlotterMenu::QEPlotterMenu (QWidget* parent) : QMenu (parent)
{
   QMenu* menu = NULL;

   this->slot = 0;

   // Ensure all actions are null unless otherwise defined.
   //
   for (int j = 0; j < ARRAY_LENGTH (this->actionList); j++) {
      this->actionList [j] = NULL;
   }

   this->setTitle (tr("Plotter General"));

   menu = new QMenu (tr("General"), this);
   this->addMenu (menu);
   this->make (menu, tr("Previous state"),          false, QEPlotterNames::PLOTTER_PREV);
   this->make (menu, tr("Next state"),              false, QEPlotterNames::PLOTTER_NEXT);

   this->make (menu, tr("White background"),        false, QEPlotterNames::PLOTTER_NORMAL_VIDEO);
   this->make (menu, tr("Black background"),        false, QEPlotterNames::PLOTTER_REVERSE_VIDEO);

   this->make (menu, tr("Play - Real time"),        false, QEPlotterNames::PLOTTER_PLAY);
   this->make (menu, tr("Pause"),                   false, QEPlotterNames::PLOTTER_PAUSE);

   menu = new QMenu (tr("Y Scale"), this);
   this->addMenu (menu);
   this->make (menu, tr("Linear Y scale"),          false, QEPlotterNames::PLOTTER_LINEAR_Y_SCALE);
   this->make (menu, tr("Log Y Scale"),             false, QEPlotterNames::PLOTTER_LOG_Y_SCALE);
   this->make (menu, tr("Manual Y Scale..."),       false, QEPlotterNames::PLOTTER_MANUAL_Y_RANGE);
   this->make (menu, tr("Y Data Range Scale"),      false, QEPlotterNames::PLOTTER_CURRENT_Y_RANGE);
   this->make (menu, tr("Dynamic Y Scale"),         false, QEPlotterNames::PLOTTER_DYNAMIC_Y_RANGE);
   this->make (menu, tr("Noramalised Scale"),       false, QEPlotterNames::PLOTTER_NORAMLISED_Y_RANGE);
   this->make (menu, tr("Fractional Scale"),        false, QEPlotterNames::PLOTTER_FRACTIONAL_Y_RANGE);

   menu = new QMenu (tr("X Scale"), this);
   this->addMenu (menu);
   this->make (menu, tr("Linear X scale"),          false, QEPlotterNames::PLOTTER_LINEAR_X_SCALE);
   this->make (menu, tr("Log X Scale"),             false, QEPlotterNames::PLOTTER_LOG_X_SCALE);
   this->make (menu, tr("Manual X Scale..."),       false, QEPlotterNames::PLOTTER_MANUAL_X_RANGE);
   this->make (menu, tr("X Data Range Scale"),      false, QEPlotterNames::PLOTTER_CURRENT_X_RANGE);
   this->make (menu, tr("Dynamic X Scale"),         false, QEPlotterNames::PLOTTER_DYNAMIC_X_RANGE);

   this->make (this, tr("Manual XY Scale..."),      false, QEPlotterNames::PLOTTER_MANUAL_XY_RANGE);

   menu = new QMenu (tr("Show"), this);
   this->addMenu (menu);
   this->make (menu, tr("Show/Hide Tool Bar"),      true,  QEPlotterNames::PLOTTER_SHOW_HIDE_TOOLBAR);
   this->make (menu, tr("Show/Hide PV Items"),      true,  QEPlotterNames::PLOTTER_SHOW_HIDE_PV_ITEMS);
   this->make (menu, tr("Show/Hide Status"),        true,  QEPlotterNames::PLOTTER_SHOW_HIDE_STATUS);
   this->make (menu, tr("Show/Hide Crosshairs"),    true,  QEPlotterNames::PLOTTER_SHOW_HIDE_CROSSHAIRS);

   this->make (this, tr("Emit Coordinates"),        false, QEPlotterNames::PLOTTER_EMIT_COORDINATES );

   // The following menu actions reflect the standard menu items.
   //
   this->addSeparator ();
   this->make (this, tr("Copy variable name"),      false, QEPlotterNames::PLOTTER_COPY_VARIABLE );
   this->make (this, tr("Copy data"),               false, QEPlotterNames::PLOTTER_COPY_DATA );
   this->make (this, tr("Paste to variable name"),  false, QEPlotterNames::PLOTTER_PASTE );

   this->addSeparator ();
   this->make (this, tr("Drag variable name"),      true, QEPlotterNames::PLOTTER_DRAG_VARIABLE );
   this->setActionChecked (QEPlotterNames::PLOTTER_DRAG_VARIABLE, true);
   this->make (this, tr("Drag data"),               true, QEPlotterNames::PLOTTER_DRAG_DATA );

   // Similar to QEAbstractDynamicWidget menu construction.
   //
   this->addSeparator ();
   this->make (this, tr("Use PV Names"),                     true, QEPlotterNames::PLOTTER_SELECT_USE_PV_NAME );
   this->make (this, tr("Use Alias Names (if available)"),   true, QEPlotterNames::PLOTTER_SELECT_USE_ALIAS_NAME);
   this->make (this, tr("Use Descriptions (if available)"),  true, QEPlotterNames::PLOTTER_SELECT_USE_DESCRIPTION );
   this->setActionChecked (QEPlotterNames::PLOTTER_SELECT_USE_ALIAS_NAME, true);

   QObject::connect (this, SIGNAL (triggered             (QAction* ) ),
                     this, SLOT   (contextMenuTriggered  (QAction* )));
}

//------------------------------------------------------------------------------
//
QEPlotterMenu::QEPlotterMenu (const int slotIn, QWidget* parent) : QMenu (parent)
{
   QMenu* menu = NULL;

   this->slot = slotIn;

   // Ensure all actions are null unless otherwise defined.
   //
   for (int j = 0; j < ARRAY_LENGTH (this->actionList); j++) {
      this->actionList [j] = NULL;
   }

   this->setTitle (tr("Plotter Item"));

   this->make (this, tr("Define... "),              false, QEPlotterNames::PLOTTER_DATA_DIALOG);

   menu = new QMenu (tr("Data PV"), this);
   this->addMenu (menu);
   this->make (menu, tr("Examine Properties"),      false, QEPlotterNames::PLOTTER_SHOW_DATA_PV_PROPERTIES);
   this->make (menu, tr("Plot in StripChart"),      false, QEPlotterNames::PLOTTER_ADD_DATA_PV_TO_STRIPCHART);
   this->make (menu, tr("Show in Scratch Pad"),     false, QEPlotterNames::PLOTTER_ADD_DATA_PV_TO_SCRATCH_PAD);
   menu->addSeparator();
   this->make (menu, tr("Copy variable name"),      false, QEPlotterNames::PLOTTER_COPY_DATA_VARIABLE);
   this->make (menu, tr("Copy data"),               false, QEPlotterNames::PLOTTER_COPY_DATA_DATA);
   this->make (menu, tr("Paste to variable name"),  false, QEPlotterNames::PLOTTER_PASTE_DATA_PV);
   menu->addSeparator();
   this->make (menu, tr("Edit PV"),                 false,  QEPlotterNames::PLOTTER_GENERAL_DATA_PV_EDIT);


   menu = new QMenu (tr("Size PV"), this);
   this->addMenu (menu);
   this->make (menu, tr("Examine Properties"),      false, QEPlotterNames::PLOTTER_SHOW_SIZE_PV_PROPERTIES);
   this->make (menu, tr("Plot in StripChart"),      false, QEPlotterNames::PLOTTER_ADD_SIZE_PV_TO_STRIPCHART);
   this->make (menu, tr("Show in Scratch Pad"),     false, QEPlotterNames::PLOTTER_ADD_SIZE_PV_TO_SCRATCH_PAD);
   menu->addSeparator();
   this->make (menu, tr("Copy variable name"),      false, QEPlotterNames::PLOTTER_COPY_SIZE_VARIABLE);
   this->make (menu, tr("Copy data"),               false, QEPlotterNames::PLOTTER_COPY_SIZE_DATA);
   this->make (menu, tr("Paste to variable name"),  false, QEPlotterNames::PLOTTER_PASTE_SIZE_PV);
   menu->addSeparator();
   this->make (menu, tr("Edit PV"),                 false,  QEPlotterNames::PLOTTER_GENERAL_SIZE_PV_EDIT);


   menu = new QMenu (tr("Scale Chart to"), this);
   this->addMenu (menu);
   this->make (menu, tr("Min to Max value"),        false, QEPlotterNames::PLOTTER_SCALE_TO_MIN_MAX);
   this->make (menu, tr("Zero to Max value "),      false, QEPlotterNames::PLOTTER_SCALE_TO_ZERO_MAX);

   menu = new QMenu (tr("Line"), this);
   this->addMenu (menu);

   // This only apply to Y data.
   //
   menu->setEnabled (this->slot > 0);
   if (this->slot > 0) {
      this->make (menu, tr("Bold"),                 true,  QEPlotterNames::PLOTTER_LINE_BOLD);
      this->make (menu, tr("Dashed"),               true,  QEPlotterNames::PLOTTER_LINE_DASHED);
      this->make (menu, tr("Dots"),                 true,  QEPlotterNames::PLOTTER_LINE_DOTS);
      this->make (menu, tr("Visible"),              true,  QEPlotterNames::PLOTTER_LINE_VISIBLE);
      menu->addSeparator ();
      this->make (menu, tr("No Filter"),            true,  QEPlotterNames::PLOTTER_LINE_NO_MEDIAN_FILTER);
      this->make (menu, tr("Median 3"),             true,  QEPlotterNames::PLOTTER_LINE_MEDIAN_3_FILTER);
      this->make (menu, tr("Median 5"),             true,  QEPlotterNames::PLOTTER_LINE_MEDIAN_5_FILTER);
      menu->addSeparator ();
      this->make (menu, tr("Colour... "),           false, QEPlotterNames::PLOTTER_LINE_COLOUR)->setEnabled (slot < 16);
   }

   this->make (this, tr("Select"),                  false, QEPlotterNames::PLOTTER_DATA_SELECT)->setEnabled (slot > 0);

   this->make (this, tr("Clear"),                   false, QEPlotterNames::PLOTTER_DATA_CLEAR);

   QObject::connect (this, SIGNAL (triggered             (QAction* )),
                     this, SLOT   (contextMenuTriggered  (QAction* )));
}

//------------------------------------------------------------------------------
//
QEPlotterMenu::~QEPlotterMenu ()
{
}

//------------------------------------------------------------------------------
//
#define SET_ACTION(attribute)                                                  \
void QEPlotterMenu::setAction##attribute (const QEPlotterNames::MenuActions action,  \
                                          const bool value)                    \
{                                                                              \
   const int t =  action - QEPlotterNames::PLOTTER_FIRST;                      \
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
void  QEPlotterMenu::setActionText (const QEPlotterNames::MenuActions actionName,
                                    const QString& caption)
{
   const int t =  actionName - QEPlotterNames::PLOTTER_FIRST;
   if (t >= 0 && t < ARRAY_LENGTH (this->actionList)) {
      QAction* action = this->actionList [t];
      if (action) action->setText (caption);
   }
}

//------------------------------------------------------------------------------
//
void QEPlotterMenu::setCheckedStates (const bool isDisplayed, const bool isBold,
                                      const bool isDashed, const bool showDots)
{
   if (this->slot > 0) {
      this->setActionChecked (QEPlotterNames::PLOTTER_LINE_VISIBLE, isDisplayed);
      this->setActionChecked (QEPlotterNames::PLOTTER_LINE_BOLD, isBold);
      this->setActionChecked (QEPlotterNames::PLOTTER_LINE_DASHED, isDashed);
      this->setActionChecked (QEPlotterNames::PLOTTER_LINE_DOTS, showDots);
   }
}

//------------------------------------------------------------------------------
//
QAction* QEPlotterMenu::make (QMenu* parent,
                              const QString& caption,
                              const bool checkable,
                              const QEPlotterNames::MenuActions menuAction)
{
   const int t =  menuAction - QEPlotterNames::PLOTTER_FIRST;

   QAction* action = NULL;

   if ((t >= 0) && (t < ARRAY_LENGTH (this->actionList))) {
      action = new QAction (caption + " ", parent);
      action->setCheckable (checkable);
      action->setData (QVariant (int (menuAction)));
      parent->addAction (action);

      this->actionList [t] = action;
   }
   return action;
}

//------------------------------------------------------------------------------
//
void QEPlotterMenu::contextMenuTriggered (QAction* selectedItem)
{
   bool okay;
   QEPlotterNames::MenuActions action;

   action = QEPlotterNames::MenuActions (selectedItem->data ().toInt (&okay));

   // Neither first nor last are actual actions.
   //
   if (okay && (action > QEPlotterNames::PLOTTER_FIRST) &&
               (action < QEPlotterNames::PLOTTER_LAST)) {
      emit this->selected (action, this->slot);
   }
}

// end
