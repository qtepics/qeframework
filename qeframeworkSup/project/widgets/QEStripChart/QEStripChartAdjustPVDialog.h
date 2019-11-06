/*  QEStripChartAdjustPVDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2019 Australian Synchrotron
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The EPICS QT Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
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
