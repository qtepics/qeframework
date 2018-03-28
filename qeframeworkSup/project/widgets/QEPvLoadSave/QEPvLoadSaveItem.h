/*  QEPvLoadSaveItem.h
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
 *  Copyright (C) 2013,2016,2017,2018 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_PV_LOAD_SAVE_ITEM_H
#define QE_PV_LOAD_SAVE_ITEM_H

#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTreeView>
#include <QVariant>
#include <QVariantList>
#include <QModelIndex>

#include <Generic.h>
#include <QCaObject.h>
#include <QCaDataPoint.h>
#include <QEArchiveManager.h>
#include <QEPvLoadSaveCommon.h>

/// This class is based on the TreeItem example specified in:
/// http://qt-project.org/doc/qt-4.8/itemviews-editabletreemodel.html
///
///  Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
///  Contact: http://www.qt-project.org/legal
///
/// A major difference is that it is derived from QObject because each leaf item
/// is associated with a PV and needs slots to receive value data. A consequence
/// of this is that this class must be exposed in a header file even though it is
/// essentially a QEPvLoadSave private class. Also there is no itemData variant
/// array - these values calculated as an when needed.  Lastly some function name
/// changes such as parent => getParent (as parent already inherited from QObject)
/// and some changes just to follow my prefered style.
///
/// Note: although QObjects provide a parent/child linking, this class manages it's
/// own parent child relationships as per the TreeItem example.
///
/// QEPvLoadSaveItem are created in one of two flavours:
/// a/ node - used for groups
/// b/ leaf - used for PVs.
/// both of which inherited from the QEPvLoadSaveItem base class
///
class QEPvLoadSaveItem : public QObject {
   Q_OBJECT
protected:
   // This is an effectivly an abstract class; by making this function protected
   // we stop QEPvLoadSaveItem class objects being created.
   //
   explicit QEPvLoadSaveItem (const QString& nodeName,
                              const QVariant& value,
                              QEPvLoadSaveItem* parent = 0);
public:
   virtual ~QEPvLoadSaveItem ();

   int columnCount () const;

   QEPvLoadSaveItem* getChild (int position) const;   // get child at row position
   QEPvLoadSaveItem* getParent () const;              // was parent in example
   int childCount () const;                           // fixed
   int childPosition () const;                        // own row number
   virtual QVariant getData (int column) const;       // was data in example
   bool insertChild (int position, QEPvLoadSaveItem* getChild); // insert single child - fixed number columns
   bool removeChildren(int position, int count);

   // There is no insertColumns/removeColumns - number colums is fixed.
   //
   void appendChild (QEPvLoadSaveItem *getChild);

   // Own functions, i.e. not from the example.
   //
   QEPvLoadSaveItem* getNamedChild (const QString& searchName);    // get child with Node name

   // Clones a QEPvLoadSaveItem and all its children if applicable.
   // Does not copy the actionConnect state, which must be done post construction
   // just like the original. Note assigned a the specifed parent.
   //
   // NOTE: child classes must override this - this function as is always returns NULL.
   //
   virtual QEPvLoadSaveItem* clone (QEPvLoadSaveItem* parent);

   // Set own model index - used for data changed signals.
   //
   // void setModelIndex (const QModelIndex& index);
   // QModelIndex getModelIndex ();
   //
   virtual void actionConnect (QObject* actionCompleteObject,
                               const char* actionCompleteSlot,
                               const char* actionInCompleteSlot);

   QStringList getNodePath ();

   virtual void setNodeName (const QString& nodeName);
   QString getNodeName () const;

   void setNodeValue (const QVariant& value);
   QVariant getNodeValue () const;
   int getElementCount () const;   // get the number of elements in value.

   // NOTE: child classes must override these functions
   //
   virtual bool getIsPV () const { return false; }
   virtual bool getIsGroup () const { return false; }

   // Called prior to readArchiveData.
   //
   static void initReadArchiveData ();

   // If this is a leaf (PV) item then performs action on associated qca channel.
   // If this a group item then command is re-issued to each child.
   //
   virtual void extractPVData ();
   virtual void applyPVData ();
   virtual void readArchiveData (const QCaDateTime& dateTime);
   virtual void abortAction ();

   // Count of number of PV leaf items at or below this node.
   // (As opposed to childCount which is number of direct children).
   //
   virtual int leafCount () const;

   // NOTE: child classes must override this - this function as is always
   // returns an empty map.
   //
   virtual QEPvLoadSaveCommon::PvNameValueMaps getPvNameValueMap () const;

protected:
   // We keep and maintain a separate list of QEPvLoadSaveItem children, as
   // opposed to using the QObject children mechanism, as QEPvLoadSaveItem
   // objects have other children such as a qcaobject::QCaObject.
   //
   QList<QEPvLoadSaveItem*> childItems;
   QEPvLoadSaveItem* parentItem;
   QVariant value;          // snap shot value
   QVariant liveValue;      // loaf only

   // The itemData created dynamically from these members.
   //
   QString nodeName;     // alias for first item in itemData

   // Archive request count - used to determine request stagger wrt time.
   //
   static int readArchiveCount;
};


//------------------------------------------------------------------------------
// Sub class for group
//
class QEPvLoadSaveGroup : public QEPvLoadSaveItem {
   Q_OBJECT
public:
   explicit QEPvLoadSaveGroup (const QString& groupName,
                               QEPvLoadSaveItem* parent = 0);
   ~QEPvLoadSaveGroup ();

   // Override QEPvLoadSaveItem functions.
   //
   QVariant getData (int column) const;
   bool getIsGroup () const { return true; }
   QEPvLoadSaveItem* clone (QEPvLoadSaveItem* parent);
   void actionConnect (QObject* actionCompleteObject,
                       const char* actionCompleteSlot,
                       const char* actionInCompleteSlot);
   void extractPVData ();
   void applyPVData ();
   void readArchiveData (const QCaDateTime& dateTime);
   void abortAction ();
   int leafCount () const;

   QEPvLoadSaveCommon::PvNameValueMaps getPvNameValueMap () const;
};


//------------------------------------------------------------------------------
// Sub class for PV leaf
//
class QEPvLoadSaveLeaf : public QEPvLoadSaveItem {
   Q_OBJECT
public:
   explicit QEPvLoadSaveLeaf (const QString& setPointPvName,
                              const QString& readBackPvName,    // "" implies setPointPvName
                              const QString& archiverPvName,    // "" implies setPointPvName
                              const QVariant& value,            // initial value
                              QEPvLoadSaveItem* parent = 0);
   ~QEPvLoadSaveLeaf ();

   // The class can support multiple PVs used for writing, reading and archive access.
   // E.g.  MOTOR.VAL for set point PV name and MOTOR.RBV for read back PV name.
   //
   void setSetPointPvName (const QString& pvName);
   QString getSetPointPvName () const;

   void setReadBackPvName (const QString& pvName);  // "" implies setPointPvName
   QString getReadBackPvName () const;

   void setArchiverPvName (const QString& pvName);  // "" implies setPointPvName
   QString getArchiverPvName () const;

   // Override QEPvLoadSaveItem functions.
   //
   QVariant getData (int column) const;
   void setNodeName (const QString& nodeName);
   bool getIsPV () const { return true; }
   QEPvLoadSaveItem* clone (QEPvLoadSaveItem* parent);
   QEPvLoadSaveCommon::PvNameValueMaps getPvNameValueMap () const;
   void actionConnect (QObject* actionCompleteObject,
                       const char* actionCompleteSlot,
                       const char* actionInCompleteSlot);
   void extractPVData ();
   void applyPVData ();
   void readArchiveData (const QCaDateTime& dateTime);
   void abortAction ();
   int leafCount () const;

signals:
   // Used for progress bar on main form.
   //
   void reportActionComplete (const QEPvLoadSaveItem* item,
                              const QEPvLoadSaveCommon::ActionKinds action,
                              const bool actionSuccessful);

   // Used when outstanding actions are aborted on main form.
   //
   void reportActionInComplete (const QEPvLoadSaveItem* item,
                                const QEPvLoadSaveCommon::ActionKinds action);

private:
   QString calcNodeName () const;  // Merges three PV names into a single node name.
   void setupQCaObjects ();        // Create/updates internal QCaObjects

   // Convert to PV item data to variant best matching the channels generic data
   // type, and hence best matching the PV server's native field type.
   //
   static QVariant convertToNativeType (const generic::generic_types gdt, const QVariant& from);

   // Conveniance function
   //
   void emitReportActionComplete (const bool actionSuccessful);

   QString setPointPvName;
   QString readBackPvName;
   QString archiverPvName;

   qcaobject::QCaObject* qcaSetPoint;
   qcaobject::QCaObject* qcaReadBack;
   QEArchiveAccess* archiveAccess;
   QCaAlarmInfo alarmInfo;
   QEPvLoadSaveCommon::ActionKinds action;
   bool actionIsComplete;
   QCaDateTime readArchiveDateTime;

private slots:
   void dataChanged (const QVariant& value, QCaAlarmInfo& alarmInfo,
                     QCaDateTime& timeStamp, const unsigned int& variableIndex);

   void setArchiveData (const QObject* userData, const bool okay,
                        const QCaDataPointList& archiveData,
                        const QString& pvName, const QString& supplementary);

   void delayedReadArchiveData ();
};

#endif    // QE_PV_LOAD_SAVE_ITEM_H
