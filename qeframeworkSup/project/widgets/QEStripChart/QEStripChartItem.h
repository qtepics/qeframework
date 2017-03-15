/*  QEStripChartItem.h
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
 *  Copyright (c) 2012,2016,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#ifndef QE_STRIP_CHART_ITEM_H
#define QE_STRIP_CHART_ITEM_H

#include <QColor>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QPoint>
#include <QPushButton>
#include <QString>
#include <QWidget>

#include <QCaAlarmInfo.h>
#include <QCaConnectionInfo.h>
#include <QCaDataPoint.h>
#include <QCaDateTime.h>
#include <QELabel.h>
#include <QCaObject.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEArchiveManager.h>
#include <persistanceManager.h>
#include <QEActionRequests.h>
#include <QEExpressionEvaluation.h>
#include <QEDisplayRanges.h>
#include <QEPluginLibrary_global.h>

#include "QEStripChart.h"
#include "QEStripChartNames.h"
#include "QEStripChartAdjustPVDialog.h"
#include "QEStripChartContextMenu.h"
#include "QEStripChartUtilities.h"

//==============================================================================
// This is essentially a private classes used soley by the QEStripChart widget.
// We have to make is public so that it can be a pukka Q_OBJECT in order to
// receive signals.
//
class QEPLUGINLIBRARYSHARED_EXPORT QEStripChartItem : public QWidget {
   Q_OBJECT
public:
   explicit QEStripChartItem (QEStripChart* chart,
                              const int slot,
                              QWidget* parent);
   virtual ~QEStripChartItem ();

   bool isInUse () const;
   bool isCalculation () const;

   void setPvName (const QString& pvName, const QString& substitutions);
   QString getPvName () const;
   QString getEgu () const;

   bool isScaled () const;

   // Resturns x (i.e. t) and y values as plotted taking into account value and time scaling.
   //
   QPointF dataPointToReal (const QCaDataPoint& point) const;

   // Following used to extract status for the context menu object.
   //
   bool getUseReceiveTime () const { return this->useReceiveTime; }
   QEArchiveInterface::How getArchiveReadHow () const { return this->archiveReadHow; }
   QEStripChartNames::LineDrawModes getLineDrawMode () const { return this->lineDrawMode; }
   QEStripChartNames::LinePlotModes getLinePlotMode () const { return this->linePlotMode; }

   // NOTE: Where ever possible I spell colour properly.
   //
public slots:
   void setColour (const QColor& colour);    // also used by colour dialog
public:
   QColor getColour ();

   // For each of these three function whren the bool doScale parameter is true, the
   // return range item is scaled using the current PV scale. When false, the "raw"
   // value is returned.
   //
   QEDisplayRanges getLoprHopr (bool doScale);          // returns CA specified operating range
   QEDisplayRanges getDisplayedMinMax (bool doScale);   // returns range of values currently plotted
   QEDisplayRanges getBufferedMinMax (bool doScale);    // returns range of values that could be plotted
   QCaDataPointList determinePlotPoints ();

   void readArchive ();                                 // initiate archive read request
   void normalise ();                                   // scale LOPR/HOPR to 0 .. 100
   void plotData ();

   // Return a reference to the point, realtime or from archive, nearest to the
   // specified time or NULL.
   // WARNING - do not store this reference. To be consider valid during the
   // processing of a single event only.
   //
   const QCaDataPoint* findNearestPoint (const QCaDateTime& searchTime) const;

   void saveConfiguration (PMElement & parentElement);
   void restoreConfiguration (PMElement & parentElement);

   QCaVariableNamePropertyManager pvNameProperyManager;

signals:
   void requestAction (const QEActionRequests&);

protected:
   bool eventFilter (QObject *obj, QEvent *event);

private:
   qcaobject::QCaObject* getQcaItem ();   // Return reference to QELabel used to stream CA updates
   void connectQcaSignals ();             // Performs qca connections if needs be.
   void setCaption ();
   void clear ();
   void highLight (bool isHigh);

   QPen getPen ();
   void plotDataPoints (const QCaDataPointList& dataPoints,
                        const bool isRealTime,
                        const Qt::PenStyle penStyle,
                        QEDisplayRanges& plottedTrackRange);

   // Perform a pvNameDropEvent 'drop'.
   //
   void pvNameDropEvent (QDropEvent *event);

   void writeTraceToFile ();
   void generateStatistics ();

   // data members
   //
   int slot;
   bool isConnected;
   bool useReceiveTime;
   QEArchiveInterface::How archiveReadHow;
   QEStripChartNames::LineDrawModes lineDrawMode;
   QEStripChartNames::LinePlotModes linePlotMode;

   QColor colour;
   ValueScaling scaling;

   QCaDataPointList historicalTimeDataPoints;
   QCaDataPointList realTimeDataPoints;
   QEDisplayRanges historicalMinMax;
   QEDisplayRanges realTimeMinMax;

   // Used to specify dash line joining historical to live data.
   //
   bool dashExists;
   QCaDataPoint dashStart;
   QCaDataPoint dashEnd;

   // Used for first point maps to chart centre scaling.
   //
   bool firstPointIsDefined;
   QCaDataPoint firstPoint;

   QEDisplayRanges displayedMinMax;

   QEArchiveAccess archiveAccess;

   QEStripChartAdjustPVDialog *adjustPVDialog;

   enum DataChartKinds { NotInUse,          // blank  - not in use - no data - no plot
                         PVData,            // use specified PV to provide plot data
                         CalculationData }; // "= ..." - use given calculation for plot data

   DataChartKinds dataKind;
   QString expression;        // when dataKind is CalculationPlot
   bool expressionIsValid;
   QEExpressionEvaluation* calculator;

   // Internal widgets.
   //
   QEStripChart *chart;

   QHBoxLayout* layout;
   QPushButton *pvSlotLetter;
   QLabel *pvName;
   QELabel *caLabel;
   QColorDialog *colourDialog;
   QEStripChartContextMenu* inUseMenu;
   QEStripChartContextMenu* emptyMenu;
   qcaobject::QCaObject *previousQcaItem;
   bool hostSlotAvailable;

   void createInternalWidgets ();
   void runSelectNameDialog (QWidget* control);

private slots:
   void newVariableNameProperty (QString pvName, QString substitutions, unsigned int slot);

   void setDataConnection (QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex);
   void setDataValue (const QVariant& value, QCaAlarmInfo& alarm,
                      QCaDateTime& datetime, const unsigned int& variableIndex);

   void setArchiveData (const QObject* userData, const bool okay, const QCaDataPointList& archiveData,
                        const QString& pvName, const QString& supplementary);

   void letterButtonClicked (bool checked);
   void contextMenuRequested (const QPoint & pos);
   void contextMenuSelected  (const QEStripChartNames::ContextMenuOptions option);
};

#endif  // QE_STRIP_CHART_ITEM_H
