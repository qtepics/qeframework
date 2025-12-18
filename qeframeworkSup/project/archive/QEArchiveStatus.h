/*  QEArchiveStatus.h
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

#ifndef QE_ARCHIVE_STATUS_H
#define QE_ARCHIVE_STATUS_H

#include <QObject>
#include <QWidget>
#include <QEGroupBox.h>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QEFrameworkLibraryGlobal.h>
#include <QEArchiveManager.h>

/// This is a non EPICS aware widget.
/// It extracts and displays states information from the archive manager.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEArchiveStatus : public QEGroupBox {
  Q_OBJECT
public:
   explicit QEArchiveStatus (QWidget* parent = 0);
   ~QEArchiveStatus ();
   QSize sizeHint () const;

public slots:
   void reReadAvailablePVs ();     // requests re-read of availble PVs from the Archives

private:
   enum Constants {
      NumberRows = 60      // maximum.
   };

   // Holds a reference to each widget
   //
   struct Rows {
      QLabel* hostNamePort;
      QLabel* endPoint;
      QLabel* state;
      QLabel* available;   // CA only
      QLabel* read;        // CA only
      QLabel* numberPVs;
      QLabel* pending;     // CA only
   };

   QEArchiveAccess::ArchiverTypes archiveType;  // treat as constant post construction
   int inUseCount;

   // Internal widgets.
   //
   QEArchiveAccess* archiveAccess;
   QVBoxLayout* verticalLayout;
   QWidget* updateFrame;
   QHBoxLayout* horizontalLayout;
   QWidget* gridFrame;
   QGridLayout* gridLayout;

   Rows rowList [NumberRows + 1];  // +1 for title row

   QPushButton* archiveUpdatePvNamesButton;
   QSpacerItem* horizontalSpacer;
   QLabel* numberJobsLabel;
   QLabel* numberOfJobs;
   QLabel* totalPVsLabel;
   QLabel* totalNumberPVs;

   void setStatusRowVisible (const int j, const bool visible);
   void createInternalWidgets ();
   void calcMinimumHeight ();

private slots:
   void archiveStatus (const QEArchiveAccess::StatusList& statusList);
   void onArchiveUpdatePvNamesClick (bool);
};

#endif  // QE_ARCHIVE_STATUS_H
