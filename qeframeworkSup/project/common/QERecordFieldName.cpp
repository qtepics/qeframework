/*  QERecordFieldName.cpp
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
 *  Copyright (c) 2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>
#include "QERecordFieldName.h"

#define DEBUG qDebug() << "QERecordFieldName::"  << __LINE__ << __FUNCTION__ << "  "


//------------------------------------------------------------------------------
//
QString QERecordFieldName::recordName (const QString & pvName)
{
   QString result;
   int dot_posn;

   result = pvName;
   dot_posn = result.indexOf (".", 0);
   if (dot_posn >= 0) {
      result.truncate (dot_posn);
   }
   return result;
}

//------------------------------------------------------------------------------
//
QString QERecordFieldName::fieldName (const QString & pvName)
{
   QString result = "VAL";
   int dot_posn;
   int fs;

   dot_posn = pvName.indexOf (".", 0);
   if (dot_posn >= 0) {
      fs = pvName.length() - dot_posn - 1;
      if (fs > 0) {
         result = pvName.right (fs);
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
QString QERecordFieldName::fieldPvName (const QString & pvName, const QString & field)
{
    return recordName (pvName) + "." + field;
}

//------------------------------------------------------------------------------
//
QString QERecordFieldName::rtypePvName (const QString & pvName)
{
   return recordName (pvName) + ".RTYP";
}

//------------------------------------------------------------------------------
//
bool QERecordFieldName::pvNameIsValid (const QString & pvName)
{
   QChar c;
   bool result;
   int j;
   int colonCount;
   int dotCount;

   if (pvName.length () == 0) {
      return false;
   }

   // Must start with a letter.
   //
   c = pvName [0];
   if ((!c.isUpper ()) && (!c.isLower())) {
      return false;
   }

   // Hypothosize all okay
   //
   result = true;
   colonCount = 0;
   dotCount = 0;
   for (j = 1; j < pvName.length (); j++) {
      c = pvName [j];

      if (c.isUpper() || c.isLower() || c.isDigit() ||
         (c == '_')   || (c == '-')) {
         // is good
      } else if (c == ':') {
         colonCount++;
      } else if ( c== '.' ) {
         dotCount++;
      } else {
         // Invalid character
         result = false;
         break;
      }
   }

   // Expected format is CCCC:SSSS[.FFFF]
   // However many beamline PVs do not strictly follow the naming
   // convension, so allow 1 or 2 colons.
   //
   if ((colonCount < 1) || (colonCount > 2) || (dotCount > 1)) {
      result = false;
   }

   return result;
}

//------------------------------------------------------------------------------
//
#define SUFFIX_NUM   13

static const char * suffixList [SUFFIX_NUM] = {
   " NPP", " PP", " CA", " CP", " CPP", " NMS", " MS",
   // truncated versions
   " N", " NP", " P", " C", " NM" , " M"
};

bool QERecordFieldName::extractPvName (const QString & item, QString & pvName)
{
   int i;
   int j;
   bool status;

   pvName = item.trimmed ();

   // Strip off standard suffix.
   //
   // Could be a little smarter, but as only a few suffix, this is okay.
   //
   for (i = 0; i < SUFFIX_NUM; i++) {
      for (j = 0; j < SUFFIX_NUM; j++) {

         if (pvName.endsWith (suffixList [j])) {
            int at = pvName.length () - (int)strlen (suffixList [j]);
            pvName.truncate (at);
         }
      }
   }

   // Validate what is left.
   //
   status = QERecordFieldName::pvNameIsValid (pvName);
   if (!status) {
      pvName.clear ();
   }
   return status;
}

// end
