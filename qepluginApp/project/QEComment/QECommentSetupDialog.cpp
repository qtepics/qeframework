/*  QECommentSetupDialog.cpp
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

#include "QECommentSetupDialog.h"
#include <QtDesigner>
#include <QDebug>
#include <QStringList>
#include <QEComment.h>

#include <ui_QECommentSetupDialog.h>

#define DEBUG  qDebug () << "QECommentSetupDialog" << __LINE__ << __FUNCTION__ << "  "

//-----------------------------------------------------------------------------
// Create the dialog
//
QECommentSetupDialog::QECommentSetupDialog (QEComment* commentIn,
                                            QWidget* parent) :
   QDialog (parent),
   ui (new Ui::QECommentSetupDialog),
   owner (commentIn)
{
   this->ui->setupUi (this);

   QString comment = this->owner->getComment();

   // Replace any "\n" sub-strings with a new line character.
   //
   comment= comment.replace ("\\n", "\n");

   this->ui->commentEdit->setPlainText (comment);
}

//-----------------------------------------------------------------------------
// Destroy the dialog
//
QECommentSetupDialog::~QECommentSetupDialog()
{
   delete this->ui;
}

//-----------------------------------------------------------------------------
// User has pressed OK
//
void QECommentSetupDialog::on_buttonBox_accepted ()
{
   QString comment = this->ui->commentEdit->toPlainText();

   // Replace any new line characters with "\n".
   //
   comment = comment.replace ("\n", "\\n");

   // Can we find the designer form window?
   // Just calling this->owner->setComment kind of works, but doesn't update
   // the property view in designer as expected.
   //
   QDesignerFormWindowInterface* formWindow = QDesignerFormWindowInterface::findFormWindow (this->owner);
   if (formWindow) {
      formWindow->cursor()->setProperty ("comment", comment);
   } else {
      this->owner->setComment (comment);
   }

   this->accept ();
}

//-----------------------------------------------------------------------------
// User has pressed cancel
//
void QECommentSetupDialog::on_buttonBox_rejected ()
{
   this->close ();
}

// end
