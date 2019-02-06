/*  Generic.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
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
 *  Copyright (c) 2009,2010,2017 Australian Synchrotron
 *
 *  Author:
 *    Anthony Owen
 *  Contact details:
 *    anthony.owen@gmail.com
 */

#include "Generic.h"

QString generic::typeImage (const generic_types type)
{
   QString result;

   switch (type) {
      case GENERIC_STRING:
         result = "GENERIC_STRING";
         break;
      case GENERIC_SHORT:
         result = "GENERIC_SHORT";
         break;
      case GENERIC_UNSIGNED_SHORT:
         result = "GENERIC_UNSIGNED_SHORT";
         break;
      case GENERIC_UNSIGNED_CHAR:
         result = "GENERIC_UNSIGNED_CHAR";
         break;
      case GENERIC_LONG:
         result = "GENERIC_LONG";
         break;
      case GENERIC_UNSIGNED_LONG:
         result = "GENERIC_UNSIGNED_LONG";
         break;
      case GENERIC_FLOAT:
         result = "GENERIC_FLOAT";
         break;
      case GENERIC_DOUBLE:
         result = "GENERIC_DOUBLE";
         break;
      case GENERIC_UNKNOWN:
         result = "GENERIC_UNKNOWN";
         break;
      default:
         result = QString ("GENERIC_UNKNOWN_TYPE_%1").arg (int (type));
         break;
   }

   return result;
}

// end
