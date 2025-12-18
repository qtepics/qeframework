/*  QEStripChartTimeDialog.h
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

#ifndef QE_STRIP_CHART_TIME_DIALOG_H
#define QE_STRIP_CHART_TIME_DIALOG_H

#include <QString>
#include <QColor>
#include <QEDialog.h>
#include <QDateTime>
#include <QEFrameworkLibraryGlobal.h>

namespace Ui {
    class QEStripChartTimeDialog;
}

/*
 * Manager class for the QEStripChartTimeDialog.ui compiled form.
 *
 * This dialog form allows the user to select a start datetime and
 * end datetime pair for the strip chart.
 */
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEStripChartTimeDialog : public QEDialog
{
    Q_OBJECT

public:
   explicit QEStripChartTimeDialog (QWidget *parent = 0);
   ~QEStripChartTimeDialog ();

   void setMaximumDateTime (QDateTime datetime);

   void setStartDateTime (QDateTime datetime);
   QDateTime getStartDateTime ();

   void setEndDateTime (QDateTime datetime);
   QDateTime getEndDateTime ();

private:
   Ui::QEStripChartTimeDialog *ui;

   QDateTime savedStartDateTime;
   QDateTime savedEndDateTime;

   QString timeFormat (const int duration);
   void calcShowDuration ();

   static QTime timeZero ();   // mid-night aka start of day.

private slots:
   void startDateClicked (const QDate & date);
   void endDateClicked (const QDate & date);

   void startTimeChanged (const QTime &time);
   void endTimeChanged (const QTime &time);

   void startSliderValueChanged (int value);
   void endSliderValueChanged (int value);

   void on_buttonBox_rejected ();
   void on_buttonBox_accepted ();
};

#endif  // QE_STRIP_CHART_TIME_DIALOG_H
