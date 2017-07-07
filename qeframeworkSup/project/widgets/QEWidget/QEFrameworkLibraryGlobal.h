/*  QEFrameworkLibraryGlobal.h
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
 *  Copyright (c) 2009,2010,2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/* Description:
 *
 * Manage the declaration of library public functions as either exported functions
 * when building the library, or imported functions when using the library.
 *
 */

#ifndef QE_FRAMEWORK_LIBRARY_GLOBAL_H
#define QE_FRAMEWORK_LIBRARY_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QMetaType>

#if defined(QE_FRAMEWORK_LIBRARY)
#  define QE_FRAMEWORK_LIBRARY_SHARED_EXPORT Q_DECL_EXPORT
#else
#  define QE_FRAMEWORK_LIBRARY_SHARED_EXPORT Q_DECL_IMPORT
#endif


// With Qt5.6 on windows using mingw 32 bit, found that widget defined enum properties
// types must be declared as a meta type if the uic auto generated ui_xxx.h file is to
// be compiled successfully; but doing this breaks Qt4.8 n Linux.
//
// TODO: The rules need to be clarified and refined.
// Is this version related, OS related, and/or import/export related?
// Whatever, do logic here once, as opposed to each header file.
//
// Rules that appear to work are that meta data declarations are required
// when bulding against the QE linrary, but not when building the plugin itself.
//
// HOW DOES THIS APPLY TO A SPLT LIBRARY??
//
#if !defined(QE_FRAMEWORK_LIBRARY)
#  define QE_DECLARE_METATYPE_IS_REQUIRED
#else
#  undef  QE_DECLARE_METATYPE_IS_REQUIRED
#endif

// Example
// #ifdef QE_DECLARE_METATYPE_IS_REQUIRED
// Q_DECLARE_METATYPE (QSimpleShape::Shapes)
// Q_DECLARE_METATYPE (QSimpleShape::TextFormats)
// #endif

#endif // QE_FRAMEWORK_LIBRARY_GLOBAL_H
