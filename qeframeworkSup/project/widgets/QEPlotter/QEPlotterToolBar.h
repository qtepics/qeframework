/*  QEPlotterToolBar.h
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

#ifndef QE_PLOTTER_TOOLBAR_H
#define QE_PLOTTER_TOOLBAR_H

#include <QFrame>
#include <QPushButton>
#include <QPushButton>
#include <QWidget>
#include <QEOneToOne.h>
#include <QEFrameworkLibraryGlobal.h>

#include "QEPlotterNames.h"

/// This class holds all the QEPlotter tool bar widget.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPlotterToolBar : public QFrame {
   Q_OBJECT
public:  
   explicit QEPlotterToolBar (QWidget *parent = 0);
   virtual ~QEPlotterToolBar ();

   static const int designHeight = 32;

   void setEnabled (const QEPlotterNames::MenuActions action, const bool enabled);

signals:
   // Note: this is the same signature as the QEPlotterMenu signal.
   // For the toolbar slot is always 0.
   //
   void selected (const QEPlotterNames::MenuActions action, const int slot);

protected:
   void resizeEvent (QResizeEvent * event);

private:
   // Map actions <==>  QPushButtons
   //
   typedef QEOneToOne <QPushButton*, QEPlotterNames::MenuActions> ButtonActionMaps;
   ButtonActionMaps buttonActionMap;

private slots:
   // Send from the various buttons on the toolbar.
   //
   void buttonClicked (bool checked = false);
};

#endif  // QE_PLOTTER_TOOLBAR_H
