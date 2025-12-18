/*  QECorrelation.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2018-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_CORRELATION_H
#define QE_CORRELATION_H

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVector>
#include <QWidget>

#include <QEEnums.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEAbstractDynamicWidget.h>
#include <QEFrame.h>
#include <QELabel.h>
#include <QEArchiveAccess.h>
#include <QEFloatingArray.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QEPVNameSelectDialog.h>
#include <QEStripChartTimeDialog.h>
#include <QETwinScaleSelectDialog.h>

namespace Ui {
   class QECorrelation;   // differed
}

// Differed class declaration - no explicit dependency on Qwt header files.
//
class QEGraphic;

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QECorrelation : public QEAbstractDynamicWidget
{
   Q_OBJECT
public:
   Q_PROPERTY (QString variableX     READ getXVariableName   WRITE setXVariableName)
   Q_PROPERTY (QString variableY     READ getYVariableName   WRITE setYVariableName)

   /// Default macro substitutions. The default is no substitutions.
   /// The format is NAME1=VALUE1[,] NAME2=VALUE2...
   /// Values may be quoted strings. For example, 'SAMPLE=SAM1, NAME = "Ref foil"'
   /// These substitutions are applied to all the variable names.
   //
   Q_PROPERTY (QString variableSubstitutions READ getVariableSubstitutions WRITE setVariableSubstitutions)

   Q_PROPERTY (double sampleInterval READ getSampleInterval  WRITE setSampleInterval)
   Q_PROPERTY (int    numberPoints   READ getNumberPoints    WRITE setNumberPoints)

   Q_PROPERTY (bool xLogarithmic     READ getXLogarithmic    WRITE setXLogarithmic)
   Q_PROPERTY (bool yLogarithmic     READ getYLogarithmic    WRITE setYLogarithmic)

public:
   enum Constants {
      NUMBER_OF_ITEMS = 2
   };

   explicit QECorrelation (QWidget *parent = 0);
   explicit QECorrelation (const QString& xVariableName,
                           QWidget *parent = 0);
   explicit QECorrelation (const QString& xVariableName,
                           const QString& yVariableName,
                           QWidget *parent = 0);
   ~QECorrelation ();

   // Property READ WRITE functions.
   //
   void setXVariableName (const QString& pvName);
   QString getXVariableName () const;

   void setYVariableName (const QString& pvName);
   QString getYVariableName () const;

   void    setVariableSubstitutions (const QString& variableNameSubstitutions);
   QString getVariableSubstitutions () const;

   void setSampleInterval (const double interval);
   double getSampleInterval () const;

   void setNumberPoints (const int number);
   int getNumberPoints () const;

   void setXLogarithmic (bool visible);
   bool getXLogarithmic () const;

   void setYLogarithmic (bool visible);
   bool getYLogarithmic () const;

   // Add PV to next available slot (if any), X then Y.
   // returns slot number 0 .. Max - 1 iff successful otherwise -1.
   //
   int addPvName (const QString& pvName);

   // Remove and clear all PVs.
   //
   void clearAllPvNames ();

protected:
   bool eventFilter (QObject* watched, QEvent* event);

   // Implementation of QEWidget's virtual funtions
   //
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);
   void establishConnection (unsigned int variableIndex);

   // We hande drag/drop internally
   //
   void mousePressEvent (QMouseEvent *event);
   void dragEnterEvent (QDragEnterEvent *event);
   void dropEvent (QDropEvent *event);
   //
   // This widget uses the setDrop/getDrop defined in QEWidget.

   // Copy/Paste
   //
   QString copyVariable ();
   QVariant copyData ();

   // override other virtual functions
   //
   void saveConfiguration (PersistanceManager* pm);
   void restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase);

private:
   QCaVariableNamePropertyManager xVariableNameManager;
   QCaVariableNamePropertyManager yVariableNameManager;

   Ui::QECorrelation* uiForm;

   QTimer* sampleTimer;
   QTimer* reDrawTimer;
   QHBoxLayout* plotLayout;   // any layout will do.
   QEGraphic* plotArea;

   QEPVNameSelectDialog* pvNameSelectDialog;
   QETwinScaleSelectDialog* scaleDialog;
   QEStripChartTimeDialog* timeDialog;
   QMenu* contextMenu;
   QEArchiveAccess* archiveAccess;

   double mSampleInterval;
   int mNumberPoints;

   int tickTimerCount;
   bool replotIsRequired;
   bool isReverse;
   bool disllowOneEnter;   // Mitigate drag drop feature

   // Essentially QVector<double> with extra functionality.
   //
   QEFloatingArray xData;
   QEFloatingArray yData;

   // Used for data retrieved from the CA archiver.
   //
   QEFloatingArray xHistoricalData;
   QEFloatingArray yHistoricalData;
   int historicalSetsReceived;

   // The display bounds.
   //
   double xMin;
   double xMax;
   double yMin;
   double yMax;

   enum PlotModes { pmDots = 10, pmLines };  // avoid any potential x/y tag mixup
   enum ScaleModes { smDynamic, smManual, smData };

   PlotModes plotMode;
   ScaleModes scaleMode;

   enum ContextMenuActions { maVoid = 0, maAddEdit, maPaste, maClear };

   QAction* pasteAction;
   QAction* clearAction;
   int actionTag;

   struct DataSets {
      QWidget* frame;
      QPushButton* letterButton;
      QLabel* pvName;
      QELabel* value;
   };

   DataSets items [NUMBER_OF_ITEMS];

   void setup ();
   void setReadOut (const QString status);
   double calculateCorrelationCoefficient ();
   void updateDataArrays ();
   void reDrawPlane ();
   void runSelectNameDialog (const int instance);

   // instance is 1 for X, 2 for Y.
   //
   void setPvName (const int instance, const QString& pvName);
   QString getPvName (const int instance) const;
   bool isInUse (const int instance) const;   // X/Y slot in use
   bool isDropAllowed () const;               // any drop at all
   bool isEventOk (QDropEvent* event) const;  // not from self and mime data has non-empty text
   bool processDropEvent (const int slot, QDropEvent* event);

   // Move to QEUtilities ??
   //
   static void tagWidget (QWidget* widget, const int tag);
   static int  getTag (const QWidget* widget, const int defaultValue = -1);

private slots:
   void setNewVariableName (QString pvName,
                            QString substitutions,
                            unsigned int vi);

   void sampleTimeout ();
   void reDrawTimeout ();
   void graphicMouseMove (const QPointF& posn);
   void graphicAreaDefinition (const QPointF& start, const QPointF& finish);

   void letterButtonClicked (bool checked);
   void contextMenuRequested (const QPoint& pos);
   void contextMenuSelected (QAction* selectedItem);
   void setArchiveData (const QObject* userData, const bool isOkay,
                        const QCaDataPointList& archiveData,
                        const QString& pvName, const QString& supplementary);

   void clearCountClick (bool);
   void dataScaleClick (bool);
   void dynamicScaleClick (bool);
   void manualScaleClick (bool);
   void plotModeSelect (bool);
   void timeSelectButtonClick (bool);
   void numberSamplesEditChange  (const double);
   void sampleIntervalEditChange (const double);
};

#endif // QE_CORRELATION_H
