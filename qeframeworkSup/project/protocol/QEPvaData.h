/*  QEPvaData.h
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

#ifndef QE_PVA_DATA_H
#define QE_PVA_DATA_H

// Header to select the appropriate header
//
#ifdef QE_INCLUDE_PV_ACCESS
  #ifdef QE_USING_PVXS
    #include "QEPvaData_PVXS.h"
  #else
    #include "QEPvaData_PVA.h"
  #endif
#else
  #include "QEPvaData_STUB.h"
#endif

#endif // QE_PVA_DATA_H
