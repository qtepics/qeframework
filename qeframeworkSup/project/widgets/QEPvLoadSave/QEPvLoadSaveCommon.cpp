/*  QEPvLoadSaveCommon.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2015-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEPvLoadSaveCommon.h"

//------------------------------------------------------------------------------
// static
void QEPvLoadSaveCommon::clear (StatusSummary& item)
{
   item.isEqualCount        = 0;
   item.isNotEqualCount     = 0;
   item.isNotAplicableCount = 0;
}

//------------------------------------------------------------------------------
// static
QEPvLoadSaveCommon::StatusSummary QEPvLoadSaveCommon::merge
   (const StatusSummary& a,
    const StatusSummary& b)
{
   StatusSummary result;
   result.isEqualCount        = a.isEqualCount        + b.isEqualCount;
   result.isNotEqualCount     = a.isNotEqualCount     + b.isNotEqualCount;
   result.isNotAplicableCount = a.isNotAplicableCount + b.isNotAplicableCount;
   return result;
}

//------------------------------------------------------------------------------
//
QEPvLoadSaveCommon::PvNameValueMaps QEPvLoadSaveCommon::merge
   (const PvNameValueMaps& a,
    const PvNameValueMaps& b)
{
   PvNameValueMaps result (a);

   QList<QString> bKeys = b.keys ();
   int n = bKeys.count();
   for (int j = 0; j < n; j++) {
      QString pvName = bKeys.value (j, "");
      if (!pvName.isEmpty () && !a.contains (pvName)) {
         double value = b.value (pvName);
         result.insert (pvName, value);
      }
   }

   return result;
}

// end
