/*  QEPvLoadSaveUtilities.h
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

#ifndef QE_LOAD_SAVE_PV_UTILITIES_H
#define QE_LOAD_SAVE_PV_UTILITIES_H

#include <qdom.h>
#include <QString>
#include <QVariant>
#include <macroSubstitution.h>

// Differed declarations - avoids mutual header inclusions.
//
class QEPvLoadSave;
class QEPvLoadSaveItem;
class QEPvLoadSaveModel;
class QEPvLoadSaveGroup;

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
                                      const QString& macroString,
                                      QString& errorMessage);

   // This function creates xml file.
   //
   static bool writeTree (const QString& filename,
                          const QEPvLoadSaveItem* root,
                          QString& errorMessage);

   // Merges three PV names into a single node name.
   // E.g  "FRED.VAL", "FRED.RBV", "FRED.RBV" => "FRED.{w:VAL;ra:RBV;}"
   //
   static QString mergePvNames (const QString& setPoint,
                                const QString& readBack,
                                const QString& archiver);

   // Splits a merged name into three separate names.
   //
  static bool splitPvNames (const QString& mergedName,
                            QString& setPoint,
                            QString& readBack,
                            QString& archiver);

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
