/*  QEPluginLibrary_global.h
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
