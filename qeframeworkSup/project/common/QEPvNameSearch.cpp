/*  QEPvNameSearch.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2021 Australian Synchrotron.
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

#include <QEPvNameSearch.h>

//------------------------------------------------------------------------------
//
QEPvNameSearch::QEPvNameSearch ()
{
   this->pvNameList.clear ();
}

//------------------------------------------------------------------------------
//
QEPvNameSearch::QEPvNameSearch (const QEPvNameSearch& other)
{
   this->setPvNameList (other.pvNameList);
}

//------------------------------------------------------------------------------
//
QEPvNameSearch::QEPvNameSearch (const QStringList& pvNameListIn)
{
   this->setPvNameList (pvNameListIn);
}

//------------------------------------------------------------------------------
//
QEPvNameSearch::~QEPvNameSearch ()
{
   this->pvNameList.clear ();
}

//------------------------------------------------------------------------------
//
void QEPvNameSearch::clear ()
{
   return this->pvNameList.clear();
}

//------------------------------------------------------------------------------
//
int QEPvNameSearch::count () const
{
   return this->pvNameList.count();
}

//------------------------------------------------------------------------------
//
void QEPvNameSearch::setPvNameList (const QStringList& pvNameListIn)
{
   this->pvNameList = pvNameListIn;

   // Ensure sorted/unique
   //
   this->pvNameList.sort ();
   this->pvNameList.removeDuplicates ();
}

//------------------------------------------------------------------------------
//
void QEPvNameSearch::addPvNameList (const QStringList& pvNameListIn)
{
   this->pvNameList.append (pvNameListIn);
   this->pvNameList.sort ();
   this->pvNameList.removeDuplicates ();
}

//------------------------------------------------------------------------------
//
QStringList QEPvNameSearch::getAllPvNames () const
{
   return this->pvNameList;
}

//------------------------------------------------------------------------------
//
QStringList QEPvNameSearch::getMatchingPvNames (const QRegExp& regExp,
                                                const bool exactMatch) const
{
   QStringList result;
   int n;
   int j;

   if (exactMatch) {
      // QStringList::filter (regExp) does not do an exact match, so we must
      // do own element-by-element filtering.
      //
      n = this->pvNameList.count ();
      for (j = 0; j < n; j++) {
         QString pvName = this->pvNameList.value (j);

         if (regExp.exactMatch (pvName)) {
            result.append (pvName);
         }
      }
   } else {
      // No exact match required - can use in built function.
      //
      result = this->pvNameList.filter (regExp);
   }
   return result;
}

//------------------------------------------------------------------------------
//
QStringList QEPvNameSearch::getMatchingPvNames (const QString& str,
                                                const Qt::CaseSensitivity cs) const
{
   return this->pvNameList.filter (str, cs);
}

// end
