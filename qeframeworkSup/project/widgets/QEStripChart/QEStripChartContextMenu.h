/*  QEStripChartContextMenu.h
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
