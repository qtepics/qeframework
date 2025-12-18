/*  QEDialog.h
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

#ifndef QEDIALOG_H
#define QEDIALOG_H

#include <QDialog>
#include <QRect>

#include <QEFrameworkLibraryGlobal.h>

/// This class provides a thin wrapper around the QDialog class, specifically the
/// exec function. There are two issues with the standard QDialog when exec-ed.
///
/// a) if the dialog has no parent and there is a single application form open then
///    the dialog is centred over the form - which is good. However if two or more forms
///    are open, it appears in the centre of the screen - okay but not ideal; and
///
/// b) when scaling applied (even null scaling) this confuses the dialog and it
///    appears in the top left corner of the screen which is not acceptable.
///
/// The overaloded wrapper function allows a widget to be specified, and if specified
/// the dialog is centred over the widget.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT  QEDialog : public QDialog {
   Q_OBJECT
public:
   explicit QEDialog (QWidget* parent = 0);

public slots:
   // Re-postion dialog to the centre of the specified widget.
   // Note: this hides the overloaded virtual fuction.
   //
   virtual int exec (QWidget* targetWidget);

protected:
   // Specify the widget with the dialog that is to be centered over the target widget.
   //
   void setSourceWidget (QWidget* sourceWidget);

private:
   // Widget (within dialog) to use to colocate over targetWidget - default is this.
   //
   QWidget* sourceWidget;

   // Widget (defined by caller) to use to colocation point.
   //
   QWidget* targetWidget;

   // Uility function to ensure specified geometry on screen.
   //
   static QRect constrainGeometry (const QRect& geometry);

private slots:
   void relocateToCenteredPosition ();
};

#endif  // QEDIALOG_H
