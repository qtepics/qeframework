/*  QEPvLoadSaveGroupNameDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2013 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
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
