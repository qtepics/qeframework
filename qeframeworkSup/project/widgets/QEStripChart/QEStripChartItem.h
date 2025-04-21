/*  QEStripChartItem.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2022 Australian Synchrotron
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

#include <QEEnums.h>
#include <QCaAlarmInfo.h>
#include <QCaConnectionInfo.h>
#include <QCaDataPoint.h>
#include <QCaDateTime.h>
#include <QELabel.h>
#include <QCaObject.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEArchiveAccess.h>
#include <persistanceManager.h>
#include <QEActionRequests.h>
#include <QEExpressionEvaluation.h>
#include <QEDisplayRanges.h>
#include <QEFrameworkLibraryGlobal.h>

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
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEStripChartItem : public QWidget {
   Q_OBJECT
public:
   explicit QEStripChartItem (QEStripChart* chart,
                              const int slot,
                              QWidget* parent);
   virtual ~QEStripChartItem ();

   bool isInUse () const;    // isPvData or isCalculation
   bool isPvData () const;
   bool isCalculation () const;

   void setPvName (const QString& pvName, const QString& substitutions);
   QString getPvName () const;
   QString getEgu () const;
   QString getDescription () const;    // Extracts the PV DESCription field.
   QString getCaptionLabel () const;   // As used in the name bar.

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

   void setAliasName (const QString& aliasName);
   QString getAliasName () const;

   // NOTE: Where ever possible I spell colour properly.
   //
public slots:
   void setColour (const QColor& colour);    // also used by colour dialog
public:
   QColor getColour () const;

   // For each of these three function whren the bool doScale parameter is true, the
   // return range item is scaled using the current PV scale. When false, the "raw"
   // value is returned.
   //
   QEDisplayRanges getLoprHopr (bool doScale) const;          // returns CA specified operating range
   QEDisplayRanges getDisplayedMinMax (bool doScale) const;   // returns range of values currently plotted
   QEDisplayRanges getBufferedMinMax (bool doScale) const;    // returns range of values that could be plotted

   // returns set of points currently displayed when doBuffered is false.
   // returns set of points currently buffered when doBuffered is true.
   //
   QCaDataPointList extractPlotPoints (const bool doBuffered) const;

   void readArchive ();                                 // initiate archive read request
   void recalcualteBufferedValues ();                   // re-calculate values.
   void normalise ();                                   // scale LOPR/HOPR to 0 .. 100
   void plotData ();                                    //
   void setCaption ();                                  // re-calc the caption

   // Extract the current value, raw PV or calculation, if it exists.
   //
   double getCurrentValue (bool& okay) const;

   // Only adds a point to the plot history if there has been a change in status or value.
   //
   typedef double CalcInputs [QEStripChart::NUMBER_OF_PVS];

   void calculateAndUpdate (const QCaDateTime& datetime,
                            const CalcInputs values);

   // Return a reference to the point, realtime or from archive, nearest to the
   // specified time or NULL.
   // WARNING - do not store this reference. To be consider valid during the
   // processing of a single event only.
   //
   const QCaDataPoint* findNearestPoint (const QCaDateTime& searchTime) const;

   void saveConfiguration (PMElement & parentElement);
   void restoreConfiguration (PMElement & parentElement);

   // Allow arbitary action to be added to the menus.
   //
   void updateMenu (QAction* action, const bool inUseMenu);

   // Initiates the dialog to write the trace to file.
   //
   void writeTraceToFile ();

   QCaVariableNamePropertyManager pvNameProperyManager;

signals:
   void requestAction (const QEActionRequests&);

protected:
   bool eventFilter (QObject *obj, QEvent *event);

private:
   qcaobject::QCaObject* getQcaItem () const; // Return reference to QELabel used to stream CA updates
   void connectQcaSignals ();                 // Performs qca slignal/slot connections if needs be.
   void clear ();
   void highLight (bool isHigh);
   void addRealTimeDataPoint (const QCaDataPoint& point);

   QPen getPen () const;
   void plotDataPoints (const QCaDataPointList& dataPoints,
                        const bool isRealTime,
                        const Qt::PenStyle penStyle,
                        QEDisplayRanges& plottedTrackRange);

   // Perform a pvNameDropEvent 'drop'.
   //
   void pvNameDropEvent (QDropEvent *event);

   void generateStatistics ();

   // data members
   //
   int slot;
   int maxRealTimePoints;
   bool isConnected;
   bool useReceiveTime;
   QEArchiveInterface::How archiveReadHow;
   QEStripChartNames::LineDrawModes lineDrawMode;
   QEStripChartNames::LinePlotModes linePlotMode;

   QString aliasName;
   QString description;
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

   // Used to determine if the calculates value has changed.
   //
   bool lastExpressionValueIsDefined;
   double lastExpressionValue;

   // Internal widgets.
   //
   QEStripChart *chart;

   QHBoxLayout* layout;
   QPushButton* pvSlotLetter;
   QLabel* pvName;
   QELabel* caLabel;
   QColorDialog* colourDialog;
   QEStripChartContextMenu* inUseMenu;
   QEStripChartContextMenu* emptyMenu;
   qcaobject::QCaObject::ObjectIdentity previousIdentity;
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
