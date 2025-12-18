/*  QEPvLoadSaveGroupNameDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
