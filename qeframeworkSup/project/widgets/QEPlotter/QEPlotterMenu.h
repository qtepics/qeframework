/*  QEPlotterMenu.h
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

#ifndef QEPLOTTERMENU_H
#define QEPLOTTERMENU_H

#include <QAction>
#include <QMenu>
#include <QObject>
#include <QWidget>

#include "QEPlotterNames.h"

/// QEPlotter PV item specific context menu.
///
class QEPlotterMenu : public QMenu {
Q_OBJECT
public:
   // General context menu constructor
   //
   explicit QEPlotterMenu (QWidget* parent = 0);

   // PV item context menu constructor
   //
   explicit QEPlotterMenu (const int slot, QWidget* parent = 0);

   // Destuctor
   //
   virtual ~QEPlotterMenu ();

   // Allow setting of specific action attributes.
   //
   void setActionChecked (const QEPlotterNames::MenuActions action, const bool checked);
   void setActionEnabled (const QEPlotterNames::MenuActions action, const bool enabled);
   void setActionVisible (const QEPlotterNames::MenuActions action, const bool visible);
   void setActionText    (const QEPlotterNames::MenuActions action, const QString& caption);

   // Convienance function for setActionChecked for four attributes.
   //
   void setCheckedStates (const bool isDisplayed, const bool isBold,
                          const bool isDashed, const bool showDots);

signals:
   // All the triggered actions from the various sub-menu items are
   // converted to an QEPlotterNames::MenuActions value.
   // Slot set as applicable, otherwise set to 0.
   //
   void selected (const QEPlotterNames::MenuActions action, const int slot);

private:
   int slot;
   QAction* actionList [QEPlotterNames::PLOTTER_LAST - QEPlotterNames::PLOTTER_FIRST];

   // Utility function to create and set up an action.
   //
   QAction* make (QMenu *parent,
                  const QString &caption,
                  const bool checkable,
                  const QEPlotterNames::MenuActions menuAction);

private slots:
   // Sent form the sub-menus and/or assoictaed actions.
   //
   void contextMenuTriggered (QAction* selectedItem);
};

#endif  // QEPLOTTERMENU_H
