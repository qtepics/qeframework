/*  QEPvLoadSaveItem.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2020 Australian Synchrotron
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

#include "QEPvLoadSaveItem.h"
#include "QEPvLoadSave.h"
#include <QEPvLoadSaveUtilities.h>

#include <QDebug>
#include <QFrame>
#include <QModelIndex>
#include <QPushButton>
#include <QWidget>
#include <QECommon.h>
#include <QEVectorVariants.h>

#define DEBUG  qDebug () << "QEPvLoadSaveItem" << __LINE__ << __FUNCTION__ << "  "

// Used by virtual functions in abstract base class that need to be overriden.
//
#define NOT_OVERRIDDEN                                               \
   DEBUG << "Class: " << this->metaObject ()->className ()           \
   << " did not overide " << __FUNCTION__ << "() function"


static const QVariant nilValue (QVariant::Invalid);

// static protected
int QEPvLoadSaveItem::readArchiveCount = 0;

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
   // Number of cols fixed at 1.
   // Could split single "Name = Value" into pair ("Name, "Value")
   return QEPvLoadSaveCommon::NUMBER_OF_COLUMNS;
}

//-----------------------------------------------------------------------------
//
QVariant QEPvLoadSaveItem::getData (int /* column */) const
{
   NOT_OVERRIDDEN;
   QVariant result;
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

   for (int r = 0; r < this->childCount(); r++) {
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
   NOT_OVERRIDDEN;
   return NULL;
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveCommon::PvNameValueMaps QEPvLoadSaveItem::getPvNameValueMap () const
{
   QEPvLoadSaveCommon::PvNameValueMaps result;

   NOT_OVERRIDDEN;

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
void QEPvLoadSaveItem::actionConnect (QObject*, const char*, const char*, const char*)
{
   NOT_OVERRIDDEN;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::extractPVData ()
{
   NOT_OVERRIDDEN;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::applyPVData ()
{
   NOT_OVERRIDDEN;
}

//-----------------------------------------------------------------------------
// static
void QEPvLoadSaveItem::initReadArchiveData ()
{
   QEPvLoadSaveItem::readArchiveCount = 0;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::readArchiveData (const QCaDateTime&)
{
   NOT_OVERRIDDEN;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveItem::abortAction ()
{
   NOT_OVERRIDDEN;
}

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveItem::leafCount () const
{
   NOT_OVERRIDDEN;
   return 0;   // avoid compiler warning
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveCommon::StatusSummary QEPvLoadSaveItem::getStatusSummary () const
{
   NOT_OVERRIDDEN;
   QEPvLoadSaveCommon::StatusSummary result;
   QEPvLoadSaveCommon::clear (result);
   return result;   // avoid compiler warning
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
QVariant QEPvLoadSaveGroup::getData (int column) const
{
   const QEPvLoadSaveCommon::ColumnKinds kind = QEPvLoadSaveCommon::ColumnKinds (column);
   QVariant result;
   QEPvLoadSaveCommon::StatusSummary summary;

   switch (kind) {
      case QEPvLoadSaveCommon::NodeName:
         result.setValue (this->nodeName);
         break;

      case QEPvLoadSaveCommon::LoadSave:
      case QEPvLoadSaveCommon::Live:
         // Groups don't have live or delta values.
         result.setValue (QString (""));
         break;

      case QEPvLoadSaveCommon::Delta:
         summary = this->getStatusSummary();
         result.setValue (QString ("(%1, %2, %3)")
                          .arg(summary.isEqualCount)
                          .arg(summary.isNotEqualCount)
                          .arg(summary.isNotAplicableCount));
         break;

      default:
         result.setValue (QString ("error"));
         break;
   }
   return result;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveGroup::actionConnect (QObject* actionCompleteObject,
                                       const char* actionSetReadOutSlot,
                                       const char* actionCompleteSlot,
                                       const char* actionInCompleteSlot)
{
   for (int j = 0; j < this->childItems.count(); j++) {
      QEPvLoadSaveItem* item = this->getChild (j);
      if (item) item->actionConnect (actionCompleteObject, actionSetReadOutSlot,
                                     actionCompleteSlot, actionInCompleteSlot);
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
void QEPvLoadSaveGroup::abortAction ()
{
   for (int j = 0; j < this->childItems.count(); j++) {
      QEPvLoadSaveItem* item = this->getChild (j);
      if (item) item->abortAction ();
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
QEPvLoadSaveCommon::StatusSummary QEPvLoadSaveGroup::getStatusSummary () const
{
   QEPvLoadSaveCommon::StatusSummary result;
   QEPvLoadSaveCommon::clear (result);

   for (int j = 0; j < this->childItems.count(); j++) {
      QEPvLoadSaveItem* item = this->getChild (j);
      if (item) {
         result = QEPvLoadSaveCommon::merge (result, item->getStatusSummary ());
      }
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
   this->actionIsComplete = true;

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
QVariant QEPvLoadSaveLeaf::getData (int column) const
{
   const QEPvLoadSaveCommon::ColumnKinds kind = QEPvLoadSaveCommon::ColumnKinds (column);
   QVariant result;
   QString valueImage;

   switch (kind) {
      case QEPvLoadSaveCommon::NodeName:
         result.setValue (this->nodeName);
         break;

      case QEPvLoadSaveCommon::LoadSave:
         if (this->value.type() == QVariant::List) {
            QVariantList vl = this->value.toList ();
            valueImage = QString (" << %1 element array >>").arg (vl.size ());

         } else if (QEVectorVariants::isVectorVariant(this->value)) {
            int n = QEVectorVariants::vectorCount (this->value);
            valueImage = QString (" << %1 element vector >>").arg (n);

         } else {
            valueImage = this->value.toString ();
         }

         result.setValue  (valueImage);
         break;

      case QEPvLoadSaveCommon::Live:
         if (this->liveValue.type() == QVariant::List) {
            QVariantList vl = this->value.toList ();
            valueImage = QString (" << %1 element array >>").arg (vl.size ());

         } else if (QEVectorVariants::isVectorVariant(this->liveValue)) {
            int n = QEVectorVariants::vectorCount (this->liveValue);
            valueImage = QString (" << %1 element vector >>").arg (n);

         } else {
            valueImage = this->liveValue.toString ();
         }

         result.setValue  (valueImage);
         break;

      case QEPvLoadSaveCommon::Delta:
         if ((this->liveValue.type() != QVariant::Invalid) &&
             (this->value.type() != QVariant::Invalid))
         {
            // Both values are defined.
            //
            bool ok1, ok2;
            double diff = this->liveValue.toDouble (&ok1) - this->value.toDouble (&ok2);
            if (ok1 && ok2) {
               // Numerical value - we can calculate a numeric difference.
               //
               result.setValue (diff);
            } else if (this->liveValue == this->value) {
               result.setValue (QString ("identical"));
            } else {
               result.setValue (QString ("different"));
            }
         } else {
            result.setValue (QString ("n/a"));
         }
         break;

      default:
         result.setValue (QString ("error"));
         break;
   }

   return result;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::setNodeName (const QString& nodeName)
{
   QString setPoint;
   QString readBack;
   QString archiver;
   bool okay;
   
   okay = QEPvLoadSaveUtilities::splitPvNames (nodeName, setPoint, readBack, archiver);
   if (okay) {
      this->setPointPvName = setPoint;
      this->readBackPvName = readBack;
      this->archiverPvName = archiver;
      this->action = QEPvLoadSaveCommon::NullAction;
      this->setupQCaObjects ();
   } else {
      emit this->setReadOut ("failed to parse: " + nodeName);
   }
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::setupQCaObjects ()
{
   QString aggragateName = this->calcNodeName ();
   QEPvLoadSaveItem::setNodeName (aggragateName);  // call parenet function

   // Remove old objects if necessary.
   // Note - we keep separate and distinct read and write objects.
   //
   if (this->qcaSetPoint) {
      delete this->qcaSetPoint;
      this->qcaSetPoint = NULL;
   }

   if (this->qcaReadBack) {
      delete this->qcaReadBack;
      this->qcaReadBack = NULL;
   }

   // Allocate new objects.
   //
   this->qcaSetPoint = new qcaobject::QCaObject (this->getSetPointPvName (), this, 0);
   this->qcaReadBack = new qcaobject::QCaObject (this->getReadBackPvName (), this, 1);

   // QCaObject does not do this automatically. Maybe it should?.
   //
   this->qcaSetPoint->setParent (this);
   this->qcaReadBack->setParent (this);

   // For the set point - we must read once to get the meta data to enable good writes.
   // For readback - we subscribe.
   //
   this->qcaSetPoint->singleShotRead ();
   this->qcaReadBack->subscribe();

   // For the read back - no read yet, but do set up the connection.
   //
   this->connect (this->qcaReadBack, SIGNAL (dataChanged (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&  )),
                  this,              SLOT   (dataChanged (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&  )));
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::actionConnect (QObject* actionCompleteObject,
                                      const char* actionSetReadOutSlot,
                                      const char* actionCompleteSlot,
                                      const char* actionInCompleteSlot)
{
   QObject::connect (this, SIGNAL (setReadOut (const QString&)),
                     actionCompleteObject, actionSetReadOutSlot);

   QObject::connect (this, SIGNAL (reportActionComplete (const QEPvLoadSaveItem*, QEPvLoadSaveCommon::ActionKinds, bool)),
                     actionCompleteObject, actionCompleteSlot);

   QObject::connect (this, SIGNAL (reportActionInComplete (const QEPvLoadSaveItem*, QEPvLoadSaveCommon::ActionKinds)),
                     actionCompleteObject, actionInCompleteSlot);
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::extractPVData ()
{
   this->action = QEPvLoadSaveCommon::Extract;
   bool okay = false;

   if (this->qcaReadBack && this->qcaReadBack->getDataIsAvailable()) {
      this->value = this->liveValue;
      okay = true;
   }

   this->actionIsComplete = okay;
   this->emitReportActionComplete (okay);
}

//-----------------------------------------------------------------------------
// private static
//
QVariant QEPvLoadSaveLeaf::convertToNativeType (const generic::generic_types gdt, const QVariant& from)
{
   QVariant result;
   bool okay;

   switch (gdt) {
      case generic::GENERIC_STRING:
         result = QVariant (from.toString ());
         okay = true;
         break;

      case generic::GENERIC_SHORT:
         result = QVariant (from.toInt (&okay));
         break;

      case generic::GENERIC_UNSIGNED_SHORT:
      case generic::GENERIC_UNSIGNED_CHAR:
         result = QVariant (from.toUInt (&okay));
         break;

      case generic::GENERIC_LONG:
         result = QVariant (from.toLongLong (&okay));
         break;

      case generic::GENERIC_UNSIGNED_LONG :
         result = QVariant (from.toULongLong (&okay));
         break;

      case generic::GENERIC_FLOAT:
      case generic::GENERIC_DOUBLE:
         result = QVariant (from.toDouble (&okay));
         break;

      case generic::GENERIC_UNKNOWN:
         result = QVariant (from.toString ());
         okay = true;
         break;

      default:
         result = from;
         okay = false;
   }

   if (!okay) {
      DEBUG << "convert failed " << generic::typeImage (gdt) << from;
   }
   return result;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::applyPVData ()
{
   this->action = QEPvLoadSaveCommon::Apply;
   this->actionIsComplete = false;

   if (this->qcaSetPoint && this->qcaSetPoint->getChannelIsConnected ()) {

      generic::generic_types gdt = this->qcaSetPoint->getDataType ();

      // Convert value, or each element if needs be, to the native PV type
      // before writing to the PV server.
      //
      QVariant nativeValue;

      if (this->value.type() == QVariant::List) {
         QVariantList valueList = this->value.toList ();
         const int n = valueList.count ();
         QVariantList nativeList;
         for (int j = 0; j < n; j++) {
            nativeList.append (this->convertToNativeType (gdt, valueList.value (j)));
         }
         nativeValue = QVariant (nativeList);

      } else if (QEVectorVariants::isVectorVariant (this->value)) {

         nativeValue = this->value;

      } else {
         // Scalar - Just use value as is, and let EPICS IOC do any required conversion.
         nativeValue = this->value;
      }

      bool status = this->qcaSetPoint->writeData (nativeValue);
      this->emitReportActionComplete (status);
   } else {
      this->emitReportActionComplete (false);
   }
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::readArchiveData (const QCaDateTime& dateTime)
{
   this->action = QEPvLoadSaveCommon::ReadArchive;
   this->actionIsComplete = false;
   this->readArchiveDateTime = dateTime;   // save parameter - used by delayedReadArchiveData

   const int n = QEPvLoadSaveLeaf::readArchiveCount++;   // increment count

   // Delay each read by 10mS. This is particularly important when extracting
   // a large number of values from the archiver, say 1000 or more. This not
   // only spreads the load, but allows time for the main thread to process
   // in-comming events.
   //
   QTimer::singleShot (10*n, this, SLOT (delayedReadArchiveData ()));
}

//-----------------------------------------------------------------------------
// slot
void QEPvLoadSaveLeaf::delayedReadArchiveData ()
{
   if (this->archiveAccess) {
      this->archiveAccess->readArchive (this, this->getNodeName (),
                                        this->readArchiveDateTime,
                                        this->readArchiveDateTime, 1,
                                        QEArchiveInterface::Linear, 0);
   } else {
      this->emitReportActionComplete (false);
   }
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::abortAction ()
{
   if (!this->actionIsComplete) {
      emit this->reportActionInComplete (this, this->action);
   }
   this->actionIsComplete = true;
}

//-----------------------------------------------------------------------------
//
int QEPvLoadSaveLeaf::leafCount () const
{
   return 1;  // by definition.
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveCommon::StatusSummary QEPvLoadSaveLeaf::getStatusSummary () const
{
   QEPvLoadSaveCommon::StatusSummary result;
   QEPvLoadSaveCommon::clear (result);    // set all zero

   if ((this->liveValue.type() != QVariant::Invalid) &&
       (this->value.type() != QVariant::Invalid))
   {
      // Both values are defined.
      //
      if (this->liveValue == this->value) {
         result.isEqualCount = 1;
      } else {
         result.isNotEqualCount = 1;
      }
   } else {
      result.isNotAplicableCount = 1;
   }

   return result;
}

//-----------------------------------------------------------------------------
//
QEPvLoadSaveCommon::PvNameValueMaps QEPvLoadSaveLeaf::getPvNameValueMap () const
{
   QEPvLoadSaveCommon::PvNameValueMaps result;
   double dval;
   bool okay;

   result.clear ();

   // Can this current value be sensible represented as a double value??
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
// Calculates a displayable node name.  Note: the content of this function has
// been relocated to QEPvLoadSaveUtilities::mergePvNames.
//
QString QEPvLoadSaveLeaf::calcNodeName () const
{
   QString result;

   result = QEPvLoadSaveUtilities::mergePvNames (this->setPointPvName,
                                                 this->readBackPvName,
                                                 this->archiverPvName);

   return result;
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::dataChanged (const QVariant& valueIn, QCaAlarmInfo& alarmInfoIn,
                                    QCaDateTime&, const unsigned int&)
{
   // Must treat enumerations as strings.
   //
   const QStringList enums = this->qcaReadBack->getEnumerations ();
   const int n = enums.count ();
   if (n > 0) {
      bool okay;
      const int index = valueIn.toInt (&okay);
      if (okay && (index >= 0) && (index < n)) {
         this->liveValue = enums.value (index);
      } else {
         this->liveValue = valueIn;
      }
   } else {
      this->liveValue = valueIn;
   }
   this->alarmInfo = alarmInfoIn;

   emit this->reportActionComplete (this, QEPvLoadSaveCommon::Update, true);
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::setArchiveData (const QObject*, const bool okay,
                                       const QCaDataPointList& dataPointList,
                                       const QString&, const QString&)
{
   if (okay && dataPointList.count() > 0) {
      QCaDataPoint item = dataPointList.value (0);

      this->value = QVariant (item.value);
      this->alarmInfo = item.alarm;

      this->emitReportActionComplete (true);
   } else {
      this->emitReportActionComplete (false);
   }
}

//-----------------------------------------------------------------------------
//
void QEPvLoadSaveLeaf::emitReportActionComplete (const bool actionSuccessful)
{
   // Only successfull actions are deemed complete
   //
   if (actionSuccessful) this->actionIsComplete = true;
   emit this->reportActionComplete (this, this->action, actionSuccessful);
}

// end
