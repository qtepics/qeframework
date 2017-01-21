/*  QEPvLoadSaveItem.cpp
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
 *  Copyright (c) 2013,2016,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>
#include <QFrame>
#include <QModelIndex>
#include <QPushButton>
#include <QWidget>

#include <QECommon.h>
#include "QEPvLoadSave.h"
#include "QEPvLoadSaveItem.h"

#define DEBUG  qDebug () << "QEPvLoadSaveItem" << __LINE__ << __FUNCTION__ << "  "

// Use by virtual functions in abstract base class that need to be overriden.
//
#define NOT_IMPLEMENTED                                              \
   DEBUG << "Class: " << this->metaObject ()->className ()           \
   << " did not overide " << __FUNCTION__ << "() function"


static const QVariant nilValue (QVariant::Invalid);

//=============================================================================
//
QEPvLoadSaveItem::QEPvLoadSaveItem (const QString & nodeNameIn,
                                    const QVariant& valueIn,
                                    QEPvLoadSaveItem* parentIn) : QObject (NULL)
{
   this->parentItem = parentIn;
   this->nodeName = nodeNameIn;
   this->value = valueIn;

   // Add to parent's own/specific QEPvLoadSaveItem child list.
   //
   if (this->parentItem) {
      this->parentItem->appendChild (this);
   }
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveItem::~QEPvLoadSaveItem ()
{
   // Remove from parent's own/specific QEPvLoadSaveItem child list.
   //
   qDeleteAll (this->childItems);
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveItem *QEPvLoadSaveItem::getChild (int position) const
{
   return this->childItems.value (position, NULL);
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveItem * QEPvLoadSaveItem::getParent () const
{
   return this->parentItem;
}

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveItem::childCount () const
{
   return this->childItems.count ();
}

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveItem::columnCount () const
{
   // Number rcols fixed at 1.
   // Could split single "Name = Value" into pair ("Name, "Value")
   return 1;
}

//-----------------------------------------------------------------------------
//
QVariant QEPvLoadSaveItem::getData (int column) const
{
   QVariant result;

   if (column == 0) {
      QString valueImage = this->nodeName;

      if (this->getIsPV ()) {
         valueImage.append (" = ");

         if (this->value.type() == QVariant::List) {
            QVariantList vl = this->value.toList ();
            valueImage.append (QString (" << %1 element array >>").arg (vl.size ()));
         } else {
            valueImage.append (this->value.toString ());
         }
      }
      result = valueImage;
   }
   return result;
}

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveItem::childPosition () const
{
   if (this->parentItem) {
      return this->parentItem->childItems.indexOf (const_cast<QEPvLoadSaveItem*>(this));
   }
   return 0;
}

//-----------------------------------------------------------------------------
//
bool QEPvLoadSaveItem::insertChild (int position, QEPvLoadSaveItem* child)
{
   if (position < 0 || position > childItems.size ()) {
      return false;
   }

   if (!child) {
      return false;
   }

   child->parentItem = this;
   this->childItems.insert (position, child);

   return true;
}

//-----------------------------------------------------------------------------
//
bool QEPvLoadSaveItem::removeChildren(int position, int count)
{
   if (position < 0 || position + count > childItems.size ()) {
      return false;
   }

   for (int row = 0; row < count; ++row) {
      delete childItems.takeAt (position);
   }

   return true;
}


//=============================================================================
// Specific, i.e. non-example, functions.
//=============================================================================
//
QEPvLoadSaveItem* QEPvLoadSaveItem::getNamedChild (const QString& searchName)
{
   QEPvLoadSaveItem* result = NULL;
   int r;

   for (r = 0; r < this->childCount(); r++) {
      QEPvLoadSaveItem* child = this->getChild (r);
      if (child->getNodeName () == searchName) {
         // found it.
         //
         result = child;
         break;
      }
   }
   return result;
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveItem* QEPvLoadSaveItem::clone (QEPvLoadSaveItem*)
{
   NOT_IMPLEMENTED;
   return NULL;
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveCommon::PvNameValueMaps QEPvLoadSaveItem::getPvNameValueMap () const
{
   QEPvLoadSaveCommon::PvNameValueMaps result;

   NOT_IMPLEMENTED;

   result.clear ();
   return result;
}

//-----------------------------------------------------------------------------
//
QStringList QEPvLoadSaveItem::getNodePath ()
{
   QStringList result;
   QEPvLoadSaveItem* parentNode;
   QEPvLoadSaveItem* grandParentNode;

   parentNode = this->getParent ();
   if (parentNode) {
      // This object has a parent.
      //
      // Want to exclude the 'internal' tree core node, i.e. start from user root.
      //
      grandParentNode = parentNode->getParent ();
      if (grandParentNode) {
         result = parentNode->getNodePath ();
         result << parentNode->getNodeName ();
      }
   }
   return result;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::setNodeName (const QString& nodeNameIn)
{
   this->nodeName = nodeNameIn;
}

//-----------------------------------------------------------------------------
//
QString QEPvLoadSaveItem::getNodeName () const
{
   return this->nodeName;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::setNodeValue (const QVariant& valueIn)
{
   this->value = valueIn;
}

//-----------------------------------------------------------------------------
//
QVariant QEPvLoadSaveItem::getNodeValue () const
{
   return this->value;
}

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveItem::getElementCount () const
{
   int result;

   if (this->value.type() == QVariant::List) {
      QVariantList vl = this->value.toList ();
      result = vl.size ();
   } else {
      result = 1;
   }
   return result;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::actionConnect (QObject*, const char*)
{
   NOT_IMPLEMENTED;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::extractPVData ()
{
   NOT_IMPLEMENTED;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::applyPVData ()
{
   NOT_IMPLEMENTED;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::readArchiveData (const QCaDateTime&)
{
   NOT_IMPLEMENTED;
}

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveItem::leafCount () const
{
   NOT_IMPLEMENTED;
   return 0;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::appendChild (QEPvLoadSaveItem *child)
{
   // Check for duplicates???
   //
   this->childItems.append (child);

   // Ensure consistency
   //
   child->parentItem = this;
}


//=============================================================================
// Sub class for group
//=============================================================================
//
QEPvLoadSaveGroup::QEPvLoadSaveGroup (const QString& groupName,
                                      QEPvLoadSaveItem* parent) :
   QEPvLoadSaveItem (groupName, nilValue, parent)
{
   // place holder
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveGroup::~QEPvLoadSaveGroup ()
{
   // place holder
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveItem* QEPvLoadSaveGroup::clone (QEPvLoadSaveItem* parent)
{
   QEPvLoadSaveGroup* result = NULL;
   result = new QEPvLoadSaveGroup (this->getNodeName (), parent);

   // Now clone each child.
   //
   for (int j = 0; j < this->childItems.count(); j++) {
      QEPvLoadSaveItem* theChild = this->getChild (j);
      theChild->clone (result);   // dispatching function
   }

   return result;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveGroup::actionConnect (QObject* actionCompleteObject,
                                       const char* actionCompleteSlot)
{
   for (int j = 0; j < this->childItems.count(); j++) {
      QEPvLoadSaveItem* item = this->getChild (j);
      if (item) item->actionConnect (actionCompleteObject, actionCompleteSlot);
   }
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveGroup::extractPVData ()
{
   for (int j = 0; j < this->childItems.count(); j++) {
      QEPvLoadSaveItem* item = this->getChild (j);
      if (item) item->extractPVData ();
   }
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveGroup::applyPVData ()
{
   for (int j = 0; j < this->childItems.count(); j++) {
      QEPvLoadSaveItem* item = this->getChild (j);
      if (item) item->applyPVData ();
   }
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveGroup::readArchiveData (const QCaDateTime& dateTime)
{
   for (int j = 0; j < this->childItems.count(); j++) {
      QEPvLoadSaveItem* item = this->getChild (j);
      if (item) item->readArchiveData (dateTime);
   }
}

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveGroup::leafCount () const
{
   int result = 0;
   for (int j = 0; j < this->childItems.count(); j++) {
      QEPvLoadSaveItem* item = this->getChild (j);
      if (item) result += item->leafCount ();
   }
   return result;
}


//-----------------------------------------------------------------------------
//
QEPvLoadSaveCommon::PvNameValueMaps QEPvLoadSaveGroup::getPvNameValueMap () const
{
   QEPvLoadSaveCommon::PvNameValueMaps result;

   result.clear ();

   // Examine each child.
   //
   for (int j = 0; j < this->childItems.count(); j++) {
      QEPvLoadSaveItem* theChild = this->getChild (j);
      QEPvLoadSaveCommon::PvNameValueMaps childMap;

      // Extract the child's PV Name map and merge into the result.
      //
      childMap = theChild->getPvNameValueMap ();   // dispatching function
      result = QEPvLoadSaveCommon::merge (result, childMap);
   }

   return result;
}


//=============================================================================
// Sub class for leaf
//=============================================================================
//
QEPvLoadSaveLeaf::QEPvLoadSaveLeaf (const QString& setPointPvNameIn,
                                    const QString& readBackPvNameIn,
                                    const QString& archiverPvNameIn,
                                    const QVariant& value,
                                    QEPvLoadSaveItem* parent) :
   QEPvLoadSaveItem (setPointPvNameIn, value, parent)
{
   this->setPointPvName = setPointPvNameIn;
   this->readBackPvName = readBackPvNameIn.isEmpty () ? this->setPointPvName  : readBackPvNameIn;
   this->archiverPvName = archiverPvNameIn.isEmpty () ? this->setPointPvName  : archiverPvNameIn;

   this->qcaSetPoint = NULL;
   this->qcaReadBack = NULL;
   this->archiveAccess = NULL;

   this->setupQCaObjects ();

   // Allow item to retrive archive data values.
   //
   this->archiveAccess = new QEArchiveAccess (this);

   this->connect (this->archiveAccess,
                  SIGNAL (setArchiveData (const QObject*, const bool, const QCaDataPointList&, const QString&, const QString&)),
                  this,
                  SLOT   (setArchiveData (const QObject*, const bool, const QCaDataPointList&, const QString&, const QString&)));
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveLeaf::~QEPvLoadSaveLeaf ()
{
   // place holder
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveItem* QEPvLoadSaveLeaf::clone (QEPvLoadSaveItem* parent)
{
   QEPvLoadSaveLeaf* result = NULL;

   result = new QEPvLoadSaveLeaf (this->getSetPointPvName (),
                                  this->getReadBackPvName (),
                                  this->getArchiverPvName (),
                                  this->getNodeValue (),
                                  parent);
   return result;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::setNodeName (const QString& nodeName)
{
   // Set all PV names the same
   this->setPointPvName = nodeName;
   this->readBackPvName = nodeName;
   this->archiverPvName = nodeName;
   this->setupQCaObjects ();
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::setupQCaObjects ()
{
   QString aggragateName = this->calcNodeName ();
   QEPvLoadSaveItem::setNodeName (aggragateName);  // call parenet function

   // Remove old objects if necessary.
   // Note - may be referecing the same object.
   //
   if (this->qcaSetPoint) delete this->qcaSetPoint;
   if (this->qcaReadBack && (this->qcaReadBack != this->qcaSetPoint)) delete this->qcaReadBack;

   // Allocate new objects. Create only one object if PV names are the same.
   //
   this->qcaSetPoint = new qcaobject::QCaObject (this->getSetPointPvName (), this, 0);
   if (this->getReadBackPvName () == this->getSetPointPvName ()) {
      this->qcaReadBack = this->qcaSetPoint;
   } else {
      this->qcaReadBack = new qcaobject::QCaObject (this->getReadBackPvName (), this, 1);
   }

   // QCaObject does not do this automatically. Maybe it should?.
   //
   this->qcaSetPoint->setParent (this);
   this->qcaReadBack->setParent (this);

   // Create only one object if PV names are the same.
   //
   if (this->getReadBackPvName () == this->getSetPointPvName ()) {
      this->qcaReadBack = this->qcaSetPoint;
   } else {
      // Create separate read back object.
      this->qcaReadBack = new qcaobject::QCaObject (this->getReadBackPvName (), this, 1);
      this->qcaReadBack->setParent (this);
   }

   this->connect (this->qcaReadBack, SIGNAL (dataChanged (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&  )),
                  this,              SLOT   (dataChanged (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&  )));
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::actionConnect (QObject* actionCompleteObject,
                                      const char* actionCompleteSlot)
{
   QObject::connect (this, SIGNAL (reportActionComplete (const QEPvLoadSaveItem*, QEPvLoadSaveCommon::ActionKinds, bool)),
                     actionCompleteObject, actionCompleteSlot);
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::extractPVData ()
{
   if (this->qcaReadBack)  {
      bool status = this->qcaReadBack->singleShotRead ();
      if (!status) {
         emit this->reportActionComplete (this, QEPvLoadSaveCommon::Extract, false);
      }
   } else {
      emit this->reportActionComplete (this, QEPvLoadSaveCommon::Extract, false);
   }
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::applyPVData ()
{
   if (this->qcaSetPoint) {
      bool status = this->qcaSetPoint->writeData (this->value);
      emit this->reportActionComplete (this, QEPvLoadSaveCommon::Apply, status);
   } else {
      emit this->reportActionComplete (this, QEPvLoadSaveCommon::Apply, false);
   }
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::readArchiveData (const QCaDateTime& dateTime)
{
   if (this->archiveAccess) {
      this->archiveAccess->readArchive (this, this->getNodeName (),
                                        dateTime, dateTime, 1,
                                        QEArchiveInterface::Linear, 0);
   } else {
      //
   }
}

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveLeaf::leafCount () const
{
   return 1;  // by definition.
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveCommon::PvNameValueMaps QEPvLoadSaveLeaf::getPvNameValueMap () const
{
   QEPvLoadSaveCommon::PvNameValueMaps result;
   double dval;
   bool okay;

   result.clear ();

   // Can this current value be sensible represtened as a double value??
   //
   dval = this->value .toDouble (&okay);
   if (okay) {
      QString key = this->getSetPointPvName ();
      result.insert (key, dval);
   }
   return result;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::setSetPointPvName (const QString& pvName)
{
   this->setPointPvName = pvName;
   this->setupQCaObjects ();
}

//-----------------------------------------------------------------------------
//
QString QEPvLoadSaveLeaf::getSetPointPvName () const
{
   return this->setPointPvName;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::setReadBackPvName (const QString& pvName)
{
   this->readBackPvName = pvName;
   this->setupQCaObjects ();
}

//-----------------------------------------------------------------------------
//
QString QEPvLoadSaveLeaf::getReadBackPvName () const
{
   return this->readBackPvName;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::setArchiverPvName (const QString& pvName)
{
   this->archiverPvName = pvName;
   this->setupQCaObjects ();
}

//-----------------------------------------------------------------------------
//
QString QEPvLoadSaveLeaf::getArchiverPvName () const
{
   return this->archiverPvName;
}

//-----------------------------------------------------------------------------
// Calcultes a displayable node name.
//
QString QEPvLoadSaveLeaf::calcNodeName () const
{
   QString result;

   if ((this->setPointPvName == this->readBackPvName) &&
       (this->readBackPvName == this->archiverPvName)) {
      // All three names are the same - just use as is.
      //
      result = setPointPvName;

   } else {
      int n =  MIN (MIN (this->setPointPvName.length (),
                         this->readBackPvName.length ()),
                    this->archiverPvName.length ());

      // Find the common, i.e. shared, prefix part of the three PV names.
      //
      int common = 0;
      for (int j = 1; j <= n; j++) {
         if (this->setPointPvName.left (j) != this->readBackPvName.left (j) ||
             this->setPointPvName.left (j) != this->archiverPvName.left (j)) break;
         common = j;
      }

      result = this->setPointPvName.left (common);

      // Extract w, r and a, the PV name specific suffixes.
      // Note: setPointPvName == result + w etc.
      //
      QString label  [3] = { "w", "r", "a" };
      QString suffix [3];

      suffix [0] = this->setPointPvName.right (this->setPointPvName.length() - common);
      suffix [1] = this->readBackPvName.right (this->readBackPvName.length() - common);
      suffix [2] = this->archiverPvName.right (this->archiverPvName.length() - common);

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

      result.append ("{");
      for (int i = 0; i < 2; i++) {
         if (!suffix [i].isEmpty ()) {
            result.append (label [i]).append(":").append (suffix [i]).append(";");
         }
      }
      result.append ("}");
   }

   return result;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::dataChanged (const QVariant& valueIn, QCaAlarmInfo& alarmInfoIn,
                                     QCaDateTime&, const unsigned int&)
{
   this->value = valueIn;
   this->alarmInfo = alarmInfoIn;
   emit this->reportActionComplete (this, QEPvLoadSaveCommon::Extract, true);
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::setArchiveData (const QObject*, const bool okay, const QCaDataPointList& dataPointList,
                                       const QString& /* pvName */ , const QString& /* supplementary */)
{
   // DEBUG << pvName << okay << supplementary;

   if (okay && dataPointList.count() > 0) {
      QCaDataPoint item = dataPointList.value (0);

      this->value = QVariant (item.value);
      this->alarmInfo = item.alarm;

      emit this->reportActionComplete (this, QEPvLoadSaveCommon::ReadArchive, true);
   } else {
      emit this->reportActionComplete (this, QEPvLoadSaveCommon::ReadArchive, false);
   }
}

// end
