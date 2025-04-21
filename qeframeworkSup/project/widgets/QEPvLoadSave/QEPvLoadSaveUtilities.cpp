/*  QEPvLoadSaveUtilities.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2025 Australian Synchrotron
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
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include "QEPvLoadSaveUtilities.h"

#include <stdlib.h>

#include <QDebug>
#include <QFile>
#include <QStringList>
#include <QVariant>
#include <QVariantList>

#include <QECommon.h>
#include <QEPlatform.h>
#include <QESettings.h>

#include "QEPvLoadSave.h"
#include "QEPvLoadSaveItem.h"
#include "QEPvLoadSaveModel.h"

#define DEBUG qDebug() << "QEPvLoadSaveUtilities" << __LINE__ << __FUNCTION__ << "  "

// Special none values.
//
static const QVariant nilValue = QVariant();

// XML tag/attribute names
//
static const QString fileTagName      = "QEPvLoadSave";
static const QString groupTagName     = "Group";
static const QString pvTagName        = "PV";       // scaler PV tag
static const QString arrayTagName     = "Array";
static const QString elementTagName   = "Element";

static const QString indexAttribute   = "Index";
static const QString nameAttribute    = "Name";
static const QString readBackNameAttribute = "ReadPV";
static const QString archiverNameAttribute = "ArchPV";
static const QString valueAttribute   = "Value";
static const QString versionAttribute = "Version";
static const QString numberAttribute  = "Number";

//------------------------------------------------------------------------------
//
QVariant QEPvLoadSaveUtilities::convert (const QString& valueImage)
{
   QVariant result = nilValue;

   int intValue;
   double doubleValue;
   bool okay;

   intValue = valueImage.toInt (&okay);
   if (okay) {
      // The image can be represented as an integer - use integer variant.
      //
      result = QVariant (intValue);
   } else {
      doubleValue = valueImage.toDouble (&okay);
      if (okay) {
         // The image can be represented as a double - use double variant.
         //
         result = QVariant (doubleValue);
      } else {
         // Default - store as is i.e. string.
         //
         result = QVariant (valueImage);
      }
   }

   return result;
}

//------------------------------------------------------------------------------
// A scaler PV could be defined as an array of one element, but this form
// provides a syntactical short cut for scaler values which are typically
// the most common in use.
//
QEPvLoadSaveItem* QEPvLoadSaveUtilities::readXmlScalerPv (const QDomElement pvElement,
                                                          const macroSubstitutionList& macroList,
                                                          QEPvLoadSaveGroup* parent)
{
   QEPvLoadSaveItem* result = NULL;

   QString setPointPvName = macroList.substitute (pvElement.attribute (nameAttribute, ""));
   QString readBackPvName = macroList.substitute (pvElement.attribute (readBackNameAttribute, ""));
   QString archiverPvName = macroList.substitute (pvElement.attribute (archiverNameAttribute, ""));

   QString valueImage = macroList.substitute (pvElement.attribute (valueAttribute, ""));

   if (setPointPvName.isEmpty() ) {
      qWarning () << __FUNCTION__ << " ignoring null PV name";
      return result;
   }

   QVariant value = QEPvLoadSaveUtilities::convert (valueImage);
   result = new QEPvLoadSaveLeaf (setPointPvName, readBackPvName, archiverPvName, value, parent);

   return result;
}


//------------------------------------------------------------------------------
//
QEPvLoadSaveItem* QEPvLoadSaveUtilities::readXmlArrayPv (const QDomElement pvElement,
                                                         const macroSubstitutionList& macroList,
                                                         QEPvLoadSaveGroup* parent)
{
   QEPvLoadSaveItem* result = NULL;
   QVariantList arrayValue;

   QString setPointPvName = macroList.substitute (pvElement.attribute (nameAttribute, ""));
   QString readBackPvName = macroList.substitute (pvElement.attribute (readBackNameAttribute, ""));
   QString archiverPvName = macroList.substitute (pvElement.attribute (archiverNameAttribute, ""));
   QString elementCountImage = pvElement.attribute (numberAttribute, "1");

   if (setPointPvName.isEmpty() ) {
      qWarning () << __FUNCTION__ << " ignoring null PV name";
      return result;
   }

   int elementCount = elementCountImage.toInt (NULL);

   // Initialise array with nil values.
   //
   for (int j = 0; j < elementCount; j++) {
      arrayValue << nilValue;
   }

   // Look for array values.
   //
   QDomElement itemElement = pvElement.firstChildElement (elementTagName);
   while (!itemElement.isNull ()) {
      bool okay;
      int index = itemElement.attribute (indexAttribute, "-1").toInt (&okay);
      if (okay && index >= 0 && index < elementCount) {
         QString valueImage = macroList.substitute (itemElement.attribute (valueAttribute, ""));
         QVariant value = QEPvLoadSaveUtilities::convert (valueImage);

         arrayValue.replace (index, value);

      } else {
         qWarning () << __FUNCTION__ << " ignoring unexpected index " << index;
      }
      itemElement = itemElement.nextSiblingElement (elementTagName);
   }

   result = new QEPvLoadSaveLeaf (setPointPvName, readBackPvName, archiverPvName,
                                  arrayValue, parent);
   return result;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveUtilities::readXmlGroup (const QDomElement groupElement,
                                          const macroSubstitutionList& macroList,
                                          QEPvLoadSaveGroup* parent,
                                          const int level)
{
   if (groupElement.isNull ()) {
      qWarning () << __FUNCTION__ << " null configElement, level => " << level;
      return;
   }

   // Parse XML using Qt's Document Object Model.
   // We look for Group and PV tags.
   //
   QDomElement itemElement = groupElement.firstChildElement ("");
   while (!itemElement.isNull ())   {

      QString tagName = itemElement.tagName ();

      if (tagName == groupTagName) {
         QString groupName = macroList.substitute (itemElement.attribute (nameAttribute));
         QEPvLoadSaveGroup* group = new QEPvLoadSaveGroup (groupName, parent);
         QEPvLoadSaveUtilities::readXmlGroup (itemElement, macroList, group, level + 1);

      } else if  (tagName == pvTagName) {
         QEPvLoadSaveUtilities::readXmlScalerPv (itemElement, macroList, parent);

      } else if  (tagName == arrayTagName) {
         QEPvLoadSaveUtilities::readXmlArrayPv (itemElement, macroList, parent);

      } else {
         qWarning () << __FUNCTION__ << " ignoring unexpected tag " << tagName;

      }

      itemElement = itemElement.nextSiblingElement ("");
   }
}

//------------------------------------------------------------------------------
//
QEPvLoadSaveItem* QEPvLoadSaveUtilities::readTree (const QString& filename,
                                                   const QString& macroString,
                                                   QString& errorMessage)
{
   macroSubstitutionList macroList (macroString);
   errorMessage = "n/a";

   if (filename.isEmpty()) {
      errorMessage = "null file filename";
      return NULL;
   }

   QFile file (filename);
   if (!file.open (QIODevice::ReadOnly)) {
      errorMessage = QString("file %1 open (read) failed").arg (filename);
      return NULL;
   }

   QDomDocument doc;
   QString errorText;
   int errorLine;
   int errorCol;

   const bool loadStatus = doc.setContent (&file, &errorText, &errorLine, &errorCol);
   if (!loadStatus) {
      errorMessage = QString ("%1:%2:%3 set content failed: %4")
                             .arg (filename)
                             .arg (errorLine)
                             .arg (errorCol)
                             .arg (errorText);
      file.close ();
      return NULL;
   }

   QDomElement docElem = doc.documentElement ();

   // The file has been read - we can now close it.
   //
   file.close ();

   // Examine top level tag name - is this the tag we expect.
   //
   if (docElem.tagName () != fileTagName) {
      errorMessage = QString ("file %1 unexpected tag <%2>")
                             .arg (filename)
                             .arg (docElem.tagName ());
      return NULL;
   }

   QString versionImage = docElem.attribute (versionAttribute).trimmed ();
   bool versionOkay;
   int version = versionImage.toInt (&versionOkay);

   if (!versionImage.isEmpty()) {
      // A version has been specified - we must ensure it is sensible.
      //
      if (!versionOkay) {
         errorMessage = QString ("file %1 invalid version %2 (integer expected)")
                                .arg (filename)
                                .arg (versionImage);
         return NULL;
      }

   } else {
      // no version - go with current version.
      //
      version = 1;
   }

   if (version != 1) {
      errorMessage = QString ("file %1 unexpected version specified %2 (out of range)")
                             .arg (filename)
                             .arg (versionImage);
      return NULL;
   }

   // Create the root item.
   //
   QEPvLoadSaveGroup* result = new QEPvLoadSaveGroup ("ROOT", NULL);

   // Parse XML using Qt's Document Object Model.
   //
   QEPvLoadSaveUtilities::readXmlGroup (docElem, macroList, result, 1);

   return result;
}

//------------------------------------------------------------------------------
// QEPvLoadSaveLeaf
void QEPvLoadSaveUtilities::writeXmlScalerPv (const QEPvLoadSaveItem* itemIn,
                                              QDomElement& pvElement)
{
   // Dynamic caste should always work - consider static caste.
   //
   const QEPvLoadSaveLeaf* item = dynamic_cast <const QEPvLoadSaveLeaf*> (itemIn);

   if (!item) return;  // Sainity check.

   const QString basePvName = item->getSetPointPvName ();
   pvElement.setAttribute (nameAttribute, basePvName);

   const QString readBackPvName = item->getReadBackPvName ();
   if (!readBackPvName.isEmpty() && (readBackPvName != basePvName)) {
      pvElement.setAttribute (readBackNameAttribute, readBackPvName);
   }

   const QString archiverPvName = item->getArchiverPvName ();
   if (!archiverPvName.isEmpty() && (archiverPvName != basePvName)) {
      pvElement.setAttribute (archiverNameAttribute, archiverPvName);
   }

   const QVariant value = item->getNodeValue ();
   const QMetaType::Type mtype = QEPlatform::metaType (value);
   if (mtype == QMetaType::UChar || mtype == QMetaType::Char) {
      // Treat as unit 8 as opposed to a character.
      const int ival = value.toInt();
      pvElement.setAttribute (valueAttribute, QString::number(ival));
   } else {
      pvElement.setAttribute (valueAttribute, value.toString ());
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveUtilities::writeXmlArrayPv (const QEPvLoadSaveItem* itemIn,
                                             QDomDocument& doc,
                                             QDomElement& arrayElement)
{
   const QEPvLoadSaveLeaf* item = dynamic_cast <const QEPvLoadSaveLeaf*> (itemIn);

   if (!item) return;  // Sainity check.

   // Note: this converts any vector variants to a QVariantList.
   //
   const QVariantList valueList = item->getNodeValue ().toList ();
   const int n = valueList.size ();

   const QString basePvName = item->getSetPointPvName ();
   arrayElement.setAttribute (nameAttribute, basePvName);

   const QString readBackPvName = item->getReadBackPvName ();
   if (!readBackPvName.isEmpty() && (readBackPvName != basePvName)) {
      arrayElement.setAttribute (readBackNameAttribute, readBackPvName);
   }

   const QString archiverPvName = item->getArchiverPvName ();
   if (!archiverPvName.isEmpty() && (archiverPvName != basePvName)) {
      arrayElement.setAttribute (archiverNameAttribute, archiverPvName);
   }

   arrayElement.setAttribute (numberAttribute, QString ("%1").arg (n));

   for (int j = 0; j < n; j++) {
      QDomElement itemElement = doc.createElement (elementTagName);
      arrayElement.appendChild (itemElement);
      itemElement.setAttribute (indexAttribute, QString ("%1").arg (j));

      const QVariant value = valueList.value (j);
      const QMetaType::Type mtype = QEPlatform::metaType (value);
      if (mtype == QMetaType::UChar || mtype == QMetaType::Char) {
         // Treat as unit 8 as opposed to a character.
         const int ival = value.toInt();
         itemElement.setAttribute (valueAttribute, QString::number(ival));
      } else {
         itemElement.setAttribute (valueAttribute, value.toString ());
      }
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveUtilities::writeXmlGroup (const QEPvLoadSaveItem* groupIn,
                                           QDomDocument& doc,
                                           QDomElement& groupElement)
{
   int n;
   int j;

   const QEPvLoadSaveGroup* group = dynamic_cast <const QEPvLoadSaveGroup*> (groupIn);

   if (!group) return;  // Sainity check.

   n = group->childCount ();
   for (j = 0; j < n; j++) {
      QEPvLoadSaveItem* child = group->getChild (j);
      QDomElement childElement;

      if (child->getIsGroup ()) {
         // This is a group node.
         //
         childElement = doc.createElement (groupTagName);
         groupElement.appendChild (childElement);
         childElement.setAttribute (nameAttribute, child->getNodeName());
         QEPvLoadSaveUtilities::writeXmlGroup (child, doc, childElement);

      } else {
         // This is a PV node. Scaler or Array?
         //
         if (child->getElementCount () > 1) {
            childElement = doc.createElement (arrayTagName);
            groupElement.appendChild (childElement);
            QEPvLoadSaveUtilities::writeXmlArrayPv (child, doc, childElement);

         } else {
            childElement = doc.createElement (pvTagName);
            groupElement.appendChild (childElement);
            QEPvLoadSaveUtilities::writeXmlScalerPv (child, childElement);
         }
      }
   }
}

//------------------------------------------------------------------------------
//
bool QEPvLoadSaveUtilities::writeTree (const QString& filename, const QEPvLoadSaveItem* root)
{
   if (filename.isEmpty () || !root) {
      qWarning () << __FUNCTION__ << "null filename and/or root node specified";
      return false;
   }

   QDomDocument doc;
   QDomElement docElem;

   doc.clear ();
   docElem = doc.createElement (fileTagName);
   docElem.setAttribute (versionAttribute, 1);

   // Add the root to the document
   //
   doc.appendChild (docElem);

   QFile file (filename);
   if (!file.open (QIODevice::WriteOnly)) {
      qDebug() << "Could not save configuration " << filename;
      return false;
   }

   QEPvLoadSaveUtilities::writeXmlGroup (root, doc, docElem);

   QTextStream ts (&file);
   ts << doc.toString (2);  // setting the indent to 2 is purely cosmetic
   file.close ();

   return true;
}

//------------------------------------------------------------------------------
// The format of a merged name, as displayed to the user is:
//
// common_prefix{r:read;w:write;a:arch}common_suffix
//
// Example:  SR15SLT02:UPPER_BLADE.{wa:VAL;r:RBV}
//
// NOTE: Change these, if needs be, to suit your PV name environment.
// We could make these adaptation parameters.
//
static const char startOptions    = '{';
static const char optionStart     = ':';
static const char optionSeparator = ';';
static const char endOptions      = '}';


//------------------------------------------------------------------------------
//
QString QEPvLoadSaveUtilities::mergePvNames (const QString& setPoint,
                                             const QString& readBackIn,
                                             const QString& archiverIn)
{
   // If readback or archiver name undefined then use set point PV name.
   //
   const QString readBack = readBackIn.isEmpty () ? setPoint : readBackIn;
   const QString archiver = archiverIn.isEmpty () ? setPoint : archiverIn;

   QString result = "";

   if ((setPoint == readBack) && (readBack == archiver)) {
      // All three names are the same - just use as is.
      //
      result = setPoint;

   } else {
      int n = MIN (MIN (setPoint.length (), readBack.length ()), 
                   archiver.length ());

      // Find the common, i.e. shared, prefix part of the three PV names.
      //
      int common = 0;
      for (int j = 1; j <= n; j++) {
         if (setPoint.left (j) != readBack.left (j) ||
             setPoint.left (j) != archiver.left (j)) break;
         common = j;
      }

      result = setPoint.left (common);

      // Extract w, r and a, the PV name specific suffixes.
      // Note: setPointPvName == result + w etc.
      //
      QString label  [3] = { "w", "r", "a" };
      QString suffix [3];

      suffix [0] = setPoint.right (setPoint.length() - common);
      suffix [1] = readBack.right (readBack.length() - common);
      suffix [2] = archiver.right (archiver.length() - common);

      // Check for two suffix being equal.
      //
      for (int i = 0; i < 2; i++) {
         for (int j = i + 1; j < 3; j++) {
            if (suffix [i] == suffix [j]) {
               // merge
               //
               label [i].append (label [j]);
               label [j] = "";
               suffix [j] = "";
            }
         }
      }

      result.append (startOptions);
      for (int i = 0; i < 3; i++) {
         if (!suffix [i].isEmpty ()) {
            result.append (label [i])
                  .append (optionStart)
                  .append (suffix [i])
                  .append (optionSeparator);
         }
      }

      result.append (endOptions);
   }

   return result;
}

//------------------------------------------------------------------------------
//
bool QEPvLoadSaveUtilities::splitPvNames (const QString& mergedName,
                                          QString& setPoint,
                                          QString& readBack,
                                          QString& archiver)
{
    bool result = false;

    // common_prefix{r:read;w:write;a:arch}common_suffix

    const int startOptionsPosn = mergedName.indexOf (startOptions);
    const int endOptionsPosn = mergedName.indexOf (endOptions);

    if ((startOptionsPosn == -1) && (endOptionsPosn == -1)) {
       // No options at all - easy.
       //
       setPoint = mergedName;
       readBack = mergedName;
       archiver= mergedName;
       result = true;

    } else if (((startOptionsPosn >= 0)  && (endOptionsPosn < startOptionsPosn)) ||
               ((startOptionsPosn == -1) && (endOptionsPosn >= 0))) {
       // Miss matched start brace and end braces.
       //
       result = false;

    } else {
       // We use a state machine to analyse the mergedName string.
       //
       enum States {
          sPrefix,
          sModes,
          sOption,
          sSuffix,
          sError
       };

       States state;
       bool w, r, a;   // defines which modes apply

       result = true;  // hypothesize all okay.
       setPoint = "";
       readBack = "";
       archiver = "";
       state = sPrefix;
       w = r = a = false;
       for (int j = 0 ; j < mergedName.size(); j++) {
          const QChar c = mergedName.at (j);

          switch (state) {
             case sPrefix:
                if (c == startOptions) {
                   w = r = a = false;
                   state = sModes;
                } else {
                   setPoint.append (c);
                   readBack.append (c);
                   archiver.append (c);
                }
                break;

             case sModes:
                if (c == ' ') {
                   // pass - allow and skip spaces
                } else if (c == 'w') {
                   w = true;
                } else if (c == 'r') {
                   r = true;
                } else if (c == 'a') {
                   a = true;
                } else if (c == optionStart) {
                   state = sOption;
                } else if (c == endOptions) {
                   if (w || r || a) {
                      // We have ...{ ...; x }   -- x = r,w or a
                      result = false;
                      state = sError;
                   }
                   state = sSuffix;
                } else {
                   // Unexpected char
                   result = false;
                   state = sError;
                }
                break;

             case sOption:
                if (c == ' ') {
                   // pass - skip spaces
                } else if (c == optionSeparator) {
                   w = r = a = false;
                   state = sModes;
                } else if (c == startOptions) {
                   result = false;
                   state = sError;
                } else if (c == endOptions) {
                   state = sSuffix;
                } else {
                   if (w) setPoint.append (c);
                   if (r) readBack.append (c);
                   if (a) archiver.append (c);
                }
                break;

             case sSuffix:
                if ((c == startOptions) || (c == endOptions)) {
                   result = false;
                   state = sError;
                } else {
                   setPoint.append (c);
                   readBack.append (c);
                   archiver.append (c);
                }
                break;

             case sError:
                result = false;
                break;
          }
       }
    }

    return result;
}

// end
