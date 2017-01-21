/*  QEPvPropertiesUtilities.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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
 *  Copyright (c) 2012,2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <stdlib.h>

#include <QDebug>
#include <QFile>
#include <QTextStream>

#include "QEPvPropertiesUtilities.h"

#define DEBUG qDebug() << "QEPvPropertiesUtilities::" << __FUNCTION__ << ":" << __LINE__


//==============================================================================
//
QERecordSpec::QERecordSpec (const QString recordTypeIn)
{
   this->recordType = recordTypeIn;
   this->clear ();
}

//------------------------------------------------------------------------------
//
QString QERecordSpec::getRecordType () const
{
   return this->recordType;
}

//------------------------------------------------------------------------------
//
QString QERecordSpec::getFieldName (const int index) const
{
   if ((0 <= index) && (index < size ())) {
      return this->at (index);
   } else {
      return "";
   }
}


//==============================================================================
//
QERecordSpecList::QERecordSpecList ()
{
    // place holder:
}


//------------------------------------------------------------------------------
//
int QERecordSpecList::findSlot (const QString recordType) const
{
   int result = -1;
   QERecordSpec * checkSpec;

   for (int j = 0; j < this->size (); j++) {
      checkSpec = this->at (j);
      if (checkSpec->getRecordType () == recordType) {
         // Found it ;-)
         //
         result = j;
         break;
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
QERecordSpec * QERecordSpecList::find (const QString recordType) const
{
   QERecordSpec *result = NULL;
   int slot;

   slot = this->findSlot (recordType);
   if (slot >= 0) {
      result = this->at (slot);
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QERecordSpecList::appendOrReplace (QERecordSpec *newRecordSpec)
{
   int slot;

   if (newRecordSpec) {
      // Do we already have a record spec for this record type.
      //
      slot = this->findSlot (newRecordSpec->getRecordType());
      if (slot >= 0) {
         // Yes ...
         QERecordSpec *previousRecordSpec;

         previousRecordSpec = this->at (slot);
         this->replace (slot, newRecordSpec);
         delete previousRecordSpec;

      } else {
         // No - just append it.
         //
         this->append (newRecordSpec);
      }
   }
}

//------------------------------------------------------------------------------
//
bool QERecordSpecList::processRecordSpecFile (const QString& filename)
{
   if (filename == "") {
      return false;
   }

   QFile record_field_file (filename);

   if (!record_field_file.open (QIODevice::ReadOnly | QIODevice::Text)) {
      return false;
   }

   QTextStream source (&record_field_file);
   QERecordSpec *recordSpec;
   QString recordType;
   unsigned int lineno;

   recordSpec = NULL;
   lineno = 0;
   while (!source.atEnd()) {
      QString line = source.readLine ().trimmed ();

      lineno++;

      // Skip empty line and comment lines.
      //
      if (line.length () == 0) continue;
      if (line.left (1) == "#") continue;

      // record types designated by: <<name>>
      //
      if ((line.left (2) == "<<") && (line.right (2) == ">>")) {

         recordType = line.mid (2, line.length() - 4).trimmed ();
         recordSpec = new QERecordSpec (recordType);
         this->appendOrReplace (recordSpec);

      } else {
         // Just a regular field.
         //
         if (recordSpec) {
            recordSpec->append (line);
         } else {
            DEBUG << "field occured before first record type: " << filename << lineno << line;
         }
      }
   }

   record_field_file.close ();
   return true;
}

// end
