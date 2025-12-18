/*  QEPvNameSearch.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
      // Double '^^" and/or '$$' are okay and we don't need to check for that.
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
