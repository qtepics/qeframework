/*  QEDialog.h
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

#ifndef QEDIALOG_H
#define QEDIALOG_H

#include <QDialog>
#include <QRect>

#include <QEPluginLibrary_global.h>

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
class QEPLUGINLIBRARYSHARED_EXPORT  QEDialog : public QDialog {
   Q_OBJECT
public:
   explicit QEDialog (QWidget* parent = 0);

public slots:
   // Re-postion dialog to the centre of the specified widget.
   //
   int exec (QWidget* targetWidget);

protected:
   // Spcified the widget on the dialog to be centered over the target widget.
   //
   void setSourceWidget (QWidget* widget);

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
