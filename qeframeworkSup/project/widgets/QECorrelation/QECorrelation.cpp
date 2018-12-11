/*  QECorrelation.cpp
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
 *  Copyright (c) 2018 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QECorrelation.h>

// In this context, we are a user of the QEFramework library as we are using
// the moc generated ui_QEGeneralEdit.h file that needs access the meta
// types to be declared.
//
#define QE_DECLARE_METATYPE_IS_REQUIRED

#include <math.h>
#include <QClipboard>
#include <QDebug>
#include <QVariant>
#include <QEGraphic.h>
#include <ui_QECorrelation.h>

#define DEBUG qDebug() << "QECorrelation" << __LINE__ << __FUNCTION__ << "  "

enum PVIndices {
   QELabelPVIndex = 0,    // We "know" this
   xPvIndex = 1,
   yPvIndex = 2
};

//---------------------------------------------------------------------------------
// Index range checking macro function.
// Set default value to nil for void functions.
//
#define INDEX_CHECK(index, defaultValue) {                                  \
   if (((index) != xPvIndex) && ((index) != yPvIndex)) {                    \
      DEBUG << "index out of range: " << index;                             \
      return defaultValue;                                                  \
   }                                                                        \
}

// These assume the index has already be validated.
//
#define NAME_LABEL(index) ( (index == xPvIndex) ? this->uiForm->PV_Label1     : this->uiForm->PV_Label2 )
#define DATA_LABEL(index) ( (index == xPvIndex) ? this->uiForm->CA_DataLabel1 : this->uiForm->CA_DataLabel2 )

static const int numberOfCorrelationPoints = 5000;
static const double minSpan = 0.001;

static const QColor clWhite (0xFF, 0xFF, 0xFF, 0xFF);
static const QColor clBlack (0x00, 0x00, 0x00, 0xFF);
static const QColor clGridLine (0xC0C0C0);
static const QColor clReverseGridLine (0x404040);

static const QString inuseStyle  ("QWidget { background-color: #e0e0e0; }");
static const QString unusedStyle ("QWidget { background-color: #c0c0c0; }");

static const QString manualScaleWindowName = "Correlation Scale";
static const QString scaleOneName = "X";
static const QString scaleTwoName = "Y";

//--------------------------------------------------------------------------------------
//
QECorrelation::QECorrelation (QWidget *parent) : QEAbstractDynamicWidget (parent)
{
   this->setup ();
}

//------------------------------------------------------------------------------
//
QECorrelation::QECorrelation (const QString& xVariableName,
                              QWidget *parent) : QEAbstractDynamicWidget (parent)
{
   this->setup ();
   this->setPvName (xPvIndex, xVariableName);
}

//------------------------------------------------------------------------------
//
QECorrelation::QECorrelation (const QString& xVariableName,
                              const QString& yVariableName,
                              QWidget *parent) : QEAbstractDynamicWidget (parent)
{
   this->setup ();
   this->setPvName (xPvIndex, xVariableName);
   this->setPvName (yPvIndex, yVariableName);
}

//------------------------------------------------------------------------------
//
void QECorrelation::setup ()
{
   // Create internal widgets.  We used designer to create the form and
   // execute the uic auto generated code.
   //
   this->uiForm = new Ui::QECorrelation ();
   this->uiForm->setupUi (this);

   // The QEGraphic widget is a internal widget, not a plugin widget available
   // in designer, so must do this programatically.
   //
   this->plotArea = new QEGraphic (NULL);
   this->plotLayout = new QHBoxLayout (this->uiForm->Plane);
   this->plotLayout->setMargin (6);
   this->plotLayout->addWidget (this->plotArea);

   // Configure parant classes
   //
   this->setEnableEditPv (false);

   // No variables managed directly by this widget - left to the embedded QELabels
   //
   this->setNumVariables (0);

   // Set default frame attributes
   //
   this->setFrameShape  (QFrame::Box);
   this->setFrameShadow (QFrame::Sunken);

   this->setAllowDrop (true);
   this->setDisplayAlarmState (false);

   // Use default context menu.
   //
   this->setupContextMenu ();
   this->setNumberOfContextMenuItems (2);

   this->sampleTimer = new QTimer (this);
   this->reDrawTimer = new QTimer (this);

   this->isReverse = false;

   this->xVariableNameManager.setVariableIndex (xPvIndex);
   this->yVariableNameManager.setVariableIndex (yPvIndex);

   QObject::connect (&this->xVariableNameManager, SIGNAL (newVariableNameProperty (QString, QString, unsigned int)),
                     this,                        SLOT   (setNewVariableName      (QString, QString, unsigned int)));

   QObject::connect (&this->yVariableNameManager, SIGNAL (newVariableNameProperty (QString, QString, unsigned int)),
                     this,                        SLOT   (setNewVariableName      (QString, QString, unsigned int)));

   this->pvNameSelectDialog = new QEPVNameSelectDialog (this);
   this->scaleDialog = new QETwinScaleSelectDialog (manualScaleWindowName, scaleOneName, scaleTwoName, this);
   this->timeDialog = new QEStripChartTimeDialog (this);

   // Set up X/Y label context menu (as opposed to widget's overall context menu).
   //
   this->contextMenu = new QMenu (this);

   QAction* action;
   action = new QAction ("Add/Edit PV Name ", this->contextMenu);
   action->setData (QVariant (int (maAddEdit)));
   this->contextMenu->addAction (action);

   action = new QAction ("Paste PV Name ", this->contextMenu );
   action->setData (QVariant (int (maPaste)));
   this->contextMenu->addAction (action);
   this->pasteAction = action;  // save a reference.

   action = new QAction ("Clear ", this->contextMenu );
   action->setData (QVariant (int (maClear)));
   this->contextMenu->addAction (action);
   this->clearAction = action;  // save a reference.

   this->uiForm->Plane->setStyleSheet ("#Plane { border: 1px solid blue; }");

   this->xMin = 0.0;
   this->xMax = 100.0;
   this->yMin = 0.0;
   this->yMax = 100.0;

   this->scaleMode = smDynamic;
   this->plotMode = pmDots;

   // Tag Dots and Lines buttons.
   //
   QECorrelation::tagWidget (this->uiForm->ColourSpeedButton1, pmDots);
   QECorrelation::tagWidget (this->uiForm->ColourSpeedButton2, pmLines);

   // Tag X PV and Y PV items
   //
   QECorrelation::tagWidget (this->uiForm->Panel_1,         xPvIndex);
   QECorrelation::tagWidget (this->uiForm->CA_DataLabel1,   xPvIndex);
   QECorrelation::tagWidget (this->uiForm->PV_Label1,       xPvIndex);
   QECorrelation::tagWidget (this->uiForm->X_Letter_Button, xPvIndex);

   QECorrelation::tagWidget (this->uiForm->Panel_2,         yPvIndex);
   QECorrelation::tagWidget (this->uiForm->CA_DataLabel2,   yPvIndex);
   QECorrelation::tagWidget (this->uiForm->PV_Label2,       yPvIndex);
   QECorrelation::tagWidget (this->uiForm->Y_Letter_Button, yPvIndex);

   // Keep consistant with On_Plot_Mode_Select
   //
   QPen pen;
   pen.setWidth (1);
   pen.setColor (QColor ("blue"));
   this->plotArea->setCurvePen (pen);
   this->plotArea->setCurveStyle (QwtPlotCurve::Lines);

   // Set up letter buttons.
   //
   QObject::connect (this->uiForm->X_Letter_Button, SIGNAL (clicked (bool)),
                     this,   SLOT   (letterButtonClicked (bool)));

   QObject::connect (this->uiForm->Y_Letter_Button, SIGNAL (clicked (bool)),
                     this,   SLOT   (letterButtonClicked (bool)));

   // Set up context menus.
   //
   QLabel* pVLabel = NULL;

   pVLabel = this->uiForm->PV_Label1;
   pVLabel->setContextMenuPolicy (Qt::CustomContextMenu);
   QObject::connect (pVLabel, SIGNAL (customContextMenuRequested (const QPoint &)),
                     this,    SLOT   (contextMenuRequested       (const QPoint &)));

   pVLabel = this->uiForm->PV_Label2;
   pVLabel->setContextMenuPolicy (Qt::CustomContextMenu);
   QObject::connect (pVLabel, SIGNAL (customContextMenuRequested (const QPoint &)),
                     this,    SLOT   (contextMenuRequested       (const QPoint &)));

   QObject::connect (this->contextMenu, SIGNAL (triggered            (QAction*)),
                     this,              SLOT   (contextMenuSelected  (QAction*)));

   // Set up drap/drop.
   //
   pVLabel = this->uiForm->PV_Label1;
   pVLabel->setAcceptDrops (true);
   pVLabel->installEventFilter (this);

   pVLabel = this->uiForm->PV_Label2;
   pVLabel->setAcceptDrops (true);
   pVLabel->installEventFilter (this);

   // Set up archiver access
   //
   this->archiveAccess = new QEArchiveAccess (this);

   QObject::connect (this->archiveAccess,
                     SIGNAL (setArchiveData (const QObject*, const bool, const QCaDataPointList&,
                                       const QString&, const QString&)),
                     this,
                     SLOT   (setArchiveData (const QObject*, const bool, const QCaDataPointList&,
                                       const QString&, const QString&)));

   // Set up mouse position tracking.
   //
   QObject::connect (this->plotArea, SIGNAL (mouseMove (const QPointF&)),
                     this, SLOT     (graphicMouseMove (const QPointF&)));

   QObject::connect (this->plotArea, SIGNAL (areaDefinition (const QPointF&, const QPointF&)),
                     this, SLOT     (graphicAreaDefinition (const QPointF&, const QPointF&)));

   this->plotArea->setAvailableMarkups (QEGraphicNames::Area | QEGraphicNames::Line);

   // Set default sample and number of points
   //
   this->setSampleInterval (1.0);
   this->setNumberPoints (600);

   // Set up timers.
   //
   QObject::connect (this->sampleTimer, SIGNAL (timeout ()),
                     this,              SLOT   (sampleTimeout ()));

   QObject::connect (this->reDrawTimer, SIGNAL (timeout ()),
                     this,        SLOT   (reDrawTimeout ()));

   int ms = (int) (1000.0 * this->uiForm->Sample_Interval_Edit->getValue ());
   this->sampleTimer->start (ms);

   this->replotIsRequired = true;  // ensure process on first tick.
   this->reDrawTimer->start (50);  // mSec == 0.05 s

   this->updateDataArrays ();

   // Do the plumbing
   //
   QObject::connect (this->uiForm->loadButton, SIGNAL (clicked ()),
                     this, SLOT (loadWidgetConfiguration ()));

   QObject::connect (this->uiForm->saveButton, SIGNAL (clicked ()),
                     this, SLOT (saveWidgetConfiguration ()));

   QObject::connect (this->uiForm->Clear_Count_Button, SIGNAL (clicked (bool)),
                     this, SLOT (clearCountClick (bool)));

   QObject::connect (this->uiForm->Data_Scale_Button, SIGNAL (clicked (bool)),
                     this, SLOT (dataScaleClick (bool)));

   QObject::connect (this->uiForm->Dynamic_Scale_Button, SIGNAL (clicked (bool)),
                     this, SLOT (dynamicScaleClick (bool)));

   QObject::connect (this->uiForm->Manual_Scale_Button, SIGNAL (clicked (bool)),
                     this, SLOT (manualScaleClick (bool)));

   QObject::connect (this->uiForm->ColourSpeedButton1, SIGNAL (clicked (bool)),
                     this, SLOT (plotModeSelect (bool)));
   QObject::connect (this->uiForm->ColourSpeedButton2, SIGNAL (clicked (bool)),
                     this, SLOT (plotModeSelect (bool)));

   QObject::connect (this->uiForm->Time_Select_Button, SIGNAL (clicked (bool)),
                     this, SLOT (timeSelectButtonClick (bool)));

   QObject::connect (this->uiForm->Number_Samples_Edit, SIGNAL (valueChanged (const double)),
                     this, SLOT (numberSamplesEditChange (const double)));

   QObject::connect (this->uiForm->Sample_Interval_Edit, SIGNAL (valueChanged (const double)),
                     this, SLOT (sampleIntervalEditChange (const double)));
}

//--------------------------------------------------------------------------------------
//
QECorrelation::~QECorrelation ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
void QECorrelation::setNewVariableName (QString variableName,
                                        QString variableNameSubstitutions,
                                        unsigned int variableIndex)
{
   INDEX_CHECK (variableIndex,);

   QELabel* caLabel = DATA_LABEL (variableIndex);
   QLabel* pvNameLabel = NAME_LABEL (variableIndex);
   QString substitutedPvName;

   caLabel->setVariableNameAndSubstitutions (variableName, variableNameSubstitutions, QELabelPVIndex);
   caLabel->setText ("");

   substitutedPvName = caLabel->getSubstitutedVariableName (QELabelPVIndex);
   pvNameLabel->setText (substitutedPvName);
}

//------------------------------------------------------------------------------
//
int QECorrelation::addPvName (const QString& pvName)
{
   QString check;

   check = this->getPvName (xPvIndex);
   if (check.isEmpty()) {
      this->setPvName (xPvIndex, pvName);
      return xPvIndex;
   }

   check = this->getPvName (yPvIndex);
   if (check.isEmpty()) {
      this->setPvName (yPvIndex, pvName);
      return yPvIndex;
   }

   return -1;
}

//------------------------------------------------------------------------------
//
void QECorrelation::clearAllPvNames ()
{
   this->setPvName (xPvIndex, "");
   this->setPvName (yPvIndex, "");
}

//------------------------------------------------------------------------------
// Overriden, but not used.
//
qcaobject::QCaObject* QECorrelation::createQcaItem (unsigned int variableIndex)
{
   DEBUG << "unexpected call, variableIndex = " << variableIndex;
   return NULL;
}

//------------------------------------------------------------------------------
//
void QECorrelation::establishConnection (unsigned int variableIndex)
{
   DEBUG << "unexpected call, variableIndex = " << variableIndex;
}

//------------------------------------------------------------------------------
// Drag and Drop - no drop to self.
//
void QECorrelation::dragMoveEvent (QDragMoveEvent *event)
{
   QFrame* sourceFrame = NULL;

   // When dropping from anither application, the event source does not exist.
   //
   if (event && event->source()) {
      sourceFrame = dynamic_cast <QFrame*> (event->source()->parent());
   }

   if (sourceFrame) {
      // Parent is of the correct type.

      // Extract frame geometry and convert to global coordinates.
      // Map to glabl requires the sourceFrame parant.
      //
      QWidget* gp = dynamic_cast <QWidget*> (sourceFrame->parent ());

      QRect fg = sourceFrame->geometry ();
      QPoint gtl = gp->mapToGlobal (fg.topLeft());
      QPoint gbr = gp->mapToGlobal (fg.bottomRight());
      QRect globalFrameRect = QRect (gtl, gbr);

      // pos is relative this, the QECorrelation widget.
      //
      QPoint globalPos = this->mapToGlobal (event->pos ());

      // Convert drop position to global coordinates as well so that we
      // check if the would be drop location is own frame.
      //
      if (globalFrameRect.contains (globalPos)) {
         event->ignore ();
         return;
      }
   }

   // Allow / re-allow drop. Allow dropping onto other slots.
   //
   event->acceptProposedAction ();
}

//------------------------------------------------------------------------------
// Copy/Paste
//
QString QECorrelation::copyVariable ()
{
   QString result;

   // Create a space seperated list of PV names.
   //
   QString xPvName = this->getPvName (xPvIndex);
   QString yPvName = this->getPvName (yPvIndex);

   if (xPvName.isEmpty() || yPvName.isEmpty()) {
      // On of both are empty
      result = xPvName + yPvName;
   } else {
      result = xPvName + " " + yPvName;
   }
   return result;
}

//------------------------------------------------------------------------------
//
QVariant QECorrelation::copyData ()
{
   QString text;

   text.append (QString ("\nx\ty\n"));

   for (int j = 0; j < xData.count (); j++) {
      text.append (QString ("%1\t%2\n").arg (this->xData[j]).arg (this->yData[j]));
   }

   return QVariant (text);
}

//------------------------------------------------------------------------------
//
void QECorrelation::saveConfiguration (PersistanceManager* pm)
{
   const QString formName = this->getPersistantName();

   PMElement formElement = pm->addNamedConfiguration (formName);

   PMElement xPvElement  = formElement.addElement ("xPvName");
   xPvElement.addValue ("Name", this->getPvName (1));

   PMElement yPvElement = formElement.addElement ("yPvName");
   yPvElement.addValue ("Name", this->getPvName (2));

   PMElement samplePeriodElement = formElement.addElement ("samplePeriod");
   samplePeriodElement.addValue ("Value", this->uiForm->Sample_Interval_Edit->getValue ());

   PMElement maximumPointsElement = formElement.addElement ("maximumPoints");
   maximumPointsElement.addValue ("Value", this->uiForm->Number_Samples_Edit->getValue ());
}

//------------------------------------------------------------------------------
//
void QECorrelation::restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase)
{
   if (restorePhase != FRAMEWORK) return;

   const QString formName = this->getPersistantName ();
   PMElement formElement = pm->getNamedConfiguration (formName);

   // Restore each PV.
   //
   PMElement xPvElement  = formElement.getElement ("xPvName");
   if (!xPvElement.isNull ()) {
      QString pvName;
      bool status;

      // Attempt to extract a PV name
      //
      status = xPvElement.getValue ("Name", pvName);
      if (status) {
         this->setPvName (1, pvName);
      }
   }

   PMElement yPvElement  = formElement.getElement ("yPvName");
   if (!xPvElement.isNull ()) {
      QString pvName;
      bool status;

      // Attempt to extract a PV name
      //
      status = yPvElement.getValue ("Name", pvName);
      if (status) {
         this->setPvName (2, pvName);
      }
   }

   PMElement samplePeriodElement = formElement.getElement ("samplePeriod");
   if (!samplePeriodElement.isNull ()) {
      double value;
      bool status;

      // Attempt to extract a PV name
      //
      status = samplePeriodElement.getValue ("Value", value);
      if (status) {
         this->uiForm->Sample_Interval_Edit->setValue (value);
      }
   }

   PMElement maximumPointsElement = formElement.getElement ("maximumPoints");
   if (!maximumPointsElement.isNull ()) {
      int value;
      bool status;

      // Attempt to extract a PV name
      //
      status = maximumPointsElement.getValue ("Value", value);
      if (status) {
         this->uiForm->Number_Samples_Edit->setValue (value);
      }
   }
}

//------------------------------------------------------------------------------
// Property READ WRITE functions.
//
void QECorrelation::setXVariableName (const QString& pvName)
{
   this->xVariableNameManager.setVariableNameProperty (pvName);
}

//------------------------------------------------------------------------------
//
QString QECorrelation::getXVariableName () const
{
   return this->xVariableNameManager.getVariableNameProperty ();
}

//------------------------------------------------------------------------------
//
void QECorrelation::setYVariableName (const QString& pvName)
{
   this->yVariableNameManager.setVariableNameProperty (pvName);
}

//------------------------------------------------------------------------------
//
QString QECorrelation::getYVariableName () const
{
   return this->yVariableNameManager.getVariableNameProperty ();
}

//------------------------------------------------------------------------------
//
void  QECorrelation::setVariableSubstitutions (const QString& defaultSubstitutions)
{
   // Use same default subsitutions for both PVs used by this widget.
   //
   this->xVariableNameManager.setSubstitutionsProperty (defaultSubstitutions);
   this->yVariableNameManager.setSubstitutionsProperty (defaultSubstitutions);
}

//------------------------------------------------------------------------------
//
QString QECorrelation::getVariableSubstitutions () const
{
   // Either one of the name managers can provide the subsitutions.
   //
   return this->xVariableNameManager.getSubstitutionsProperty ();
}

//------------------------------------------------------------------------------
//
void QECorrelation::setSampleInterval (const double interval)
{
   this->mSampleInterval = LIMIT (interval, 0.2, 3600.0);
   this->uiForm->Sample_Interval_Edit->setValue (this->mSampleInterval);
   this->updateDataArrays ();
}

//------------------------------------------------------------------------------
//
double QECorrelation::getSampleInterval () const
{
   return this->mSampleInterval;
}

//------------------------------------------------------------------------------
//
void QECorrelation::setNumberPoints (const int number)
{
   this->mNumberPoints = LIMIT (number, 4, 5000);
   this->uiForm->Number_Samples_Edit->setValue (this->mNumberPoints);
   this->updateDataArrays ();
}

//------------------------------------------------------------------------------
//
int QECorrelation::getNumberPoints () const
{
   return this->mNumberPoints;
}

//------------------------------------------------------------------------------
//
void QECorrelation::setXLogarithmic (bool isLog)
{
   this->plotArea->setXLogarithmic (isLog);
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
bool QECorrelation::getXLogarithmic () const
{
   return this->plotArea->getXLogarithmic ();
}

//------------------------------------------------------------------------------
//
void QECorrelation::setYLogarithmic (bool isLog)
{
   this->plotArea->setYLogarithmic (isLog);
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
bool QECorrelation::getYLogarithmic () const
{
   return this->plotArea->getYLogarithmic ();
}

//------------------------------------------------------------------------------
//
void QECorrelation::setReadOut (const QString status)
{
    message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STATUS);
    this->sendMessage (status, mt);
}

//------------------------------------------------------------------------------
// Refer to http://en.wikipedia.org/wiki/Correlation_and_dependence
//
double QECorrelation::calculateCorrelationCoefficient ()
{
   double result;
   int number;
   double sumX;
   double sumY ;
   double sumXX;
   double sumYY ;
   double sumXY;
   double meanX;
   double meanY ;
   double meanXX;
   double meanYY ;
   double meanXY;
   double varX;
   double varY;
   double sdX;
   double sdY;

   number = this->xData.count ();
   if (number <= 0) return 0.0;

   // Sum x, x^2, y, y^2 and xy.
   //
   sumX  = 0.0;
   sumY  = 0.0;
   sumXX = 0.0;
   sumYY = 0.0;
   sumXY = 0.0;

   for (int j = 0; j < number; j++) {
      double x = this->xData.value (j);
      double y = this->yData.value (j);

      sumX  += x;
      sumXX += x*x;
      sumY  += y;
      sumYY += y*y;
      sumXY += x*y;
   }

   // Calculate means.
   //
   meanX  = sumX / number;
   meanY  = sumY / number;
   meanXX = sumXX / number;
   meanYY = sumYY / number;
   meanXY = sumXY / number;

   // Calculate X and Y variances.
   //
   varX = meanXX - meanX * meanX;
   varY = meanYY - meanY * meanY;

   // Check for zero variance in X and/or Y - need to avoid divide by 0.
   //
   // Theoretically, we need only test for zero, but rounding errors
   // mean we can end up with very small negative numbers.
   //
   if (varX > 0.0 && varY > 0.0) {

      sdX = sqrt (varX);
      sdY = sqrt (varY);

      result = (meanXY - meanX * meanY) / (sdX * sdY);
   } else {
      result = 0.0;
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QECorrelation::reDrawPlane ()
{
   // Prolog
   //
   // Do dynamic scaling if applicable.
   //
   if (this->scaleMode == smDynamic) {
      this->xMin = this->xData.minimumValue (0.0);
      this->xMax = this->xData.maximumValue (100.0);
      this->yMin = this->yData.minimumValue (0.0);
      this->yMax = this->yData.maximumValue (100.0);
   }

   // Set up working copies.
   //
   double TX_Min = this->xMin;
   double TX_Max = this->xMax;
   double TY_Min = this->yMin;
   double TY_Max = this->yMax;

   // Ensure sensible.
   //
   if (TX_Max < TX_Min + minSpan) {
      double T = (TX_Min + TX_Max)/2.0;
      TX_Min = T - 0.5*minSpan;
      TX_Max = T + 0.5*minSpan;
   }

   if (TY_Max < TY_Min + minSpan) {
      double T = (TY_Min + TY_Max)/2.0;
      TY_Min = T - 0.5*minSpan;
      TY_Max = T + 0.5*minSpan;
   }

   // First release any/all previously allocated curves.
   //
   this->plotArea->releaseCurves ();

   this->plotArea->setXRange (TX_Min, TX_Max, QEGraphic::SelectByValue, 5, false);
   this->plotArea->setYRange (TY_Min, TY_Max, QEGraphic::SelectByValue, 5, false);


   // Set up brackground and grid.
   //
   QColor background;
   QColor grid;
   if (this->isReverse) {
      background = clBlack;
      grid = clReverseGridLine;
   } else {
      background = clWhite;
      grid = clGridLine;
   }

   this->plotArea->setBackgroundColour (background);

   QPen pen;
   pen.setColor (grid);
   pen.setWidth (1);
   pen.setStyle (Qt::DashLine);
   this->plotArea->setGridPen (pen);

   pen.setColor (QColor ("blue"));
   pen.setStyle (Qt::SolidLine);

   switch (this->plotMode) {
      case pmDots:
         pen.setWidth (4);
         this->plotArea->setCurvePen (pen);
         this->plotArea->setCurveStyle (QwtPlotCurve::Dots);
         break;

      case pmLines:
         pen.setWidth (1);
         this->plotArea->setCurvePen (pen);
         this->plotArea->setCurveStyle (QwtPlotCurve::Lines);
         break;
   }

   this->plotArea->plotCurveData (this->xData, this->yData);
   this->plotArea->replot ();

   // Ensure next redraw timer timeout only invokes plot if needs be.
   //
   this->replotIsRequired = false;
}

//------------------------------------------------------------------------------
//
void QECorrelation::updateDataArrays ()
{
   double samplePeriod;
   int maximumPoints;
   double maximumPeriod;
   double currentPeriod;
   int extra;
   int number;
   double correlation;

   samplePeriod = this->uiForm->Sample_Interval_Edit->getValue ();
   maximumPoints = (int) this->uiForm->Number_Samples_Edit->getValue ();

   // Purge old data. Both x and y should be in step with the same number
   // of "extra" points, but no harm done being pedantic here.
   //
   extra = this->xData.count () - maximumPoints;
   if (extra > 0) {
      this->xData.remove (0, extra);   // remove 1st extra values
   }

   extra = this->yData.count () - maximumPoints;
   if (extra > 0) {
      this->yData.remove (0, extra);   // remove 1st extra values
   }

   number = this->xData.count ();
   maximumPeriod = samplePeriod * maximumPoints;
   currentPeriod = samplePeriod * number;

   correlation = this->calculateCorrelationCoefficient ();

   this->uiForm->Number_Points_Label->setText (QString ("%1").arg (number));
   this->uiForm->Maximum_Sample_Label->setText (QEUtilities::intervalToString (maximumPeriod, 0, false));
   this->uiForm->Ongoing_Sample_Label->setText (QEUtilities::intervalToString (currentPeriod, 0, false));
   this->uiForm->Correlation_Value_Label->setText (QString ("%1").arg (correlation, 0, 'f', 4));

   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QECorrelation::sampleTimeout ()
{
   qcaobject::QCaObject* xQCa;
   qcaobject::QCaObject* yQCa;

   double x = 0.0;
   double y = 0.0;

   xQCa = this->uiForm->CA_DataLabel1->getQcaItem (QELabelPVIndex);
   yQCa = this->uiForm->CA_DataLabel2->getQcaItem (QELabelPVIndex);

   if (xQCa && yQCa) {
      if (xQCa->getDataIsAvailable() && yQCa->getDataIsAvailable()) {

         x = xQCa->getFloatingValue ();
         y = yQCa->getFloatingValue ();

         this->xData.append (x);
         this->yData.append (y);

         this->updateDataArrays ();
      }
   }
}

//------------------------------------------------------------------------------
//
void QECorrelation::reDrawTimeout ()
{
   this->tickTimerCount = (this->tickTimerCount + 1) % 20;

   if ((this->tickTimerCount % 20) == 0) {
      // 20th update, i.e. 1 second has passed - must replot.
      this->replotIsRequired = true;
   }

   if (this->replotIsRequired) {
      this->reDrawPlane ();  // clears replotIsRequired
   }
}

//------------------------------------------------------------------------------
// Cribbed from QEPlotter - refactor ??
//
void QECorrelation::graphicMouseMove (const QPointF& posn)
{
   QString mouseReadOut;
   QString f;
   QPointF slope;

   mouseReadOut = "";

   f.sprintf (" x: %+.6g", posn.x ());
   mouseReadOut.append (f);

   f.sprintf ("  y: %+.6g", posn.y ());
   mouseReadOut.append (f);

   if (this->plotArea->getSlopeIsDefined (slope)) {
      const double dx = slope.x ();
      const double dy = slope.y ();

      f.sprintf ("  dx: %+.6g", dx);
      mouseReadOut.append (f);

      f.sprintf ("  dy: %+.6g", dy);
      mouseReadOut.append (f);

      // Calculate slope, but avoid the divide by 0.
      //
      mouseReadOut.append ("  dy/dx: ");
      if (dx != 0.0) {
         f.sprintf ("%+.6g", dy/dx);
      } else {
         if (dy != 0.0) {
            f.sprintf ("%sinf", (dy >= 0.0) ? "+" : "-");
         } else {
            f.sprintf ("n/a");
         }
      }
      mouseReadOut.append (f);
   }

   this->setReadOut (mouseReadOut);
}

//------------------------------------------------------------------------------
//
void QECorrelation::graphicAreaDefinition (const QPointF& start, const QPointF& finish)
{
   QPoint distance = this->plotArea->pixelDistance (start, finish);

   // The QEGraphic validates the selection, i.e. that user has un-ambiguously
   // selected x scaling or y scaling. Need only figure out which one.
   //
   if (ABS (distance.y ()) >=  ABS (distance.x ())) {
      // Makeing a Y scale adjustment.
      //
      this->yMin = finish.y ();
      this->yMax = start.y ();

   } else {
      // Makeing a X scale adjustment.
      //
      this->xMin = start.x ();
      this->xMax = finish.x ();
   }

   this->scaleMode = smManual;
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QECorrelation::letterButtonClicked (bool)
{
   QWidget* theSender = dynamic_cast <QWidget*> (this->sender ());
   const int tag = QECorrelation::getTag (theSender);
   INDEX_CHECK (tag, );

   this->runSelectNameDialog (tag);
}

//------------------------------------------------------------------------------
//
void QECorrelation::contextMenuRequested (const QPoint& pos)
{
   QWidget* theSender = dynamic_cast <QWidget*> (this->sender ());
   const int tag = QECorrelation::getTag (theSender);
   INDEX_CHECK (tag, );

   const QPoint golbalPos = theSender->mapToGlobal (pos);
   this->actionTag = tag;

   // Taylor context menu to current circumstances.
   //
   QELabel* caDataLabel = (tag == xPvIndex) ?  this->uiForm->CA_DataLabel1 : this->uiForm->CA_DataLabel2;
   bool inUse = !caDataLabel->getSubstitutedVariableName(QELabelPVIndex).isEmpty ();
   this->pasteAction->setEnabled (!inUse);
   this->clearAction->setEnabled (inUse);

   this->contextMenu->exec (golbalPos);
}

//------------------------------------------------------------------------------
// TODO Refactor out common code. from contextMenuSelected, runSelectNameDialog
// and processDropEvent
//------------------------------------------------------------------------------
//
void QECorrelation::contextMenuSelected (QAction* selectedItem)
{
   const int tag = this->actionTag;
   INDEX_CHECK (tag, );

   QLabel* pvNameLabel  = NAME_LABEL (tag);
   QELabel* caDataLabel = DATA_LABEL (tag);

   QClipboard *cb = NULL;
   QString pasteText;

   bool okay;
   int iData = selectedItem->data ().toInt (&okay);

   ContextMenuActions maData = ContextMenuActions (iData);

   switch (maData) {

      case maAddEdit:
         this->runSelectNameDialog (tag);
         break;

      case maPaste:
         cb = QApplication::clipboard ();
         pasteText = cb->text().trimmed();

         if (pasteText.isEmpty ()) break;
         this->setPvName (tag, pasteText);
         break;

      case maClear:
         caDataLabel->deactivate ();
         this->xData.clear ();
         this->yData.clear ();
         caDataLabel->setVariableNameAndSubstitutions ("", "", QELabelPVIndex);
         caDataLabel->setText ("-");
         caDataLabel->setDefaultStyle (unusedStyle);
         pvNameLabel->setText ("");
         this->replotIsRequired = true;
         break;

      default:
         DEBUG << "contextMenuTriggered - invalid" << iData;
         break;
   }
}

//------------------------------------------------------------------------------
//
void QECorrelation::runSelectNameDialog (const int instance)
{
   INDEX_CHECK (instance, );

   QLabel* pvNameLabel  = NAME_LABEL (instance);
   QELabel* caDataLabel = DATA_LABEL (instance);

   QString currentName = caDataLabel->getSubstitutedVariableName (QELabelPVIndex);
   this->pvNameSelectDialog->setPvName (currentName);
   int n = this->pvNameSelectDialog->exec (pvNameLabel);
   if (n == 1) {
      // User has selected okay.
      //
      QString newName = this->pvNameSelectDialog->getPvName ();
      if (newName != currentName ) {
         this->setPvName (instance, newName);
      }
   }
}

//------------------------------------------------------------------------------
//
void QECorrelation::setPvName (const int instance, const QString& rawPvName)
{
   const QString pvName = rawPvName.trimmed ();

   INDEX_CHECK (instance, );

   QLabel* pvNameLabel  = NAME_LABEL (instance);
   QELabel* caDataLabel = DATA_LABEL (instance);

   caDataLabel->deactivate ();
   this->xData.clear ();
   this->yData.clear ();

   caDataLabel->setVariableNameAndSubstitutions (pvName, "", QELabelPVIndex);
   pvNameLabel->setText (pvName);

   if (!pvName.isEmpty()) {
      caDataLabel->setDefaultStyle (inuseStyle);
      caDataLabel->activate();
   } else {
      caDataLabel->setDefaultStyle (unusedStyle);
   }

   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
QString QECorrelation::getPvName (const int instance) const
{
   static const QString nullString = "";
   INDEX_CHECK (instance, nullString);
   QELabel* caDataLabel = DATA_LABEL (instance);
   return caDataLabel->getSubstitutedVariableName (QELabelPVIndex);
}

//------------------------------------------------------------------------------
//
bool QECorrelation::processDropEvent (const int tag, QDropEvent* event)
{
   if (!event) return false;  // sanity check.

   // If no text available, do nothing
   //
   if (!event->mimeData()->hasText ()) {
      event->ignore ();
      return false;
   }

   // Get the drop data
   //
   const QMimeData *mime = event->mimeData ();

   // If there is any text, drop the text
   //
   QString dropText = mime->text ();
   if (!dropText.isEmpty ()) {
      // Carry out the drop action
      //
      this->setPvName (tag, dropText);
   }

   // Tell the dropee that the drop has been acted on
   //
   event->accept ();
   // event->acceptProposedAction ();

   return true;
}

//------------------------------------------------------------------------------
//
bool QECorrelation::eventFilter (QObject *obj, QEvent* event)
{
   const QEvent::Type type = event->type ();

   switch (type) {

      case QEvent::DragEnter:
         if ((obj == this->uiForm->PV_Label1) ||
             (obj == this->uiForm->PV_Label2)) {
            QDragEnterEvent* dragEnterEvent = static_cast<QDragEnterEvent*> (event);
            dragEnterEvent->setDropAction (Qt::CopyAction);
            dragEnterEvent->accept ();  // only if not in use.
            return true;
         }
         break;


      case QEvent::DragMove:
         if ((obj == this->uiForm->PV_Label1) ||
             (obj == this->uiForm->PV_Label2)) {
            QDragMoveEvent* dragMoveEvent = static_cast<QDragMoveEvent*> (event);
            dragMoveEvent->accept ();  // only if not in use. ELSE  dragMoveEvent->ignore ();
            return true;
         }
         break;


      case QEvent::DragLeave:
         // no action
         break;


      case QEvent::Drop:
         if (obj == this->uiForm->PV_Label1) {
            QDropEvent* dragDropEvent = static_cast<QDropEvent*> (event);
            return this->processDropEvent (1, dragDropEvent);

         } else if (obj == this->uiForm->PV_Label2) {
            QDropEvent* dragDropEvent = static_cast<QDropEvent*> (event);
            return this->processDropEvent (2, dragDropEvent);

         }
         break;

      default:
         // Just fall through
         break;
   }

   return false; // we did not handle this event
}


//------------------------------------------------------------------------------
//
void QECorrelation::clearCountClick (bool)
{
   this->xData.clear ();
   this->yData.clear ();
   this->updateDataArrays ();
}

//------------------------------------------------------------------------------
//
void QECorrelation::dataScaleClick (bool)
{
   this->scaleMode = smData;

   this->xMin = this->xData.minimumValue (0.0);
   this->xMax = this->xData.maximumValue (100.0);

   this->yMin = this->yData.minimumValue (0.0);
   this->yMax = this->yData.maximumValue (100.0);

   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QECorrelation::dynamicScaleClick (bool)
{
   this->scaleMode = smDynamic;
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QECorrelation::manualScaleClick (bool)
{
   QETwinScaleSelectDialog::ScaleLimit scaleOneLimits;
   QETwinScaleSelectDialog::ScaleLimit scaleTwoLimits;

   scaleOneLimits.min = this->xMin;
   scaleOneLimits.max = this->xMax;
   scaleTwoLimits.min = this->yMin;
   scaleTwoLimits.max = this->yMax;

   this->scaleDialog->setActiveMap (scaleOneLimits, scaleTwoLimits);
   int n = this->scaleDialog->exec (this);
   if (n == 1) {
      this->scaleDialog->getActiveMap (scaleOneLimits, scaleTwoLimits);

      this->xMin = scaleOneLimits.min;
      this->xMax = scaleOneLimits.max;
      this->yMin = scaleTwoLimits.min;
      this->yMax = scaleTwoLimits.max;

      this->scaleMode = smManual;
      this->replotIsRequired = true;
   }
}

//------------------------------------------------------------------------------
//
void QECorrelation::plotModeSelect (bool)
{
   QPushButton* theSender = dynamic_cast <QPushButton*> (this->sender ());
   const int tag = QECorrelation::getTag (theSender, -1);

   if ((tag == pmDots) || (tag == pmLines)) {   // sainity check
      this->plotMode = PlotModes (tag);
      this->replotIsRequired = true;
   }
}

//------------------------------------------------------------------------------
//
void QECorrelation::timeSelectButtonClick (bool)
{
   QString pvName1 = this->getPvName (xPvIndex);
   QString pvName2 = this->getPvName (yPvIndex);

   if (pvName1.isEmpty() || pvName2.isEmpty()) {
      this->setReadOut ("Both PVs must be defined before archive data may be requested");
      return;
   }

   double interval = this->uiForm->Number_Samples_Edit->getValue() *
                     this->uiForm->Sample_Interval_Edit->getValue();

   QDateTime endTime = QDateTime::currentDateTime().toLocalTime ();
   QDateTime startTime = endTime.addSecs (int (-interval));

   this->timeDialog->setStartDateTime (startTime);
   this->timeDialog->setEndDateTime (endTime);
   int n = this->timeDialog->exec (this);
   if (n == 1) {
      // User has selected okay.
      //
      this->historicalSetsReceived = 0;
      this->xHistoricalData.clear ();
      this->yHistoricalData.clear ();

      startTime = this->timeDialog->getStartDateTime ();
      endTime = this->timeDialog->getEndDateTime ();

      // Add 5% spare each side - its just the way the archiver works.
      //
      interval = startTime.secsTo (endTime);
      double extra = interval * 0.05;   // 5%

      startTime = startTime.addSecs (int (-extra));
      endTime   = endTime.addSecs   (int (+extra));

      this->archiveAccess->readArchive (this->uiForm->Panel_1, pvName1, startTime, endTime,
                                        numberOfCorrelationPoints, QEArchiveInterface::Linear, 0);

      this->archiveAccess->readArchive (this->uiForm->Panel_2, pvName2, startTime, endTime,
                                        numberOfCorrelationPoints, QEArchiveInterface::Linear, 0);

      this->setReadOut ("Archive data request issued");
   }
}

//------------------------------------------------------------------------------
//
void QECorrelation::setArchiveData (const QObject* userData, const bool isOkay,
                                    const QCaDataPointList& archiveData,
                                    const QString& /* pvName */,
                                    const QString& supplementary)
{
   const QWidget* userFrame = dynamic_cast <const QWidget*> (userData);
   const int tag = QECorrelation::getTag (userFrame);
   INDEX_CHECK (tag, );
   
   if (!isOkay) {
      this->setReadOut (supplementary);
      return;
   }

   const double samplePeriod = this->uiForm->Sample_Interval_Edit->getValue ();

   // Note: we assume that the dialog's the start/end time have not changed since
   // wthe archiver data was requested.
   //
   const QDateTime endTime = this->timeDialog->getEndDateTime ();

   // Resample data into appropriate chunks.
   //
   QCaDataPointList resampledSource;
   resampledSource.resample (archiveData, samplePeriod, endTime);

   const int n = MIN (resampledSource.count (), numberOfCorrelationPoints);

   for (int j = 0; j < n; j++) {
      double dataumValue = resampledSource.value (j).value;
      if (tag == xPvIndex) {
         this->xHistoricalData.append (dataumValue);
      } else {
         this->yHistoricalData.append (dataumValue);
      }
   }

   this->historicalSetsReceived += 1;

   this->setReadOut (QString ("%1%2 data set received")
                    .arg (this->historicalSetsReceived)
                    .arg (this->historicalSetsReceived == 1 ? "st" : "nd"));

   if (this->historicalSetsReceived == 2) {
      this->xData = this->xHistoricalData;
      this->yData = this->yHistoricalData;
      this->updateDataArrays ();
   }
}

//------------------------------------------------------------------------------
//
void QECorrelation::numberSamplesEditChange  (const double)
{
   this->updateDataArrays ();
}

//------------------------------------------------------------------------------
//
void QECorrelation::sampleIntervalEditChange (const double value)
{
   int mSec = (int) (1000.0 * value);
   this->sampleTimer->stop ();
   this->sampleTimer->start (mSec);
   this->updateDataArrays ();
}


#define TAG_NAME    "__QE_CORRELATION__"

//------------------------------------------------------------------------------
// static
void QECorrelation::tagWidget (QWidget* widget, const int tag)
{
   if (!widget) return;     // sanity check
   widget->setProperty (TAG_NAME, tag);
}

//------------------------------------------------------------------------------
// static
int QECorrelation::getTag (const QWidget* widget, const int defaultValue)
{
   if (!widget) return defaultValue;     // sanity check

   QVariant v = widget->property (TAG_NAME);
   bool okay;
   int result = v.toInt (&okay);
   if (!okay) result = defaultValue;
   return result;
}

// end
