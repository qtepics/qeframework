/*  PasswordDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2022 Australian Synchrotron
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
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
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
