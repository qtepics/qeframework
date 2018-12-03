/*  macroSubstitution.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2018 Australian Synchrotron
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
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QE_MACRO_SUBSTITUTION_H
#define QE_MACRO_SUBSTITUTION_H

#include <QString>
#include <QList>
#include <QEFrameworkLibraryGlobal.h>


/**
 * Description:
 *
 * A class to manage macro substitutions.
 * Macro substitutions are often provioded as a string of keys and values.
 *
 * This class parses such strings, and manages macro substitutions using
 * a list of keys and values.
 *
 */

// Macro substitution key/value pair
// Added initially as a class so extra meta information could be attached to
// each substitution (such as its source)
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT macroSubstitution
{
public:
   explicit macroSubstitution();
   explicit macroSubstitution (const QString& key,
                               const QString& value);
   ~macroSubstitution();

   QString key;
   QString value;
};

// Set of macro substitutions
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT macroSubstitutionList
{
public:
   explicit macroSubstitutionList();                        // Constructor - empty list
   explicit macroSubstitutionList (const QString& string);  // Constructor - parse string
   ~macroSubstitutionList();                                // Destructor

   void addMacroSubstitutions (const QString& string);      // Add substitutions - from a string

   QString substitute (const QString& string) const;        // Perform macro substitutions on a string
   QString getString() const;                               // Return the substitution as a clean comma delimited string

   int getCount() const;                                    // Return the number of substitutions
   const QString getKey (const unsigned int i) const;       // Return a key (given an index)
   const QString getValue (const unsigned int i) const;     // Return a value (given an index)
   const QString getValue (const QString& key) const;       // Return a value (given a key)

private:
   QList<macroSubstitution> parts;                          // List of key/value pairs

   void static substituteKey (QString& string,
                              QString key,
                              const QString value);         // Substitution a single key/value
   void append (const QString& key,
                const QString& value);                      // Add a key/value pair to the list
};

#endif // QE_MACRO_SUBSTITUTION_H
