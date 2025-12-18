/*  QEExitButton.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2025-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_EXIT_BUTTON_H
#define QE_EXIT_BUTTON_H

#include <QPushButton>
#include <QSize>
#include <QString>
#include <QWidget>
#include <QEFrameworkLibraryGlobal.h>

/// This class provides an an exit button.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEExitButton :
      public QPushButton
{
   Q_OBJECT

   /// This property controls the behaviour of the widget.
   /// When exitProgram is false (default), it will cloese the current window.
   /// When exitProgram is true, it will close the entire application.
   ///
   Q_PROPERTY (bool exitProgram  READ getExitProgram  WRITE setExitProgram)

public:
   explicit QEExitButton (QWidget* parent = 0);
   virtual ~QEExitButton();

   QSize sizeHint () const;

   void setExitProgram (const bool exitProgram);
   bool getExitProgram () const;

private:
   bool mExitProgram;

private slots:
   void onClick (bool);
};

#endif // QE_EXIT_BUTTON_H
