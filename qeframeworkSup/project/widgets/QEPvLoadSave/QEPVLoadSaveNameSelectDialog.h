/*  QEPVLoadSaveNameSelectDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2021 Australian Synchrotron
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

#ifndef QE_PV_LOAD_SAVE_NAME_SELECT_DIALOG_H
#define QE_PV_LOAD_SAVE_NAME_SELECT_DIALOG_H

#include <QComboBox>
#include <QString>
#include <QStringList>
#include <QWidget>
#include <QEDialog.h>
#include <QEFrameworkLibraryGlobal.h>

namespace Ui {
   class QEPVLoadSaveNameSelectDialog;
}

/// Manager class for the QEPVLoadSaveNameSelectDialog.ui compiled form.
/// This is a copy of QEPVNameSelectDialog that allows three PV names to be
/// specified, i.e.:
///   a read/get PV name;
///   a write/put PV name; and
///   an archive/fetch PV name.
///
// Consider: Could we use inheritance, or monkey patching?
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPVLoadSaveNameSelectDialog : public QEDialog
{
   Q_OBJECT
public:
   explicit QEPVLoadSaveNameSelectDialog (QWidget *parent = 0);
   ~QEPVLoadSaveNameSelectDialog ();

   void setPvNames (const QString& setPointPvName,
                    const QString& readBackPvName,    // "" implies setPointPvName
                    const QString& archiverPvName);   // "" implies setPointPvName
   void getPvNames (QString& setPointPvName,
                    QString& readBackPvName,
                    QString& archiverPvName);

protected:
   void closeEvent (QCloseEvent * e);

private:
   void applyFilter ();

   Ui::QEPVLoadSaveNameSelectDialog *ui;

   // PV Name Types
   enum PvTypes {
      ptWrite = 0,   // setPoint PV
      ptRead,        // readBack PV
      ptArch,        // archiver PV
      PT_NUMBER      // Must be last
   };

   QComboBox* pvNameEdit [PT_NUMBER];
   QString originalPvName [PT_NUMBER];
   bool returnIsMasked;
   QStringList filteredNames;

private slots:
   void filterEditReturnPressed ();
   void filterEditingFinished ();

   void helpClicked (bool checked);
   void clearClicked (bool checked);

   void on_buttonBox_rejected ();
   void on_buttonBox_accepted ();
};

#endif  // QE_PV_LOAD_SAVE_NAME_SELECT_DIALOG_H
