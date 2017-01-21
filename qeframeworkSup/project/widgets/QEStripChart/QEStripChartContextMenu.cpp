/*  QEStripChartContextMenu.cpp
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
 *  Copyright (c) 2013 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QVariant>
#include <QDebug>
#include "QEStripChartContextMenu.h"
#include <QECommon.h>


#define DEBUG  qDebug () << "QEStripChartContextMenu::" <<  __FUNCTION__  << ":" << __LINE__


//------------------------------------------------------------------------------
//
QEStripChartContextMenu::QEStripChartContextMenu (bool inUseIn, QWidget *parent) : QMenu (parent)
{
   QMenu *menu;
   QAction *action;
   QEStripChartNames::ContextMenuOptions option;

   unsigned int j;

   this->inUse = inUseIn;

   this->setTitle ("PV Item");

   this->serverTime = NULL;
   this->serverTime = NULL;

   for (j = 0; j < ARRAY_LENGTH (this->archiveModeActions); j++) {
      this->archiveModeActions [j] = NULL;
   }

   for (j = 0; j < ARRAY_LENGTH (this->lineDrawModeActions); j++) {
      this->lineDrawModeActions [j] = NULL;
   }

   for (j = 0 ; j < ARRAY_LENGTH (this->predefinedPVs); j++) {
      this->predefinedPVs [j] = NULL;
   }

   // Note: action items are not enabled until corresponding functionallity is implemented.
   //
   if (inUseIn) {
      this->make (this, "Read Archive",                        false, QEStripChartNames::SCCM_READ_ARCHIVE);

      menu = new QMenu ("Scale chart to this PV's", this);
      this->addMenu (menu);
      this->make (menu, "HOPR/LOPR values",                    false, QEStripChartNames::SCCM_SCALE_CHART_AUTO);
      this->make (menu, "Plotted min/max values",              false, QEStripChartNames::SCCM_SCALE_CHART_PLOTTED);
      this->make (menu, "Buffered min/max values",             false, QEStripChartNames::SCCM_SCALE_CHART_BUFFERED);

      menu = new QMenu ("Adjust/Scale this PV", this);
      this->addMenu (menu);
      this->make (menu, "Reset",                               false, QEStripChartNames::SCCM_SCALE_PV_RESET);
      this->make (menu, "General...",                          false, QEStripChartNames::SCCM_SCALE_PV_GENERAL);
      this->make (menu, "HOPR/LOPR values map to chart range", false, QEStripChartNames::SCCM_SCALE_PV_AUTO);
      this->make (menu, "Plotted values map to chart range",   false, QEStripChartNames::SCCM_SCALE_PV_PLOTTED);
      this->make (menu, "Buffered values map to chart range",  false, QEStripChartNames::SCCM_SCALE_PV_BUFFERED);
      this->make (menu, "First value maps to chart centre",    false, QEStripChartNames::SCCM_SCALE_PV_CENTRE);

      menu = new QMenu ("Mode", this);
      this->addMenu (menu);
      this->make (menu, "Rectangular",                         false, QEStripChartNames::SCCM_PLOT_RECTANGULAR)->setEnabled (false);
      this->make (menu, "Smooth",                              false, QEStripChartNames::SCCM_PLOT_SMOOTH)->setEnabled (false);
      this->serverTime =
      this->make (menu, "User PV Process Time",                true,  QEStripChartNames::SCCM_PLOT_SERVER_TIME);
      this->clientTime =
      this->make (menu, "Use Receive Time",                    true,  QEStripChartNames::SCCM_PLOT_CLIENT_TIME);

      menu->addSeparator();
      this->archiveModeActions [QEArchiveInterface::Linear] =
      this->make (menu, "Linear",                              true,  QEStripChartNames::SCCM_ARCH_LINEAR);
      this->archiveModeActions [QEArchiveInterface::PlotBinning] =
      this->make (menu, "Plot Binning",                        true,  QEStripChartNames::SCCM_ARCH_PLOTBIN);
      this->archiveModeActions [QEArchiveInterface::Raw] =
      this->make (menu, "Raw",                                 true,  QEStripChartNames::SCCM_ARCH_RAW);
      this->archiveModeActions [QEArchiveInterface::SpreadSheet] =
      this->make (menu, "Spread Sheet",                        true,  QEStripChartNames::SCCM_ARCH_SHEET);
      this->archiveModeActions [QEArchiveInterface::Averaged] =
      this->make (menu, "Averaged",                            true,  QEStripChartNames::SCCM_ARCH_AVERAGED);

      menu = new QMenu ("Line", this);
      this->addMenu (menu);
      this->lineDrawModeActions [QEStripChartNames::ldmHide] =
      this->make (menu, "Hide",                                true,  QEStripChartNames::SCCM_LINE_HIDE);
      this->lineDrawModeActions [QEStripChartNames::ldmRegular] =
      this->make (menu, "Regular",                             true,  QEStripChartNames::SCCM_LINE_REGULAR);
      this->lineDrawModeActions [QEStripChartNames::ldmBold] =
      this->make (menu, "Bold",                                true,  QEStripChartNames::SCCM_LINE_BOLD);
      this->make (menu, "Colour...",                           false, QEStripChartNames::SCCM_LINE_COLOUR);

      this->make (this, "Edit PV Name...",                     false, QEStripChartNames::SCCM_PV_EDIT_NAME);

      this->make (this, "Write PV trace to file...",           false, QEStripChartNames::SCCM_PV_WRITE_TRACE);

      this->make (this, "Generate Statistics",                 false, QEStripChartNames::SCCM_PV_STATS);

      this->make (this, "Add to predefined PV names",          false, QEStripChartNames::SCCM_ADD_TO_PREDEFINED);

      this->make (this, "Clear",                               false, QEStripChartNames::SCCM_PV_CLEAR);

   } else {

      this->make (this, "Add PV Name...",                      false, QEStripChartNames::SCCM_PV_ADD_NAME);
      this->make (this, "Paste PV Name ",                      false, QEStripChartNames::SCCM_PV_PASTE_NAME);
      this->make (this, "Colour...",                           false, QEStripChartNames::SCCM_LINE_COLOUR);
      this->addSeparator ();

      for (j = 0 ; j < ARRAY_LENGTH (this->predefinedPVs); j++) {
         option = QEStripChartNames::ContextMenuOptions (QEStripChartNames::SCCM_PREDEFINED_01 + j);
         action = this->make (this, "", false, option);
         action->setVisible (false);
         this->predefinedPVs [j] = action;
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
void QEStripChartContextMenu::setPredefinedNames (const QStringList & pvList)
{
   unsigned int j;
   QAction *action;

   if (!this->inUse) {
      for (j = 0 ; j < ARRAY_LENGTH (this->predefinedPVs); j++) {
         action = this->predefinedPVs [j];
         if (!action) continue;
         if ((int) j < pvList.count ()) {
            action->setText (pvList.value(j) + " ");
            action->setVisible (true);
         } else {
            action->setVisible (false);
         }
      }
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartContextMenu::setUseReceiveTime  (const bool useReceiveTime)
{
   if (this->serverTime) {
      this->serverTime->setChecked (!useReceiveTime);
   }
   if (this->clientTime) {
      this->clientTime->setChecked (useReceiveTime);
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartContextMenu::setArchiveReadHow (const QEArchiveInterface::How how)
{
   int j;
   QAction *action;

   for (j = 0; j < ARRAY_LENGTH (this->archiveModeActions); j++) {
      action = this->archiveModeActions [j];
      if (!action) continue;
      action->setChecked (j == (int) how);
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartContextMenu::setLineDrawMode (const QEStripChartNames::LineDrawModes mode)
{
   int j;
   QAction *action;

   for (j = 0; j < ARRAY_LENGTH (this->lineDrawModeActions); j++) {
      action = this->lineDrawModeActions [j];
      if (!action) continue;
      action->setChecked (j == (int) mode);
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
QAction* QEStripChartContextMenu::make (QMenu *parent,
                                        const QString & caption,
                                        const bool checkable,
                                        const QEStripChartNames::ContextMenuOptions option)
{
   QAction* action;

   action = new QAction (caption, parent);
   action->setCheckable (checkable);
   action->setData (QVariant (int (option)));
   parent->addAction (action);
   return action;
}

// end
