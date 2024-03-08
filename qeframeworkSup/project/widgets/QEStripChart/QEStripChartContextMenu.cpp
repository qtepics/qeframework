/*  QEStripChartContextMenu.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2019 Australian Synchrotron
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

#include "QEStripChartContextMenu.h"
#include <QVariant>
#include <QDebug>
#include <QECommon.h>


#define DEBUG  qDebug () << "QEStripChartContextMenu" << __LINE__ <<  __FUNCTION__  << "  "


//------------------------------------------------------------------------------
//
QEStripChartContextMenu::QEStripChartContextMenu (bool inUseIn, QWidget *parent) : QMenu (parent)
{
   QMenu *menu;
   QAction *action;

   this->inUse = inUseIn;

   // Ensure all actions are null unless otherwise defined.
   //
   for (int j = 0; j < ARRAY_LENGTH (this->actionList); j++) {
      this->actionList [j] = NULL;
   }

   this->setTitle ("PV Item");

   // Note: action items are not enabled until corresponding functionallity is implemented.
   //
   if (inUseIn) {
      this->make (this, tr("Read Archive"),                        false, QEStripChartNames::SCCM_READ_ARCHIVE);
      this->make (this, tr("Re Calculate"),                        false, QEStripChartNames::SCCM_RECALCULATE);

      this->setIsCalculation (false);

      menu = new QMenu (tr("Scale chart to this PV's"), this);
      this->addMenu (menu);
      this->make (menu, tr("HOPR/LOPR values"),                    false, QEStripChartNames::SCCM_SCALE_CHART_AUTO);
      this->make (menu, tr("Plotted min/max values"),              false, QEStripChartNames::SCCM_SCALE_CHART_PLOTTED);
      this->make (menu, tr("Buffered min/max values"),             false, QEStripChartNames::SCCM_SCALE_CHART_BUFFERED);

      menu = new QMenu (tr("Adjust/Scale this PV"), this);
      this->addMenu (menu);
      this->make (menu, tr("Reset"),                               false, QEStripChartNames::SCCM_SCALE_PV_RESET);
      this->make (menu, tr("General..."),                          false, QEStripChartNames::SCCM_SCALE_PV_GENERAL);
      this->make (menu, tr("HOPR/LOPR values map to chart range"), false, QEStripChartNames::SCCM_SCALE_PV_AUTO);
      this->make (menu, tr("Plotted values map to chart range"),   false, QEStripChartNames::SCCM_SCALE_PV_PLOTTED);
      this->make (menu, tr("Plotted values map to upper 3rd of chart"),    false, QEStripChartNames::SCCM_SCALE_PV_PLOTTED_UPPER);
      this->make (menu, tr("Plotted values map to centre 3rd of chart"),   false, QEStripChartNames::SCCM_SCALE_PV_PLOTTED_CENTRE);
      this->make (menu, tr("Plotted values map to lower 3rd of chart"),    false, QEStripChartNames::SCCM_SCALE_PV_PLOTTED_LOWER);
      this->make (menu, tr("Buffered values map to chart range"),  false, QEStripChartNames::SCCM_SCALE_PV_BUFFERED);
      this->make (menu, tr("First value maps to chart centre"),    false, QEStripChartNames::SCCM_SCALE_PV_FIRST_CENTRE);

      menu = new QMenu (tr("Mode"), this);
      this->addMenu (menu);

      this->make (menu, tr("Rectangular"),                         true,  QEStripChartNames::SCCM_PLOT_RECTANGULAR);
      this->make (menu, tr("Smooth"),                              true,  QEStripChartNames::SCCM_PLOT_SMOOTH);

      this->make (menu, tr("User PV Process Time"),                true,  QEStripChartNames::SCCM_PLOT_SERVER_TIME);
      this->make (menu, tr("Use Receive Time"),                    true,  QEStripChartNames::SCCM_PLOT_CLIENT_TIME);

      menu->addSeparator();
      this->make (menu, tr("Linear"),                              true,  QEStripChartNames::SCCM_ARCH_LINEAR);
      this->make (menu, tr("Plot Binning"),                        true,  QEStripChartNames::SCCM_ARCH_PLOTBIN);
      this->make (menu, tr("Raw"),                                 true,  QEStripChartNames::SCCM_ARCH_RAW);
      this->make (menu, tr("Spread Sheet"),                        true,  QEStripChartNames::SCCM_ARCH_SHEET);
      this->make (menu, tr("Averaged"),                            true,  QEStripChartNames::SCCM_ARCH_AVERAGED);

      menu = new QMenu (tr("Line"), this);
      this->addMenu (menu);
      this->make (menu, tr("Hide"),                                true,  QEStripChartNames::SCCM_LINE_HIDE);
      this->make (menu, tr("Regular"),                             true,  QEStripChartNames::SCCM_LINE_REGULAR);
      this->make (menu, tr("Bold"),                                true,  QEStripChartNames::SCCM_LINE_BOLD);
      this->make (menu, tr("Colour..."),                           false, QEStripChartNames::SCCM_LINE_COLOUR);

      this->make (this, tr("Edit PV Name..."),                     false, QEStripChartNames::SCCM_PV_EDIT_NAME);

      this->make (this, tr("Write PV trace to file..."),           false, QEStripChartNames::SCCM_PV_WRITE_TRACE);

      this->make (this, tr("Generate Statistics"),                 false, QEStripChartNames::SCCM_PV_STATS);

      this->make (this, tr("Add to predefined PV names"),          false, QEStripChartNames::SCCM_ADD_TO_PREDEFINED);

      this->make (this, tr("Clear"),                               false, QEStripChartNames::SCCM_PV_CLEAR);

   } else {

      this->make (this, tr("Add PV Name..."),                      false, QEStripChartNames::SCCM_PV_ADD_NAME);
      this->make (this, tr("Paste PV Name "),                      false, QEStripChartNames::SCCM_PV_PASTE_NAME);
      this->make (this, tr("Colour..."),                           false, QEStripChartNames::SCCM_LINE_COLOUR);
      this->addSeparator ();

      for (int j = 0 ; j < QEStripChartNames::NumberPrefefinedItems; j++) {
         QEStripChartNames::ContextMenuOptions option;
         option = QEStripChartNames::ContextMenuOptions (QEStripChartNames::SCCM_PREDEFINED_01 + j);
         action = this->make (this, "", false, option);
         action->setVisible (false);
      }
   }

   QObject::connect (this, SIGNAL (triggered             (QAction* ) ),
                     this, SLOT   (contextMenuTriggered  (QAction* )));
}

//------------------------------------------------------------------------------
//
QEStripChartContextMenu::~QEStripChartContextMenu ()
{
   // no special action
}


//------------------------------------------------------------------------------
//
#define SET_ACTION(attribute)                                                  \
void QEStripChartContextMenu::setAction##attribute                             \
      (const QEStripChartNames::ContextMenuOptions option,                     \
       const bool value)                                                       \
{                                                                              \
   const int t = option - QEStripChartNames::ContextMenuItemFirst;             \
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
void QEStripChartContextMenu::setActionText
    (const QEStripChartNames::ContextMenuOptions option, const QString& caption)
{
   const int t = option - QEStripChartNames::ContextMenuItemFirst;
   if (t >= 0 && t < ARRAY_LENGTH (this->actionList)) {
      QAction* action = this->actionList [t];
      if (action) action->setText (caption);
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartContextMenu::setIsCalculation (const bool isCalculation)
{
   // Go Visible ??
   this->setActionEnabled (QEStripChartNames::SCCM_READ_ARCHIVE, !isCalculation);
   this->setActionEnabled (QEStripChartNames::SCCM_RECALCULATE,  isCalculation);
}

//------------------------------------------------------------------------------
//
void QEStripChartContextMenu::setPredefinedNames (const QStringList& pvList)
{
   if (this->inUse) return;

   for (int j = 0 ; j < QEStripChartNames::NumberPrefefinedItems; j++) {
      QEStripChartNames::ContextMenuOptions option;
      option = QEStripChartNames::ContextMenuOptions (QEStripChartNames::SCCM_PREDEFINED_01 + j);

      if (j < pvList.count ()) {
         this->setActionText(option, pvList.value(j) + " ");
         this->setActionVisible (option, true);
      } else {
         this->setActionVisible (option, false);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartContextMenu::setUseReceiveTime  (const bool useReceiveTime)
{
   this->setActionChecked (QEStripChartNames::SCCM_PLOT_SERVER_TIME, !useReceiveTime);
   this->setActionChecked (QEStripChartNames::SCCM_PLOT_CLIENT_TIME, useReceiveTime);
}

//------------------------------------------------------------------------------
//
void QEStripChartContextMenu::setArchiveReadHow (const QEArchiveInterface::How how)
{
   // Maps How types to options.
   // NOTE: If the QEArchiveInterface::How defition changes, so must this.
   //
   static const QEStripChartNames::ContextMenuOptions optionMap [] = {
      QEStripChartNames::SCCM_ARCH_RAW,
      QEStripChartNames::SCCM_ARCH_SHEET,
      QEStripChartNames::SCCM_ARCH_AVERAGED,
      QEStripChartNames::SCCM_ARCH_PLOTBIN,
      QEStripChartNames::SCCM_ARCH_LINEAR
   };

   for (int j = 0; j < ARRAY_LENGTH (optionMap); j++) {
      this->setActionChecked (optionMap [j], (j == int(how)));
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartContextMenu::setLineDrawMode (const QEStripChartNames::LineDrawModes mode)
{
   // Maps How types to options.
   // NOTE: If the LineDrawModes defition changes, so must this.
   //
   static const QEStripChartNames::ContextMenuOptions optionMap [] = {
      QEStripChartNames::SCCM_LINE_HIDE,
      QEStripChartNames::SCCM_LINE_REGULAR,
      QEStripChartNames::SCCM_LINE_BOLD
   };

   for (int j = 0; j < ARRAY_LENGTH (optionMap); j++) {
      this->setActionChecked (optionMap [j], (j == int(mode)));
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartContextMenu::setLinePlotMode (const QEStripChartNames::LinePlotModes mode)
{
   // Maps How types to options.
   // NOTE: If the LinePlotModes defition changes, so must this.
   //
   static const QEStripChartNames::ContextMenuOptions optionMap [] = {
      QEStripChartNames::SCCM_PLOT_RECTANGULAR,
      QEStripChartNames::SCCM_PLOT_SMOOTH
   };

   for (int j = 0; j < ARRAY_LENGTH (optionMap); j++) {
      this->setActionChecked (optionMap [j], (j == int(mode)));
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartContextMenu::contextMenuTriggered (QAction* selectedItem)
{
   bool okay;
   QEStripChartNames::ContextMenuOptions option;

   option = QEStripChartNames::ContextMenuOptions (selectedItem->data ().toInt (&okay));

   if (okay &&
       (option >= QEStripChartNames::ContextMenuItemFirst) &&
       (option <= QEStripChartNames::ContextMenuItemLast)) {
      emit this->contextMenuSelected (option);
   }
}

//------------------------------------------------------------------------------
//
QAction* QEStripChartContextMenu::make (QMenu* parent,
                                        const QString& caption,
                                        const bool checkable,
                                        const QEStripChartNames::ContextMenuOptions option)
{
   const int t =  option - QEStripChartNames::ContextMenuItemFirst;

   QAction* action = NULL;

   if ((t >= 0) && (t < ARRAY_LENGTH (this->actionList))) {
      action = new QAction (caption + " ", parent);
      action->setCheckable (checkable);
      action->setData (QVariant (int (option)));
      parent->addAction (action);
      this->actionList [t] = action;  // save in action list
   }
   return action;
}

// end
