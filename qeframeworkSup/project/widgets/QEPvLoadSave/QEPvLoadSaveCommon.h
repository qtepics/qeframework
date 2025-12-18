/*  QEPvLoadSaveCommon.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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

   // Summary of number of items equal, not equal or undefined.
   // Passed up from the item leafs
   //
   struct StatusSummary {
      int isEqualCount;
      int isNotEqualCount;
      int isNotAplicableCount;
   };

   // Clears/initialises a StatusSummary structure.
   //
   static void clear (StatusSummary& item);

   // Merges, i.e essentially adds, two StatusSummary items.
   //
   static StatusSummary merge (const StatusSummary& a,
                               const StatusSummary& b);

   // Define a name value map type, used by the graphical comparison tool.
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
