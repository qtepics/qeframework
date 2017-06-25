/*  QEMenuButtonItem.h
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
 *  Copyright (c) 2015,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_MENU_BUTTON_ITEM_H
#define QE_MENU_BUTTON_ITEM_H

#include <QAction>
#include <QDataStream>
#include <QMenu>
#include <QList>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <QVariant>
#include <qdom.h>

#include <QEPluginLibrary_global.h>
#include <applicationLauncher.h>
#include <QEActionRequests.h>
#include <QEStringFormatting.h>
#include <QEMenuButtonData.h>

/// This class is based on the TreeItem example specified in:
/// http://qt-project.org/doc/qt-4.8/itemviews-editabletreemodel.html
///
///  Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
///  Contact: http://www.qt-project.org/legal
///
/// Differences: there is no itemData variant array - these values calculated
/// as an when needed.  Also some function name changes such as parent => getParent
/// and some changes just to follow QE prefered style.
///
class QEPLUGINLIBRARYSHARED_EXPORT QEMenuButtonItem
{
public:
   explicit QEMenuButtonItem (const QString& name,
                              const bool isSubMenuContainer,
                              QEMenuButtonItem* parent = NULL);
   ~QEMenuButtonItem ();

   QString getName () const;
   bool getIsSubMenuContainer () const;

   // Sets up class instance by reading the element and
   //
   QDomElement createDomElement (QDomDocument& doc) const;
   bool extractFromDomElement (const QDomElement& element);

   // Only one of these returns an object whilst the other returns NULL.
   // Which function returns an object depends on the type of QEMenuButtonItem
   // type, i.e. as per getIsSubMenuContainer.
   //
   QAction* constructAction (QMenu* parent);
   QMenu*   constructMenu   (QMenu* parent);

   // Model functions
   // There is no insertColumns/removeColumns - number colums is fixed.
   //
   int columnCount () const;                          // fixed at 1
   QEMenuButtonItem* getChild (const int position) const;  // get child at row position
   QEMenuButtonItem* getParent () const;              // was parent in example
   int childCount () const;                           // fixed
   int childPosition () const;                        // own row number
   QVariant getData (const int column) const;         // was data in example

   bool insertChild (const int position, QEMenuButtonItem* child); // insert single child - fixed number columns
   bool removeChildren (const int position, const int count);

private:
   void appendChild (QEMenuButtonItem* theChild);

   typedef QList<QEMenuButtonItem*> QEMenuButtonItemList;
   QEMenuButtonItemList childItems;
   QEMenuButtonItem* parentItem;

public:   // need to be public to allow << and >> operators access
   QString name;
   bool isSubMenuContainer;
   QEMenuButtonData data;   // only applies to non-containers
};

// Conversion between object and data stream encoding.
// Could not get to work as class functions
//
QDataStream& operator<< (QDataStream& stream, QEMenuButtonItem& that);
QDataStream& operator>> (QDataStream& stream, QEMenuButtonItem& that);

#endif // QE_MENU_BUTTON_ITEM_H
