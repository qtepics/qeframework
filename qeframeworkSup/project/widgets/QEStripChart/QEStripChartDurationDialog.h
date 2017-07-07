/*  QEStripChartDurationDialog.h
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

#ifndef QE_STRIP_CHART_DURATION_DIALOG_H
#define QE_STRIP_CHART_DURATION_DIALOG_H

#include <QString>
#include <QColor>
#include <QEDialog.h>
#include <QTime>
#include <QEFrameworkLibraryGlobal.h>

namespace Ui {
   class QEStripChartDurationDialog;
}

/*
 * Manager class for the QEStripChartDurationDialog.ui compiled form.
 *
 * This dialog form allows the user to select an arbitary duration.
 */
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEStripChartDurationDialog : public QEDialog
{
    Q_OBJECT

public:
   explicit QEStripChartDurationDialog (QWidget *parent = 0);
   ~QEStripChartDurationDialog ();

   void setDuration (int secs);
   int getDuration () const;

private:
   Ui::QEStripChartDurationDialog* ui;
   int duration;

   void calcShowDuration ();

private slots:
   void daysChanged (const int days);
   void timeChanged (const QTime &time);

   void on_buttonBox_rejected ();
   void on_buttonBox_accepted ();
};

#endif  // QE_STRIP_CHART_DURATION_DIALOG_H
