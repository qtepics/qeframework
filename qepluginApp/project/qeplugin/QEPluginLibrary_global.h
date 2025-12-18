/*  QEPluginLibrary_global.h
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
 * when building the plugin library, or imported functions when using the library.
 *
 */

#ifndef QE_PLUGIN_LIBRARY_GLOBAL_H
#define QE_PLUGIN_LIBRARY_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QEPLUGIN_LIBRARY)
#  define QEPLUGINLIBRARYSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QEPLUGINLIBRARYSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QE_PLUGIN_LIBRARY_GLOBAL_H
