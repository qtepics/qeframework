/*  QECommentSetupDialog.h
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
