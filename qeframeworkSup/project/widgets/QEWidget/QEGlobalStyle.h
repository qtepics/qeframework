/*  QEGlobalStyle.h
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
 *  Copyright (c) 2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_GLOBAL_STYLE_H
#define QE_GLOBAL_STYLE_H

#include <QEPluginLibrary_global.h>

/// This class sets the QApplication instancse style sheet, and hence is a
/// global style that is applicable to all widgets. As this functionality is
/// invoked from the QEWidget constructor, this style applies equally within
/// designer, qegui or any other display mangers using at least one QEWidget.
///
class QEPLUGINLIBRARYSHARED_EXPORT QEGlobalStyle
{
public:
   /// Constructor
   ///
   QEGlobalStyle ();

   /// Destructor
   ///
   ~QEGlobalStyle ();

   /// Checks for the existance of the QE_GLOBAL_STYLE_SHEET environment variable
   /// and if defined appends the specified style to the application style sheet.
   /// If the (trimmed) value of the environment variable starts with "file://"
   /// then the rest of the environment variable will be interpreted as file
   /// name path.
   ///
   void apply ();

private:
   static bool styleAlreadyApplied;   /// Ensures style appended to the global style once only.
};

#endif // QE_GLOBAL_STYLE_H
