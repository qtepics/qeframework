/*  QEPvaCheck.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2018-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
