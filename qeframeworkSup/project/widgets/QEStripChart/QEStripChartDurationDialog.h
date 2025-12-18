/*  QEStripChartDurationDialog.h
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
