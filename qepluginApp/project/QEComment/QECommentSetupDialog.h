/*  QECommentSetupDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2025-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_COMMENT_SETUP_DIALOG_H
#define QE_COMMENT_SETUP_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QEPluginLibrary_global.h>

// Differed declaration.
//
namespace Ui {
   class QECommentSetupDialog;
}

class QEComment;

class QEPLUGINLIBRARYSHARED_EXPORT QECommentSetupDialog : public QDialog
{
   Q_OBJECT

public:
   explicit QECommentSetupDialog (QEComment* owner, QWidget* parent);
   ~QECommentSetupDialog ();

private:
   Ui::QECommentSetupDialog* ui;
   QEComment* owner;

private slots:
   void on_buttonBox_accepted ();
   void on_buttonBox_rejected ();
};

#endif // QE_COMMENT_SETUP_DIALOG_H
