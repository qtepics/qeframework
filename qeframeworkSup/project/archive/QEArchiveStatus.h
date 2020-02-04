/*  QEArchiveStatus.h
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

#ifndef QE_ARCHIVE_STATUS_H
#define QE_ARCHIVE_STATUS_H

#include <QObject>
#include <QWidget>
#include <QEGroupBox.h>
#include <QGridLayout>
#include <QLabel>
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
   void reReadAvailablePVs ();     // requests re-read of availble PVs from the CA Archives

private:
   // Internal widgets.
   //
   QEArchiveAccess* archiveAccess;
   QGridLayout* gridLayout;

   enum Constants {
      NumberRows = 40   // maximum.
   };

   // Holds reference to each widget
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

   Rows rowList [NumberRows + 1];
   int inUseCount;

   void setStatusRowVisible (const int j, const bool visible);
   void createInternalWidgets ();
   void calcMinimumHeight ();

private slots:
   void archiveStatus (const QEArchiveAccess::StatusList& statusList);

};

#endif  // QE_ARCHIVE_STATUS_H
