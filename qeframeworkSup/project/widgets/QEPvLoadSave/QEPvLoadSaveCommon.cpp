/*  QEPvLoadSaveCommon.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2015-2019 Australian Synchrotron
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
