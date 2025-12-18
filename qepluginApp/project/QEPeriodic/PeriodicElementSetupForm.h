/*  PeriodicElementSetupForm.h
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

#ifndef QE_PERIODIC_ELEMENT_SETUP_FORM_H
#define QE_PERIODIC_ELEMENT_SETUP_FORM_H

#include <QWidget>
#include <QEPluginLibrary_global.h>

namespace Ui {
    class PeriodicElementSetupForm;
}

class userInfoStruct;

class QEPLUGINLIBRARYSHARED_EXPORT PeriodicElementSetupForm : public QWidget
{
    Q_OBJECT

public:
    explicit PeriodicElementSetupForm(const int element,
                                      userInfoStruct* userInfo,
                                      QWidget *parent = 0);
    ~PeriodicElementSetupForm();

private:
    Ui::PeriodicElementSetupForm *ui;
};

#endif // QE_PERIODIC_ELEMENT_SETUP_FORM_H
