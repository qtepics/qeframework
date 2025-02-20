/*  QEExitButton.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2025 Australian Synchrotron
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
 *    andrews@ansto.gov.au
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
