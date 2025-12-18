/*  PasswordDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>
#include <QEFrameworkLibraryGlobal.h>

namespace Ui {
class PasswordDialog;
}

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT PasswordDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit PasswordDialog(QWidget *parent = 0);
    ~PasswordDialog();
    
private slots:
    void on_buttonBox_accepted();

private:
    Ui::PasswordDialog *ui;
};

#endif // PASSWORDDIALOG_H
