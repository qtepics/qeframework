/*  QEPVNameSelectDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2021 Australian Synchrotron
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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

#ifndef QE_PVNAME_SELECT_DIALOG_H
#define QE_PVNAME_SELECT_DIALOG_H

#include <QString>
#include <QStringList>
#include <QWidget>
#include <QEDialog.h>
#include <QEFrameworkLibraryGlobal.h>

namespace Ui {
   class QEPVNameSelectDialog;
}

/*
 * Manager class for the QEPVNameSelectDialog.ui compiled form.
 */
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPVNameSelectDialog : public QEDialog
{
   Q_OBJECT

public:
   explicit QEPVNameSelectDialog (QWidget *parent = 0);
   ~QEPVNameSelectDialog ();

   void setPvName (QString pvNameIn);
   QString getPvName ();

   // Set and get a global arbitary PV names list to augment the PV names
   // extracted from the archiver.
   //
   static void setPvNameList (const QStringList& pvNameList);
   static QStringList getPvNameList ();

protected:
   void closeEvent (QCloseEvent * e);

private:
   void applyFilter ();

   Ui::QEPVNameSelectDialog *ui;
   QString originalPvName;
   bool returnIsMasked;
   QStringList filteredNames;
   static QStringList pvNameList;

private slots:
   void filterEditReturnPressed ();
   void filterEditingFinished ();
   void editTextChanged (const QString &);
   void helpClicked (bool checked);
   void clearClicked (bool checked);

   void on_buttonBox_rejected ();
   void on_buttonBox_accepted ();
};

#endif  // QE_PVNAME_SELECT_DIALOG_H
