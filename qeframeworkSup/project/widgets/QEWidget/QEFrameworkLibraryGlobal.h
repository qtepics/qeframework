/*  QEFrameworkLibraryGlobal.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2009-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
// be compiled successfully; but doing this breaks Qt4.8 on Linux.
//
// TODO: The rules need to be clarified and refined.
// Is this version related, OS related, and/or import/export related?
// Whatever, do logic here once, as opposed to each header file.
//
// Rules that appear to work are that meta data declarations are required
// when bulding against the QE library, but not when building the plugin itself.
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
