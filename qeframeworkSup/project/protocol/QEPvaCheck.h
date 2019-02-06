/*  QEPvaCheck.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (C) 2018 Australian Synchrotron
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
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_PVA_CHECK_H
#define QE_PVA_CHECK_H

#include <epicsVersion.h>

// This header file should be included in any file that conditionally includes
// pv/pvAccess.h,  pv/Data.h, etc. This will force a compilation warning if PVA
// support requested but not available in the version of EPICS base being used.
//
// The QE_PVACCESS_SUPPORT macro is set in framework.pro which itself is derived
// from the QE_PVACCESS_SUPPORT environment variable.
//
#ifdef QE_PVACCESS_SUPPORT

// TODO modifiy to allow EPICS 4 with 3.15/3.16
//
#if EPICS_VERSION >= 7
#define QE_INCLUDE_PV_ACCESS
#else
// We error as opposed to issuing a warning, as we will fail when cannot find
// pvData etc. libraries at link time anyways.
//
#error Including PV ACCESS support requires EPICS base 7 or later. If PV ACCESS support not required then unset QE_PVACCESS_SUPPORT environment variable.
#undef QE_INCLUDE_PV_ACCESS
#endif

#else

// Belts 'n' braces
#undef QE_INCLUDE_PV_ACCESS

#endif    // QE_PVACCESS_SUPPORT

#endif    // QE_PVA_CHECK_H
