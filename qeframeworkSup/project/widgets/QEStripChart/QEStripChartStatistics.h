/*  QEStripChartStatistics.h
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
 *  Copyright (c) 2013,2016 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#ifndef QE_STRIP_CHART_STATISTICS_H
#define QE_STRIP_CHART_STATISTICS_H

#include <QString>
#include <QWidget>
#include <QCaDataPoint.h>
#include <QEPluginLibrary_global.h>

namespace Ui {
class QEStripChartStatistics;
}

class QEStripChartItem;

class QEPLUGINLIBRARYSHARED_EXPORT QEStripChartStatistics : public QWidget
{
   Q_OBJECT

public:
   explicit QEStripChartStatistics (const QString& pvName,
                                    const QString& egu,
                                    const QCaDataPointList& dataList,
                                    QEStripChartItem* owner,
                                    QWidget *parent = 0);
   ~QEStripChartStatistics();
   
private:
   void processDataList (const QCaDataPointList& dataList);
   void clearLabels ();
   Ui::QEStripChartStatistics *ui;
   QEStripChartItem* owner;
   QString pvName;
   QString egu;

private slots:
   void updateClicked (bool);

};

#endif // QE_STRIP_CHART_STATISTICS_H
