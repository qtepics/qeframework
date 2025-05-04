/*  QEPvLoadSaveModel.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2024 Australian Synchrotron
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

#include "QEPvLoadSaveModel.h"
#include <QDebug>
#include <QList>
#include <QMimeData>
#include <QVariant>
#include <QEPlatform.h>
#include <QEScaling.h>
#include "QEPvLoadSave.h"
#include "QEPvLoadSaveItem.h"
#include "QEPvLoadSaveUtilities.h"

#define DEBUG  qDebug () << "QEPvLoadSaveModel" << __LINE__ << __FUNCTION__ << "  "

static const QVariant nilValue = QVariant();

//-----------------------------------------------------------------------------
//
QEPvLoadSaveModel::QEPvLoadSaveModel (QTreeView* treeViewIn, QEPvLoadSave* parent) : QAbstractItemModel (parent)
{
   // Save calling parameter
   //
   this->owner = parent;
   this->treeView = treeViewIn;
   this->requestedInsertItem = NULL;
   this->selectedItem = NULL;

   // The core item is a QTreeView/QAbstractItemModel artefact
   // Note: this item does not/must not have a parent.
   // It is a place holder - not visible per se.
   // Note to be confused with the user ROOT item.
   //
   this->coreItem = new QEPvLoadSaveGroup ("Core", NULL);
   this->heading = "";

   // Associate this model with the treeView.
   //
   this->treeView->setModel (this);        // tree is a widget

   this->treeView->installEventFilter (this);

   // Create a tree selection model.
   //
   this->treeSelectionModel = new QItemSelectionModel (this, this);
   this->treeView->setSelectionModel (this->treeSelectionModel);

   QObject::connect (this->treeSelectionModel, SIGNAL (selectionChanged (const QItemSelection&, const QItemSelection&)),
                     this,                     SLOT   (selectionChanged (const QItemSelection&, const QItemSelection&)));
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveModel::~QEPvLoadSaveModel ()
{
   if (this->coreItem) delete this->coreItem;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveModel::setHeading (const QString& headingIn)
{
   this->heading = headingIn;
}

//-----------------------------------------------------------------------------
//
QString QEPvLoadSaveModel::getHeading () const
{
   return this->heading;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveModel::setupModelData (QEPvLoadSaveItem* rootItem, const QString& headingIn)
{
   // Removed exisiting user root item (not model core item).
   //
   QEPvLoadSaveItem* oldRootItem = this->getRootItem ();
   if (oldRootItem) {
      this->removeItemFromModel (oldRootItem);
      oldRootItem = NULL;
      this->heading = "";
   }

   if (rootItem) {
      this->heading = headingIn;
      this->addItemToModel (rootItem, this->coreItem);
      this->modelUpdated ();
   }
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveModel::modelUpdated ()
{
   emit this->layoutChanged ();
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveModel::itemUpdated (const QEPvLoadSaveItem* item,
                                     const QEPvLoadSaveCommon::ColumnKinds kind,
                                     const bool updateParent)
{
   if (!item) return;
   QModelIndex index = this->getIndex (item, int (kind));
   emit this->dataChanged (index, index);  // this causes tree view to update

   if (updateParent) {
      // Call this method recursively - ensure we stop.
      QEPvLoadSaveItem* parentItem = item->getParent();
      if (parentItem && (parentItem != this->coreItem)) {
         this->itemUpdated (parentItem, kind, updateParent);
      }
   }
}

//-----------------------------------------------------------------------------
//
bool QEPvLoadSaveModel::addItemToModel (QEPvLoadSaveItem* item, QEPvLoadSaveItem* parentItem)
{
   bool result = false;

   // sanity checks: item must exist and specified parent.
   //
   if (item && parentItem) {
      QModelIndex parentIndex= this->getIndex (parentItem, 0);
      int number = parentItem->childCount ();

      this->requestedInsertItem = item;                // Save reference item - we use this in insertRows.
      result = this->insertRow (number, parentIndex);  // We always append items.
      this->requestedInsertItem = NULL;                // Remove dangling reference.

      // item calls this resursively down the QEPvLoadSaveItem tree.
      //
      item->actionConnect (this,
                           SLOT (acceptSetReadOut (const QString&)),
                           SLOT (acceptActionComplete (const QEPvLoadSaveItem*, QEPvLoadSaveCommon::ActionKinds, bool)),
                           SLOT (acceptActionInComplete (const QEPvLoadSaveItem*, QEPvLoadSaveCommon::ActionKinds)));
   }
   return result;
}

//-----------------------------------------------------------------------------
//
bool QEPvLoadSaveModel::removeItemFromModel (QEPvLoadSaveItem* item)
{
   bool result = false;

   // sanity checks: item must exist and specified parent.
   //
   if (item) {
      QEPvLoadSaveItem* parentItem = item->getParent ();

      if (parentItem) {
         QModelIndex pi = this->getIndex (parentItem, 0);
         int row = item->childPosition ();

         if (row >= 0) {
            // Ensure no dangling references - can we do better?
            //
            this->selectedItem = NULL;
            result = this->removeRow (row, pi);
         } else {
            DEBUG << "fail  row" << row;
         }
      } else {
         DEBUG << "fail  no parent";
      }
   }
   return result;
}

//-----------------------------------------------------------------------------
//
bool QEPvLoadSaveModel::mergeItemInToItem (QEPvLoadSaveItem* item, QEPvLoadSaveItem* targetItem)
{
   if (!item) return false;
   if (!targetItem) return false;

   QString nodeName = item->getNodeName ();
   QEPvLoadSaveItem* counterPart = targetItem->getNamedChild (nodeName);

   if (counterPart) {
      // counter part exists - check types match
      //
      if (item->getIsPV () != counterPart->getIsPV ()) {
         DEBUG << "*** PV/Group conflict" << nodeName;
         return false;
      }

      if (item->getIsPV ()) {
         // Copy value
         //
         counterPart->setNodeValue (item->getNodeValue ());
         this->itemUpdated (counterPart, QEPvLoadSaveCommon::NodeName, false);
      } else {
         // Copy children.
         //
         int j;

         for (j = 0; j < item->childCount (); j++) {
            // recursive call.
            //
            this->mergeItemInToItem (item->getChild (j), counterPart);
         }
      }

   } else {
      // counter part does not exists.
      //
      counterPart = item->clone (NULL);
      this->addItemToModel (counterPart, targetItem);
   }

   return true;
}

//-----------------------------------------------------------------------------
//
bool QEPvLoadSaveModel::mergeItemInToModel (QEPvLoadSaveItem* item)
{
   if (!item) return false;

   QStringList location = item->getNodePath ();  // Starts from ROOT, excludes core  and item itself.
   if (location.size () >= 1 && location.value (0) != "ROOT") return false;

   QEPvLoadSaveItem* parentItem;
   int s;

   // Create item's path in this model.
   //
   parentItem = this->coreItem;
   for (s = 0; s < location.size (); s++) {
      QString nodeName = location.value (s);
      QEPvLoadSaveItem* nextItem;

      nextItem = parentItem->getNamedChild (nodeName);

      if (nextItem) {
         // already exists.
         //
         if (nextItem->getIsPV ()) {
            // And item cannot be both a group and a PV.
            //
            DEBUG << "*** PV/Group conflict" << nodeName;
            return false;
         }
      } else {
         // does not exists - let's create it.
         //
         nextItem = new QEPvLoadSaveGroup (nodeName, NULL);
         this->addItemToModel (nextItem, parentItem);
      }

      parentItem = nextItem;
   }

   return this->mergeItemInToItem (item, parentItem);
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveModel::setReadOut (const QString& text)
{
   if (this->owner != NULL) {
      this->owner->setReadOut (text);
   } else {
      DEBUG << text;
   }
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveModel::extractPVData ()
{
   // core always exists, and it will find root if it exists.
   //
   this->coreItem->extractPVData ();
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveModel::applyPVData ()
{
   this->coreItem->applyPVData ();
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveModel::readArchiveData (const QCaDateTime& dateTime)
{
   QEPvLoadSaveItem::initReadArchiveData ();
   this->coreItem->readArchiveData (dateTime);
}

//-----------------------------------------------------------------------------
//
void  QEPvLoadSaveModel::abortAction ()
{
   this->coreItem->abortAction ();
}

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveModel::leafCount () const
{
   return this->coreItem->leafCount ();
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveCommon::PvNameValueMaps QEPvLoadSaveModel::getPvNameValueMap () const
{
   return this->coreItem->getPvNameValueMap ();
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveItem* QEPvLoadSaveModel::getRootItem ()
{
   return this->coreItem->getChild (0);
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveModel::acceptSetReadOut (const QString& text)
{
   this->setReadOut (text);
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveModel::acceptActionComplete (const QEPvLoadSaveItem* item,
                                              const QEPvLoadSaveCommon::ActionKinds action,
                                              const bool actionSuccessful)
{
   if (!item) return;

   switch (action) {
      case QEPvLoadSaveCommon::Extract:
      case QEPvLoadSaveCommon::ReadArchive:
         this->itemUpdated (item, QEPvLoadSaveCommon::LoadSave, false);  // this causes tree view to update
         this->itemUpdated (item, QEPvLoadSaveCommon::Delta, true);      // this causes tree view to update
         break;

      case QEPvLoadSaveCommon::Apply:
         // no change per se - do nothing.
         break;

      case QEPvLoadSaveCommon::Update:
         this->itemUpdated (item, QEPvLoadSaveCommon::Live, false);   // this causes tree view to update
         this->itemUpdated (item, QEPvLoadSaveCommon::Delta, true);   // this causes tree view to update
         return;   // no forward

      default:
         return;   // no forward
   }

   // Forward
   //
   emit this->reportActionComplete (item, action, actionSuccessful);
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveModel::acceptActionInComplete (const QEPvLoadSaveItem* item,
                                                const QEPvLoadSaveCommon::ActionKinds action)
{
   // Just forward as is.
   //
   emit this->reportActionInComplete (item, action);
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveModel::selectionChanged (const QItemSelection& selected, const QItemSelection& /* deselected*/ )
{
   QModelIndexList list;
   int n;

   list = selected.indexes ();
   n = list.size ();

   // We expect only one row to be selected.
   //
   if (n == QEPvLoadSaveCommon::NUMBER_OF_COLUMNS) {
      QModelIndex s = list.value (0);
      QEPvLoadSaveItem* item = this->indexToItem (s);
      this->selectedItem = item;
      if (item) {
         int count = item->leafCount ();
         QString text = tr("selected ");
         if (item->getIsPV ()) {
            text.append (item->getNodeName ());
         } else {
            text.append (QString ("%1").arg (count).append (" item"));
            if (count != 1) text.append ("s");
         }
         this->setReadOut (text);
      }
   } else {
      // Don't allow multiple selections (yet)
      //
      this->selectedItem = NULL;
   }
}

//-----------------------------------------------------------------------------
//
bool QEPvLoadSaveModel::processDropEvent (QEPvLoadSaveItem* parentItem, QDropEvent *event)
{
   QString dropText;

   if (!parentItem) return false;  // sanity check.

   if (parentItem->getIsPV ()) {
      // Don't drop on to PV as such, but create a sibling...
      //
      parentItem = parentItem->getParent ();
   }

   if (!parentItem) return false;  // sanity check.
   if (!event) return false;  // sanity check.

   // If no text available, do nothing
   //
   if (!event->mimeData()->hasText ()){
      event->ignore ();
      return false;
   }

   // Get the drop data
   //
   const QMimeData *mime = event->mimeData ();

   // If there is any text, drop the text
   //
   dropText = mime->text ();
   if (!dropText.isEmpty ()) {
      QEPvLoadSaveItem* item;

      // Carry out the drop action
      // Parse dialog text e.g. of the form "ID3:MOTOR01{w:.VAL;ra:.RBV;}"
      // and split into three separate names.
      //
      QString setPoint;
      QString readBack;
      QString archiver;
      bool okay;
      okay = QEPvLoadSaveUtilities::splitPvNames (dropText, setPoint, readBack, archiver);
      if (okay) {
         item = new QEPvLoadSaveLeaf (setPoint, readBack, archiver, nilValue, NULL);
         this->addItemToModel (item, parentItem);
      } else {
         this->setReadOut ("failed to parse: " + dropText);
      }
   }

   // Tell the dropee that the drop has been acted on
   //
   if (event->source () == this->treeView) {
      event->setDropAction(Qt::CopyAction);
      event->accept ();
   } else {
      event->acceptProposedAction ();
   }

   return true;
}

//-----------------------------------------------------------------------------
//
bool QEPvLoadSaveModel::eventFilter (QObject *obj, QEvent* event)
{
   const QEvent::Type type = event->type ();
   QPoint pos;
   QModelIndex index;
   QEPvLoadSaveItem* item = NULL;
   QString nodeName;

   // The row size scales, buy the cursor is fixed size.
   //
   int dragOffset = QEScaling::scale (17) + 18;

   switch (type) {

      case QEvent::DragEnter:
         if (obj == this->treeView) {
            QDragEnterEvent* dragEnterEvent = static_cast<QDragEnterEvent*> (event);
            pos = QEPlatform::positionOf (dragEnterEvent);
            pos.setY (pos.y () - dragOffset);
            item = this->itemAtPos (pos);
            nodeName = item ? item->getNodeName () : "nil";
            dragEnterEvent->setDropAction (Qt::CopyAction);
            dragEnterEvent->accept ();
            this->setReadOut (nodeName);
            return true;
         }
         break;


      case QEvent::DragMove:
         if (obj == this->treeView) {
            QDragMoveEvent* dragMoveEvent = static_cast<QDragMoveEvent*> (event);
            pos = QEPlatform::positionOf (dragMoveEvent);
            pos.setY (pos.y () - dragOffset);
            item = this->itemAtPos (pos);
            nodeName = item ? item->getNodeName () : "nil";
            if (item) {
               dragMoveEvent->accept ();
            } else {
               dragMoveEvent->ignore ();
            }
            index = this->treeView->indexAt (pos);
            // Is there a better way to high-light?
            //
            this->treeSelectionModel->setCurrentIndex (index, QItemSelectionModel::SelectCurrent);
            //this->setReadOut (nodeName);
            return true;
         }
         break;


      case QEvent::DragLeave:
         if (obj == this->treeView) {
            // QDragLeaveEvent* dragLeaveEvent = static_cast<QDragLeaveEvent*> (event);
            this->setReadOut ("");
            return true;
         }
         break;


      case QEvent::Drop:
         if (obj == this->treeView) {
            QDropEvent* dragDropEvent = static_cast<QDropEvent*> (event);
            pos = QEPlatform::positionOf (dragDropEvent);
            pos.setY (pos.y () - dragOffset);
            item = this->itemAtPos (pos);
            if (item) {
               return this->processDropEvent (item, dragDropEvent);
            }
         }
         break;

      default:
         // Just fall through
         break;
   }

   return false; // we did not handle this event
}


//=============================================================================
// Overriden model functions
//=============================================================================
//
QVariant QEPvLoadSaveModel::data (const QModelIndex& index, int role) const
{
   if (!index.isValid ()) {
      return QVariant ();
   }

   if (role != Qt::DisplayRole && role != Qt::EditRole) {
      return QVariant ();
   }

   QEPvLoadSaveItem *item = this->getItem (index);

   return item->getData (index.column ());
}

//-----------------------------------------------------------------------------
//
bool QEPvLoadSaveModel::setData (const QModelIndex&, const QVariant&, int)
{
   return false;
}

//-----------------------------------------------------------------------------
//
Qt::ItemFlags QEPvLoadSaveModel::flags (const QModelIndex & index) const
{
   if (!index.isValid ()) {
      return Qt::NoItemFlags;
   } else {
      return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
   }
}

//-----------------------------------------------------------------------------
//
QVariant QEPvLoadSaveModel::headerData (int section, Qt::Orientation orientation, int role) const
{
   const QEPvLoadSaveCommon::ColumnKinds kind = QEPvLoadSaveCommon::ColumnKinds (section);
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
       switch (kind) {
          case QEPvLoadSaveCommon::NodeName:  return QVariant (this->heading);
          case QEPvLoadSaveCommon::LoadSave:  return QVariant (tr("load/save"));
          case QEPvLoadSaveCommon::Live:      return QVariant (tr("live"));
          case QEPvLoadSaveCommon::Delta:     return QVariant (tr("delta"));
          default: return QVariant ("");
       }
   }

   return QVariant ();
}

//-----------------------------------------------------------------------------
//
bool QEPvLoadSaveModel::setHeaderData (int, Qt::Orientation, const QVariant&, int)
{
   return false;
}

//-----------------------------------------------------------------------------
//
QModelIndex QEPvLoadSaveModel::index (int row, int column, const QModelIndex& parent) const
{
   if (parent.isValid() && parent.column() != 0) {
        return QModelIndex();
   }

   QEPvLoadSaveItem *parentItem = this->getItem (parent);
   QEPvLoadSaveItem *childItem = parentItem->getChild (row);

   if (childItem) {
      return this->createIndex (row, column, childItem);
   } else {
      return QModelIndex ();
   }
}

//-----------------------------------------------------------------------------
//
QModelIndex QEPvLoadSaveModel::parent (const QModelIndex & child) const
{
   if (!child.isValid ()) {
      return QModelIndex ();
   }

   QEPvLoadSaveItem *childItem = this->getItem (child);
   if (!childItem) {
      DEBUG << "null childItem " << child;
   }
   QEPvLoadSaveItem *parentItem = childItem->getParent ();

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

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveModel::rowCount (const QModelIndex & parent) const
{
   QEPvLoadSaveItem* parentItem = this->getItem (parent);

   if (parentItem) return  parentItem->childCount();

   // Unexpected
   //
   return 0;
}

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveModel::columnCount (const QModelIndex& /* parent */ ) const
{
   return this->coreItem->columnCount ();
}

//-----------------------------------------------------------------------------
//
bool QEPvLoadSaveModel::insertRows (int position, int rows, const QModelIndex& parent)
{
   QEPvLoadSaveItem *parentItem = this->getItem (parent);
   bool success = false;

   // insertRows - ensure we can deal with this request.
   //
   if (this->requestedInsertItem && rows == 1) {
      this->beginInsertRows (parent, position, position);
      success = parentItem->insertChild (position, this->requestedInsertItem);
      this->endInsertRows ();
   }
   return success;
}

//-----------------------------------------------------------------------------
//
bool QEPvLoadSaveModel::removeRows (int position, int rows, const QModelIndex& parent)
{
   QEPvLoadSaveItem *parentItem = this->getItem (parent);
   bool success = true;
   int last = position + rows - 1;

   this->beginRemoveRows (parent, position, last);
   success = parentItem->removeChildren (position, rows);
   this->endRemoveRows ();

   return success;
}


//=============================================================================
// Utility function to hide the nasty static cast and stuff.
//=============================================================================
//
QEPvLoadSaveItem* QEPvLoadSaveModel::indexToItem (const QModelIndex& index) const
{
   QEPvLoadSaveItem* result = NULL;
   if (index.isValid ()) {
      result = static_cast <QEPvLoadSaveItem *>(index.internalPointer ());
   }
   return result;
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveItem *QEPvLoadSaveModel::getItem (const QModelIndex &index) const
{
   QEPvLoadSaveItem* result = this->coreItem;

   if (index.isValid ()) {
      QEPvLoadSaveItem* temp;
      temp = static_cast <QEPvLoadSaveItem *>(index.internalPointer ());
      if (temp) result = temp;
   }
   // If not set then still the coreItem.
   return result;
}

//-----------------------------------------------------------------------------
//
QModelIndex QEPvLoadSaveModel::getIndex (const QEPvLoadSaveItem* item, const int col)
{
   QModelIndex result;  // invalid by default (which is really getCoreIndex)

   if (item == this->coreItem) {
      result = this->getCoreIndex ();
   } else
   if (item) {
      int row = item->childPosition ();
      if (row >= 0) {
         result = this->createIndex (row, col, (QEPvLoadSaveItem*)item);
      }
   }
   return result;
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveItem* QEPvLoadSaveModel::itemAtPos (const QPoint& pos) const
{
   QEPvLoadSaveItem* result = NULL;

   if (this->treeView) {
      QModelIndex index = this->treeView->indexAt (pos);
      result = this->indexToItem (index);
   }
   return result;
}

// end
