/*  QEArchiveNameSearch.cpp
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
 *  Copyright (c) 2013 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>
#include <QRegExp>

#include <QECommon.h>
#include <QEPvNameSearch.h>

#include "QEArchiveNameSearch.h"

#define DEBUG  qDebug () << "QEArchiveNameSearch::" << __FUNCTION__ << __LINE__


//==============================================================================
//
QEArchiveNameSearch::QEArchiveNameSearch (QWidget* parent) : QEFrame (parent)
{
   this->archiveAccess = new QEArchiveAccess (this);
   this->delayedText = new QEDelayedText (0.25, this);
   this->createInternalWidgets ();

   // Use standard context menu - start with full option set and remove
   // thos items not applicable to the name search widget.
   //
   ContextMenuOptionSets menuSet = contextMenu::defaultMenuSet ();
   menuSet.remove (contextMenu::CM_PASTE);
   menuSet.remove (contextMenu::CM_COPY_DATA);
   menuSet.remove (contextMenu::CM_DRAG_DATA);

   this->setupContextMenu (menuSet);

   QObject::connect (this->lineEdit, SIGNAL  (returnPressed       ()),
                     this,           SLOT    (searchReturnPressed ()));

   // Connect lineEdit to this widget via delayedText.
   //
   this->delayedText->doubleConnect (this->lineEdit, SIGNAL (textEdited (const QString&)),
                                     this,           SLOT   (textEdited (const QString&)));

   QObject::connect (this->listWidget, SIGNAL (itemSelectionChanged ()),
                     this,             SLOT   (itemSelectionChanged ()));
}

//------------------------------------------------------------------------------
//
QEArchiveNameSearch::~QEArchiveNameSearch ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
void QEArchiveNameSearch::search ()
{
   QString searchText;
   QStringList parts;
   QStringList matchingNames;

   searchText = this->lineEdit->text ().trimmed ();

   if (searchText.isEmpty ()) {
      this->listWidget->clear ();
      this->setReadOut ("There are no matching names");
      return;
   }

   // TODO: Replace special reg exp characters (such as '.', '$' and '\' ) with the
   // escaped character sequences.

   // Spilt the patterns into parts.
   //
   parts = searchText.split (QRegExp ("\\s+"), QString::SkipEmptyParts);
   matchingNames.clear ();

   QEPvNameSearch findNames (QEArchiveAccess::getAllPvNames ());

   // Use each part to find a set of matching names, and then merge the list.
   //
   for (int p = 0; p < parts.count (); p++) {
      QString part = parts.value (p);
      QStringList partMatches;

      // QEArchiveAccess ensures the list is sorted.
      // Find nay names containing this string (and ignore case as well).
      //
      partMatches = findNames.getMatchingPvNames (part, Qt::CaseInsensitive);

      // Now nmerge the lists.
      //
      matchingNames.append (partMatches);
      matchingNames.sort ();
      matchingNames.removeDuplicates ();
   }

   // Use names to populate the list.
   //
   this->listWidget->clear ();
   this->listWidget->addItems (matchingNames);

   int n = matchingNames.count ();
   if (n == 1) {
      this->setReadOut ("There is 1 matching name");
   } else {
      this->setReadOut (QString ("There are %1 matching names").arg (n));
   }
}

//------------------------------------------------------------------------------
//
void QEArchiveNameSearch::setReadOut (const QString& text)
{
   message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STATUS);
   this->sendMessage (text, mt);
}

//------------------------------------------------------------------------------
//
void QEArchiveNameSearch::textEdited (const QString&)
{
   this->search ();
}

//------------------------------------------------------------------------------
//
void QEArchiveNameSearch::searchReturnPressed ()
{
   this->search ();
}

//------------------------------------------------------------------------------
//
void  QEArchiveNameSearch::itemSelectionChanged ()
{
   QList<QListWidgetItem*> itemList;
   int n;

   itemList = this->listWidget->selectedItems ();
   n = itemList.count ();
   if (n > 0) {
      this->dragThis->setEnabled (true);
      this->dragThis->setToolTip (" Any selected PV name(s) may be dragged from here ");
   } else {
      this->dragThis->setEnabled (false);
      this->dragThis->setToolTip ("");
   }
   this->setNumberOfContextMenuItems (n);
}

//------------------------------------------------------------------------------
//
QStringList QEArchiveNameSearch::getSelectedNames () const
{
   QList<QListWidgetItem*> itemList;
   QStringList result;
   int n;

   itemList = this->listWidget->selectedItems ();
   n = itemList.count ();
   for (int j = 0; j < n; j++) {
      QListWidgetItem* item = itemList.value (j);
      result.append (item->text ());
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEArchiveNameSearch::clear ()
{
   this->lineEdit->setText ("");
   this->listWidget->clear ();
}

//------------------------------------------------------------------------------
//
QVariant QEArchiveNameSearch::getDrop ()
{
   QVariant result;

   // Can only sensibly drag strings.
   //
   result = QVariant (this->getSelectedNames ().join (" "));
   return result;
}

//------------------------------------------------------------------------------
//
QString QEArchiveNameSearch::copyVariable ()
{
   QString result;

   result = this->getSelectedNames ().join (" ");
   return result;
}

//------------------------------------------------------------------------------
//
QSize QEArchiveNameSearch::sizeHint () const
{
   return QSize (700, 260);
}

//------------------------------------------------------------------------------
//
void QEArchiveNameSearch::createInternalWidgets ()
{
   this->setMinimumSize (512, 212);

   this->verticalLayout = new QVBoxLayout (this);
   this->verticalLayout->setSpacing (4);
   this->verticalLayout->setContentsMargins (2, 4, 2, 2);

   this->searchFrame = new QFrame (this);
   this->searchFrame->setMinimumSize (QSize(0, 40));
   this->searchFrame->setFrameShape (QFrame::StyledPanel);
   this->searchFrame->setFrameShadow (QFrame::Raised);

   this->horizontalLayout = new QHBoxLayout (this->searchFrame);
   this->horizontalLayout->setSpacing (8);
   this->horizontalLayout->setContentsMargins (6, 4, 6, 4);

   this->dragThis = new QLabel (this->searchFrame);
   this->dragThis->setMinimumSize (24, 24);
   this->dragThis->setMaximumSize (24, 24);
   QPixmap icon (":/qe/archive/drag_icon.png");
   this->dragThis->setPixmap (icon);
   this->dragThis->setScaledContents (true);
   this->dragThis->setEnabled (false);

   this->horizontalLayout->addWidget (this->dragThis);

   this->lineEdit = new QLineEdit (this->searchFrame);
   this->lineEdit->setToolTip ("Enter partial PV names(s) and press return");

   this->horizontalLayout->addWidget (this->lineEdit);

   this->verticalLayout->addWidget (this->searchFrame);

   this->listWidget = new QListWidget (this);
   this->listWidget->setMinimumSize (QSize (500, 156));
   this->listWidget->setMaximumSize (QSize (1000, 16777215));

   QFont font;
   font.setFamily (QString::fromUtf8 ("Monospace"));

   this->listWidget->setFont (font);
   this->listWidget->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
   this->listWidget->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
   this->listWidget->setSelectionMode (QAbstractItemView::ExtendedSelection);
   this->listWidget->setUniformItemSizes (true);

   this->verticalLayout->addWidget (listWidget);

   this->listWidget->setCurrentRow (-1);
}

// end
