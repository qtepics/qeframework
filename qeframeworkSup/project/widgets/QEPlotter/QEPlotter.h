/*  QEPlotter.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2019 Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
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

#ifndef QE_PLOTTER_H
#define QE_PLOTTER_H

#include <QColor>
#include <QCheckBox>
#include <QColorDialog>
#include <QFrame>
#include <QHash>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QScrollArea>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVector>
#include <QWidget>

#include <QEFrameworkLibraryGlobal.h>
#include <QEActionRequests.h>
#include <QEFloatingArray.h>
#include <QEFloatingFormatting.h>
#include <QEIntegerFormatting.h>
#include <QEAbstractDynamicWidget.h>
#include <QEExpressionEvaluation.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEResizeableFrame.h>
#include <QEGraphicNames.h>
#include <QEStripChartRangeDialog.h>
#include <QETwinScaleSelectDialog.h>

#include "QEPlotterNames.h"
#include "QEPlotterItemDialog.h"
#include "QEPlotterMenu.h"
#include "QEPlotterState.h"
#include "QEPlotterToolBar.h"

// Differed class declaration - no explicit dependency on Qwt header files.
//
class QEGraphic;

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPlotter : public QEAbstractDynamicWidget
{
   Q_OBJECT
public:

   /// Default macro substitutions. The default is no substitutions.
   /// The format is NAME1=VALUE1[,] NAME2=VALUE2...
   /// Values may be quoted strings. For example, 'SAMPLE=SAM1, NAME = "Ref foil"'
   /// These substitutions are applied to all the variable names.
   //
   Q_PROPERTY (QString variableSubstitutions READ getVariableSubstitutions WRITE setVariableSubstitutions)

   // Layout control
   //
   Q_PROPERTY (bool enableContextMenu  READ getEnableConextMenu  WRITE setEnableConextMenu)
   Q_PROPERTY (bool toolBarIsVisible   READ getToolBarVisible    WRITE setToolBarVisible)
   Q_PROPERTY (bool pvItemsIsVisible   READ getPvItemsVisible    WRITE setPvItemsVisible)
   Q_PROPERTY (bool statusIsVisible    READ getStatusVisible     WRITE setStatusVisible)

   // normal/reverse - default is normal.
   // Keep same enum names as strip chart
   enum VideoModes {
      normal,          // white background
      reverse          // black backgound
   };

   Q_ENUMS (VideoModes)

   Q_PROPERTY (VideoModes videoMode    READ getVideoMode         WRITE setVideoMode)

   // Default is false - i.e. linear
   Q_PROPERTY (bool xLogarithmic       READ getXLogarithmic      WRITE setXLogarithmic)
   Q_PROPERTY (bool yLogarithmic       READ getYLogarithmic      WRITE setYLogarithmic)

   // Default is true - i.e. visible
   Q_PROPERTY (bool axisEnableX        READ getAxisEnableX       WRITE setAxisEnableX)
   Q_PROPERTY (bool axisEnableY        READ getAxisEnableY       WRITE setAxisEnableY)

   // Manual scaling
   //
   Q_PROPERTY (double xMinimum         READ getXMinimum          WRITE setXMinimum)
   Q_PROPERTY (double xMaximum         READ getXMaximum          WRITE setXMaximum)
   Q_PROPERTY (double yMinimum         READ getYMinimum          WRITE setYMinimum)
   Q_PROPERTY (double yMaximum         READ getYMaximum          WRITE setYMaximum)

   // Context menu legend
   //
   Q_PROPERTY (QString contextMenuEmitText    READ getMenuEmitText      WRITE setMenuEmitText)

   // Data and Size properties,
   //
   Q_PROPERTY (QString DataVariableX   READ getDataPVX  WRITE setDataPVX)
   Q_PROPERTY (QString DataVariableA   READ getDataPVA  WRITE setDataPVA)
   Q_PROPERTY (QString DataVariableB   READ getDataPVB  WRITE setDataPVB)
   Q_PROPERTY (QString DataVariableC   READ getDataPVC  WRITE setDataPVC)
   Q_PROPERTY (QString DataVariableD   READ getDataPVD  WRITE setDataPVD)
   Q_PROPERTY (QString DataVariableE   READ getDataPVE  WRITE setDataPVE)
   Q_PROPERTY (QString DataVariableF   READ getDataPVF  WRITE setDataPVF)
   Q_PROPERTY (QString DataVariableG   READ getDataPVG  WRITE setDataPVG)
   Q_PROPERTY (QString DataVariableH   READ getDataPVH  WRITE setDataPVH)
   Q_PROPERTY (QString DataVariableI   READ getDataPVI  WRITE setDataPVI)
   Q_PROPERTY (QString DataVariableJ   READ getDataPVJ  WRITE setDataPVJ)
   Q_PROPERTY (QString DataVariableK   READ getDataPVK  WRITE setDataPVK)
   Q_PROPERTY (QString DataVariableL   READ getDataPVL  WRITE setDataPVL)
   Q_PROPERTY (QString DataVariableM   READ getDataPVM  WRITE setDataPVM)
   Q_PROPERTY (QString DataVariableN   READ getDataPVN  WRITE setDataPVN)
   Q_PROPERTY (QString DataVariableO   READ getDataPVO  WRITE setDataPVO)
   Q_PROPERTY (QString DataVariableP   READ getDataPVP  WRITE setDataPVP)

   Q_PROPERTY (QString SizeVariableX   READ getSizePVX  WRITE setSizePVX)
   Q_PROPERTY (QString SizeVariableA   READ getSizePVA  WRITE setSizePVA)
   Q_PROPERTY (QString SizeVariableB   READ getSizePVB  WRITE setSizePVB)
   Q_PROPERTY (QString SizeVariableC   READ getSizePVC  WRITE setSizePVC)
   Q_PROPERTY (QString SizeVariableD   READ getSizePVD  WRITE setSizePVD)
   Q_PROPERTY (QString SizeVariableE   READ getSizePVE  WRITE setSizePVE)
   Q_PROPERTY (QString SizeVariableF   READ getSizePVF  WRITE setSizePVF)
   Q_PROPERTY (QString SizeVariableG   READ getSizePVG  WRITE setSizePVG)
   Q_PROPERTY (QString SizeVariableH   READ getSizePVH  WRITE setSizePVH)
   Q_PROPERTY (QString SizeVariableI   READ getSizePVI  WRITE setSizePVI)
   Q_PROPERTY (QString SizeVariableJ   READ getSizePVJ  WRITE setSizePVJ)
   Q_PROPERTY (QString SizeVariableK   READ getSizePVK  WRITE setSizePVK)
   Q_PROPERTY (QString SizeVariableL   READ getSizePVL  WRITE setSizePVL)
   Q_PROPERTY (QString SizeVariableM   READ getSizePVM  WRITE setSizePVM)
   Q_PROPERTY (QString SizeVariableN   READ getSizePVN  WRITE setSizePVN)
   Q_PROPERTY (QString SizeVariableO   READ getSizePVO  WRITE setSizePVO)
   Q_PROPERTY (QString SizeVariableP   READ getSizePVP  WRITE setSizePVP)

   Q_PROPERTY (QString AliasNameX      READ getAliasX   WRITE setAliasX)
   Q_PROPERTY (QString AliasNameA      READ getAliasA   WRITE setAliasA)
   Q_PROPERTY (QString AliasNameB      READ getAliasB   WRITE setAliasB)
   Q_PROPERTY (QString AliasNameC      READ getAliasC   WRITE setAliasC)
   Q_PROPERTY (QString AliasNameD      READ getAliasD   WRITE setAliasD)
   Q_PROPERTY (QString AliasNameE      READ getAliasE   WRITE setAliasE)
   Q_PROPERTY (QString AliasNameF      READ getAliasF   WRITE setAliasF)
   Q_PROPERTY (QString AliasNameG      READ getAliasG   WRITE setAliasG)
   Q_PROPERTY (QString AliasNameH      READ getAliasH   WRITE setAliasH)
   Q_PROPERTY (QString AliasNameI      READ getAliasI   WRITE setAliasI)
   Q_PROPERTY (QString AliasNameJ      READ getAliasJ   WRITE setAliasJ)
   Q_PROPERTY (QString AliasNameK      READ getAliasK   WRITE setAliasK)
   Q_PROPERTY (QString AliasNameL      READ getAliasL   WRITE setAliasL)
   Q_PROPERTY (QString AliasNameM      READ getAliasM   WRITE setAliasM)
   Q_PROPERTY (QString AliasNameN      READ getAliasN   WRITE setAliasN)
   Q_PROPERTY (QString AliasNameO      READ getAliasO   WRITE setAliasO)
   Q_PROPERTY (QString AliasNameP      READ getAliasP   WRITE setAliasP)

   // There is no X colour - we plot the Ys agaist X.
   //
   Q_PROPERTY (QColor  ColourA         READ getColourA  WRITE setColourA)
   Q_PROPERTY (QColor  ColourB         READ getColourB  WRITE setColourB)
   Q_PROPERTY (QColor  ColourC         READ getColourC  WRITE setColourC)
   Q_PROPERTY (QColor  ColourD         READ getColourD  WRITE setColourD)
   Q_PROPERTY (QColor  ColourE         READ getColourE  WRITE setColourE)
   Q_PROPERTY (QColor  ColourF         READ getColourF  WRITE setColourF)
   Q_PROPERTY (QColor  ColourG         READ getColourG  WRITE setColourG)
   Q_PROPERTY (QColor  ColourH         READ getColourH  WRITE setColourH)
   Q_PROPERTY (QColor  ColourI         READ getColourI  WRITE setColourI)
   Q_PROPERTY (QColor  ColourJ         READ getColourJ  WRITE setColourJ)
   Q_PROPERTY (QColor  ColourK         READ getColourK  WRITE setColourK)
   Q_PROPERTY (QColor  ColourL         READ getColourL  WRITE setColourL)
   Q_PROPERTY (QColor  ColourM         READ getColourM  WRITE setColourM)
   Q_PROPERTY (QColor  ColourN         READ getColourN  WRITE setColourN)
   Q_PROPERTY (QColor  ColourO         READ getColourO  WRITE setColourO)
   // There is no P colour. It is fixed as black (or white for reverse video).

   /// Allows specification of alias as a 'single property', which is also a slot.
   ///
   Q_PROPERTY (QStringList aliasNames  READ getAliasNameSet  WRITE setAliasNameSet)

public:
   explicit QEPlotter (QWidget *parent = 0);
   ~QEPlotter();
   QSize sizeHint () const;

   enum Constants {
      NUMBER_OF_PLOTS = 16,
      NUMBER_OF_SLOTS = 17     // includes the X slot
   };

   // Single function for all 'Data Set' properties.
   // Slot 0 is the X PV slot.
   //
public slots:
   void    setXYDataPV (const int slot, const QString& pvName);
public:
   QString getXYDataPV (const int slot) const;

   void    setXYSizePV (const int slot, const QString& pvName);
   QString getXYSizePV (const int slot) const;

public slots:
   void    setXYAlias (const int slot, const QString& alias);
public:
   QString getXYAlias (const int slot) const;

   void   setXYColour (const int slot, const QColor& colour);
   QColor getXYColour (const int slot) const;

   void setXYLineVisible (const int slot, const bool isVisible);
   bool getXYLineVisible (const int slot) const;

   void setXYLineBold (const int slot, const bool isBold);
   bool getXYLineBold (const int slot) const;

   void setXYLineDashed (const int slot, const bool isDashed);
   bool getXYLineDashed (const int slot) const;

   void setXYLineHasDots (const int slot, const bool hasDots);
   bool getXYLineHasDots (const int slot) const;

   void setEnableConextMenu (bool enable);
   bool getEnableConextMenu () const;

   void setMenuEmitText  (const QString& text);
   QString getMenuEmitText () const;

   void setToolBarVisible (bool visible);
   bool getToolBarVisible () const;

   void setPvItemsVisible (bool visible);
   bool getPvItemsVisible () const;

   void setStatusVisible (bool visible);
   bool getStatusVisible () const;

   void setAxisEnableX (bool axisEnableX);
   bool getAxisEnableX () const;

   void setAxisEnableY (bool axisEnableY);
   bool getAxisEnableY () const;

   void setVideoMode (const VideoModes mode);
   VideoModes getVideoMode () const;

   void setXLogarithmic (bool visible);
   bool getXLogarithmic () const;

   void setYLogarithmic (bool visible);
   bool getYLogarithmic () const;

   void setXMinimum (const double xMinimum);
   double getXMinimum () const;

   void setXMaximum (const double xMaximum);
   double getXMaximum () const;

   void setYMinimum (const double yMinimum);
   double getYMinimum () const;

   void setYMaximum (const double yMaximum);
   double getYMaximum () const;

   // Control paused state.
   //
   void setIsPaused (bool paused);
   bool getIsPaused () const;

   int addPvName (const QString& pvName);        // Add name to next available Y PV slot.
   void clearAllPvNames ();                      // Clear all PV names.

   int  getCrosshairIndex () const;

   // Property access READ and WRITE functions.
   // We can define the access functions using a macro.
   // Alas, due to SDK/moc limitation, we cannot embedded the property
   // definitions in a macro.
   //
   #define PROPERTY_ACCESS(letter, slot)                                                 \
      void    setDataPV##letter (QString name)  { this->setXYDataPV (slot, name); }      \
      QString getDataPV##letter () const { return this->getXYDataPV (slot); }            \
                                                                                         \
      void    setSizePV##letter (QString name)  { this->setXYSizePV (slot, name); }      \
      QString getSizePV##letter () const { return this->getXYSizePV (slot); }            \
                                                                                         \
      void    setAlias##letter  (QString name)  { this->setXYAlias (slot, name); }       \
      QString getAlias##letter  () const { return this->getXYAlias (slot); }             \
                                                                                         \
      void    setColour##letter (QColor colour)  { this->setXYColour (slot, colour); }   \
      QColor  getColour##letter () const { return this->getXYColour (slot); }


   PROPERTY_ACCESS  (X, 0)
   PROPERTY_ACCESS  (A, 1)
   PROPERTY_ACCESS  (B, 2)
   PROPERTY_ACCESS  (C, 3)
   PROPERTY_ACCESS  (D, 4)
   PROPERTY_ACCESS  (E, 5)
   PROPERTY_ACCESS  (F, 6)
   PROPERTY_ACCESS  (G, 7)
   PROPERTY_ACCESS  (H, 8)
   PROPERTY_ACCESS  (I, 9)
   PROPERTY_ACCESS  (J, 10)
   PROPERTY_ACCESS  (K, 11)
   PROPERTY_ACCESS  (L, 12)
   PROPERTY_ACCESS  (M, 13)
   PROPERTY_ACCESS  (N, 14)
   PROPERTY_ACCESS  (O, 15)
   PROPERTY_ACCESS  (P, 16)

   #undef PROPERTY_ACCESS

public slots:
   // Allows the chart range to be specified externally.
   // Chart mode will become fixed scale (as opposed to dynamic or fractional)
   //
   void setXRange (const double xMinimum, const double xMaximum);
   void setYRange (const double yMinimum, const double yMaximum);

signals:
   // Indicates data index cossponding to (vertical) crosshairs.
   // Emitted each time the cross hairs are moved.
   //
   void crosshairIndexChanged (int value);

   // Emitted once each time the PLOTTER_EMIT_COORDINATES context menu item selected.
   // We emit real world coordinates.
   //
   void coordinateSelected    (QPointF xyvalue);
   void xCoordinateSelected   (double xvalue);
   void yCoordinateSelected   (double yvalue);

   // Set, get and emit set of active data PV names.
   // Note: this applies to the data PV names only and does not include any sizing PVs.
   //
public slots:
   void setDataPvNameSet (const QStringList& pvNameSet);
   void setAliasNameSet (const QStringList& aliasNameSet);

   // Set Data PV and alias.
   //
   void setPlotterEntry (const int slot, const QString& pvName, const QString& alias);

public:
   QStringList getDataPvNameSet () const;
   QStringList getAliasNameSet () const;

   QEGraphic* getGraphic () const;   // allow access to the embedded graphic widget.

signals:
   void pvDataNameSetChanged (const QStringList& nameSet);
   void alaisNameSetChanged (const QStringList& nameSet);

   // Signal to 'launch a GUI'
   //
   void requestAction (const QEActionRequests&);

protected:
   // Implementation of QEWidget's virtual funtions
   //
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);
   void establishConnection (unsigned int variableIndex);
   void activated ();
   bool eventFilter (QObject *obj, QEvent *event);

   // Drag and drop
   //
   void mousePressEvent (QMouseEvent *event)    { qcaMousePressEvent (event); }
   void dragEnterEvent (QDragEnterEvent *event) { qcaDragEnterEvent (event, false); }
   void dropEvent (QDropEvent *event)           { qcaDropEvent (event, true); }
   // This widget uses the setDrop/getDrop defined in QEWidget.

   // Copy paste
   //
   QString copyVariable ();
   QVariant copyData ();

   void saveConfiguration (PersistanceManager* pm);
   void restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase);

   int findSlot (QObject *obj);
   QString getXYExpandedDataPV (const int slot) const;
   QString getXYExpandedSizePV (const int slot) const;

private:
   // Internal widgets.
   //
   QVBoxLayout* vLayout;
   QHBoxLayout* hLayout;
   QVBoxLayout* plotLayout;
   QHBoxLayout* statusLayout;
   QVBoxLayout* itemLayout;

   QEResizeableFrame* toolBarResize;
   QEPlotterToolBar* toolBar;
   QFrame* theMainFrame;
   QFrame* statusFrame;

   QFrame* plotFrame;
   QEGraphic* plotArea;

   QEResizeableFrame* itemResize;
   QScrollArea* itemScrollArea;
   QFrame* itemFrame;

   // Status items
   //
   QLabel* slotIndicator;
   QLabel* minLabel;
   QLabel* minValue;
   QLabel* maxLabel;
   QLabel* maxValue;
   QLabel* maxAtLabel;
   QLabel* maxAtValue;
   QLabel* fwhmLabel;   // Full Width (at) Half Max
   QLabel* fwhmValue;
   QLabel* comLabel;    // Centre Of Mass
   QLabel* comValue;

   QEStripChartRangeDialog* rangeDialog;
   QETwinScaleSelectDialog* twinRangeDialog;
   QColorDialog *colourDialog;
   QEPlotterItemDialog* dataDialog;
   QEPlotterMenu* generalContextMenu;

   bool xAxisIsEnabled;
   bool yAxisIsEnabled;

   // State data
   //
   bool isReverse;       // vs. Normal
   bool isPaused;        // vs. Updating
   double fixedMinX;
   double fixedMaxX;
   double fixedMinY;
   double fixedMaxY;
   QEPlotterNames::ScaleModes xScaleMode;
   QEPlotterNames::ScaleModes yScaleMode;
   QEPlotterStateList  stateList;

   bool enableConextMenu;
   bool toolBarIsVisible;
   bool pvItemsIsVisible;
   bool statusIsVisible;

   int selectedDataSet;
   static QTimer* tickTimer;
   int tickTimerCount;
   int  crosshairIndex;
   bool crosshairsAreRequired;  // controls both plotting and signal emmisions
   bool replotIsRequired;
   bool pvNameSetChangeInhibited;
   bool alaisSetChangeInhibited;
   QEIntegerFormatting  integerFormatting;
   QEFloatingFormatting floatingFormatting;

   bool    contextMenuIsOverGraphic;
   QPointF contextMenuRequestPosition;   // only meaninful when contextMenuIsOverGraphic is true.
   QString contextMenuEmitText;

   // Range of (unscaled) values of last plot.
   //
   double currentMinX;                     // current (as in last plotted) min X value.
   double currentMaxX;                     // ditto max X value
   double currentMinY;                     // ditto min Y value
   double currentMaxY;                     // ditto max Y value

   enum DataPlotKinds { NotInUse,          // blank  - not in use - no data - no plot
                        DataPVPlot,        // use specified PV to provide plot data
                        CalculationPlot }; // "= ..." - use given calculation for plot data

   enum SizePlotKinds { NotSpecified,      // blank - use maximum, available no. points
                        Constant,          // "[0-9]*" - used fixed integer as number of points
                        SizePVName };      // use speficed PV to provide number of points.


   class DataSets {
   public:
      explicit DataSets ();
      ~DataSets ();

      void setContext (QEPlotter* owner, int slot);
      void clear ();
      int getSlot () const;
      bool isInUse () const;
      int actualSize () const;
      int effectiveSize () const;
      QString getDataData () const;
      QString getSizeData () const;

      QCaVariableNamePropertyManager dataVariableNameManager;
      QCaVariableNamePropertyManager sizeVariableNameManager;
      DataPlotKinds dataKind;
      SizePlotKinds sizeKind;
      QString letter;
      QString pvName;
      QString aliasName;
      QString expression;        // when dataKind is CalculationPlot
      bool expressionIsValid;
      QEExpressionEvaluation* calculator;
      bool dataIsConnected;
      bool sizeIsConnected;
      int fixedSize;           // size set by user/designer
      int dbSize;              // size as defined by PV.
      QEFloatingArray data;
      QEFloatingArray dyByDx;

      // Min max values used when last plotted.
      //
      double plottedMin;
      double plottedMax;

      // n/a for the X data set, Y data sets only.
      //
      QColor colour;
      bool isDisplayed;
      bool isBold;
      bool isDashed;
      bool showDots;
      int median;  // 1 or 3 or 5

      // Widgets.
      //
      QFrame* frame;
      QHBoxLayout* frameLayout;
      QPushButton* letterButton;
      QLabel* itemName;
      QCheckBox* checkBox;
      QEPlotterMenu* itemMenu;
   private:
      QEPlotter* owner;
      int slot;
   };

   // Slot 0 used for X data - some redundancy (e.g. colour)
   //
   DataSets xy [1 + NUMBER_OF_PLOTS];

   // Allows an int, i.e. slot number, to be associated with any arbitary
   // object, and in this case a widget.
   //
   typedef QHash <const QObject*, int> ObjectToIntLookup;
   ObjectToIntLookup widgetToSlot;

   void createSlotWidgets (const int slot);
   void createInternalWidgets ();
   void selectDataSet (const int slot);
   void highLight (const int slot, const bool isHigh);

   void calcCrosshairIndex (const double x);

   void plot ();
   int maxActualYSizes () const;
   void doAnyCalculations ();

   // Calculate stats
   void processSelectedItem (const QEFloatingArray& xdata,
                             const QEFloatingArray& ydata,
                             const double yMin, const double yMax);

   void setReadOut (const QString& text);

   void captureState (QEPlotterState& state);
   void applyState (const QEPlotterState& state);

   void pushState ();
   void prevState ();
   void nextState ();

   bool connectMenuOrToolBar (QWidget* item) const;

   void sendRequestAction (const QString& action, const QString& pvName);

   // Perform a pvNameDropEvent 'drop'
   //
   void pvNameDropEvent (const int slot, QDropEvent *event);

   // Property READ WRITE functions.
   //
   void    setVariableSubstitutions (QString variableNameSubstitutions);
   QString getVariableSubstitutions () const;

   // Provides consistant interpretation of variableIndex.
   // Must be consistent with variableIndex allocation in the contructor.
   //
   bool isDataIndex   (const unsigned int vi) const;
   bool isSizeIndex   (const unsigned int vi) const;
   bool isXIndex      (const unsigned int vi) const;
   bool isYIndex      (const unsigned int vi) const;
   int  slotOf        (const unsigned int vi) const;
   unsigned int viOfDataSlot (const int slot) const;
   unsigned int viOfSizeSlot (const int slot) const;

   void updateLabel (const int slot);
   void setToolTipSummary ();
   void runDataDialog (const int slot, QWidget* control);

public slots:
   void setNewVariableName (QString variableName,
                            QString variableNameSubstitutions,
                            unsigned int variableIndex);

   void dataConnectionChanged (QCaConnectionInfo& connectionInfo,
                               const unsigned int& variableIndex);

   void dataArrayChanged (const QVector<double>& values,
                          QCaAlarmInfo& alarmInfo,
                          QCaDateTime& timeStamp,
                          const unsigned int& variableIndex);

   void sizeConnectionChanged (QCaConnectionInfo& connectionInfo,
                               const unsigned int& variableIndex);

   void sizeValueChanged (const long& value,
                          QCaAlarmInfo& alarmInfo,
                          QCaDateTime& timeStamp,
                          const unsigned int& variableIndex);

private slots:
   void letterButtonClicked (bool checked);
   void checkBoxStateChanged (int state);
   void tickTimeout ();

   void generalContextMenuRequested (const QPoint& pos);
   void itemContextMenuRequested (const QPoint& pos);

   // Handles all context menu and tool bar actions.
   //
   void menuSelected (const QEPlotterNames::MenuActions action, const int slot);

   // Handles signals from the plot object.
   //
   void plotMouseMove  (const QPointF& posn);
   void zoomInOut      (const QPointF& about, const int zoomAmount);

   void scaleSelect    (const QPointF& origin, const QPointF& offset);
   void lineSelected   (const QPointF& origin, const QPointF& offset);
   void markupMove     (const QEGraphicNames::Markups markup, const QPointF& position);

   friend class DataSets;
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEPlotter::VideoModes)
#endif

#endif // QE_PLOTTER_H
