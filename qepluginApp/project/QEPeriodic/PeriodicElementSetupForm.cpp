/*  PeriodicElementSetupForm.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2011-2022 Australian Synchrotron
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
  This class manages a each of the element sub forms in the dialog for
  setting up a QEPeriodic in Qt designer.
 */

#include "PeriodicElementSetupForm.h"
#include "ui_PeriodicElementSetupForm.h"
#include <QECommon.h>
#include <QEPeriodic.h>

PeriodicElementSetupForm::PeriodicElementSetupForm(const int element,
                                                   userInfoStruct* userInfo,
                                                   QWidget* parent) :
    QWidget(parent),
    ui(new Ui::PeriodicElementSetupForm)
{
    ui->setupUi(this);

    const QEPeriodic::elementInfoStruct* info = &QEPeriodic::elementInfo [element];

    ui->frame->setToolTip( info->name );
    ui->checkBoxEnable->setToolTip( "Check if " + info->name + " is to be selected by user" );
    QString text;
    text.sprintf( "%-2s %3d", info->symbol.toStdString().c_str(), info->number );
    ui->checkBoxEnable->setText( text );
    ui->checkBoxEnable->setChecked( userInfo->enable );
    QString styleSheet;
    styleSheet = QEUtilities::colourToStyle( QEPeriodic::categoryColour( info->category ) );
    ui->checkBoxEnable->setStyleSheet( styleSheet );
    ui->lineEditValue1->setText( QString::number( userInfo->value1 ) );
    ui->lineEditValue1->setCursorPosition( 0 );
    ui->lineEditValue2->setText( QString::number( userInfo->value2 ) );
    ui->lineEditValue2->setCursorPosition( 0 );
    ui->lineEditString->setText( userInfo->elementText );
    ui->lineEditString->setCursorPosition( 0 );
}

PeriodicElementSetupForm::~PeriodicElementSetupForm()
{
    delete ui;
}

// end
