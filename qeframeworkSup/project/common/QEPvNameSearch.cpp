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
QStringList QEPvNameSearch::getMatchingPvNames (const QRegularExpression& reIn,
                                                const bool exactMatch) const
{
   QStringList result;

   QRegularExpression re = reIn;
   if (exactMatch) {
      // Bracket the original expression with '^' and '$', and form a modified expression.
      // Double '^^" and/or '$$' are okay and we don;t need to check for that.
      //
      QString pattern = QString ("^") + reIn.pattern() + QString ("$");
      re.setPattern (pattern);
   }
   result = this->pvNameList.filter (re);
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
