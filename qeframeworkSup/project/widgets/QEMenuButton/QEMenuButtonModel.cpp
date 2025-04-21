/*  QEMenuButtonModel.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2015-2024 Australian Synchrotron
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

#include "QEMenuButtonModel.h"

#include <QDebug>
#include <QMimeData>
#include <qdom.h>

#include <QEMenuButton.h>
#include <QEMenuButtonItem.h>

#define DEBUG  qDebug () << "QEMenuButtonModel"  << __LINE__ << __FUNCTION__ << "  "

// XML tag/attribute names etc.
//
static const QString mainTagName      = "MenuButton";
static const QString versionAttrName  = "Version";
static const QString mineType         = "application/vnd.text.list";


//------------------------------------------------------------------------------
//
QEMenuButtonModel::QEMenuButtonModel (QEMenuButton* parent) :
   QAbstractItemModel (parent),
   owner (parent)
{
   // The core item is a QTreeView/QAbstractItemModel artefact
   // Note: this item does not/must not have a parent.
   // It is a place holder - not visible per se.
   //
   this->coreItem = new QEMenuButtonItem ("coreItem", true, this->owner, NULL);
   this->heading = "Menu Tree";

   this->requestedInsertItem = NULL;
}

//------------------------------------------------------------------------------
//
QEMenuButtonModel::~QEMenuButtonModel()
{
   if (this->coreItem) delete this->coreItem;
}

//------------------------------------------------------------------------------
//
bool QEMenuButtonModel::parseXml (const QString& xml)
{
   QDomDocument doc;
   QString errorText;
   int errorLine;
   int errorCol;

   if (!doc.setContent (xml, false, &errorText, &errorLine, &errorCol)) {
      qWarning () << QString ("%1:%2").arg (errorLine).arg (errorCol)
                  << " set content failed " << errorText;
      return false;
   }

   QDomElement docElem = doc.documentElement ();

   // Examine top level tag name - is this the tag we expect?
   //
   if (docElem.tagName () != mainTagName) {
      qWarning ()  << "QEMenuButtonModel: unexpected tag name " << docElem.tagName ();
      return false;
   }

   // Verify version.
   //
   QString versionImage = docElem.attribute (versionAttrName).trimmed ();
   bool versionOkay;
   int version = versionImage.toInt (&versionOkay);

   if (!versionOkay || (version != 1)) {
      qWarning () << "QEMenuButtonModel: invalid/unexpected version specified: " << versionImage;
      return false;
   }

   QEMenuButtonItem* replacementCore = new QEMenuButtonItem ("coreItem", true, this->owner, NULL);

   // The core element is not serilaised, it is a model artefact.
   // This code snippet (~ 8 lines) mirrors QEMenuButtonItem::extractFromDomElement
   //
   bool status = true;
   QDomElement childElement = docElem.firstChildElement ("");
   while (!childElement.isNull ()) {
      QEMenuButtonItem* subItem;
      subItem = new QEMenuButtonItem (">>undefined<<", false, this->owner, replacementCore);
      status = subItem->extractFromDomElement (childElement);
      if (!status) break;
      childElement = childElement.nextSiblingElement ("");
   }

   if (!status) {
      qWarning () << "QEMenuButtonModel: failed to parse document element tree";
      delete replacementCore;   // tidy up.
      return false;
   }

   // Delete old tree, and replace with new one.
   //
   delete this->coreItem;
   this->coreItem = replacementCore;
   return true;
}

//------------------------------------------------------------------------------
//
QString QEMenuButtonModel::serialiseXml () const
{
   QDomDocument doc;
   QDomElement docElem;

   doc.clear ();
   docElem = doc.createElement (mainTagName);
   docElem.setAttribute (versionAttrName, 1);
   doc.appendChild (docElem);

   // The core element is not serilaised, it is a model artefact.
   // This code snippet (~ 8 lines) mirrors QEMenuButtonItem::createDomElement.
   //
   const int n = this->coreItem->childCount ();
   for (int j = 0; j < n; j++) {
      QEMenuButtonItem* child = this->coreItem->getChild (j);
      if (child) {
         QDomElement childElement = child->createDomElement (doc);  // recursive
         docElem.appendChild (childElement);
      }
   }

   // Remove newlines - designer's property manager does not handle properties
   // with many many lines very well.
   //
   return doc.toString ().replace ("\n", "");
}

//------------------------------------------------------------------------------
//
QString QEMenuButtonModel::emptyXml ()
{
   // Easier just to do it than dom around.
   //
   return QString ("<%1 %2=\"1\"/>").arg (mainTagName).arg (versionAttrName);
}

//------------------------------------------------------------------------------
//
bool QEMenuButtonModel::constructMenu (QMenu* rootMenu)
{
   if (!rootMenu) return false;

   // First remove any exist Actions/Submenu items
   //
   rootMenu->clear ();

   // The core element is does not become part of the menu.
   // This code snippet (~16 lines) mirrors QEMenuButtonItem::constructMenu.
   //
   const int n = this->coreItem->childCount ();
   for (int j = 0; j < n; j++) {
      QEMenuButtonItem* child = this->coreItem->getChild (j);
      if (child) {
         // No need to check (j > 0) - Qt does that for free.
         if (child->data.separator) {
            rootMenu->addSeparator ();
         }

         // Note: only one of constructMenu/constructAction returns an object.
         //
         QMenu* childMenu = child->constructMenu (rootMenu);
         if (childMenu) rootMenu->addMenu (childMenu);

         QAction* childAction = child->constructAction (rootMenu);
         if (childAction) rootMenu->addAction(childAction);
      }
   }
   return true;
}

//------------------------------------------------------------------------------
//
bool QEMenuButtonModel::addItemToModel (QEMenuButtonItem* item,
                                        QEMenuButtonItem* parentItem,
                                        const int position)
{
   bool result = false;

   if (!parentItem) parentItem = this->coreItem;

   // sanity checks: item must exist and specified parent.
   //
   if (item && parentItem) {
      QModelIndex parentIndex= this->getIndex (parentItem);
      int number = (position >= 0) ? position : parentItem->childCount ();

      this->requestedInsertItem = item;          // Save reference item - we use this in insertRows which is an overloaded function..
      result = this->insertRow (number, parentIndex);
      this->requestedInsertItem = NULL;          // Remove dangling reference.
   }
   return result;
}

//------------------------------------------------------------------------------
//
bool QEMenuButtonModel::removeItemFromModel (QEMenuButtonItem* item)
{
   bool result = false;

   // sanity checks: item must exist and have a specified parent.
   //
   if (item) {
      QEMenuButtonItem* parentItem = item->getParent ();

      if (parentItem) {
         QModelIndex pi = this->getIndex (parentItem);
         int row = item->childPosition ();

         if (row >= 0) {
            result = this->removeRow (row, pi);
         } else {
            DEBUG << "fail - negative row" << row;
         }
      } else {
         DEBUG << "fail - no parent";
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEMenuButtonModel::modelUpdated ()
{
    emit this->layoutChanged ();
}

//------------------------------------------------------------------------------
//
void QEMenuButtonModel::itemUpdated (const QEMenuButtonItem* item)
{
   if (!item) return;

   QModelIndex index = this->getIndex (item);
   emit this->dataChanged (index, index);  // this causes tree view to update

}


//------------------------------------------------------------------------------
// Override (pure abstract) virtual functions.
//
QVariant QEMenuButtonModel::data (const QModelIndex& index, int role) const
{
   if (!index.isValid ()) {
      return QVariant ();
   }

   if (role != Qt::DisplayRole && role != Qt::EditRole) {
      return QVariant ();
   }

   QEMenuButtonItem* item = this->getItem (index);

   return item->getData (index.column ());
}

//------------------------------------------------------------------------------
//
QVariant QEMenuButtonModel::headerData (int section, Qt::Orientation orientation,
                                        int role) const
{
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
       switch (section) {
       case 0:  return QVariant (this->heading);
       default: return QVariant ("");
       }
   }

   return QVariant ();
}

//------------------------------------------------------------------------------
//
QModelIndex QEMenuButtonModel::index (int row, int column,
                                      const QModelIndex& parentIndex) const
{
   if (parentIndex.isValid() && parentIndex.column() != 0) {
        return QModelIndex();
   }

   QEMenuButtonItem* parentItem = this->getItem (parentIndex);
   QEMenuButtonItem* childItem = parentItem->getChild (row);

   if (childItem) {
      return this->createIndex (row, column, childItem);
   } else {
      return QModelIndex ();
   }
}

//------------------------------------------------------------------------------
//
QModelIndex QEMenuButtonModel::parent (const QModelIndex& childIndex) const
{
   if (!childIndex.isValid ()) {
      return QModelIndex ();
   }

   QEMenuButtonItem* childItem = this->getItem (childIndex);
   if (!childItem) {
      DEBUG << "null childItem " << childIndex;
      return QModelIndex ();
   }

   QEMenuButtonItem* parentItem = childItem->getParent ();
   if (parentItem == this->coreItem) {
      return QModelIndex ();
   }

   if (parentItem) {
      int row = parentItem->childPosition ();
      if (row >= 0) {
         return this->createIndex (row, 0, parentItem);
      }
   }

   // Unexpected
   //
   return QModelIndex ();
}

//------------------------------------------------------------------------------
//
int QEMenuButtonModel::rowCount (const QModelIndex& parentIndex) const
{
   QEMenuButtonItem* parentItem = this->getItem (parentIndex);

   if (parentItem) return  parentItem->childCount();

   // Unexpected
   //
   return 0;
}

//------------------------------------------------------------------------------
//
int QEMenuButtonModel::columnCount (const QModelIndex& /* parent */) const
{
   return this->coreItem->columnCount ();
}

//------------------------------------------------------------------------------
// Override other functions
//
Qt::ItemFlags QEMenuButtonModel::flags (const QModelIndex &index) const
{
   if (index.isValid ()) {
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable |
             Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled;
   } else {
      return Qt::ItemIsDropEnabled;
   }
}

//------------------------------------------------------------------------------
//
bool QEMenuButtonModel::setData (const QModelIndex&, const QVariant&, int)
{
    return false;
}

//------------------------------------------------------------------------------
//
bool QEMenuButtonModel::setHeaderData (int, Qt::Orientation, const QVariant&, int)
{
     return false;
}

//------------------------------------------------------------------------------
//
bool QEMenuButtonModel::insertRows (int position, int rows,
                                    const QModelIndex& parentIndex)
{
   QEMenuButtonItem* parentItem = this->getItem (parentIndex);
   bool success = false;

   // insertRows - ensure we can deal with this request.
   //
   if (this->requestedInsertItem && rows == 1) {
      this->beginInsertRows (parentIndex, position, position);
      success = parentItem->insertChild (position, this->requestedInsertItem);
      this->endInsertRows ();
   }
   return success;

}

//------------------------------------------------------------------------------
//
bool QEMenuButtonModel::removeRows (int position, int rows,
                                    const QModelIndex& parentIndex)
{
   QEMenuButtonItem* parentItem = this->getItem (parentIndex);
   bool success = true;
   int last = position + rows - 1;

   this->beginRemoveRows (parentIndex, position, last);
   success = parentItem->removeChildren (position, rows);
   this->endRemoveRows ();

   return success;
}


//==============================================================================
// Drag/drop
//==============================================================================
//
Qt::DropActions QEMenuButtonModel::supportedDropActions() const
{
    return  Qt::MoveAction | Qt::CopyAction;
}

//------------------------------------------------------------------------------
//
QStringList QEMenuButtonModel::mimeTypes() const
{
   QStringList result;
   result << mineType;
   return result;
}

//------------------------------------------------------------------------------
//
QMimeData* QEMenuButtonModel::mimeData (const QModelIndexList& indexes) const
{
   QMimeData* mimeData = new QMimeData();
   QByteArray encodedData;

   QDataStream stream (&encodedData, QIODevice::WriteOnly);
   foreach (QModelIndex index, indexes) {
      if (index.isValid ()) {
         QEMenuButtonItem* item = this->getItem (index);
         stream << *item;
      }
   }

   mimeData->setData (mineType, encodedData);
   return mimeData;
}

//-----------------------------------------------------------------------------
//
bool QEMenuButtonModel::dropMimeData (const QMimeData* data, Qt::DropAction action,
                                      int /* row */, int column,
                                      const QModelIndex& parentIndex)
{
   if (action == Qt::IgnoreAction) {
      DEBUG << "action == Qt::IgnoreAction";
      return true;
   }

   if (!data->hasFormat (mineType)) {
      DEBUG << QString ("not '%1'").arg (mineType);
      return false;
   }

   if (column > 0) {
      DEBUG << "column > 0 , i.e." << column;
      return false;
   }

   QByteArray encodedData = data->data (mineType);
   QDataStream stream (&encodedData, QIODevice::ReadOnly);

   if (stream.atEnd ()) {
      DEBUG << "Empty stream";
      return false;
   }

   QEMenuButtonItem* attachTo = NULL;
   int attachPosition = -1; // i.e. at end
   attachTo = this->getItem (parentIndex);
   if (!attachTo->getIsSubMenuContainer ()) {
      // Item has not been dropped on a submenu, so we attach to this menu item's
      // parent at the current position.
      //
      attachPosition = attachTo->childPosition ();
      attachTo = attachTo->getParent();
   }

   // We currently expect only one item (albeit a submenu container with its own
   // child menu items).
   //
   QEMenuButtonItem* item = new QEMenuButtonItem ("", false, this->owner, NULL);
   stream >> *item;
   this->addItemToModel (item, attachTo, attachPosition);

   return true;
}


//==============================================================================
// Utility function to hide the nasty static cast and stuff.
//==============================================================================
//
QEMenuButtonItem* QEMenuButtonModel::indexToItem (const QModelIndex& index) const
{
   QEMenuButtonItem* result = NULL;

   if (index.isValid ()) {
      result = static_cast <QEMenuButtonItem *>(index.internalPointer ());
   }
   return result;
}

//-----------------------------------------------------------------------------
//
QEMenuButtonItem* QEMenuButtonModel::getItem (const QModelIndex &index) const
{
   QEMenuButtonItem* result = this->coreItem;

   if (index.isValid ()) {
      QEMenuButtonItem* temp;
      temp = static_cast <QEMenuButtonItem *>(index.internalPointer ());
      if (temp) result = temp;
   }
   // If not set then still the coreItem.
   return result;
}

//-----------------------------------------------------------------------------
//
QModelIndex QEMenuButtonModel::getIndex (const QEMenuButtonItem* item) const
{
   QModelIndex result;  // invalid by default (which is really getCoreIndex)

   if (item == this->coreItem) {
      result = this->getCoreIndex ();
   } else {
      if (item) {
         int row = item->childPosition ();
         if (row >= 0) {
            result = this->createIndex (row, 0, (QEMenuButtonItem*)item);
         }
      }
   }
   return result;
}

// end
