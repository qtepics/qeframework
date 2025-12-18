/*  QEImageMarkupThickness.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEImageMarkupThickness.h"
#include "ui_QEImageMarkupThickness.h"

QEImageMarkupThickness::QEImageMarkupThickness(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QEImageMarkupThickness)
{
    ui->setupUi(this);
}

QEImageMarkupThickness::~QEImageMarkupThickness()
{
    delete ui;
}

void QEImageMarkupThickness::on_buttonBox_accepted()
{
    int value = ui->spinBox->value();
    thickness = (unsigned int)((value<1)?1:value);
}

void QEImageMarkupThickness::setThickness(unsigned int thicknessIn )
{
    thickness = thicknessIn;
    ui->spinBox->setValue( thickness );
}

unsigned int QEImageMarkupThickness::getThickness()
{
    return thickness;
}
