/*  QEPvNameSearch.h
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

#ifndef QE_PV_NAME_SEARCH_H
#define QE_PV_NAME_SEARCH_H

#include <QRegExp>
#include <QString>
#include <QStringList>

#include <QEFrameworkLibraryGlobal.h>

/// Provides a basic name search capability. Extracted from archiver manager
/// in ordger to provide a more flexibility, i.e. sets of PV names which can
/// be source from any source.
//
// QEPvNameSearch is essentially just a contrainer/wrapper around a QStringList
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPvNameSearch {
public:
   explicit QEPvNameSearch ();
   explicit QEPvNameSearch (const QEPvNameSearch& other);
   explicit QEPvNameSearch (const QStringList& pvNameList);
   virtual ~QEPvNameSearch ();

   // Clears/empties the QEPvNameSearch object.
   //
   void clear ();

   // Number of PV names held in the QEPvNameSearch object.
   //
   int count () const;

   // Note: for setPvNameList and addPvNameList the list is sorted
   // and duplicates are removed.
   //
   void setPvNameList (const QStringList& pvNameList);
   void addPvNameList (const QStringList& pvNameList);

   QStringList getAllPvNames () const;

   // The getMatchingPVnames functions allow the caller to extract a subset of
   // available PV names. The first uses a regular expression and allows for
   // sophisticated pattern matching. The second just returns a list of all the
   // PV names containing the substring str. The cs parameter determines whether
   // the string comparison is case sensitive or case insensitive.
   //
   QStringList getMatchingPvNames (const QRegExp& regExp, const bool exactMatch) const;
   QStringList getMatchingPvNames (const QString& str, const Qt::CaseSensitivity cs) const;

private:
   QStringList pvNameList;
};

#endif // QE_PV_NAME_SEARCH_H
