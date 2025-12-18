/*  PasswordDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/*
 * Description:
 *
 * This class provides a dialog for altering the user level passwords.
 * On instantiation, it reads the current passwords from the profile and on 'OK' updates the passwords.
 */

#include "PasswordDialog.h"
#include "ui_PasswordDialog.h"
#include <QEEnums.h>
#include <ContainerProfile.h>

//------------------------------------------------------------------------------
//
PasswordDialog::PasswordDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasswordDialog)
{
    ContainerProfile profile;

    this->ui->setupUi(this);

    this->ui->lineEditUser->setText( profile.getUserLevelPassword( QE::User ) );
    this->ui->lineEditScientist->setText( profile.getUserLevelPassword( QE::Scientist ) );
    this->ui->lineEditEngineer->setText( profile.getUserLevelPassword( QE::Engineer ) );
}

//------------------------------------------------------------------------------
//
PasswordDialog::~PasswordDialog()
{
    delete this->ui;
}

//------------------------------------------------------------------------------
//
void PasswordDialog::on_buttonBox_accepted()
{
    ContainerProfile profile;

    profile.setUserLevelPassword( QE::User, this->ui->lineEditUser->text() );
    profile.setUserLevelPassword( QE::Scientist, this->ui->lineEditScientist->text() );
    profile.setUserLevelPassword( QE::Engineer, this->ui->lineEditEngineer->text() );
}

// end
