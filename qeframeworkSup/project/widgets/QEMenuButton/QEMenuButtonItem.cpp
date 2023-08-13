/*  QEMenuButtonItem.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2015-2023 Australian Synchrotron
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include "QEMenuButtonItem.h"
#include <QDebug>
#include <QEMenuButton.h>

#define DEBUG  qDebug () << "QEMenuButtonItem" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QEMenuButtonItem::QEMenuButtonItem (const QString& nameIn,
                                    const bool subMenuContainerIn,
                                    QEMenuButton* ownerIn,
                                    QEMenuButtonItem* parent)
{
   this->name = nameIn;
   this->isSubMenuContainer = subMenuContainerIn;
   this->owner = ownerIn;
   this->parentItem = parent;

   // Add to parent's own/specific QEMenuButtonItem child list.
   //
   if (this->parentItem) {
      this->parentItem->appendChild (this);
   }

   // The QEMenuButtonData constructor sets all values to default state.
}

//------------------------------------------------------------------------------
//
QEMenuButtonItem::~QEMenuButtonItem ()
{
   qDeleteAll (this->childItems);
   this->childItems.clear ();
}

//------------------------------------------------------------------------------
//
QString QEMenuButtonItem::getName () const
{
   return this->name;
}

//------------------------------------------------------------------------------
//
bool QEMenuButtonItem::getIsSubMenuContainer () const
{
   return this->isSubMenuContainer;
}

//------------------------------------------------------------------------------
//
QDomElement QEMenuButtonItem::createDomElement (QDomDocument& doc) const
{
   QDomElement result;

   // Do essentially common processing first.
   //
   result = doc.createElement (this->isSubMenuContainer ? "Menu" : "Item");
   result.setAttribute ("Name", this->getName ());

   if (this->data.separator) {   // false by default
      result.setAttribute ("Separator", "true");
   }

   if (this->isSubMenuContainer) {
      // Node item
      //
      const int n = this->childCount ();
      for (int j = 0; j < n; j++) {
         QEMenuButtonItem* child = this->getChild (j);
         if (child) {
            QDomElement childElement = child->createDomElement (doc);  // recursive
            result.appendChild (childElement);
         }
      }

   } else {
      // Leaf item.
      //
      QDomElement itemElement;

#define ADD_SUB_ELEMENT(tag, value) {                                        \
   QDomElement subItemElement = doc.createElement (tag);                     \
   QDomText text = doc.createTextNode (value);                               \
   subItemElement.appendChild (text);                                        \
   itemElement.appendChild (subItemElement);                                 \
}


      // None of these option are mutually exclusive.
      //
      if (!this->data.programName.isEmpty()) {
         itemElement = doc.createElement ("Program");

         ADD_SUB_ELEMENT ("Name", this->data.programName);

         // Add each argument as its own dom element
         for (int a = 0; a < this->data.programArguments.count(); a++) {
             ADD_SUB_ELEMENT ("Argument", this->data.programArguments.value (a));
         }
         ADD_SUB_ELEMENT ("Start_Option", QEMenuButtonData::psoToString (this->data.programStartupOption));

         result.appendChild (itemElement);
      }

      if (!this->data.uiFilename.isEmpty()) {
         itemElement = doc.createElement ("Window");

         ADD_SUB_ELEMENT ("UiFile", this->data.uiFilename);
         ADD_SUB_ELEMENT ("MacroSubstitutions", this->data.prioritySubstitutions);
         ADD_SUB_ELEMENT ("Customisation", this->data.customisationName);
         ADD_SUB_ELEMENT ("Create_Option", QEMenuButtonData::optionToString (this->data.creationOption));

         result.appendChild (itemElement);
      }

      if (!this->data.variable.isEmpty()) {
         itemElement = doc.createElement ("Variable");

         ADD_SUB_ELEMENT ("Name", this->data.variable);
         ADD_SUB_ELEMENT ("Value", this->data.variableValue);
         ADD_SUB_ELEMENT ("Format", QEMenuButtonData::formatToString (this->data.format));

         result.appendChild (itemElement);
      }

#undef ADD_SUB_ELEMENT

   }
   return result;
}

//------------------------------------------------------------------------------
// Object already exists (with default settings)
// This function can change isSubMenuContainer
//
bool QEMenuButtonItem::extractFromDomElement (const QDomElement& element)
{
   if (element.isNull ()) {
      qWarning () << __FUNCTION__ << __LINE__ << " null element";
      return false;
   }

   bool result = true;   // hypothesize okay

   QString itemTagName = element.tagName ();
   this->name = element.attribute ("Name").trimmed ();

   QString x = element.attribute ("Separator").trimmed ();
   this->data.separator = (x == "true");

   if (itemTagName == "Menu") {
      // Node item
      //
      this->isSubMenuContainer = true;

      QDomElement childElement = element.firstChildElement ("");
      while (!childElement.isNull ()) {
         QEMenuButtonItem* subItem;
         subItem = new QEMenuButtonItem (">>undefined<<", false, this->owner, this);
         result = subItem->extractFromDomElement (childElement);
         if (!result) break;
         childElement = childElement.nextSiblingElement ("");
      }

   } else if (itemTagName == "Item") {

      // Leaf item.
      //
      this->isSubMenuContainer = false;
      QDomElement itemElement;

#define EXTRACT_FROM_ELEMENT(name, value, def)  {                             \
   QDomNode textElement = itemElement.namedItem (name);                       \
   QDomNode node = textElement.firstChild ().toText ();                       \
   QDomText text = node.toText ();                                            \
   value = (text.isNull()) ? QString (def) : text.nodeValue();                \
}


      // Parse XML using Qt's Document Object Model.
      // We look for required tags
      //
      QString tagName;
      QString argText;
      QString enumText;

      itemElement = element.firstChildElement ("");
      while (!itemElement.isNull ())   {

         tagName = itemElement.tagName ();

         if (tagName == "Program") {

            EXTRACT_FROM_ELEMENT ("Name", this->data.programName, "");

            QDomElement argElement;
            bool atLeastOne = false;
            this->data.programArguments.clear();
            argElement = itemElement.firstChildElement ("Argument");
            while (!argElement.isNull ()) {
               QDomNode argNode = argElement.firstChild ().toText ();
               QDomText argText = argNode.toText();
               QString value = argText.nodeValue();
               this->data.programArguments.append(value);
               atLeastOne = true;
               argElement = argElement.nextSiblingElement ("Argument");
            }

            if (!atLeastOne) {
               // Deprecated
               EXTRACT_FROM_ELEMENT ("Arguments", argText, "");
               this->data.programArguments = QEMenuButtonData::split (argText);
            }

            EXTRACT_FROM_ELEMENT ("Start_Option", enumText, "0");

            this->data.programStartupOption = QEMenuButtonData::stringToPso (enumText);

         } else if (tagName == "Window") {

            EXTRACT_FROM_ELEMENT ("UiFile", this->data.uiFilename, "");
            EXTRACT_FROM_ELEMENT ("MacroSubstitutions", this->data.prioritySubstitutions, "");
            EXTRACT_FROM_ELEMENT ("Customisation", this->data.customisationName, "");
            EXTRACT_FROM_ELEMENT ("Create_Option", enumText, "0");
            this->data.creationOption = QEMenuButtonData::stringToOption (enumText);

         } else if (tagName == "Variable") {

            EXTRACT_FROM_ELEMENT ("Name", this->data.variable, "");
            EXTRACT_FROM_ELEMENT ("Value", this->data.variableValue, "");
            EXTRACT_FROM_ELEMENT ("Format", enumText, "0");
            this->data.format = QEMenuButtonData::stringToFormat (enumText);

         } else {
            DEBUG << "warning: ignoring unexpected tag " << tagName;
         }

         itemElement = itemElement.nextSiblingElement ("");
      }

#undef EXTRACT_FROM_ELEMENT

   } else {
      DEBUG << "warning: unexpected element tag:" << itemTagName;
      result = false;
   }

   return result;
}

//------------------------------------------------------------------------------
//
QString QEMenuButtonItem::getSubstitutedName () const
{
   QString result;
   if (this->owner) {
      result = this->owner->substituteThis (this->name);
   } else {
      result = this->name;
   }
   return result;
}


//------------------------------------------------------------------------------
//
QAction* QEMenuButtonItem::constructAction (QMenu* parent)
{
   QAction* result = NULL;

   if (!this->getIsSubMenuContainer()) {
      result = new QAction (this->getSubstitutedName(), parent);
      result->setData (this->data.toVariant());
   }

   return result;
}

//------------------------------------------------------------------------------
//
QMenu* QEMenuButtonItem::constructMenu (QMenu* parent)
{
   QMenu* result = NULL;

   if (this->getIsSubMenuContainer ()) {
      result = new QMenu (this->getSubstitutedName(), parent);

      const int n = this->childCount ();
      for (int j = 0; j < n; j++) {
         QEMenuButtonItem* child = this->getChild (j);
         if (child) {
            // No need to check (j > 0) - Qt does that for free.
            if (child->data.separator) {
               result->addSeparator ();
            }

            // Note: only one of constructMenu/constructAction returns an object.
            //
            QMenu* childMenu = child->constructMenu (result);
            if (childMenu) result->addMenu (childMenu);

            QAction* childAction = child->constructAction (result);
            if (childAction) result->addAction(childAction);
         }
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEMenuButtonItem::appendChild (QEMenuButtonItem* child)
{
   // Check for duplicates???
   //
   this->childItems.append (child);
   child->parentItem = this;  // Ensure consistency
}

//------------------------------------------------------------------------------
//
int QEMenuButtonItem::columnCount () const
{
   return 1; // Number of cols fixed at 1.
}

//------------------------------------------------------------------------------
//
QEMenuButtonItem* QEMenuButtonItem::getChild (const int position) const
{
   return this->childItems.value (position, NULL);
}

//------------------------------------------------------------------------------
//
QEMenuButtonItem* QEMenuButtonItem::getParent () const
{
   return this->parentItem;
}

//------------------------------------------------------------------------------
//
int QEMenuButtonItem::childCount () const
{
   return this->childItems.count ();
}

//------------------------------------------------------------------------------
//
int QEMenuButtonItem::childPosition () const
{
   if (this->parentItem) {
      return this->parentItem->childItems.indexOf (const_cast<QEMenuButtonItem*>(this));
   }
   return 0;
}

//------------------------------------------------------------------------------
//
QVariant QEMenuButtonItem::getData (const int column) const
{
   QVariant result;
   if (column == 0) {
      if (this->isSubMenuContainer) {
         result = QVariant (this->name + " >");
      } else {
         result = QVariant (this->name);
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
bool QEMenuButtonItem::insertChild (const int position, QEMenuButtonItem* child)
{
   if (position < 0 || (position > this->childItems.size ())) {
       return false;
   }

   if (!child) {
      return false;
   }

   child->parentItem = this;
   this->childItems.insert (position, child);

   return true;
}

//------------------------------------------------------------------------------
//
bool QEMenuButtonItem::removeChildren(const int position, const int count)
{
   if (position < 0 || (position + count > this->childItems.size ())) {
       return false;
   }

   for (int row = 0; row < count; ++row) {
       delete this->childItems.takeAt (position);
   }

   return true;
}


//------------------------------------------------------------------------------
//
QDataStream& operator<< (QDataStream& stream, QEMenuButtonItem& that)
{
   stream << that.name;
   stream << that.isSubMenuContainer;
   if (that.isSubMenuContainer) {
      // Node item
      //
      const int n = that.childCount ();
      stream << n;
      for (int j = 0; j < n; j++) {
         QEMenuButtonItem* child = that.getChild (j);
         if (child) {
            stream << *child;
         }
      }
   } else {
      stream << that.data;
   }

   return stream;
}

//------------------------------------------------------------------------------
//
QDataStream& operator>> (QDataStream& stream, QEMenuButtonItem& that)
{
   stream >> that.name;
   stream >> that.isSubMenuContainer;
   if (that.isSubMenuContainer) {
      // Node item
      //
      int n;
      stream >> n;
      for (int j = 0; j < n; j++) {
         QEMenuButtonItem* child = new QEMenuButtonItem ("", false, that.owner, &that);
         stream >> *child;
      }
   } else {
      stream >> that.data;
   }
   return stream;
}

// end
