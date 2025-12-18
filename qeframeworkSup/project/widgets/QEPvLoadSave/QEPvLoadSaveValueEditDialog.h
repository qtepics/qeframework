/*  QEPvLoadSaveValueEditDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QEPV_LOAD_SAVE_VALUE_EDIT_DIALOG_H
#define QEPV_LOAD_SAVE_VALUE_EDIT_DIALOG_H

#include <QString>
#include <QVariant>

#include <QEDialog.h>

namespace Ui {
   class QEPvLoadSaveValueEditDialog;
}

/*
 * Manager class for the QEPvLoadSaveValueEditDialog.ui compiled form.
 */
class QEPvLoadSaveValueEditDialog : public QEDialog
{
   Q_OBJECT

public:
   explicit QEPvLoadSaveValueEditDialog (QWidget *parent = 0);
   ~QEPvLoadSaveValueEditDialog ();

   void setPvName (const QString& pvName);

   void setValue (const QVariant& valueList);

   QVariant getValue () const;

private:
   void captureText ();      // copy edit widget text into selected valueList element.
   void outputText ();       // copy selected valueList element to edit widget text.

   QVariantList valueList;   // we work with a list, even for scaler values.
   int currentIndex;
   Ui::QEPvLoadSaveValueEditDialog *ui;

private slots:
   void elementIndexChanged (int newIndex);
   void numberElementsChanged (int numberOfElements);
   void on_buttonBox_rejected ();
   void on_buttonBox_accepted ();
};

#endif  // QEPV_LOAD_SAVE_VALUE_EDIT_DIALOG_H
