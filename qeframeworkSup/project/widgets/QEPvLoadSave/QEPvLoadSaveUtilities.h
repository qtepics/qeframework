/*  QEPvLoadSaveUtilities.h
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
 *  Copyright (c) 2013 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_LOAD_SAVE_PV_UTILITIES_H
#define QE_LOAD_SAVE_PV_UTILITIES_H

#include <QVariant>
#include <macroSubstitution.h>

// Differed declarations - avoids mutual header inclusions.
//
class QEPvLoadSave;
class QEPvLoadSaveItem;
class QEPvLoadSaveModel;

//------------------------------------------------------------------------------
// All functions are static.
//
class QEPvLoadSaveUtilities {
public:

   // This function reads xml file.
   // Example file:
   //
   // <QEPvLoadSave version="1">
   //    <!-- No need to specify top level ROOT group in file -->
   //
   //    <Group Name="Colour Values">
   //       <PV Name="REDGUM:MONITOR" Value = "5" />
   //    </Group>
   //
   //    <Group Name="Joe Motors">
   //       <PV Name="JOE:MTR01.VAL" ReadPV="JOE:MTR01.RBV" Value = "2.51" />
   //       <PV Name="JOE:MTR02.VAL" ReadPV="JOE:MTR02.RBV" Value = "1.98" />
   //    </Group>
   //
   //    <Group Name="Colour Status">
   //       <!-- Empty group -->
   //    </Group>
   //
   //    <Group Name="Nested" >
   //       <Group Name="Inner" >
   //          <Array Name="PS-OCH-B-2-1:READ_RAMP_CMD"  Number="8" >
   //             <Element Index="0" Value="4.0"   />
   //             <Element Index="1" Value="5.6"   />
   //             <Element Index="2" Value="3.5"   />
   //             <Element Index="3" Value="11.5"  />
   //             <Element Index="4" Value="-12.1" />
   //             <Element Index="5" Value="55.8"  />
   //             <Element Index="6" Value="73.73" />
   //             <Element Index="7" Value="0.0"   />
   //          </Array>
   //       </Group>
   //    </Group>
   //
   //    <PV Name="FS01:BEAM_MODE" Value = "User Beam - Top Up"  />
   //
   // </QEPvLoadSave>
   //
   static QEPvLoadSaveItem* readTree (const QString& filename,
                                      const QString& macroString);

   // This function creates xml file.
   //
   static bool writeTree (const QString& filename, const QEPvLoadSaveItem* root);

private:
   // XML read write local utilitiy functions.
   //
   static QVariant convert (const QString& valueImage);

   static QEPvLoadSaveItem* readXmlScalerPv (const QDomElement pvElement,
                                             const macroSubstitutionList& macroList,
                                             QEPvLoadSaveGroup* parent);

   static QEPvLoadSaveItem* readXmlArrayPv (const QDomElement pvElement,
                                            const macroSubstitutionList& macroList,
                                            QEPvLoadSaveGroup* parent);

   static void readXmlGroup (const QDomElement groupElement,
                             const macroSubstitutionList& macroList,
                             QEPvLoadSaveGroup* parent,
                             const int level);


   static void writeXmlScalerPv (const QEPvLoadSaveItem* item,
                                 QDomElement& pvElement);

   static void writeXmlArrayPv (const QEPvLoadSaveItem* item,
                                QDomDocument& doc,
                                QDomElement& pvElement);

   static void writeXmlGroup (const QEPvLoadSaveItem* group,
                              QDomDocument& doc,
                              QDomElement& groupElement);
};

# endif  // QE_LOAD_SAVE_PV_UTILITIES_H
