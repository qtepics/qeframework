/*  QEImageMarkupThickness.h
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

#ifndef QEIMAGEMARKUPTHICKNESS_H
#define QEIMAGEMARKUPTHICKNESS_H

#include <QDialog>

namespace Ui {
    class QEImageMarkupThickness;
}

class QEImageMarkupThickness : public QDialog
{
    Q_OBJECT

public:
    explicit QEImageMarkupThickness(QWidget *parent = 0);
    ~QEImageMarkupThickness();
    void setThickness(unsigned int thicknessIn );
    unsigned int getThickness();

private:
    Ui::QEImageMarkupThickness *ui;
    unsigned int thickness;

private slots:
    void on_buttonBox_accepted();
};

#endif // QEIMAGEMARKUPTHICKNESS_H
