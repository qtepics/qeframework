/*  QEPvLoadSaveUtilities.cpp
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
 *  Copyright (c) 2013 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <stdlib.h>

#include <QDebug>
#include <qdom.h>
#include <QFile>
#include <QESettings.h>
#include <QStringList>
#include <QVariant>
#include <QVariantList>

#include "QEPvLoadSave.h"
#include "QEPvLoadSaveItem.h"
#include "QEPvLoadSaveModel.h"

#include "QEPvLoadSaveUtilities.h"

#define DEBUG qDebug() << "QEPvLoadSaveUtilities::" << __FUNCTION__ << ":" << __LINE__

// Special none values.
//
static const QVariant nilValue (QVariant::Invalid);

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

   int iv;
   double dv;
   bool okay;

   iv = valueImage.toInt (&okay);
   if (okay) {
      // The image can be represented as an integer - use integer variant.
      //
      result = QVariant (iv);
   } else {
      dv = valueImage.toDouble (&okay);
      if (okay) {
         // The image can be represented as a double - use double variant.
         //
         result = QVariant (dv);
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
   QVariant value (QVariant::Invalid);

   QString setPointPvName = macroList.substitute (pvElement.attribute (nameAttribute, ""));
   QString readBackPvName = macroList.substitute (pvElement.attribute (readBackNameAttribute, ""));
   QString archiverPvName = macroList.substitute (pvElement.attribute (archiverNameAttribute, ""));

   QString valueImage = macroList.substitute (pvElement.attribute (valueAttribute, ""));

   if (setPointPvName.isEmpty() ) {
      qWarning () << __FUNCTION__ << " ignoring null PV name";
      return result;
   }

   value = QEPvLoadSaveUtilities::convert (valueImage);
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

   QString pvName = macroList.substitute (pvElement.attribute (nameAttribute));
   QString elementCountImage = pvElement.attribute (numberAttribute, "1");

   if (pvName.isEmpty() ) {
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

   result = new QEPvLoadSaveLeaf (pvName, "", "", arrayValue, parent);
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
                                                   const QString& macroString)
{
   QEPvLoadSaveGroup* result = NULL;
   macroSubstitutionList macroList (macroString);

   if (filename.isEmpty()) {
      qWarning () << __FUNCTION__ << " null file filename";
      return result;
   }

   QFile file (filename);
   if (!file.open (QIODevice::ReadOnly)) {
      qWarning () << __FUNCTION__ << filename  << " file open (read) failed";
      return result;
   }

   QDomDocument doc;
   QString errorText;
   int errorLine;
   int errorCol;

   if (!doc.setContent (&file, &errorText, &errorLine, &errorCol)) {
      qWarning () << QString ("%1:%2:%3").arg (filename).arg (errorLine).arg (errorCol)
                  << " set content failed " << errorText;
      file.close ();
      return result;
   }

   QDomElement docElem = doc.documentElement ();

   // The file has been read - we can now close it.
   //
   file.close ();

   // Examine top level tag name - is this the tag we expect.
   //
   if (docElem.tagName () != fileTagName) {
      qWarning () << filename  << " unexpected tag name " << docElem.tagName ();
      return result;
   }

   QString versionImage = docElem.attribute (versionAttribute).trimmed ();
   bool versionOkay;
   int version = versionImage.toInt (&versionOkay);

   if (!versionImage.isEmpty()) {
      // A version has been specified - we must ensure it is sensible.
      //
      if (!versionOkay) {
         qWarning () << filename  << " invalid version string " << versionImage << " (integer expected)";
         return result;
      }

   } else {
      // no version - go with current version.
      //
      version = 1;
   }

   if (version != 1) {
      qWarning () << filename  << " unexpected version specified " << versionImage << " (out of range)";
      return result;
   }

   // Create the root item.
   //
   result = new QEPvLoadSaveGroup ("ROOT", NULL);

   // Parse XML using Qt's Document Object Model.
   //
   QEPvLoadSaveUtilities::readXmlGroup (docElem, macroList, result, 1);

   return result;
}


//------------------------------------------------------------------------------
//QEPvLoadSaveLeaf
void QEPvLoadSaveUtilities::writeXmlScalerPv (const QEPvLoadSaveItem* itemIn,
                                              QDomElement& pvElement)
{
   // Dynamic caste should always work - consider static caste.
   //
   const QEPvLoadSaveLeaf* item = dynamic_cast <const QEPvLoadSaveLeaf*> (itemIn);

   if (!item) return;  // Sainity check.

   QString basePvName;
   QString otherPvName;

   basePvName = item->getSetPointPvName ();
   pvElement.setAttribute (nameAttribute, basePvName);

   otherPvName = item->getReadBackPvName ();
   if (!otherPvName.isEmpty() && (otherPvName != basePvName)) {
      pvElement.setAttribute (readBackNameAttribute, otherPvName);
   }

   otherPvName = item->getArchiverPvName ();
   if (!otherPvName.isEmpty() && (otherPvName != basePvName)) {
      pvElement.setAttribute (archiverNameAttribute, otherPvName);
   }

   QVariant value = item->getNodeValue ();
   pvElement.setAttribute (valueAttribute, value.toString ());
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveUtilities::writeXmlArrayPv (const QEPvLoadSaveItem* itemIn,
                                             QDomDocument& doc,
                                             QDomElement& arrayElement)
{
   const QEPvLoadSaveLeaf* item = dynamic_cast <const QEPvLoadSaveLeaf*> (itemIn);

   if (!item) return;  // Sainity check.

   QVariantList valueList = item->getNodeValue ().toList ();
   QVariant value = valueList.value (0);
   int n = valueList.size ();

   arrayElement.setAttribute (nameAttribute, item->getNodeName ());
   arrayElement.setAttribute (numberAttribute, QString ("%1").arg (n));

   for (int j = 0; j < n; j++) {
      QDomElement itemElement = doc.createElement (elementTagName);
      arrayElement.appendChild (itemElement);
      itemElement.setAttribute (indexAttribute, QString ("%1").arg (j));

      value = valueList.value (j);
      itemElement.setAttribute (valueAttribute, value.toString ());
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

// end
