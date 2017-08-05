/*  QEArchiveNameSearch.h
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

#ifndef QEARCHIVENAMESEARCH_H
#define QEARCHIVENAMESEARCH_H

#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QWidget>

#include <QEFrame.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QEDelayedText.h>

#include <QEArchiveManager.h>


/// This is a non EPICS aware widget.
/// It provides a simple user means to find archived PV names.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEArchiveNameSearch : public QEFrame {
  Q_OBJECT
public:
   explicit QEArchiveNameSearch (QWidget* parent = 0);
   ~QEArchiveNameSearch ();

   QStringList getSelectedNames () const;
   void clear ();

protected:
   QSize sizeHint () const;

   // Drag, no drop
   //
   void mousePressEvent (QMouseEvent *event)    { qcaMousePressEvent (event ); }
   QVariant getDrop();

   // Copy, no paste
   //
   QString copyVariable ();  // Function a widget may implement to perform a 'copy variable' operation

private:
   void createInternalWidgets ();
   void search ();
   void setReadOut (const QString& text);

   QEArchiveAccess *archiveAccess;
   QEDelayedText* delayedText;

   // Internal widgets.
   //
   QVBoxLayout *verticalLayout;
   QFrame *searchFrame;
   QHBoxLayout *horizontalLayout;
   QLabel* dragThis;
   QLineEdit *lineEdit;
   QListWidget *listWidget;

private slots:
   void textEdited (const QString &);
   void searchReturnPressed ();
   void itemSelectionChanged ();
};

#endif  // QEARCHIVENAMESEARCH_H 
