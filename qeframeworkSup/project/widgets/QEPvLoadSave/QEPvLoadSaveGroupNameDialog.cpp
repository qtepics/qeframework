/*  QEPvLoadSaveGroupNameDialog.cpp
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
 *  Copyright (c) 2012 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#include <QDebug>
#include <QStringList>

#include <QEPvLoadSaveGroupNameDialog.h>
#include <ui_QEPvLoadSaveGroupNameDialog.h>

//------------------------------------------------------------------------------
//
QEPvLoadSaveGroupNameDialog::QEPvLoadSaveGroupNameDialog (QWidget *parent) :
      QEDialog (parent),
      ui (new Ui::QEPvLoadSaveGroupNameDialog)
{
   this->ui->setupUi (this);
}

//------------------------------------------------------------------------------
//
QEPvLoadSaveGroupNameDialog::~QEPvLoadSaveGroupNameDialog ()
{
   delete ui;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveGroupNameDialog::setGroupName (QString groupNameIn)
{
   this->ui->groupEdit->setText (groupNameIn);
}


//------------------------------------------------------------------------------
//
QString QEPvLoadSaveGroupNameDialog::getGroupName ()
{
   return this->ui->groupEdit->text ().trimmed ();
}

//------------------------------------------------------------------------------
//
bool QEPvLoadSaveGroupNameDialog::isClear ()
{
   return (this->getGroupName() == "");
}


//------------------------------------------------------------------------------
// User has pressed OK
//
void QEPvLoadSaveGroupNameDialog::on_buttonBox_accepted ()
{
   this->accept ();
}

//------------------------------------------------------------------------------
// User has pressed Cancel
//
void QEPvLoadSaveGroupNameDialog::on_buttonBox_rejected ()
{
   this->close ();
}

// end
