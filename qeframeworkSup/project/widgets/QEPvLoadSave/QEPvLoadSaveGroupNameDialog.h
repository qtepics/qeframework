/*  QEPvLoadSaveGroupNameDialog.h
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

#ifndef QEPV_LOAD_SAVE_GROUP_NAME_DIALOG_H
#define QEPV_LOAD_SAVE_GROUP_NAME_DIALOG_H

#include <QString>
#include <QEDialog.h>

namespace Ui {
   class QEPvLoadSaveGroupNameDialog;
}

/*
 * Manager class for the QEPvLoadSaveGroupNameDialog.ui compiled form.
 */
class QEPvLoadSaveGroupNameDialog : public QEDialog
{
   Q_OBJECT

public:
   explicit QEPvLoadSaveGroupNameDialog (QWidget *parent = 0);
   ~QEPvLoadSaveGroupNameDialog ();

   void setGroupName (QString pvNameIn);
   QString getGroupName ();

   bool isClear ();

private:
   Ui::QEPvLoadSaveGroupNameDialog *ui;

private slots:
   void on_buttonBox_rejected ();
   void on_buttonBox_accepted ();
};

#endif  // QEPV_LOAD_SAVE_GROUP_NAME_DIALOG_H
