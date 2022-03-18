/*  QEMenuButtonModel.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2015-2022 Australian Synchrotron
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

#ifndef QE_MENU_BUTTON_MODEL_H
#define QE_MENU_BUTTON_MODEL_H

#include <QObject>
#include <QMenu>
#include <QString>
#include <QVariant>

#include <QEFrameworkLibraryGlobal.h>
#include <QAbstractItemModel>

// The model is serialsed as a string so that is can be stored as a string
// within the ui file containing the QEButton.
// The string format is XML, specifically is consistant with that use
// by the customisation file. Hopefully we can leverage off the comminality.
//
// Format: // much duplicated from windowCustomisation.
//
// <MenuButton version="1">
//
//   <Item Name="Search" Separator="False">
//       <Program>
//          <Name>firefox</Name>
//          <Arguments>www.google.com</Arguments>
//          <Start_Option>Terminal</Start_Option>
//       </Program>
//   </Item>
//
//   <Item Name="ROI 2">
//       <Window>
//           <UiFile>ROI.ui</UiFile>
//           <MacroSubstitutions>REGION=2</MacroSubstitutions>
//           <Customisation>ROI</Customisation>
//           <Create_Option>NewWindow</Create_Option>
//       </Window>
//   </Item>
//
//   <Item Name="Reset">
//       <Variable>
//          <Name>SR17ID02FST01:RESET_CMD<Name>
//          <Value>1</Value>
//          <Format>Integer</Format>
//       </Variable>
//       <Separator/>
//   </Item>
//
//
//   <Item Name="Combo">
//       <Separator/>
//       <Program> ... </Program>
//       <Window> ... </Window>
//       <Variable> ... </Variable>
//   </Item>
//
//   <Menu Name="Submenu">
//      <Separator/>
//      <Item ...>
//      </Item>
//      <Item ...>
//      </Item>
//      <Menu Name="Sub-submenu">
//         <Item ...>
//         </Item>
//      </Menu>
//   </Menu>
//
// </MenuButton>
//

// Differed declaration - avoids mutual header inclusions.
//
class QEMenuButton;
class QEMenuButtonItem;

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEMenuButtonModel : public QAbstractItemModel
{
   Q_OBJECT
public:
   explicit QEMenuButtonModel (QEMenuButton* parent);
   ~QEMenuButtonModel ();

   bool parseXml (const QString& xml);  // create model from XML string
   QString serialiseXml () const;       // serialise model into XML string.
   static QString emptyXml ();          // quazi serialise empty model (without actually crateing a model)

   bool constructMenu (QMenu* rootMenu);  // constuct menu hierarchy from model.

   // If index is invalid, then returns null.
   // This is like getItem () but for external use.
   // It returns NULL as opposed to ref to core/root item
   //
   QEMenuButtonItem* indexToItem (const QModelIndex& index) const;
   QModelIndex getIndex (const QEMenuButtonItem* item) const;

   bool addItemToModel (QEMenuButtonItem* item,
                        QEMenuButtonItem* parentItem,
                        const int position = -1);   // -1 means at end
   bool removeItemFromModel (QEMenuButtonItem* item);

   void modelUpdated ();
   void itemUpdated (const QEMenuButtonItem* item);

protected:
   // QAbstractItemModel demands that thse are implemented.
   // Override (pure abstract) virtual functions.
   //
   QVariant data       (const QModelIndex& index, int role) const;
   QVariant headerData (int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

   QModelIndex index   (int row, int column,
                        const QModelIndex& parent = QModelIndex()) const;
   QModelIndex parent  (const QModelIndex& child) const;

   int rowCount        (const QModelIndex& parent = QModelIndex()) const;
   int columnCount     (const QModelIndex& parent = QModelIndex()) const;

   // Read-only tree models only need to provide the above functions.
   // The following public functions provide support for editing and resizing.
   //
   Qt::ItemFlags flags (const QModelIndex &index) const;

   bool setData  (const QModelIndex &index, const QVariant &value,
                  int role = Qt::EditRole);
   bool setHeaderData  (int section, Qt::Orientation orientation,
                        const QVariant &value, int role = Qt::EditRole);
   bool insertRows     (int position, int rows,
                        const QModelIndex& parent = QModelIndex());
   bool removeRows     (int position, int rows,
                        const QModelIndex& parent = QModelIndex());

   // We do not override insertColumns/removeColumns - this is fixed

   // Allow drag/drop
   //
   Qt::DropActions supportedDropActions() const;
   QStringList mimeTypes() const;
   QMimeData* mimeData (const QModelIndexList& indexes) const;
   bool dropMimeData (const QMimeData* data, Qt::DropAction action,
                      int row, int column, const QModelIndex& parent);

private:
   // The model index associated with the core item is an invalid index.
   // (as per the URL ref).
   //
   QModelIndex getCoreIndex () const { return QModelIndex (); }

   // Like indexToItem but returns coreItem if index is invalid.
   //
   QEMenuButtonItem* getItem (const QModelIndex &index) const;

   QEMenuButtonItem* coreItem;  // the tree view root item - must exist - provides headings
   QString heading;

   QEMenuButtonItem* requestedInsertItem;     //
   QEMenuButton* owner;
};


#endif // QE_MENU_BUTTON_MODEL_H
