/*  QEPVNameSelectDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_PVNAME_SELECT_DIALOG_H
#define QE_PVNAME_SELECT_DIALOG_H

#include <QFrame>
#include <QPushButton>
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
   explicit QEPVNameSelectDialog (QWidget* parent = 0);
   explicit QEPVNameSelectDialog (const bool allowCalcHelp,
                                  QWidget* parent = 0);
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
   void commonConstructor();
   void applyFilter ();
   void helpClicked (QFrame* f1, QPushButton* b1,
                     QFrame* f2, QPushButton* b2);
   const bool m_allowCalcHelp;
   Ui::QEPVNameSelectDialog* const ui;  // ptr is const, not the referenced object
   QString originalPvName;
   bool returnIsMasked;
   QStringList filteredNames;
   static QStringList pvNameList;

private slots:
   void filterEditReturnPressed ();
   void filterEditingFinished ();
   void editTextChanged (const QString &);
   void regexHelpClicked (bool checked);
   void calcHelpClicked (bool checked);
   void clearClicked (bool checked);

   void on_buttonBox_rejected ();
   void on_buttonBox_accepted ();
};

#endif  // QE_PVNAME_SELECT_DIALOG_H
