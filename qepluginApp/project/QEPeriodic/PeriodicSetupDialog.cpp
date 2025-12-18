/*  PeriodicSetupDialog.cpp
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
  This class manages a dialog for setting up a QEPeriodic in Qt designer.
 */

#include "PeriodicSetupDialog.h"
#include "ui_PeriodicSetupDialog.h"
#include "PeriodicElementSetupForm.h"
#include <QGridLayout>
#include <QEPeriodic.h>
#include <QLabel>
#include <QFrame>
#include <QCheckBox>
#include <QLineEdit>
#include <QECommon.h>

// Create the dialog
PeriodicSetupDialog::PeriodicSetupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PeriodicSetupDialog)
{
    ui->setupUi(this);

    QEPeriodic *plugin = qobject_cast<QEPeriodic *>(parent);
    if(!plugin) return;

    // Populate the table
    QGridLayout* periodicGrid = this->findChild<QGridLayout *>("periodicGridLayout");
    if( periodicGrid )
    {
        periodicGrid->setSpacing( 4 );

        // Populate the table elements
        for( int i = 0; i < NUM_ELEMENTS; i++ )
        {
            elements[i] = new PeriodicElementSetupForm( i, &plugin->userInfo[i], this );

            periodicGrid->addWidget( elements[i],
                                     QEPeriodic::elementInfo[i].tableRow,
                                     QEPeriodic::elementInfo[i].tableCol );
        }

        // Populate unused rows and columns
        QLabel* label;

        // ... Lanthanides indicators
        label = new QLabel( this );
        label->setText( "*" );
        label->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
        label->setFixedWidth (20);
        periodicGrid->addWidget( label, 5, 3 );

        label = new QLabel( this );
        label->setText( "*" );
        label->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
        label->setFixedWidth (20);
        periodicGrid->addWidget( label, 8, 3 );

        // ... Actinides indicators
        label = new QLabel( this );
        label->setText( "**" );
        label->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
        label->setFixedWidth (20);
        periodicGrid->addWidget( label, 6, 3 );

        label = new QLabel( this );
        label->setText( "**" );
        label->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
        label->setFixedWidth (20);
        periodicGrid->addWidget( label, 9, 3 );

        // ... Force empty row 7 to remain
        label = new QLabel( this );
        label->setText( "" );
        periodicGrid->addWidget( label, 7, 10 );

        // Make empty row 7 narrower than the rest
        for( int i = 0; i < 10; i++ )
            periodicGrid->setRowStretch( i, (i==7) ? 2 : 10); // row 7 stretch = 2, all other rows stretch = 10

        // Make "**" col 3 narrower than the rest
        for( int i = 0; i < 19; i++ )
            periodicGrid->setColumnStretch( i, (i==3) ? 1 : 10); // col 3 stretch = 2, all other rows stretch = 10
    }
}

// Destroy the dialog
PeriodicSetupDialog::~PeriodicSetupDialog()
{
    delete ui;
}

// User has pressed OK
void PeriodicSetupDialog::on_buttonBox_accepted()
{
    QEPeriodic *plugin = qobject_cast<QEPeriodic *>(this->parent());
    if( plugin )
    {
        for( int i = 0; i < NUM_ELEMENTS; i++ )
        {
            QCheckBox* enableButton = elements[i]->findChild<QCheckBox *>("checkBoxEnable");
            if( enableButton )
            {
                plugin->userInfo[i].enable = enableButton->isChecked();
            }

            bool ok;
            QLineEdit* value1 = elements[i]->findChild<QLineEdit *>("lineEditValue1");
            if( value1 )
            {
                plugin->userInfo[i].value1 = value1->text().toDouble( &ok );
            }

            QLineEdit* value2 = elements[i]->findChild<QLineEdit *>("lineEditValue2");
            if( value2 )
            {
                plugin->userInfo[i].value2 = value2->text().toDouble( &ok );
            }

            QLineEdit* elementText = elements[i]->findChild<QLineEdit *>("lineEditString");
            if( elementText )
            {
                plugin->userInfo[i].elementText = elementText->text();
            }
        }

        plugin->updateUserInfoSource();

        accept();

    }
}

// User has pressed cancel
void PeriodicSetupDialog::on_buttonBox_rejected()
{
    this->close();
}

// end
