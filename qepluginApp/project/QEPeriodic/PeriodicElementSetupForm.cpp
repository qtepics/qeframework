/*  PeriodicElementSetupForm.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2011-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
    text = QString::asprintf( "%-2s %3d", info->symbol.toStdString().c_str(), info->number );
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
