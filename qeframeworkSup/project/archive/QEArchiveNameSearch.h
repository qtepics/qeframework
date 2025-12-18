/*  QEArchiveNameSearch.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_ARCHIVE_NAME_SEARCH_H
#define QE_ARCHIVE_NAME_SEARCH_H

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

#endif  // QE_ARCHIVE_NAME_SEARCH_H
