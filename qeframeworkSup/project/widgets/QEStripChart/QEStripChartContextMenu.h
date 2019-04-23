/*  QEStripChartContextMenu.h
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

#ifndef QE_STRIP_CHART_CONTEXT_MENU_H
#define QE_STRIP_CHART_CONTEXT_MENU_H

#include <QAction>
#include <QMenu>
#include <QObject>
#include <QWidget>
#include <QStringList>
#include <contextMenu.h>
#include <QEArchiveInterface.h>
#include <QEFrameworkLibraryGlobal.h>
#include "QEStripChartNames.h"

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEStripChartContextMenu : public QMenu {
Q_OBJECT
public:
   /// Construct strip chart item context menu.
   /// This menu item creates all required sub menu items.
   /// inUse set true for an inuse slot, i.e. already has a PV allocated.
   /// inUse set false for an empty slot.
   explicit QEStripChartContextMenu (bool inUse, QWidget *parent = 0);
   virtual ~QEStripChartContextMenu ();

   // Allow setting of specific action attributes.
   //
   void setActionChecked (const QEStripChartNames::ContextMenuOptions option, const bool checked);
   void setActionEnabled (const QEStripChartNames::ContextMenuOptions option, const bool enabled);
   void setActionVisible (const QEStripChartNames::ContextMenuOptions option, const bool visible);
   void setActionText    (const QEStripChartNames::ContextMenuOptions option, const QString& caption);

   // Conveniance functions for above.
   //
   void setIsCalculation   (const bool isCalculation);
   void setPredefinedNames (const QStringList& pvList);
   void setUseReceiveTime  (const bool useReceiveTime);
   void setArchiveReadHow  (const QEArchiveInterface::How how);
   void setLineDrawMode    (const QEStripChartNames::LineDrawModes mode);
   void setLinePlotMode    (const QEStripChartNames::LinePlotModes mode);

signals:
   // All the triggered actions from the various sub-menu items are
   // converted to an ContextMenuOptions value.
   void contextMenuSelected (const QEStripChartNames::ContextMenuOptions);

private:
   bool inUse;

   QAction* actionList [QEStripChartNames::NumberMenuItems];

   // Utility function to create and set up an action.
   //
   QAction* make (QMenu* parent,
                  const QString &caption,
                  const bool checkable,
                  const QEStripChartNames::ContextMenuOptions option);

private slots:
   void contextMenuTriggered (QAction* selectedItem);
};

#endif  // QE_STRIP_CHART_CONTEXT_MENU_H
