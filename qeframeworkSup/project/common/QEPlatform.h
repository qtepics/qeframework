/*  QEPlatform.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2015,2017 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#ifndef QE_PLATFORM_H
#define QE_PLATFORM_H

#include <Qt>
#include <QtGlobal>
#include <QEFrameworkLibraryGlobal.h>

/// Geneal purpose platform specific functions used to hide compiler, environment
/// and version specific features. This locates all the #indef and the like tests
/// into one centralised and consistent location.
/// We use a class of static methods as opposed to a set of regular functions.
/// This provide a name space, but also allows inheritance if needs be.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPlatform {
public:
   /// This function test if the specified double floating point number is 'Not a Number'.
   static bool isNaN (const double x);

   /// This function test if the specified double floating point number is +/-Infinity.
   static bool isInf (const double x);

protected:
   QEPlatform () { }
};

/// Pick correct definition for middle button.
/// Note, at time of writing (building on qt4.6 through qt5.3) MidButton would
/// work for all, but it is due to be removed in the future.
///
#if QT_VERSION < 0x040700
#define MIDDLE_BUTTON Qt::MidButton
#else
#define MIDDLE_BUTTON Qt::MiddleButton
#endif

# endif // QE_PLATFORM_H
