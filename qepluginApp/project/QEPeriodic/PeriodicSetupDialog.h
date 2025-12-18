/*  PeriodicSetupDialog.h
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

#ifndef QE_PERIODIC_SETUP_DIALOG_H
#define QE_PERIODIC_SETUP_DIALOG_H

#include <QDialog>
#include "PeriodicElementSetupForm.h"
#include <QEPeriodic.h>
#include <QEPluginLibrary_global.h>

namespace Ui {
    class PeriodicSetupDialog;
}

class QEPLUGINLIBRARYSHARED_EXPORT PeriodicSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PeriodicSetupDialog(QWidget *parent = 0);
    ~PeriodicSetupDialog();

private:
    Ui::PeriodicSetupDialog *ui;

        PeriodicElementSetupForm* elements[NUM_ELEMENTS];

private slots:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
};

#endif // QE_PERIODIC_SETUP_DIALOG_H
