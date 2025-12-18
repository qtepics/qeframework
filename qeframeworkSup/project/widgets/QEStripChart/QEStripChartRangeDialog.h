/*  QEStripChartRangeDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_STRIP_CHART_RANGE_DIALOG_H
#define QE_STRIP_CHART_RANGE_DIALOG_H

#include <QString>
#include <QColor>
#include <QEDialog.h>
#include <QDateTime>
#include <QEFrameworkLibraryGlobal.h>

namespace Ui {
    class QEStripChartRangeDialog;
}

/*
 * Manager class for the QEStripChartRangeDialog.ui compiled form.
 *
 * This dialog form allows the user to select a start datetime and
 * end datetime pair for the strip chart.
 */
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEStripChartRangeDialog : public QEDialog
{
    Q_OBJECT

public:
   explicit QEStripChartRangeDialog (QWidget *parent = 0);
   ~QEStripChartRangeDialog ();

    void setRange (const double min, const double max);
    double getMinimum ();
    double getMaximum ();

private:
   Ui::QEStripChartRangeDialog *ui;
   double minimum;
   double maximum;
   bool returnIsMasked;

private slots:
   void minReturnPressed ();
   void maxReturnPressed ();

   // These conection are made by QDialog and associates.
   //
   void on_buttonBox_rejected ();
   void on_buttonBox_accepted ();
};

#endif  // QE_STRIP_CHART_RANGE_DIALOG_H
