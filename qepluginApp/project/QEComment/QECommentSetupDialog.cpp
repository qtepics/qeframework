/*  QECommentSetupDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2025 Australian Synchrotron
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
 *    andrews@ansto.gov.au
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
