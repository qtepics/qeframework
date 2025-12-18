/*  imageMarkupLegendSetText.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2015-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

// This dialog is used to prompt a user for a new legend for a markup being shown over an
// image in a QEImage widget.
// This dialog is accessed from the markup context menu.

#include "imageMarkupLegendSetText.h"
#include "ui_imageMarkupLegendSetText.h"

// Create the "Set legend" dialog displaying the existing legend
imageMarkupLegendSetText::imageMarkupLegendSetText( QString existingLegend, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::imageMarkupLegendSetText)
{
    ui->setupUi(this);
    ui->lineEditLegend->setText( existingLegend );
}

// Destructor
imageMarkupLegendSetText::~imageMarkupLegendSetText()
{
    delete ui;
}

// Return the new legend text
QString imageMarkupLegendSetText::getLegend()
{
    return ui->lineEditLegend->text();
}
