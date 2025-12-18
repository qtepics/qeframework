/*  QEStripChartStatistics.h
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

#ifndef QE_STRIP_CHART_STATISTICS_H
#define QE_STRIP_CHART_STATISTICS_H

#include <QString>
#include <QWidget>
#include <QCaDataPoint.h>
#include <QEAxisPainter.h>
#include <QEFrameworkLibraryGlobal.h>

namespace Ui {
   class QEStripChartStatistics;
}

class QEStripChartItem;

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEStripChartStatistics : public QWidget
{
   Q_OBJECT

public:
   explicit QEStripChartStatistics (const QString& pvName,
                                    const QString& egu,
                                    const QCaDataPointList& dataList,
                                    QEStripChartItem* owner,
                                    QWidget *parent = 0);
   ~QEStripChartStatistics();

protected:
   bool eventFilter (QObject* watched, QEvent* event);

private:
   void processDataList (const QCaDataPointList& dataList);
   void clearLabels ();
   void paintDistribution ();

   Ui::QEStripChartStatistics *ui;
   QEStripChartItem* owner;
   QString pvName;
   QString egu;

   QEAxisPainter* xAxis;
   QEAxisPainter* yAxis;

   double valueMean;
   double valueStdDev;
   double valueTotal;

   double distributionData [200];
   int distributionCount;
   double distributionIncrement;

private slots:
   void updateClicked (bool);

};

#endif // QE_STRIP_CHART_STATISTICS_H
