/*  QEPVLoadSaveNameSelectDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2021-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
