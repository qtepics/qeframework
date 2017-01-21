/*
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
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
 *  Copyright (c) 2014 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
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
