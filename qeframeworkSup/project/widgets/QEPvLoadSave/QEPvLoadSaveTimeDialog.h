/*  QEPvLoadSaveTimeDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2016-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_PV_LOAD_SAVE_TIME_DIALOG_H
#define QE_PV_LOAD_SAVE_TIME_DIALOG_H

#include <QString>
#include <QColor>
#include <QEDialog.h>
#include <QDateTime>
#include <QEFrameworkLibraryGlobal.h>

namespace Ui {
    class QEPvLoadSaveTimeDialog;
}

/*
 * Manager class for the QEPvLoadSaveTimeDialog.ui compiled form.
 *
 * This dialog form allows the user to select a datetime.
 */
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPvLoadSaveTimeDialog : public QEDialog
{
    Q_OBJECT

public:
   explicit QEPvLoadSaveTimeDialog (QWidget* parent = 0);
   ~QEPvLoadSaveTimeDialog ();

   void setMaximumDateTime (const QDateTime& datetime);

   void setDateTime (const QDateTime& datetime);
   QDateTime getDateTime ();

private:
   Ui::QEPvLoadSaveTimeDialog* ui;

   QDateTime savedDateTime;

private slots:
   void dateClicked (const QDate & date);
   void timeChanged (const QTime &time);
   void sliderValueChanged (int value);

   void on_buttonBox_rejected ();
   void on_buttonBox_accepted ();
};

#endif  // QE_PV_LOAD_SAVE_TIME_DIALOG_H
