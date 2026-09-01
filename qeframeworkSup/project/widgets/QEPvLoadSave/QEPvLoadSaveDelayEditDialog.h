/*  QEPvLoadSaveDelayEditDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QEPV_LOAD_SAVE_DELAY_EDIT_DIALOG_H
#define QEPV_LOAD_SAVE_DELAY_EDIT_DIALOG_H

#include <QEDialog.h>

namespace Ui {
   class QEPvLoadSaveDelayEditDialog;
}

/*
 * Manager class for the QEPvLoadSaveDelayEditDialog.ui compiled form.
 */
class QEPvLoadSaveDelayEditDialog : public QEDialog
{
   Q_OBJECT

public:
   explicit QEPvLoadSaveDelayEditDialog (QWidget *parent = 0);
   ~QEPvLoadSaveDelayEditDialog ();

   void setDelay (const double delay);
   double getDelay () const;

private:
   void captureText ();      // copy edit widget text into selected delayList element.
   void outputText ();       // copy selected delayList element to edit widget text.

   double delay;
   Ui::QEPvLoadSaveDelayEditDialog *ui;

private slots:
   void on_buttonBox_rejected ();
   void on_buttonBox_accepted ();
};

#endif  // QEPV_LOAD_SAVE_DELAY_EDIT_DIALOG_H
