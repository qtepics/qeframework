/*  QEPlotterItemDialog.h
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

#ifndef QEPLOTTERITEMDIALOG_H
#define QEPLOTTERITEMDIALOG_H

#include <QString>
#include <QEDialog.h>

namespace Ui {
    class QEPlotterItemDialog;
}

/*
 * Manager class for the QEPlotterItemDialog.ui compiled form.
 */
class QEPlotterItemDialog : public QEDialog
{
    Q_OBJECT

public:
   explicit QEPlotterItemDialog (QWidget *parent = 0);
   ~QEPlotterItemDialog ();

   void setFieldInformation (const QString dataIn, const QString aliasIn, const QString sizeIn);
   void getFieldInformation (QString& dataOut,     QString& aliasOut,     QString& sizeOut);

private:
   Ui::QEPlotterItemDialog *ui;
   bool returnIsMasked;

private slots:
   void dataEditReturnPressed ();
   void aliasEditReturnPressed ();
   void sizeEditReturnPressed ();

   void on_buttonBox_rejected ();
   void on_buttonBox_accepted ();
   void clearButtonClicked (bool checked = false);
   void colourButtonClicked (bool checked = false);
};

#endif  // QEPLOTTERITEMDIALOG_H
