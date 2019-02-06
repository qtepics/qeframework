/*  Generic.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2018 Australian Synchrotron
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
 *    Anthony Owen
 *  Contact details:
 *    anthony.owen@gmail.com
 */

#ifndef QE_GENERIC_H
#define QE_GENERIC_H

#include <QString>

// The QEFramework abstracts the data types away from the underying CA data
// types. This will be usefull for when PV Access data types are introduced.
//
namespace generic {

   // Abstracted data types.
   //
   enum generic_types { GENERIC_STRING,
                        GENERIC_SHORT,
                        GENERIC_UNSIGNED_SHORT,
                        GENERIC_UNSIGNED_CHAR,
                        GENERIC_LONG,
                        GENERIC_UNSIGNED_LONG,
                        GENERIC_FLOAT,
                        GENERIC_DOUBLE,
                        GENERIC_UNKNOWN };

   // Mainly used for debug/message info.
   //
   QString typeImage (const generic_types type);
}

#endif  // QE_GENERIC_H
