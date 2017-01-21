/*  QEStripChartRangeDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2012,2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_STRIP_CHART_RANGE_DIALOG_H
#define QE_STRIP_CHART_RANGE_DIALOG_H

#include <QString>
#include <QColor>
#include <QEDialog.h>
#include <QDateTime>
#include <QEPluginLibrary_global.h>

namespace Ui {
    class QEStripChartRangeDialog;
}

/*
 * Manager class for the QEStripChartRangeDialog.ui compiled form.
 *
 * This dialog form allows the user to select a start datetime and
 * end datetime pair for the strip chart.
 */
class QEPLUGINLIBRARYSHARED_EXPORT QEStripChartRangeDialog : public QEDialog
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
