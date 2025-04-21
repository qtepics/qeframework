/*  QEStripChart.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2024 Australian Synchrotron
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
 */

#ifndef QE_STRIP_CHART_H
#define QE_STRIP_CHART_H

#include <QAction>
#include <QBoxLayout>
#include <QColor>
#include <QDateTime>
#include <QGridLayout>
#include <QMenu>
#include <QMouseEvent>
#include <QObject>
#include <QPointF>
#include <QSize>
#include <QTimer>
#include <QVariant>
#include <QScrollArea>

#include <QEEnums.h>
#include <QCaAlarmInfo.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QCaDateTime.h>
#include <QCaConnectionInfo.h>
#include <QEDragDrop.h>
#include <QEAbstractDynamicWidget.h>
#include <QEWidget.h>
#include <persistanceManager.h>
#include <QEResizeableFrame.h>
#include <QEPVNameSelectDialog.h>
#include <QEGraphicNames.h>
#include <QEArchiveAccess.h>

#include "QEStripChartNames.h"
#include "QEStripChartToolBar.h"
#include "QEStripChartRangeDialog.h"
#include "QEStripChartTimeDialog.h"
#include "QEStripChartDurationDialog.h"
#include "QEStripChartState.h"

// Differed class declaration - no explicit dependency on Qwt header files.
//
class QEGraphic;

// Avoid mutual header references.
//
class QEStripChartItem;

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEStripChart :
   public QEAbstractDynamicWidget
{
   Q_OBJECT

public:
   Q_PROPERTY (int     duration   READ getDuration               WRITE setDuration)
   Q_PROPERTY (double  yMinimum   READ getYMinimum               WRITE setYMinimum)
   Q_PROPERTY (double  yMaximum   READ getYMaximum               WRITE setYMaximum)

   // normal/reverse - default is normal.
   //
   Q_PROPERTY (QE::VideoModes videoMode  READ getVideoMode       WRITE videoModeSelected)

   // manual/dynamic - default is dynamic.
   // Use a restricted range as design time.
   //
   enum PropertyChartYRanges {
      manual = QEStripChartNames::manual,
      dynamic = QEStripChartNames::dynamic
   };
   Q_ENUM (PropertyChartYRanges)

   Q_PROPERTY (PropertyChartYRanges
                       chartRange READ getYRangeMode             WRITE setYRangeMode)

   // Linear/log scale - default is linear.
   //
   Q_PROPERTY (QEStripChartNames::YScaleModes
                       scaleMode  READ getYScaleMode             WRITE yScaleModeSelected)

   // Layout control
   //
   Q_PROPERTY (bool enableContextMenu  READ getEnableConextMenu  WRITE setEnableConextMenu)
   Q_PROPERTY (bool toolBarIsVisible   READ getToolBarVisible    WRITE setToolBarVisible)
   Q_PROPERTY (bool pvItemsIsVisible   READ getPvItemsVisible    WRITE setPvItemsVisible)
   Q_PROPERTY (int numberPvsVisible    READ getNumberPvsVisible  WRITE setNumberPvsVisible)

   // Note, a property macro in the form 'Q_PROPERTY(QString variableName READ ...' doesn't work.
   // A property name ending with 'Name' results in some sort of string a variable being displayed,
   // but will only accept alphanumeric and won't generate callbacks on change.
   //
   Q_PROPERTY (QString variable1  READ getPropertyVariableName1  WRITE setPropertyVariableName1)
   Q_PROPERTY (QString variable2  READ getPropertyVariableName2  WRITE setPropertyVariableName2)
   Q_PROPERTY (QString variable3  READ getPropertyVariableName3  WRITE setPropertyVariableName3)
   Q_PROPERTY (QString variable4  READ getPropertyVariableName4  WRITE setPropertyVariableName4)
   Q_PROPERTY (QString variable5  READ getPropertyVariableName5  WRITE setPropertyVariableName5)
   Q_PROPERTY (QString variable6  READ getPropertyVariableName6  WRITE setPropertyVariableName6)
   Q_PROPERTY (QString variable7  READ getPropertyVariableName7  WRITE setPropertyVariableName7)
   Q_PROPERTY (QString variable8  READ getPropertyVariableName8  WRITE setPropertyVariableName8)
   Q_PROPERTY (QString variable9  READ getPropertyVariableName9  WRITE setPropertyVariableName9)
   Q_PROPERTY (QString variable10 READ getPropertyVariableName10 WRITE setPropertyVariableName10)
   Q_PROPERTY (QString variable11 READ getPropertyVariableName11 WRITE setPropertyVariableName11)
   Q_PROPERTY (QString variable12 READ getPropertyVariableName12 WRITE setPropertyVariableName12)
   Q_PROPERTY (QString variable13 READ getPropertyVariableName13 WRITE setPropertyVariableName13)
   Q_PROPERTY (QString variable14 READ getPropertyVariableName14 WRITE setPropertyVariableName14)
   Q_PROPERTY (QString variable15 READ getPropertyVariableName15 WRITE setPropertyVariableName15)
   Q_PROPERTY (QString variable16 READ getPropertyVariableName16 WRITE setPropertyVariableName16)

   Q_PROPERTY (QString aliasName1       READ getAliasName1     WRITE setAliasName1)
   Q_PROPERTY (QString aliasName2       READ getAliasName2     WRITE setAliasName2)
   Q_PROPERTY (QString aliasName3       READ getAliasName3     WRITE setAliasName3)
   Q_PROPERTY (QString aliasName4       READ getAliasName4     WRITE setAliasName4)
   Q_PROPERTY (QString aliasName5       READ getAliasName5     WRITE setAliasName5)
   Q_PROPERTY (QString aliasName6       READ getAliasName6     WRITE setAliasName6)
   Q_PROPERTY (QString aliasName7       READ getAliasName7     WRITE setAliasName7)
   Q_PROPERTY (QString aliasName8       READ getAliasName8     WRITE setAliasName8)
   Q_PROPERTY (QString aliasName9       READ getAliasName9     WRITE setAliasName9)
   Q_PROPERTY (QString aliasName10      READ getAliasName10    WRITE setAliasName10)
   Q_PROPERTY (QString aliasName11      READ getAliasName11    WRITE setAliasName11)
   Q_PROPERTY (QString aliasName12      READ getAliasName12    WRITE setAliasName12)
   Q_PROPERTY (QString aliasName13      READ getAliasName13    WRITE setAliasName13)
   Q_PROPERTY (QString aliasName14      READ getAliasName14    WRITE setAliasName14)
   Q_PROPERTY (QString aliasName15      READ getAliasName15    WRITE setAliasName15)
   Q_PROPERTY (QString aliasName16      READ getAliasName16    WRITE setAliasName16)

   // NOTE: Where ever possible I spell colour properly.
   //
   Q_PROPERTY (QColor  colour1    READ getColour1Property        WRITE setColour1Property)
   Q_PROPERTY (QColor  colour2    READ getColour2Property        WRITE setColour2Property)
   Q_PROPERTY (QColor  colour3    READ getColour3Property        WRITE setColour3Property)
   Q_PROPERTY (QColor  colour4    READ getColour4Property        WRITE setColour4Property)
   Q_PROPERTY (QColor  colour5    READ getColour5Property        WRITE setColour5Property)
   Q_PROPERTY (QColor  colour6    READ getColour6Property        WRITE setColour6Property)
   Q_PROPERTY (QColor  colour7    READ getColour7Property        WRITE setColour7Property)
   Q_PROPERTY (QColor  colour8    READ getColour8Property        WRITE setColour8Property)
   Q_PROPERTY (QColor  colour9    READ getColour9Property        WRITE setColour9Property)
   Q_PROPERTY (QColor  colour10   READ getColour10Property       WRITE setColour10Property)
   Q_PROPERTY (QColor  colour11   READ getColour11Property       WRITE setColour11Property)
   Q_PROPERTY (QColor  colour12   READ getColour12Property       WRITE setColour12Property)
   Q_PROPERTY (QColor  colour13   READ getColour13Property       WRITE setColour13Property)
   Q_PROPERTY (QColor  colour14   READ getColour14Property       WRITE setColour14Property)
   Q_PROPERTY (QColor  colour15   READ getColour15Property       WRITE setColour15Property)
   // Colour 16 is hard coded black/white

   /// Macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2...
   /// Values may be quoted strings. For example, 'SAMPLE=SAM1, NAME = "Ref foil"'
   /// These substitutions are applied to all the variable names.
   Q_PROPERTY (QString variableSubstitutions READ  getVariableNameSubstitutionsProperty
                                             WRITE setVariableNameSubstitutionsProperty)

public:
   enum Constants {
      NUMBER_OF_PVS = 16
   };

   // Constructors
   //
   explicit QEStripChart (QWidget * parent = 0);
   ~QEStripChart ();

   QSize sizeHint () const;

   // Start time, end time, and duration (seconds).
   // Three values, but only two are indpendent.
   //
   // Setting end time keeps duration fixed and adjusts start time.
   // Setting duration keeps end time fixed and adjusts start time.
   // Setting a duration value < 1 limited to 1 second.
   //
   QDateTime getStartDateTime () const;

   QDateTime getEndDateTime () const;
   void setEndDateTime (QDateTime endDateTimeIn);

   int getDuration () const;
   void setDuration (int durationIn);

   double getYMinimum () const;
   void setYMinimum (const double yMinimumIn);

   double getYMaximum () const;
   void setYMaximum (const double yMaximumIn);

   // Combine setYMinimum and setYMaximum.
   //
   void setYRange (const double yMinimumIn, const double yMaximumIn);

   void setPvName (const int slot, const QString& pvName);
   QString getPvName (const int slot) const;

   // Add PV to next avialble slot (if any).
   // returns slot number 0 .. Max - 1 iff successful otherwise -1.
   //
   int addPvName (const QString& pvName);

   // Clear all PV names.
   //
   void clearAllPvNames ();

   PropertyChartYRanges getYRangeMode () const;
   void setYRangeMode (const PropertyChartYRanges scale);

   QE::VideoModes getVideoMode () const;
   QEStripChartNames::YScaleModes getYScaleMode () const;

   // Allow arbitary action to be added to the item menus.
   // Note: The chart takes owbership of these actions.
   // Set inUseMenu true for slot used menu, false for empty menu.
   // This specifically added for kubili display manager.
   //
   void updateItemMenu (const int slot, QAction* action, const bool inUseMenu);

public slots:
   void videoModeSelected (const QE::VideoModes mode);
   void yRangeSelected (const QEStripChartNames::ChartYRanges scale);
   void yScaleModeSelected (const QEStripChartNames::YScaleModes mode);
   void writeTraceToFile (const int slot);

protected:
   bool eventFilter (QObject* watched, QEvent* event);

   QMenu* buildContextMenu ();                        // Build the Strip Chart specific context menu
   void contextMenuTriggered (int selectedItemNum);   // An action was selected from the context menu

   // Override QEAbstractDynamicWidget functions.
   //
   void pvLabelModeChanged ();

   // Override QWidget functions - call up standard handlers defined in QEDragDrop.
   // Drag and Drop
   void mousePressEvent (QMouseEvent *event)    { this->qcaMousePressEvent (event); }
   void dragEnterEvent (QDragEnterEvent *event) { this->qcaDragEnterEvent (event);  }
   void dropEvent (QDropEvent *event)           { this->qcaDropEvent (event, true); }
   //
   // This widget uses the default setDrop/getDrop defined in QEWidget.

   // Copy / paste
   //
   QString copyVariable ();
   QVariant copyData ();

   // override virtual functions
   //
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);
   void establishConnection (unsigned int variableIndex);
   void saveConfiguration (PersistanceManager* pm);
   void restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase);

   // Also used by QEStripChartItem
   //
   friend class QEStripChartItem;

   void addToPredefinedList (const QString & pvName);
   QStringList getPredefinedPVNameList () const;
   QString getPredefinedItem (int i) const;
   void setReplotIsRequired () { this->replotIsRequired = true; }
   void evaluateAllowDrop ();

private:
   void doCurrentValueCalculations ();      // calculates based on current chart values

   void plotData ();
   QDateTime timeAt (const double x) const; // get time corresponding to graphic x coord.

   // Internal widgets and state data.
   //
   QEPVNameSelectDialog* pvNameSelectDialog;

   QEStripChartToolBar* toolBar;
   QEResizeableFrame* toolBarResize;

   QFrame* pvFrame;
   QGridLayout *pvGridLayout;

   QScrollArea* pvScrollArea;
   QEResizeableFrame* pvResizeFrame;

   QEGraphic* plotArea;
   QFrame* plotFrame;

   QVBoxLayout* layout1;
   QVBoxLayout* layout2;

   QEStripChartItem* items [NUMBER_OF_PVS];

   bool isNormalVideo;

   // State data
   //
   QEStripChartNames::ChartYRanges chartYScale;
   QEStripChartNames::YScaleModes yScaleMode;
   QEStripChartNames::ChartTimeModes chartTimeMode;
   double timeScale;             // 1 => units are seconds, 60 => x units are minutes, etc.
   QString timeUnits;

   QEStripChartStateList chartStateList;

   // Timer to keep strip chart scrolling
   //
   static QTimer* tickTimer;
   int tickTimerCount;
   bool replotIsRequired;

   // Chart time range in seconds.
   //
   int duration;
   QDateTime endDateTime;
   Qt::TimeSpec timeZoneSpec;

   QEStripChartTimeDialog *timeDialog;
   QEStripChartDurationDialog *durationDialog;
   QEArchiveAccess* archiveAccess;

   // Chart lower/upper range
   //
   double yMinimum;
   double yMaximum;
   QEStripChartRangeDialog* yRangeDialog;

   // Items associated with selected point / box markup.
   //
   int selectedPointSlot;
   QCaDateTime selectedPointDateTime;
   double selectedPointValue;

   bool enableConextMenu;
   bool toolBarIsVisible;
   bool pvItemsIsVisible;
   int numberPvsVisible;

   // Functions
   //
   void createInternalWidgets ();

   QEStripChartItem* getItem (const int slot) const;
   void calcDisplayMinMax ();
   const QCaDataPoint* findNearestPoint (const QPointF& posn,
                                         int& slot) const;

   void setReadOut (const QString& text);
   void setNormalBackground (const bool isNormalVideo);

   void pushState ();
   void prevState ();
   void nextState ();

   void captureState (QEStripChartState& chartState);
   void applyState (const QEStripChartState& chartState);

public:
   // Property support functions.
   //
   void setEnableConextMenu (bool enable);
   bool getEnableConextMenu () const;

   void setToolBarVisible (bool visible);
   bool getToolBarVisible () const;

   void setPvItemsVisible (bool visible);
   bool getPvItemsVisible () const;

   void setNumberPvsVisible (int number);
   int getNumberPvsVisible () const;

   void    setVariableNameProperty (const int slot, const QString& pvName);
   QString getVariableNameProperty (const int slot) const;

   void    setVariableNameSubstitutionsProperty (const QString& variableNameSubstitutions);
   QString getVariableNameSubstitutionsProperty() const;

   void    setAliasName (const int slot, const QString& aliasName);
   QString getAliasName (const int slot) const;

   void   setColourProperty (const int slot, const QColor& color);
   QColor getColourProperty (const int slot) const;

   // The actual subsitutions are handled by embedded chart items and associated QELabels,
   // this String used just supports the property.
   //
   QString variableNameSubstitutions;

   // Property access READ and WRITE functions.
   // We can define the access functions using a macro.
   // Alas, due to SDK limitation, we cannot embedded the property definition itself in a macro.
   //
   #define PROPERTY_ACCESS(slot)                                                                                  \
      void    setPropertyVariableName##slot (QString& name) { this->setVariableNameProperty (slot - 1, name); }   \
      QString getPropertyVariableName##slot () const { return this->getVariableNameProperty (slot - 1); }         \
                                                                                                                  \
      void    setAliasName##slot (QString& aliasName) { this->setAliasName (slot - 1, aliasName); }               \
      QString getAliasName##slot () const      { return this->getAliasName (slot - 1); }                          \
                                                                                                                  \
      void   setColour##slot##Property (QColor& colour) { this->setColourProperty (slot - 1, colour); }           \
      QColor getColour##slot##Property () const  { return this->getColourProperty (slot - 1); }


   PROPERTY_ACCESS  (1)
   PROPERTY_ACCESS  (2)
   PROPERTY_ACCESS  (3)
   PROPERTY_ACCESS  (4)
   PROPERTY_ACCESS  (5)
   PROPERTY_ACCESS  (6)
   PROPERTY_ACCESS  (7)
   PROPERTY_ACCESS  (8)
   PROPERTY_ACCESS  (9)
   PROPERTY_ACCESS  (10)
   PROPERTY_ACCESS  (11)
   PROPERTY_ACCESS  (12)
   PROPERTY_ACCESS  (13)
   PROPERTY_ACCESS  (14)
   PROPERTY_ACCESS  (15)
   PROPERTY_ACCESS  (16)

#undef PROPERTY_ACCESS

   void menuSetYScale (QEStripChartNames::ChartYRanges ys);

private slots:
   void tickTimeout ();

   // From tool bar
   //
   void stateSelected (const QEStripChartNames::StateModes mode);
   void durationSelected (const int seconds);
   void selectDuration ();
   void timeZoneSelected (const Qt::TimeSpec timeZoneSpec);
   void playModeSelected (const QEStripChartNames::PlayModes mode);
   void readArchiveSelected ();

   // From the QEGraphic plot object.
   //
   void plotMouseMove  (const QPointF& posn);
   void zoomInOut      (const QPointF& about, const int zoomAmount);
   void scaleSelect    (const QPointF& origin, const QPointF& offset);
   void lineSelected   (const QPointF& origin, const QPointF& offset);
   void markupMove     (const QEGraphicNames::Markups markup, const QPointF& position = QPointF (0.0, 0.0));

   // From archiver access.
   //
   void archiveStatus (const QEArchiveAccess::StatusList& statusList);
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEStripChart::PropertyChartYRanges)
#endif

#endif   // QE_STRIP_CHART_H
