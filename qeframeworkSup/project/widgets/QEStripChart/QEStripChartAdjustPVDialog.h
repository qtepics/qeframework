/*  QEStripChartAdjustPVDialog.h
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

#ifndef QE_STRIP_CHART_ADJUST_DIALOG_H
#define QE_STRIP_CHART_ADJUST_DIALOG_H

#include <QString>
#include <QColor>
#include <QEDialog.h>
#include <QDateTime>

#include <QEDisplayRanges.h>
#include <QEStripChartUtilities.h>
#include <QEFrameworkLibraryGlobal.h>

namespace Ui {
    class QEStripChartAdjustPVDialog;
}

/**
 * Manager class for the QEStripChartAdjustPVDialog.ui compiled form.
 *
 * This dialog form allows the user to select a start datetime and
 * end datetime pair for the strip chart.
 */
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEStripChartAdjustPVDialog :
      public QEDialog
{
    Q_OBJECT

public:
   explicit QEStripChartAdjustPVDialog (QWidget *parent = 0);
   ~QEStripChartAdjustPVDialog ();

    void setValueScaling (const ValueScaling & valueScale);
    ValueScaling getValueScaling () const;

    // Support data for scaling calculations.
    //
    void setSupport (const double min, const double max,
                     const QEDisplayRanges& loprHopr,
                     const QEDisplayRanges& plotted,
                     const QEDisplayRanges& buffered);

private:
   Ui::QEStripChartAdjustPVDialog *ui;

   ValueScaling valueScale;

   QEDisplayRanges loprHopr;
   QEDisplayRanges plotted;
   QEDisplayRanges buffered;

   double chartMinimum;
   double chartMaximum;
   bool returnIsMasked;

   // local utility conveniance function
   void useSelectedRange (const QEDisplayRanges& selectedRange);

private slots:
   void originReturnPressed ();
   void slopeReturnPressed ();
   void offsetReturnPressed ();

   void resetButtonClicked (bool checked = false);
   void loprHoprButtonClicked (bool checked = false);
   void plottedButtonClicked (bool checked = false);
   void bufferedButtonClicked (bool checked = false);
   void lowerButtonClicked (bool checked = false);
   void centreButtonClicked (bool checked = false);
   void upperButtonClicked (bool checked = false);

   // These conection are made by QDialog and associates.
   //
   void on_buttonBox_rejected ();
   void on_buttonBox_accepted ();
};

#endif  // QE_STRIP_CHART_ADJUST_DIALOG_H
