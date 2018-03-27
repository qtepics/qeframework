/*  QEPvLoadSaveCommon.h
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
 *  Copyright (c) 2013,2018 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_PV_LOAD_SAVE_COMMON_H
#define QE_PV_LOAD_SAVE_COMMON_H

#include <QMap>
#include <QString>
#include <QObject>
#include <QEFrameworkLibraryGlobal.h>

// Define PV Load/Save specific names (enumerations)
// We use a QObject as opposed to a namespace as this allows us leverage off the
// meta object compiler output, specifically allows us to use the enumToString
// and stringToEnum functions in QEUtilities.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPvLoadSaveCommon : public QObject {
   Q_OBJECT
public:
   // Defines the completed action kinds.
   //
   enum ActionKinds {
      NullAction = 0,    //
      Apply,             //
      Extract,           //
      ReadArchive,       //
      Update             //
   };

   // Defines the columns used in the model tree.
   //
   enum ColumnKinds {
      NodeName = 0,      // group or PV name
      LoadSave,          // snap shot value
      Live,              // current value
      Delta,             // = live - snap shot value where calculateable
      NUMBER_OF_COLUMNS  // must be last
   };

   // Define a name value map type, used by the graphical comparisioon tool.
   //
   typedef QMap<QString, double> PvNameValueMaps;

   // Merge two PV name value maps. Specifically merge b into a and return
   // result. If the PV name is in both maps, then the value from the first
   // map, i.e. a, takes priority.
   //
   static PvNameValueMaps merge (const PvNameValueMaps& a,
                                 const PvNameValueMaps& b);
};

#endif  // QE_PV_LOAD_SAVE_COMMON_H
