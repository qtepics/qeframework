/*  imageMarkupLegendSetText.h
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

#ifndef IMAGEMARKUPLEGENDSETTEXT_H
#define IMAGEMARKUPLEGENDSETTEXT_H

#include <QDialog>

namespace Ui {
    class imageMarkupLegendSetText;
}

class imageMarkupLegendSetText : public QDialog
{
    Q_OBJECT

public:
    explicit imageMarkupLegendSetText( QString existingLegend, QWidget *parent = 0);
    ~imageMarkupLegendSetText();
    QString getLegend();

private:
    Ui::imageMarkupLegendSetText *ui;
};

#endif // IMAGEMARKUPLEGENDSETTEXT_H
