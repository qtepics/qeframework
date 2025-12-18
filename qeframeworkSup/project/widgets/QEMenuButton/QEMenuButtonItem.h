/*  QEMenuButtonItem.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2015-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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

#include <QEFrameworkLibraryGlobal.h>
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

// Differed declaration - avoids mutual header inclusions.
//
class QEMenuButton;

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEMenuButtonItem
{
public:
   explicit QEMenuButtonItem (const QString& name,
                              const bool isSubMenuContainer,
                              QEMenuButton* owner,
                              QEMenuButtonItem* parent);
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
   QString getSubstitutedName () const;

   typedef QList<QEMenuButtonItem*> QEMenuButtonItemList;
   QEMenuButtonItemList childItems;
   QEMenuButtonItem* parentItem;

public:   // need to be public to allow << and >> operators access
   QEMenuButton* owner;
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
