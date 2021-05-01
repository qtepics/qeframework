/*  QEPvPropertiesUtilities.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2021 Australian Synchrotron
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

/* The QEPvPropertiesUtilities class provides utilities for the QEPvProperties widget.
 */

#ifndef QE_PV_PROPERTIES_UTILITIES_H
#define QE_PV_PROPERTIES_UTILITIES_H

#include <QList>
#include <QString>
#include <QStringList>


//------------------------------------------------------------------------------
// This class provides a named (by record type: ai, bo, calc etc) list of
// record field names.
//
class QERecordSpec {
public:
   explicit QERecordSpec (const QString& recordType);
   ~QERecordSpec ();

   void append (const QString item);

   int size () const;

   QString getRecordType () const;

   // If index is out of bounds, this function returns "".
   //
   QString getFieldName (const int index) const;

   // Indicates if long string mode is applicable.
   // If index is out of bounds, this function returns false.
   //
   bool fieldMayUseCharArray (const int index) const;

   // Indicates if the field is an INLINK, OUTLINK or FWDLINK field, or is
   // otherwise used to hold a PV name (e.g. sscan record PV name fields).
   // If index is out of bounds, this function returns false.
   //
   bool fieldIsLinkField (const int index) const;

   // Returns field description.
   // If index is out of bounds, this function returns "".
   //
   QString getDescription (const int index) const;

private:
   QString getFieldText (const int index) const;
   QStringList list;
   QString recordType;
};


//------------------------------------------------------------------------------
// This class provides a list of RecordSpec specs, with additional find functions.
//
class QERecordSpecList : public QList< QERecordSpec *> {
public:
   QERecordSpecList ();
   QERecordSpec* find (const QString& recordType) const;
   void appendOrReplace (QERecordSpec* recordSpec);

   bool processRecordSpecFile (const QString& filename);

private:
   int findSlot (const QString& recordType) const;
};

# endif  // QE_PV_PROPERTIES_UTILITIES_H
