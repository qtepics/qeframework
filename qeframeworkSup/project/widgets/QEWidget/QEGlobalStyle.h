/*  QEGlobalStyle.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2017-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_GLOBAL_STYLE_H
#define QE_GLOBAL_STYLE_H

#include <QEFrameworkLibraryGlobal.h>

/// This class sets the QApplication instancse style sheet, and hence is a
/// global style that is applicable to all widgets. As this functionality is
/// invoked from the QEWidget constructor, this style applies equally within
/// designer, qegui or any other display mangers using at least one QEWidget.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEGlobalStyle
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
