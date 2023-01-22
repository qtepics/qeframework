/*  QEPlotter.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2023 Australian Synchrotron.
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

#include "QEPlotter.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QCheckBox>
#include <QApplication>
#include <QMimeData>

#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <QEGraphicNames.h>
#include <QEGraphic.h>

#include <QEPlatform.h>
#include <QECommon.h>
#include <QEFloating.h>
#include <QEInteger.h>
#include <QEScaling.h>


#define DEBUG qDebug() << "QEPlotter"  << __LINE__ << __FUNCTION__ << "  "

static const QColor clWhite (0xFF, 0xFF, 0xFF, 0xFF);
static const QColor clBlack (0x00, 0x00, 0x00, 0xFF);
static const QColor clGridLine (0xC0C0C0);
static const QColor clReverseGridLine (0x404040);
static const QColor clStatus (0xF0F0F0);

static const QString letterStyle ("QWidget { background-color: #e8e8e8; }");

// Define default colours: essentially RGB byte triplets
//
static const QColor item_colours [QEPlotter::NUMBER_OF_SLOTS] = {
   clWhite,
   QColor (0xFF0000), QColor (0x0000FF), QColor (0x008000), QColor (0xFF8000),
   QColor (0x4080FF), QColor (0x800000), QColor (0x008080), QColor (0x808000),
   QColor (0x800080), QColor (0x00FF00), QColor (0x00FFFF), QColor (0xFFFF00),
   QColor (0x8F00C0), QColor (0xC0008F), QColor (0xB040B0), clBlack
};

static const QString item_labels [QEPlotter::NUMBER_OF_SLOTS] = {
   QString ("X"),
   QString ("A"), QString ("B"), QString ("C"), QString ("D"),
   QString ("E"), QString ("F"), QString ("G"), QString ("H"),
   QString ("I"), QString ("J"), QString ("K"), QString ("L"),
   QString ("M"), QString ("N"), QString ("O"), QString ("P")
};


static const QEExpressionEvaluation::InputKinds Normal = QEExpressionEvaluation::Normal;
static const QEExpressionEvaluation::InputKinds Primed = QEExpressionEvaluation::Primed;

// We use a shared time for all QEPlotters.
//
QTimer* QEPlotter::tickTimer = NULL;

//==============================================================================
// Slot range checking macro function.
// Set default to nil for void functions.
//
#define SLOT_CHECK(slot, default) {                           \
   if ((slot < 0) || (slot >= ARRAY_LENGTH (this->xy))) {     \
      DEBUG << "slot out of range: " << slot;                 \
      return default;                                         \
   }                                                          \
}


//==============================================================================
// QEPlotter
//==============================================================================
//
void QEPlotter::createSlotWidgets (const int slot)
{
   SLOT_CHECK(slot,);

   QFrame* frame = new QFrame (this->itemFrame);
   frame->setFixedHeight (16);
   this->itemLayout->addWidget (frame);
   if (slot == 0) {
      this->itemLayout->addSpacing (4);
   }

   QHBoxLayout* frameLayout = new QHBoxLayout (frame);
   frameLayout->setContentsMargins (0, 0, 0, 0);
   frameLayout->setSpacing (2);

   QPushButton* letter = new QPushButton (frame);
   QLabel* label = new QLabel (frame);
   QCheckBox* box = NULL;

   letter->setFixedWidth (20);
   letter->setFixedHeight (16);
   letter->setText (item_labels [slot]);
   letter->setStyleSheet (letterStyle);
   frameLayout->addWidget (letter);

   QObject::connect (letter, SIGNAL ( clicked (bool)),
                     this,   SLOT   ( letterButtonClicked (bool)));

   label->setMinimumWidth (16);
   label->setMaximumWidth (400);
   label->setIndent (6);
   label->setStyleSheet (QEUtilities::colourToStyle (item_colours [slot]));
   label->setAcceptDrops (true);
   label->installEventFilter (this);
   label->setContextMenuPolicy (Qt::CustomContextMenu);
   frameLayout->addWidget (label);

   QObject::connect (label, SIGNAL ( customContextMenuRequested (const QPoint &)),
                     this,  SLOT   ( itemContextMenuRequested (const QPoint &)));

   if (slot == 0) {
      frameLayout->addSpacing (4 + 17);
   } else {
      frameLayout->addSpacing (4);
      box = new QCheckBox (frame);
      box->setFixedWidth (17);
      box->setChecked (true);
      frameLayout->addWidget (box);

      QObject::connect (box,  SIGNAL (stateChanged (int)),
                        this, SLOT   (checkBoxStateChanged (int)));
   }

   // Save widget references.
   //
   this->xy [slot].frame = frame;
   this->xy [slot].frameLayout = frameLayout;
   this->xy [slot].letterButton = letter;
   this->xy [slot].itemName = label;
   this->xy [slot].checkBox = box;

   // Setup widget to slot mapping.
   //
   this->widgetToSlot.insert (frame, slot);
   this->widgetToSlot.insert (letter, slot);
   this->widgetToSlot.insert (label, slot);
   this->widgetToSlot.insert (box, slot);

   // Add spacer at the bottom of the last item.
   //
   if (slot == ARRAY_LENGTH (this->xy) - 1) {
      QSpacerItem* verticalSpacer = new QSpacerItem (10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
      this->itemLayout->addItem (verticalSpacer);
   }
}

//------------------------------------------------------------------------------
//
void QEPlotter::createInternalWidgets ()
{
   int slot;
   QString styleSheet;

   // Main layout.
   //
   this->vLayout = new QVBoxLayout (this);
   this->vLayout->setContentsMargins (4, 4, 4, 4);
   this->vLayout->setSpacing (4);

   // Create tool bar frame and tool buttons.
   //
   this->toolBar = new QEPlotterToolBar (); // this will become parented by toolBarResize

   // Connect various tool bar signal to the plotter slot.
   // The tool bar generates the same signals at the context menus.
   //
   this->connectMenuOrToolBar (this->toolBar);

   this->toolBarResize = new QEResizeableFrame (QEResizeableFrame::BottomEdge, 8, 8 + this->toolBar->designHeight, this);
   this->toolBarResize->setFrameShape (QFrame::StyledPanel);
   this->toolBarResize->setFrameShadow (QFrame::Raised);
   this->toolBarResize->setFixedHeight (8 + this->toolBar->designHeight);
   this->toolBarResize->setMinimumWidth (300);
   this->toolBarResize->setMaximumWidth (1920);
   this->toolBarResize->setWidget (this->toolBar);
   this->vLayout->addWidget (this->toolBarResize);

   this->theMainFrame = new QFrame (this);
   this->theMainFrame->setFrameShape (QFrame::NoFrame);
   this->theMainFrame->setFrameShadow (QFrame::Plain);
   this->vLayout->addWidget (this->theMainFrame);

   this->statusFrame = new QFrame (this);
   this->statusFrame->setFrameShape (QFrame::StyledPanel);
   this->statusFrame->setFrameShadow (QFrame::Raised);
   this->statusFrame->setFixedHeight (30);
   this->vLayout->addWidget (this->statusFrame);

   // Inside main frame - layout left to right.
   //
   this->hLayout = new QHBoxLayout (this->theMainFrame);
   this->hLayout->setContentsMargins (0, 0, 0, 0);
   this->hLayout->setSpacing (4);

   this->plotFrame = new QFrame (this->theMainFrame);
   this->plotFrame->setFrameShape (QFrame::StyledPanel);
   this->plotFrame->setFrameShadow (QFrame::Raised);
   this->hLayout->addWidget (this->plotFrame);

   // Inside plot frame - whole thing.
   //
   this->plotLayout = new QVBoxLayout (this->plotFrame);
   this->plotLayout->setContentsMargins (4, 4, 4, 4);
   this->plotLayout->setSpacing (4);

   this->plotArea = new QEGraphic (this->plotFrame);
   this->plotArea->setAvailableMarkups
         (QEGraphicNames::Area | QEGraphicNames::Line | QEGraphicNames::CrossHair |
          QEGraphicNames::VerticalMarker_1 | QEGraphicNames::HorizontalMarker_1);

   this->plotLayout->addWidget (this->plotArea);

   QObject::connect (this->plotArea, SIGNAL (mouseMove     (const QPointF&)),
                     this,           SLOT   (plotMouseMove (const QPointF&)));

   QObject::connect (this->plotArea, SIGNAL (markupMove     (const QEGraphicNames::Markups, const QPointF&)),
                     this,           SLOT   (markupMove     (const QEGraphicNames::Markups, const QPointF&)));

   QObject::connect (this->plotArea, SIGNAL (wheelRotate   (const QPointF&, const int)),
                     this,           SLOT   (zoomInOut     (const QPointF&, const int)));

   QObject::connect (this->plotArea, SIGNAL (areaDefinition (const QPointF&, const QPointF&)),
                     this,           SLOT   (scaleSelect    (const QPointF&, const QPointF&)));

   QObject::connect (this->plotArea, SIGNAL (lineDefinition (const QPointF&, const QPointF&)),
                     this,           SLOT   (lineSelected   (const QPointF&, const QPointF&)));

   // Create the resizeable frame.
   //
   this->itemResize = new QEResizeableFrame (QEResizeableFrame::LeftEdge, 60, 400, this->theMainFrame);
   this->itemResize->setFrameShape (QFrame::StyledPanel);
   this->itemResize->setFrameShadow (QFrame::Raised);
   this->itemResize->setFixedWidth (256);
   this->hLayout->addWidget (this->itemResize);

   // Create the scroll area - set as resizeable frame widget.
   //
   this->itemScrollArea = new QScrollArea (NULL); // re-parented.
   this->itemScrollArea->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
   this->itemScrollArea->setWidgetResizable (true);    // MOST IMPORTANT
   this->itemResize->setWidget (this->itemScrollArea);

   // Create item frame - set as scroll area widget.
   //
   this->itemFrame = new QFrame (NULL); // re-parented.
   this->itemFrame->setFrameShape (QFrame::NoFrame);
   this->itemFrame->setFrameShadow (QFrame::Plain);
   this->itemScrollArea->setWidget (this->itemFrame);

   this->itemLayout = new QVBoxLayout (this->itemFrame);
   this->itemLayout->setContentsMargins (2, 2, 2, 2);
   this->itemLayout->setSpacing (4);

   for (slot = 0; slot < ARRAY_LENGTH (this->xy); slot++) {
      this->createSlotWidgets (slot);
   }

   // Status frame.
   //
   // Inside status frame - layout left to right.
   //
   this->statusLayout = new QHBoxLayout (this->statusFrame);
   this->statusLayout->setContentsMargins (4, 4, 4, 4);
   this->statusLayout->setSpacing (8);

   this->slotIndicator = new QLabel ("", this->statusFrame);
   this->minLabel   = new QLabel ("Min:", this->statusFrame);
   this->minValue   = new QLabel ("-", this->statusFrame);
   this->maxLabel   = new QLabel ("Max:", this->statusFrame);
   this->maxValue   = new QLabel ("-", this->statusFrame);
   this->maxAtLabel = new QLabel ("Max At:", this->statusFrame);
   this->maxAtValue = new QLabel ("-", this->statusFrame);
   this->fwhmLabel  = new QLabel ("FWHM:", this->statusFrame);
   this->fwhmValue  = new QLabel ("-", this->statusFrame);
   this->comLabel   = new QLabel ("CoM:", this->statusFrame);
   this->comValue   = new QLabel ("-", this->statusFrame);

   this->slotIndicator->setFixedWidth (40);
   this->minLabel->setFixedWidth (32);
   this->maxLabel->setFixedWidth (32);
   this->maxAtLabel->setFixedWidth (52);
   this->fwhmLabel->setFixedWidth (48);
   this->comLabel->setFixedWidth (36);

   styleSheet = QEUtilities::colourToStyle (clStatus);
   this->slotIndicator->setStyleSheet (styleSheet);
   this->slotIndicator->setAlignment (Qt::AlignHCenter);

#define SET_VALUE_LABEL(nameValue, tip)  {              \
   this->nameValue->setStyleSheet  (styleSheet);        \
   this->nameValue->setIndent (6);                      \
   this->nameValue->setAlignment (Qt::AlignRight);      \
   this->nameValue->setToolTip (tip);                   \
}

   SET_VALUE_LABEL (minValue, "Minimum Value");
   SET_VALUE_LABEL (maxValue, "Maximum Value");
   SET_VALUE_LABEL (maxAtValue, "Maximum Value X co-ordinate");
   SET_VALUE_LABEL (fwhmValue, "Full Width at Half Maximum");
   SET_VALUE_LABEL (comValue, "Centre of Mass (median position)");

#undef SET_VALUE_LABEL

   this->statusLayout->addWidget (this->slotIndicator);
   this->statusLayout->addWidget (this->minLabel);
   this->statusLayout->addWidget (this->minValue);
   this->statusLayout->addWidget (this->maxLabel);
   this->statusLayout->addWidget (this->maxValue);
   this->statusLayout->addWidget (this->maxAtLabel);
   this->statusLayout->addWidget (this->maxAtValue);
   this->statusLayout->addWidget (this->fwhmLabel);
   this->statusLayout->addWidget (this->fwhmValue);
   this->statusLayout->addWidget (this->comLabel);
   this->statusLayout->addWidget (this->comValue);

   this->colourDialog = new QColorDialog (this);
   this->colourDialog->setOption (QColorDialog::ShowAlphaChannel, true);

   this->dataDialog = new QEPlotterItemDialog (this);
   this->rangeDialog = new QEStripChartRangeDialog (this);
   this->rangeDialog->setWindowTitle ("Plotter Y Range");
   this->twinRangeDialog = new QETwinScaleSelectDialog ("Plotter XY Range", "X", "Y", this);
}


//==============================================================================
// DataSets
//==============================================================================
//
QEPlotter::DataSets::DataSets ()
{
   this->calculator = new QEExpressionEvaluation (true);
   this->dataKind = NotInUse;
   this->sizeKind = NotSpecified;
   // this->colour = item_colours [slot];
   this->fixedSize = 0;
   this->dbSize = 0;
   this->pvName = "";
   this->aliasName = "";
   this->description = "";
   this->expression = "";
   this->expressionIsValid = false;
   this->dataIsConnected = false;
   this->sizeIsConnected = false;
   this->isDisplayed = true;
   this->isBold = false;
   this->isDashed = false;
   this->showDots = false;
   this->median = 1;

   this->frame = NULL;
   this->frameLayout = NULL;
   this->letterButton = NULL;
   this->itemName = NULL;
   this->checkBox = NULL;
   this->itemMenu = NULL;
}

//------------------------------------------------------------------------------
//
QEPlotter::DataSets::~DataSets ()
{
   delete this->calculator;
}

//------------------------------------------------------------------------------
//
void QEPlotter::DataSets::setContext (QEPlotter* ownerIn, int slotIn)
{
   this->owner = ownerIn;
   this->slot = slotIn;
   this->letter = item_labels [this->slot];
}

//------------------------------------------------------------------------------
//
void QEPlotter::DataSets::clear ()
{
   this->data.clear ();
   this->dyByDx.clear ();
}

//------------------------------------------------------------------------------
//
bool QEPlotter::DataSets::isInUse () const
{
   return (this->dataKind != NotInUse);
}

//------------------------------------------------------------------------------
//
int QEPlotter::DataSets::actualSize () const
{
   // use array (waveform) PV size or zero.
   //
   return (this->dataKind == QEPlotter::DataPVPlot) ? this->data.size () : 0;
}

//------------------------------------------------------------------------------
//
int QEPlotter::DataSets::effectiveSize () const
{
   int result = 0;

   switch (this->sizeKind) {

      case QEPlotter::NotSpecified:

         // The size has not been specified - we must choose best value we can
         // based on the specified data array (waveform) PVs.
         //
         switch (this->dataKind) {

            case QEPlotter::NotInUse:
               // No PV spcified either.
               //
               if (this->slot == 0) {
                  // This is the X - choose the maximum of all the y data sizes.
                  // There is an implied calculation X [s] = s
                  //
                  result = this->owner->maxActualYSizes ();
               } else {
                  // This is a Y item - zero is the only sensible choice here.
                  //
                  result = 0;
               }
               break;

            case QEPlotter::CalculationPlot:
               // Choose the maximum of all the actual Y data sizes.
               //
               if (this->slot == 0) {
                  // There is an actual calculation.
                  //
                  result = this->owner->maxActualYSizes ();
               } else {
                  // Go with the effective X size here.
                  // This may be driven by maxActualYSizes or may be a constant or a PV.
                  //
                  result = this->owner->xy [0].effectiveSize ();
               }
               break;


            case QEPlotter::DataPVPlot:
               // Use size of the specified array (waveform) PV to provide the size.
               //
               result = this->data.size ();
               break;
         }

         break;

      case QEPlotter::Constant:
         // Size specified as a constant - just use as is.
         //
         result = this->fixedSize;
         break;

      case QEPlotter::SizePVName:
         // Size specified as a PV - just use value (if we have it).
         //
         result = this->sizeIsConnected ? this->dbSize : 0;
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
QString QEPlotter::DataSets::getDataData () const
{
   const int fw = 12;   // field width
   const int n = this->data.count ();
   QString result;
   DataSets* dx = &this->owner->xy [0];

   result = "\n";

   if (this == dx) {
      // x/this data only
      result.append (QString ("%1\n").arg ("X", fw));
      for (int j = 0 ; j < n; j++) {
         result.append ( QString ("%1\n").arg (this->data[j], fw));
      }
   } else {
      // x and y/this data
      result.append (QString ("%1\t%2\n").arg ("X", fw).arg (this->letter, fw));
      for (int j = 0 ; j < n; j++) {
         result.append ( QString ("%1\t%2\n").arg (dx->data[j], fw).arg (this->data[j], fw));
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
QString QEPlotter::DataSets::getSizeData () const
{
   const int n = this->data.count ();
   QString result;

   result = QString ("%1").arg (n);
   return result;
}


//==============================================================================
// QEPlotter
//==============================================================================
//
QEPlotter::QEPlotter (QWidget* parent) : QEAbstractDynamicWidget (parent)
{
   QCaVariableNamePropertyManager* vpnm;

   this->xAxisIsEnabled = true;
   this->yAxisIsEnabled = true;

   this->widgetToSlot.clear();
   this->createInternalWidgets ();

   this->setContextMenuPolicy (Qt::CustomContextMenu);
   QObject::connect (this, SIGNAL (customContextMenuRequested  (const QPoint &)),
                     this, SLOT   (generalContextMenuRequested (const QPoint &)));

   // The actual meni widget is created as and when needed.
   //
   this->generalContextMenu = NULL;

   this->setNumVariables (TOTAL_VI_NUMBER);

   for (int slot = 0; slot < ARRAY_LENGTH (this->xy); slot++) {
      this->xy [slot].setContext (this, slot);   // set owner and own slot number.
      this->xy [slot].colour = item_colours [slot];

      this->updateLabel (slot);

      // Set variable index numbers.
      // Must be consistent with the isDataIndex ()  etc. functions.
      //
      this->xy [slot].dataVariableNameManager.setVariableIndex (PVS_PER_SLOT*slot + 0);
      this->xy [slot].sizeVariableNameManager.setVariableIndex (PVS_PER_SLOT*slot + 1);
   }

   this->xMarkerVariableNameManager.setVariableIndex (NONE_SLOT_VI_BASE + 0);
   this->yMarkerVariableNameManager.setVariableIndex (NONE_SLOT_VI_BASE + 1);


   // Configure the panel.
   //
   this->setFrameShape (QFrame::Panel);
   this->setFrameShadow (QFrame::Plain);
   this->setMinimumSize (240, 120);

   this->contextMenuIsOverGraphic = false;
   this->contextMenuRequestPosition = QPointF (0.0, 0.0);
   this->contextMenuEmitLegend = "Emit Coordinates";
   this->enableConextMenu = true;
   this->toolBarIsVisible = true;
   this->pvItemsIsVisible = true;
   this->statusIsVisible = true;
   this->useFullLengthArraySubscriptions = false;  // go with modern behaviour by default.

   this->isReverse = false;
   this->isPaused = false;
   this->selectedDataSet = 0;
   this->crosshairIndex = -1;
   this->crosshairsAreRequired = false;
   this->pvNameSetChangeInhibited = false;
   this->alaisSetChangeInhibited = false;

   this->setAllowDrop (false);
   this->setDisplayAlarmStateOption (standardProperties::DISPLAY_ALARM_STATE_NEVER);

   // Set up a connection to recieve variable name property changes
   //
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   for (int slot = 0 ; slot < ARRAY_LENGTH (this->xy); slot++) {
      vpnm = &this->xy [slot].dataVariableNameManager;
      QObject::connect (vpnm, SIGNAL (newVariableNameProperty (QString, QString, unsigned int)),
                        this, SLOT   (setNewVariableName      (QString, QString, unsigned int)));

      vpnm = &this->xy [slot].sizeVariableNameManager;
      QObject::connect (vpnm, SIGNAL (newVariableNameProperty (QString, QString, unsigned int)),
                        this, SLOT   (setNewVariableName      (QString, QString, unsigned int)));
   }

   vpnm = &this->xMarkerVariableNameManager;
   QObject::connect (vpnm, SIGNAL (newVariableNameProperty (QString, QString, unsigned int)),
                     this, SLOT   (setNewVariableName      (QString, QString, unsigned int)));

   vpnm = &this->yMarkerVariableNameManager;
   QObject::connect (vpnm, SIGNAL (newVariableNameProperty (QString, QString, unsigned int)),
                     this, SLOT   (setNewVariableName      (QString, QString, unsigned int)));


   // Connect action requests to consumer, e.g. qegui.
   //
   QObject* consumer = this->getGuiLaunchConsumer ();
   if (consumer) {
      QObject::connect (this,      SIGNAL (requestAction (const QEActionRequests& )),
                        consumer,  SLOT   (requestAction (const QEActionRequests& )));
   }

   this->xScaleMode = QEPlotterNames::smDynamic;
   this->yScaleMode = QEPlotterNames::smDynamic;

   this->currentMinX = this->fixedMinX = 0.0;
   this->currentMaxX = this->fixedMaxX = 1.0;
   this->currentMinY = this->fixedMinY = 0.0;
   this->currentMaxY = this->fixedMaxY = 1.0;

   this->replotIsRequired = true; // ensure process on first tick.
   this->tickTimerCount = 0;

   // Create QEPlotter timer if needs be.
   //
   if (QEPlotter::tickTimer == NULL) {
      QEPlotter::tickTimer = new QTimer (NULL);
      QEPlotter::tickTimer->start (50);  // mSec == 0.05s - refresh plot check at ~20Hz.
   }
   connect (QEPlotter::tickTimer, SIGNAL (timeout ()), this, SLOT (tickTimeout ()));

   this->setToolTipSummary ();
   this->pushState ();  // baseline state - there is always at least one.

   // Somethings just need to be done post construction in order to work as expected.
   //
   QTimer::singleShot(1, this, SLOT(postContruction ()));
}

//------------------------------------------------------------------------------
//
void QEPlotter::postContruction ()
{
   this->itemResize->setFixedWidth (256);
}

//------------------------------------------------------------------------------
//
QEPlotter::~QEPlotter () { }

//------------------------------------------------------------------------------
//
QSize QEPlotter::sizeHint () const {
   return QSize (600, 500);
}

//------------------------------------------------------------------------------
//
bool QEPlotter::isSlotIndex   (const unsigned int vi) const { return (vi < NONE_SLOT_VI_BASE);}
bool QEPlotter::isMarkerIndex (const unsigned int vi) const { return (vi >= NONE_SLOT_VI_BASE) and (vi < TOTAL_VI_NUMBER); }
bool QEPlotter::isDataIndex   (const unsigned int vi) const { return isSlotIndex (vi) && ((vi % PVS_PER_SLOT) == 0); }
bool QEPlotter::isSizeIndex   (const unsigned int vi) const { return isSlotIndex (vi) && ((vi % PVS_PER_SLOT) == 1); }
bool QEPlotter::isXIndex      (const unsigned int vi) const { return isSlotIndex (vi) && (vi < PVS_PER_SLOT); }
bool QEPlotter::isYIndex      (const unsigned int vi) const { return isSlotIndex (vi) && !isXIndex (vi); }
int  QEPlotter::slotOf        (const unsigned int vi) const { return vi / PVS_PER_SLOT; }
unsigned int QEPlotter::viOfDataSlot (const int slot) const { return PVS_PER_SLOT * slot + 0; }
unsigned int QEPlotter::viOfSizeSlot (const int slot) const { return PVS_PER_SLOT * slot + 1; }

//------------------------------------------------------------------------------
//
void QEPlotter::updateLabel (const int slot)
{
   SLOT_CHECK (slot,);

   DataSets* ds = &this->xy [slot];
   QString caption;
   QEAbstractDynamicWidget::PVLabelMode labelMode;
   qcaobject::QCaObject* qca = NULL;

   caption.clear ();

   switch (ds->dataKind) {
      case NotInUse:
         break;

      case DataPVPlot:
         labelMode = this->getPVLabelMode ();

         // If an alias name/description is defined - use it if requested.
         //
         switch (labelMode) {
            case QEAbstractDynamicWidget::useAliasName:
               if (!ds->aliasName.isEmpty() && ds->aliasName != "<>") {
                  caption.append (ds->aliasName);
               } else {
                  caption.append (ds->pvName);
               }
               break;

            case QEAbstractDynamicWidget::useDescription:
               // First refresh description - if we can.
               //
               qca = this->getQcaItem (viOfDataSlot (slot));
               if (qca) {
                  ds->description = qca->getDescription();
               }

               if (!ds->description.isEmpty()) {
                  caption.append (ds->description);
               } else {
                  caption.append (ds->pvName);
               }
               break;

            case QEAbstractDynamicWidget::usePvName:
            default:
               caption.append (ds->pvName);
               break;
         }
         break;

      case CalculationPlot:
         if (ds->expressionIsValid) {
            caption.append (":= ");
            caption.append (ds->expression);
         } else {
            caption.append ("invalid expr.");
         }
         break;

   }

   if (ds->itemName) {
      ds->itemName->setText (caption);
   }
}

//------------------------------------------------------------------------------
//
void QEPlotter::letterButtonClicked (bool)
{
   QPushButton* button =  dynamic_cast <QPushButton*> (this->sender ());
   int slot;

   slot = this->findSlot (button);
   if (slot >= 0) {
      this->runDataDialog (slot, button);
   }
}

//------------------------------------------------------------------------------
//
void QEPlotter::checkBoxStateChanged (int state)
{
   QCheckBox* box = dynamic_cast <QCheckBox*> (this->sender ());
   int slot;

   slot = this->findSlot (box);
   if (slot >= 0) {
      this->xy [slot].isDisplayed = (state == Qt::Checked);
      this->replotIsRequired = true;
   }
}

//------------------------------------------------------------------------------
//
void QEPlotter::setNewVariableName (QString variableName,
                                    QString variableNameSubstitutions,
                                    unsigned int variableIndex)
{
   // Deal with marker special case first.
   //
   if (this->isMarkerIndex (variableIndex)) {
      // Note: essentially calls establishConnection and then createQcaItem.
      //
      this->setVariableNameAndSubstitutions (variableName, variableNameSubstitutions, variableIndex);
      return;
   }

   const int slot = this->slotOf (variableIndex);
   SLOT_CHECK (slot,);

   // First clear out any status - this is a new PV name or cleared PV name.
   // Note: we must clear the xxxxIsConnect state - we do not get an initial
   // xxxxConnectionChanged (set not connected) signal.
   //
   if (this->isDataIndex (variableIndex)) {
      this->xy [slot].dataKind = NotInUse;
      this->xy [slot].dataIsConnected = false;
   } else if (this->isSizeIndex (variableIndex)) {
      this->xy [slot].sizeKind = NotSpecified;
      this->xy [slot].sizeIsConnected = false;
   } else {
      DEBUG << "Unexpected variableIndex" << variableIndex;
   }

   // Note: essentially calls establishConnection and then createQcaItem.
   //
   this->setVariableNameAndSubstitutions (variableName, variableNameSubstitutions, variableIndex);

   if (this->isDataIndex (variableIndex)) {
      QString pvName;
      pvName = this->getSubstitutedVariableName (variableIndex).trimmed ();
      this->xy [slot].pvName = pvName;
      this->updateLabel (slot);
   }

   this->replotIsRequired = true;
   this->setToolTipSummary ();

   // This prevents infinite looping in the case of cyclic connections.
   //
   this->pvNameSetChangeInhibited = true;
   emit this->pvDataNameSetChanged (this->getDataPvNameSet ());
   this->pvNameSetChangeInhibited = false;
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. QCaObjects that streams doubles and integers are required.
//
qcaobject::QCaObject* QEPlotter::createQcaItem (unsigned int variableIndex)
{

   qcaobject::QCaObject* result = NULL;
   QString pvName;
   int size;
   bool okay;

   pvName = this->getSubstitutedVariableName (variableIndex).trimmed ();

   if (this->isDataIndex (variableIndex)) {
      const int slot = this->slotOf (variableIndex);
      SLOT_CHECK (slot, NULL);

      // Has designer/user defined a calculation (as opposed to a PV name)?.
      // Note: no valid PV name starts with =.
      //
      if (pvName.left (1).compare (QString ("=")) == 0) {

         this->xy [slot].dataKind = CalculationPlot;
         this->xy [slot].expression = pvName.remove (0, 1);

         okay = this->xy[slot].calculator->initialise (this->xy[slot].expression);
         this->xy [slot].expressionIsValid = okay;

      } else {
         this->xy [slot].dataKind = DataPVPlot;
         result = new QEFloating (pvName, this, &this->floatingFormatting, variableIndex);

         if (result && !this->useFullLengthArraySubscriptions) {
            // Only read effective number, e.g. as defied by .NORD for a waveform record.
            //
            result->setRequestedElementCount (0);
         }
      }

      this->replotIsRequired = true;
      this->updateLabel (slot);

   } else if (this->isSizeIndex (variableIndex)) {

      const int slot = this->slotOf (variableIndex);
      SLOT_CHECK (slot, NULL);

      // Has designer/user just set an integer (as opposed to a PV name)?.
      // Note: no sensible PV names are just integers.
      //
      size = pvName.toInt (&okay);
      if (okay) {
         this->xy [slot].sizeKind = Constant;
         this->xy [slot].fixedSize = size;
         this->xy [slot].dbSize = 0;
         this->replotIsRequired = true;

      } else {
         // Assume it is a PV.
         //
         this->xy [slot].sizeKind = SizePVName;
         result = new QEInteger (pvName, this, &this->integerFormatting, variableIndex);
         this->xy [slot].fixedSize = 0;
         this->xy [slot].dbSize = 0;
      }

   } else if (this->isMarkerIndex(variableIndex)) {
      result = new QEFloating (pvName, this, &this->floatingFormatting, variableIndex);
   }

   return result;
}

//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QEPlotter::establishConnection (unsigned int variableIndex)
{
   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject* qca = createConnection (variableIndex);

   if (!qca) {
      return;
   }

   if (this->isDataIndex (variableIndex)) {
      const int slot = this->slotOf (variableIndex);
      SLOT_CHECK (slot,);
      DataSets* ds = &this->xy [slot];

      ds->clear ();  // Clear any old data.

      QObject::connect (qca, SIGNAL (connectionChanged     (QCaConnectionInfo &, const unsigned int &)),
                        this, SLOT  (dataConnectionChanged (QCaConnectionInfo &, const unsigned int &)));

      QObject::connect (qca, SIGNAL (floatingArrayChanged (const QVector<double>&, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)),
                        this, SLOT  (dataArrayChanged     (const QVector<double>&, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)));

      // Get, or at least, initiate fatching the description.
      //
      if (qca) {
         ds->description = qca->getDescription();
      }

   } else if (this->isSizeIndex (variableIndex)) {
      QObject::connect (qca, SIGNAL (connectionChanged     (QCaConnectionInfo &, const unsigned int &)),
                        this, SLOT  (sizeConnectionChanged (QCaConnectionInfo &, const unsigned int &)));

      QObject::connect (qca, SIGNAL (integerChanged   (const long &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)),
                        this, SLOT  (sizeValueChanged (const long &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)));

   } else if (this->isMarkerIndex(variableIndex)) {
      QObject::connect (qca, SIGNAL (connectionChanged       (QCaConnectionInfo &, const unsigned int &)),
                        this, SLOT  (markerConnectionChanged (QCaConnectionInfo &, const unsigned int &)));

      QObject::connect (qca, SIGNAL (floatingChanged    (const double &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)),
                        this, SLOT  (markerValueChanged (const double &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)));
   }
}

//------------------------------------------------------------------------------
//
void QEPlotter::activated ()
{
   // This prevents infinite looping in the case of cyclic signal-slot connections.
   //
   this->pvNameSetChangeInhibited  = true;
   emit this->pvDataNameSetChanged (this->getDataPvNameSet ());
   this->pvNameSetChangeInhibited  = false;

   this->alaisSetChangeInhibited = true;
   emit this->alaisNameSetChanged (this->getAliasNameSet ());
   this->alaisSetChangeInhibited = false;
}

//------------------------------------------------------------------------------
//
int QEPlotter::findSlot (QObject *obj)
{
   return this->widgetToSlot.value (obj, -1);
}

//------------------------------------------------------------------------------
//
QString QEPlotter::getXYExpandedDataPV (const int slot) const
{
   return this->getSubstitutedVariableName (2*slot);
}

//------------------------------------------------------------------------------
//
QString QEPlotter::getXYExpandedSizePV (const int slot) const
{
   return this->getSubstitutedVariableName (2*slot + 1);
}

//------------------------------------------------------------------------------
//
void QEPlotter::selectDataSet (const int slot)
{
   QString styleSheet;
   QString text;

   SLOT_CHECK (slot,);

   if (this->selectedDataSet == slot) {
      this->selectedDataSet = 0;  // none.
   } else {
      this->selectedDataSet = slot;
   }

   if (this->selectedDataSet > 0) {
      text = item_labels [slot];
      styleSheet = this->xy [slot].itemName->styleSheet ();
   } else {
      text = "";
      styleSheet = QEUtilities::colourToStyle (clStatus);

      this->minValue->setText ("-");
      this->maxValue->setText ("-");
      this->maxAtValue->setText ("-");
      this->fwhmValue->setText ("-");
      this->comValue->setText ("-");
   }

   this->slotIndicator->setText (text);
   this->slotIndicator->setStyleSheet (styleSheet);

   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEPlotter::highLight (const int slot, const bool isHigh)
{
   QString styleSheet;

   SLOT_CHECK (slot,);

   if (isHigh) {
      styleSheet = QEUtilities::colourToStyle (clWhite);
   } else {
      styleSheet = QEUtilities::colourToStyle (this->xy [slot].colour);
   }

   this->xy [slot].itemName->setStyleSheet (styleSheet);
}

//------------------------------------------------------------------------------
//
void QEPlotter::sendRequestAction (const QString& action, const QString& pvName)
{
   if (!pvName.isEmpty ()) {
      QEActionRequests request (action, pvName);
      emit this->requestAction (request);
   }
}

//------------------------------------------------------------------------------
//
void QEPlotter::generalContextMenuRequested (const QPoint& pos)
{
   const QPoint golbalPos = this->mapToGlobal (pos);

   // Save current mouse status/position.
   //
   this->contextMenuIsOverGraphic = this->plotArea->globalPosIsOverCanvas (golbalPos);
   this->contextMenuRequestPosition = this->plotArea->getRealMousePosition ();

   // Because they take so long top create (~ 50mSec), menu are only created as
   // and when needed. A one off 50 mS hardly noticable, while an 900 mS delay
   // when widget created, especially if there are several instances on form,
   // stands out like a sore thumb.
   //
   if (!this->generalContextMenu)  {
      this->generalContextMenu = new QEPlotterMenu (this);
      QEScaling::applyToWidget (this->generalContextMenu);
      this->connectMenuOrToolBar (this->generalContextMenu);
   }

   // Disable/enable show/hide menu items.
   //
   this->generalContextMenu->setActionEnabled (QEPlotterNames::PLOTTER_SHOW_HIDE_TOOLBAR,
                                               this->enableConextMenu);
   this->generalContextMenu->setActionEnabled (QEPlotterNames::PLOTTER_SHOW_HIDE_PV_ITEMS,
                                               this->enableConextMenu);
   this->generalContextMenu->setActionEnabled (QEPlotterNames::PLOTTER_SHOW_HIDE_STATUS,
                                               this->enableConextMenu);
   this->generalContextMenu->setActionEnabled (QEPlotterNames::PLOTTER_EMIT_COORDINATES,
                                               this->contextMenuIsOverGraphic);

   this->generalContextMenu->setActionText (QEPlotterNames::PLOTTER_EMIT_COORDINATES,
                                            this->contextMenuEmitLegend);

   // Set current checked states.
   //
   this->generalContextMenu->setActionChecked (QEPlotterNames::PLOTTER_SHOW_HIDE_CROSSHAIRS,
                                               this->crosshairsAreRequired);
   this->generalContextMenu->setActionChecked (QEPlotterNames::PLOTTER_SHOW_HIDE_TOOLBAR,
                                               this->getToolBarVisible ());
   this->generalContextMenu->setActionChecked (QEPlotterNames::PLOTTER_SHOW_HIDE_PV_ITEMS,
                                               this->getPvItemsVisible ());
   this->generalContextMenu->setActionChecked (QEPlotterNames::PLOTTER_SHOW_HIDE_STATUS,
                                               this->getStatusVisible ());

   // Set dragging variable/data check boxes as appropriate.
   //
   this->generalContextMenu->setActionChecked (QEPlotterNames::PLOTTER_DRAG_VARIABLE,
                                               this->isDraggingVariable ());
   this->generalContextMenu->setActionChecked (QEPlotterNames::PLOTTER_DRAG_DATA,
                                               !this->isDraggingVariable ());

   // Set up Use PV name, Alias and or descritiom.
   //
   const QEAbstractDynamicWidget::PVLabelMode plm = this->getPVLabelMode ();
   this->generalContextMenu->setActionChecked (QEPlotterNames::PLOTTER_SELECT_USE_PV_NAME,
                                               plm == QEAbstractDynamicWidget::usePvName);
   this->generalContextMenu->setActionChecked (QEPlotterNames::PLOTTER_SELECT_USE_ALIAS_NAME,
                                               plm == QEAbstractDynamicWidget::useAliasName);
   this->generalContextMenu->setActionChecked (QEPlotterNames::PLOTTER_SELECT_USE_DESCRIPTION,
                                               plm == QEAbstractDynamicWidget::useDescription);

   this->generalContextMenu->exec (golbalPos);
}

//------------------------------------------------------------------------------
//
void QEPlotter::itemContextMenuRequested (const QPoint& pos)
{
   QObject *obj = this->sender();   // who sent the signal.
   int slot = this->findSlot (obj);
   QPoint golbalPos;

   SLOT_CHECK (slot,);
   DataSets* ds = &(this->xy [slot]);

   // Differed creation - see generalContextMenuRequested comment.
   //
   if (!this->xy [slot].itemMenu) {
      this->xy [slot].itemMenu = new QEPlotterMenu (slot, this);
      QEScaling::applyToWidget (this->xy [slot].itemMenu);
      this->connectMenuOrToolBar (this->xy [slot].itemMenu);
   }

   // Allow paste PV menu if and only if we have something to paste?
   //
   bool pasteAllowed = !(QApplication::clipboard ()->text ().trimmed().isEmpty ());

   ds->itemMenu->setActionEnabled (QEPlotterNames::PLOTTER_PASTE_DATA_PV, pasteAllowed);
   ds->itemMenu->setActionEnabled (QEPlotterNames::PLOTTER_PASTE_SIZE_PV, pasteAllowed);

   // Allow edit PV menu if and only if we are using the engineer use level.
   //
   bool inEngineeringMode = (this->getUserLevel () == userLevelTypes::USERLEVEL_ENGINEER);

   ds->itemMenu->setActionVisible (QEPlotterNames::PLOTTER_GENERAL_DATA_PV_EDIT, inEngineeringMode);
   ds->itemMenu->setActionVisible (QEPlotterNames::PLOTTER_GENERAL_SIZE_PV_EDIT, inEngineeringMode);

   if (slot > 0) {
      // Only meaningful for y data sets.
      //
      ds->itemMenu->setCheckedStates (ds->isDisplayed, ds->isBold, ds->isDashed, ds->showDots);

      ds->itemMenu->setActionChecked (QEPlotterNames::PLOTTER_LINE_NO_MEDIAN_FILTER, (ds->median == 1));
      ds->itemMenu->setActionChecked (QEPlotterNames::PLOTTER_LINE_MEDIAN_3_FILTER, (ds->median == 3));
      ds->itemMenu->setActionChecked (QEPlotterNames::PLOTTER_LINE_MEDIAN_5_FILTER, (ds->median == 5));
   }

   golbalPos = ds->itemName->mapToGlobal (pos);
   ds->itemMenu->exec (golbalPos, 0);
}

//------------------------------------------------------------------------------
// Both the QEPlotterMenu and QEPlotterToolBar widgets use the same
// signal signature.
//
bool QEPlotter::connectMenuOrToolBar (QWidget* item) const
{
   return QObject::connect (item, SIGNAL (selected     (const QEPlotterNames::MenuActions, const int)),
                            this, SLOT   (menuSelected (const QEPlotterNames::MenuActions, const int)));
}

//------------------------------------------------------------------------------
//
void QEPlotter::runDataDialog (const int slot, QWidget* control)
{
   SLOT_CHECK (slot,);
   int n;

   // Note: we populate the dialog with the expanded variable name.
   //
   this->dataDialog->setFieldInformation (this->getXYExpandedDataPV (slot),
                                          this->getXYAlias (slot),
                                          this->getXYExpandedSizePV (slot));

   n = this->dataDialog->exec (control ? control: this);
   if (n == 1) {
      QString newData;
      QString newAlias;
      QString newSize;

      this->dataDialog->getFieldInformation (newData, newAlias, newSize);
      this->setXYDataPV (slot, newData);
      this->setXYAlias  (slot, newAlias);
      this->setXYSizePV (slot, newSize);
      this->replotIsRequired = true;
   }
}


//------------------------------------------------------------------------------
//
void QEPlotter::menuSelected (const QEPlotterNames::MenuActions action, const int slot)
{
   SLOT_CHECK (slot,);  // We know slot is 0 (i.e valid) when not used.

   QWidget* wsender = dynamic_cast <QWidget*> (this->sender ());
   QClipboard* cb = NULL;
   QString copyText;
   QString pasteText;
   QString pvName;
   DataSets* ds = &(this->xy [slot]);
   QETwinScaleSelectDialog::ScaleLimit xScale;
   QETwinScaleSelectDialog::ScaleLimit yScale;
   int n;

   switch (action) {

      case QEPlotterNames::PLOTTER_SHOW_HIDE_CROSSHAIRS:
         this->crosshairsAreRequired = !this->crosshairsAreRequired;
         if (this->contextMenuIsOverGraphic) {
            // Display cross hairs at current mouse location.
            //
            this->plotArea->setMarkupVisible (QEGraphicNames::CrossHair, this->crosshairsAreRequired);
            this->plotArea->setMarkupPosition (QEGraphicNames::CrossHair, this->contextMenuRequestPosition);
         } else {
            // Mouse not over the graphic - use previos location.
            this->plotArea->setMarkupVisible (QEGraphicNames::CrossHair, this->crosshairsAreRequired);
         }
         this->replotIsRequired = true;
         break;

      case QEPlotterNames::PLOTTER_SHOW_HIDE_TOOLBAR:
         this->setToolBarVisible (! this->getToolBarVisible ());
         break;

      case QEPlotterNames::PLOTTER_SHOW_HIDE_PV_ITEMS:
         this->setPvItemsVisible (! this->getPvItemsVisible ());
         break;

      case QEPlotterNames::PLOTTER_SHOW_HIDE_STATUS:
         this->setStatusVisible (! this->getStatusVisible ());
         break;

      case QEPlotterNames::PLOTTER_EMIT_COORDINATES:
         emit this->coordinateSelected  (this->contextMenuRequestPosition);
         emit this->xCoordinateSelected (this->contextMenuRequestPosition.x ());
         emit this->yCoordinateSelected (this->contextMenuRequestPosition.y ());
         break;

      case QEPlotterNames::PLOTTER_PREV:
         this->prevState ();
         break;

      case QEPlotterNames::PLOTTER_NEXT:
         this->nextState ();
         break;

      case QEPlotterNames::PLOTTER_NORMAL_VIDEO:
         this->setVideoMode (normal);
         this->pushState ();
         break;

      case QEPlotterNames::PLOTTER_REVERSE_VIDEO:
         this->setVideoMode (reverse);
         this->pushState ();
         break;

      case QEPlotterNames::PLOTTER_LINEAR_Y_SCALE:
         this->plotArea->setYLogarithmic (false);
         this->pushState ();
         break;

      case QEPlotterNames::PLOTTER_LOG_Y_SCALE:
         this->plotArea->setYLogarithmic (true);
         this->pushState ();
         break;

      case QEPlotterNames::PLOTTER_MANUAL_Y_RANGE:
         this->rangeDialog->setWindowTitle ("Plotter Y Range");
         this->rangeDialog->setRange (this->fixedMinY, this->fixedMaxY);
         n = this->rangeDialog->exec (wsender ? wsender : this);
         if (n == 1) {
            // User has selected okay.
            //
            this->yScaleMode = QEPlotterNames::smFixed;
            this->fixedMinY = this->rangeDialog->getMinimum ();
            this->fixedMaxY = this->rangeDialog->getMaximum ();
            this->pushState ();
         }
         break;

      case QEPlotterNames::PLOTTER_CURRENT_Y_RANGE:
         this->yScaleMode = QEPlotterNames::smFixed;
         this->fixedMinY = this->currentMinY;
         this->fixedMaxY = this->currentMaxY;
         this->pushState ();
         break;

      case QEPlotterNames::PLOTTER_DYNAMIC_Y_RANGE:
         this->yScaleMode = QEPlotterNames::smDynamic;
         this->pushState ();
         break;

      case QEPlotterNames::PLOTTER_LINEAR_X_SCALE:
         this->plotArea->setXLogarithmic (false);
         this->pushState ();
         break;

      case QEPlotterNames::PLOTTER_LOG_X_SCALE:
         this->plotArea->setXLogarithmic (true);
         this->pushState ();
         break;

      case QEPlotterNames::PLOTTER_MANUAL_X_RANGE:
         this->rangeDialog->setWindowTitle ("Plotter X Range");
         this->rangeDialog->setRange (this->fixedMinX, this->fixedMaxX);
         n = this->rangeDialog->exec (wsender ? wsender : this);
         if (n == 1) {
            // User has selected okay.
            //
            this->xScaleMode = QEPlotterNames::smFixed;
            this->fixedMinX = this->rangeDialog->getMinimum ();
            this->fixedMaxX = this->rangeDialog->getMaximum ();
            this->pushState ();
         }
         break;

      case QEPlotterNames::PLOTTER_CURRENT_X_RANGE:
         this->xScaleMode = QEPlotterNames::smFixed;
         this->fixedMinX = this->currentMinX;
         this->fixedMaxX = this->currentMaxX;
         this->pushState ();
         break;

      case QEPlotterNames::PLOTTER_DYNAMIC_X_RANGE:
         this->xScaleMode = QEPlotterNames::smDynamic;
         this->pushState ();
         break;

      case QEPlotterNames::PLOTTER_NORAMLISED_Y_RANGE:
         this->yScaleMode = QEPlotterNames::smNormalised;
         this->fixedMinY = 0.0;
         this->fixedMaxY = 1.0;
         this->pushState ();
         break;

      case QEPlotterNames::PLOTTER_FRACTIONAL_Y_RANGE:
         this->yScaleMode = QEPlotterNames::smFractional;
         this->fixedMinY = 0.0;
         this->fixedMaxY = 1.0;
         this->pushState ();
         break;

      case QEPlotterNames::PLOTTER_MANUAL_XY_RANGE:
         // Like PLOTTER_MANUAL_X_RANGE and PLOTTER_MANUAL_Y_RANGE but
         // does both at once.
         xScale.min = this->fixedMinX;
         xScale.max = this->fixedMaxX;
         yScale.min = this->fixedMinY;
         yScale.max = this->fixedMaxY;
         this->twinRangeDialog->setActiveMap (xScale, yScale);
         n = this->twinRangeDialog->exec (wsender ? wsender : this);
         if (n == 1) {
            // User has selected okay.
            //
            this->xScaleMode = QEPlotterNames::smFixed;
            this->yScaleMode = QEPlotterNames::smFixed;
            this->twinRangeDialog->getActiveMap (xScale, yScale);
            this->fixedMinX = xScale.min;
            this->fixedMaxX = xScale.max;
            this->fixedMinY = yScale.min;
            this->fixedMaxY = yScale.max;
            this->pushState ();
         }
         break;

      case  QEPlotterNames::PLOTTER_PLAY:
         this->isPaused = false;
         // TODO - request framefork to resend data for all in-use channels.
         this->pushState ();
         break;

      case QEPlotterNames::PLOTTER_PAUSE:
         this->isPaused = true;
         this->pushState ();
         break;

         // These just call the standard context menu processing.
         //
      case QEPlotterNames::PLOTTER_COPY_VARIABLE:
         this->contextMenuTriggered (contextMenu::CM_COPY_VARIABLE);
         break;

      case QEPlotterNames::PLOTTER_COPY_DATA:
         this->contextMenuTriggered (contextMenu::CM_COPY_DATA);
         break;

      case QEPlotterNames::PLOTTER_PASTE:
         this->contextMenuTriggered (contextMenu::CM_PASTE);
         break;

      case QEPlotterNames::PLOTTER_DRAG_VARIABLE:
         this->contextMenuTriggered (contextMenu::CM_DRAG_VARIABLE);
         break;

      case QEPlotterNames::PLOTTER_DRAG_DATA:
         this->contextMenuTriggered (contextMenu::CM_DRAG_DATA);
         break;

      case QEPlotterNames::PLOTTER_SELECT_USE_PV_NAME:
         this->contextMenuTriggered (QEPlotter::ADWCM_SELECT_USE_PV_NAME);
         break;

      case QEPlotterNames::PLOTTER_SELECT_USE_ALIAS_NAME:
         this->contextMenuTriggered (QEPlotter::ADWCM_SELECT_USE_ALIAS_NAME);
         break;

      case QEPlotterNames::PLOTTER_SELECT_USE_DESCRIPTION:
         this->contextMenuTriggered (QEPlotter::ADWCM_SELECT_USE_DESCRIPTION);
         break;

         //----------------------------------------------------------------------------
         // PV item specific.
         //
      case QEPlotterNames::PLOTTER_LINE_BOLD:
         ds->isBold = !ds->isBold;
         this->replotIsRequired = true;
         break;

      case QEPlotterNames::PLOTTER_LINE_DASHED:
         ds->isDashed = !ds->isDashed;
         this->replotIsRequired = true;
         break;

      case QEPlotterNames::PLOTTER_LINE_DOTS:
         ds->showDots = !ds->showDots;
         this->replotIsRequired = true;
         break;

      case QEPlotterNames::PLOTTER_LINE_VISIBLE:
         ds->isDisplayed = !ds->isDisplayed;
         ds->checkBox->setChecked (ds->isDisplayed);
         this->replotIsRequired = true;
         break;

      case QEPlotterNames::PLOTTER_LINE_NO_MEDIAN_FILTER:
         ds->median = 1;
         this->replotIsRequired = true;
         break;

      case QEPlotterNames::PLOTTER_LINE_MEDIAN_3_FILTER:
         ds->median = 3;
         this->replotIsRequired = true;
         break;

      case QEPlotterNames::PLOTTER_LINE_MEDIAN_5_FILTER:
         ds->median = 5;
         this->replotIsRequired = true;
         break;

      case QEPlotterNames::PLOTTER_LINE_COLOUR:
         this->colourDialog->setCurrentColor (ds->colour);
         n = this->colourDialog->exec();
         if (n == 1) {
            ds->colour = this->colourDialog->currentColor ();
            ds->itemName->setStyleSheet (QEUtilities::colourToStyle (ds->colour) );

            this->replotIsRequired = true;
         }
         break;

      case QEPlotterNames::PLOTTER_DATA_SELECT:
         if (slot > 0) {
            this->selectDataSet (slot);
         }
         break;

      case QEPlotterNames::PLOTTER_DATA_DIALOG:
         this->runDataDialog (slot, wsender);
         break;

      case  QEPlotterNames::PLOTTER_PASTE_DATA_PV:
         cb = QApplication::clipboard ();
         pasteText = cb->text ().trimmed ();

         if (! pasteText.isEmpty ()) {
            this->setXYDataPV (slot, pasteText);
            this->replotIsRequired = true;
         }
         break;

      case  QEPlotterNames::PLOTTER_PASTE_SIZE_PV:
         cb = QApplication::clipboard ();
         pasteText = cb->text ().trimmed ();

         if (! pasteText.isEmpty ()) {
            this->setXYSizePV (slot, pasteText);
            this->replotIsRequired = true;
         }
         break;


      case QEPlotterNames::PLOTTER_COPY_DATA_VARIABLE:
         // Note: we copy the expanded variable name.
         //
         copyText = this->getSubstitutedVariableName (2 * slot);
         cb = QApplication::clipboard ();
         cb->setText (copyText);
         break;

      case QEPlotterNames::PLOTTER_COPY_SIZE_VARIABLE:
         // Note: we copy the expanded variable name.
         //
         copyText = this->getSubstitutedVariableName (2 * slot + 1);
         cb = QApplication::clipboard ();
         cb->setText (copyText);
         break;


      case QEPlotterNames::PLOTTER_COPY_DATA_DATA:
         copyText = ds->getDataData ();
         cb = QApplication::clipboard ();
         cb->setText (copyText);
         break;

      case QEPlotterNames::PLOTTER_COPY_SIZE_DATA:
         copyText = ds->getSizeData ();
         cb = QApplication::clipboard ();
         cb->setText (copyText);
         break;


      case QEPlotterNames::PLOTTER_SHOW_DATA_PV_PROPERTIES:
         pvName = this->getXYExpandedDataPV (slot);
         this->sendRequestAction (QEActionRequests::actionPvProperties (), pvName);
         break;

      case QEPlotterNames::PLOTTER_SHOW_SIZE_PV_PROPERTIES:
         pvName = this->getXYExpandedSizePV (slot);
         this->sendRequestAction (QEActionRequests::actionPvProperties (), pvName);
         break;


      case QEPlotterNames::PLOTTER_ADD_DATA_PV_TO_STRIPCHART:
         pvName = this->getXYExpandedDataPV (slot);
         this->sendRequestAction (QEActionRequests::actionStripChart (), pvName);
         break;

      case QEPlotterNames::PLOTTER_ADD_SIZE_PV_TO_STRIPCHART:
         pvName = this->getXYExpandedSizePV (slot);
         this->sendRequestAction (QEActionRequests::actionStripChart (), pvName);
         break;


      case QEPlotterNames::PLOTTER_ADD_DATA_PV_TO_SCRATCH_PAD:
         pvName = this->getXYExpandedDataPV (slot);
         this->sendRequestAction (QEActionRequests::actionScratchPad (), pvName);
         break;

      case QEPlotterNames::PLOTTER_ADD_SIZE_PV_TO_SCRATCH_PAD:
         pvName = this->getXYExpandedSizePV (slot);
         this->sendRequestAction (QEActionRequests::actionScratchPad (), pvName);
         break;


      case QEPlotterNames::PLOTTER_GENERAL_DATA_PV_EDIT:
         pvName = this->getXYExpandedDataPV (slot);
         this->sendRequestAction (QEActionRequests::actionGeneralPvEdit (), pvName);
         break;

      case QEPlotterNames::PLOTTER_GENERAL_SIZE_PV_EDIT:
         pvName = this->getXYExpandedSizePV (slot);
         this->sendRequestAction (QEActionRequests::actionGeneralPvEdit (), pvName);
         break;


      case QEPlotterNames::PLOTTER_DATA_CLEAR:
         this->setXYDataPV (slot, "");
         this->setXYAlias  (slot, "");
         this->setXYSizePV (slot, "");
         this->replotIsRequired = true;
         break;

      case QEPlotterNames::PLOTTER_SCALE_TO_MIN_MAX:
         if ((slot > 0) && (ds->dataKind == DataPVPlot || ds->dataKind == CalculationPlot)) {
            this->fixedMinY = ds->plottedMin;
            this->fixedMaxY = ds->plottedMax;
            this->yScaleMode = QEPlotterNames::smFixed;
         }
         break;

      case QEPlotterNames::PLOTTER_SCALE_TO_ZERO_MAX:
         if ((slot > 0) && (ds->dataKind == DataPVPlot || ds->dataKind == CalculationPlot)) {
            this->fixedMinY = 0;
            this->fixedMaxY = ds->plottedMax;
            this->yScaleMode = QEPlotterNames::smFixed;
         }
         break;

      case QEPlotterNames::PLOTTER_LOAD_CONFIG:
         this->loadWidgetConfiguration ();
         break;

      case QEPlotterNames::PLOTTER_SAVE_CONFIG:
         this->saveWidgetConfiguration ();
         break;

      default:
         DEBUG << slot <<  action;
         break;
   }
}

//------------------------------------------------------------------------------
//
void QEPlotter::pvNameDropEvent (const int slot, QDropEvent *event)
{
   SLOT_CHECK (slot,);

   // If no text available, do nothing
   //
   if (!event->mimeData()->hasText ()){
      event->ignore ();
      return;
   }

   // Get the drop data
   //
   const QMimeData *mime = event->mimeData ();

   // If there is any text, drop the text

   if (!mime->text().isEmpty ()) {
      // Get the component textual parts
      //
      QStringList pieces = QEUtilities::split (mime->text ());

      // Carry out the drop action
      //
      this->setXYDataPV (slot, pieces.value (0, ""));
      this->setXYSizePV (slot, pieces.value (1, ""));
      this->setXYAlias  (slot, pieces.value (2, ""));
   }

   // Tell the dropee that the drop has been acted on
   //
   if (event->source() == this) {
      event->setDropAction(Qt::CopyAction);
      event->accept();
   } else {
      event->acceptProposedAction ();
   }
}

//------------------------------------------------------------------------------
//
void QEPlotter::setReadOut (const QString& text)
{
   message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STATUS);
   this->sendMessage (text, mt);
}

//------------------------------------------------------------------------------
//
void QEPlotter::captureState (QEPlotterState& state)
{
   // Capture current state.
   //
   state.isXLogarithmic = this->plotArea->getXLogarithmic ();
   state.isYLogarithmic = this->plotArea->getYLogarithmic ();
   state.isReverse = this->isReverse;
   state.isPaused = this->isPaused;
   state.xMinimum = this->fixedMinX;
   state.xMaximum = this->fixedMaxX;
   state.xScaleMode = this->xScaleMode;
   state.yMinimum = this->fixedMinY;
   state.yMaximum = this->fixedMaxY;
   state.yScaleMode = this->yScaleMode;
}

//------------------------------------------------------------------------------
//
void QEPlotter::applyState (const QEPlotterState& state)
{
   this->plotArea->setXLogarithmic (state.isXLogarithmic);
   this->plotArea->setYLogarithmic (state.isYLogarithmic);
   this->isReverse  = state.isReverse;
   this->isPaused  = state.isPaused;
   this->fixedMinX  = state.xMinimum;
   this->fixedMaxX  = state.xMaximum;
   this->xScaleMode  = state.xScaleMode;
   this->fixedMinY  = state.yMinimum;
   this->fixedMaxY  = state.yMaximum;
   this->yScaleMode  = state.yScaleMode;
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEPlotter::pushState ()
{
   QEPlotterState state;

   // Capture current state.
   //
   this->captureState (state);
   this->stateList.push (state);

   // Enable/disble buttons according to availability.
   //
   this->toolBar->setEnabled (QEPlotterNames::PLOTTER_PREV, (this->stateList.prevAvailable ()));
   this->toolBar->setEnabled (QEPlotterNames::PLOTTER_NEXT, (this->stateList.nextAvailable ()));
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEPlotter::prevState ()
{
   QEPlotterState state;

   if (this->stateList.prev (state)) {
      this->applyState (state);
      this->toolBar->setEnabled (QEPlotterNames::PLOTTER_PREV, (this->stateList.prevAvailable ()));
      this->toolBar->setEnabled (QEPlotterNames::PLOTTER_NEXT, (this->stateList.nextAvailable ()));
   }
}

//------------------------------------------------------------------------------
//
void QEPlotter::nextState ()
{
   QEPlotterState state;

   if (this->stateList.next (state)) {
      this->applyState (state);
      this->toolBar->setEnabled (QEPlotterNames::PLOTTER_PREV, (this->stateList.prevAvailable ()));
      this->toolBar->setEnabled (QEPlotterNames::PLOTTER_NEXT, (this->stateList.nextAvailable ()));
   }
}

//------------------------------------------------------------------------------
//
void QEPlotter::calcCrosshairIndex (const double x)
{
   DataSets* xs = &this->xy [0];   // use a alias pointer for brevity
   int newIndex;
   int number;

   newIndex = -1;
   number = xs->data.count ();
   for (int j = number - 1; j >= 0; j--) {
      if (x >= xs->data.value (j)) {
         // found it
         newIndex = j;
         break;
      }
   }

   // Has the vertical crosshair index changed?
   //
   if (this->crosshairIndex != newIndex) {
      this->crosshairIndex = newIndex;
      if (this->crosshairsAreRequired) {
         emit this->crosshairIndexChanged (this->crosshairIndex);
      }
   }
}

//------------------------------------------------------------------------------
//
int QEPlotter::getCrosshairIndex () const
{
   return this->crosshairIndex;
}

//------------------------------------------------------------------------------
//
void QEPlotter::markupMove (const QEGraphicNames::Markups markup, const QPointF& position)
{
   if (markup == QEGraphicNames::CrossHair) {
      // Determine and emit new vertical crosshair index if required.
      //
      this->calcCrosshairIndex (position.x ());
   }
}

//------------------------------------------------------------------------------
//
void QEPlotter::setDataPvNameSet (const QStringList& pvNameSet)
{
   // Stop infinite signal slot loops.
   //
   if (!this->pvNameSetChangeInhibited) {
      for (int slot = 0; slot < ARRAY_LENGTH (this->xy); slot++) {
         QString pvName = pvNameSet.value (slot, "");
         this->setNewVariableName (pvName, "", 2*slot + 0);
         this->setNewVariableName ("",     "", 2*slot + 1);
      }
   }
}

//------------------------------------------------------------------------------
//
QStringList QEPlotter::getDataPvNameSet () const
{
   QStringList result;

   for (int slot = 0; slot < ARRAY_LENGTH (this->xy); slot++) {
      QString pvName;

      if (this->xy [slot].dataKind == DataPVPlot) {
         pvName = this->getXYExpandedDataPV (slot);
      } else {
         // Either not in use or more importantly CalculationPlot which is
         // not a PV per se.
         //
         pvName = "";
      }
      result.append (pvName);
   }

   return result;
}

//------------------------------------------------------------------------------
// slot
//
void QEPlotter::setAliasNameSet (const QStringList& aliasNameSet)
{
   // Stop infinite signal slot loops.
   //
   if (!this->alaisSetChangeInhibited) {
      for (int slot = 0; slot < ARRAY_LENGTH (this->xy); slot++) {
         QString alias = aliasNameSet.value (slot, "");
         this->setXYAlias (slot, alias);
      }
   }
}

//------------------------------------------------------------------------------
//
QStringList QEPlotter::getAliasNameSet () const
{
   QStringList result;

   for (int slot = 0; slot < ARRAY_LENGTH (this->xy); slot++) {
      QString alias = this->getXYAlias (slot);
      result.append (alias);
   }

   return result;
}

//------------------------------------------------------------------------------
//
QEGraphic* QEPlotter::getGraphic () const
{
   return this->plotArea;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setPlotterEntry (const int slot, const QString& pvName, const QString& alias)
{
   SLOT_CHECK (slot,);

   this->setXYDataPV (slot, pvName);
   this->setXYAlias (slot, alias);
}

//------------------------------------------------------------------------------
//
void QEPlotter::plotMouseMove (const QPointF& posn)
{
   QString mouseReadOut;
   QString f;
   QPointF slope;

   mouseReadOut = "";

   f = QString::asprintf ("  x: %+.6g", posn.x ());
   mouseReadOut.append (f);

   f = QString::asprintf ("  y: %+.6g", posn.y ());
   mouseReadOut.append (f);

   if (this->plotArea->getSlopeIsDefined (slope)) {
      const double dx = slope.x ();
      const double dy = slope.y ();

      f = QString::asprintf ("  dx: %+.6g", dx);
      mouseReadOut.append (f);

      f = QString::asprintf ("  dy: %+.6g", dy);
      mouseReadOut.append (f);

      // Calculate slope, but avoid the divide by 0.
      //
      mouseReadOut.append ("  dy/dx: ");
      if (dx != 0.0) {
         f = QString::asprintf ("%+.6g", dy/dx);
      } else {
         if (dy != 0.0) {
            f = QString::asprintf ("%sinf", (dy >= 0.0) ? "+" : "-");
         } else {
            f = QString::asprintf ("n/a");
         }
      }
      mouseReadOut.append (f);
   }

   this->setReadOut (mouseReadOut);
}

//------------------------------------------------------------------------------
//
void QEPlotter::zoomInOut (const QPointF& about, const int zoomAmount)
{
   if (zoomAmount) {
      // We really only need the sign of the zoomAmount.
      //
      const double factor = (zoomAmount >= 0) ? 0.95 : (1.0 / 0.95);

      double newMin;
      double newMax;

      if (this->plotArea->getYLogarithmic ()) {
         const double logAboutY = LOG10 (about.y ());

         newMin = EXP10 (logAboutY + (LOG10 (this->fixedMinY) - logAboutY) * factor);
         newMax = EXP10 (logAboutY + (LOG10 (this->fixedMaxY) - logAboutY) * factor);
      } else {
         newMin = about.y () + (this->fixedMinY - about.y ()) * factor;
         newMax = about.y () + (this->fixedMaxY - about.y ()) * factor;
      }

      this->setYRange (newMin, newMax);
      this->pushState ();
   }
}

//------------------------------------------------------------------------------
//
void QEPlotter::setXRange (const double xMinimumIn, const double xMaximumIn)
{
   this->fixedMinX = xMinimumIn;
   this->fixedMaxX = xMaximumIn;
   if (this->xScaleMode == QEPlotterNames::smDynamic) {
      this->xScaleMode = QEPlotterNames::smFixed;
   }
   this->pushState ();
}

//------------------------------------------------------------------------------
//
void QEPlotter::setYRange (const double yMinimumIn, const double yMaximumIn)
{
   if (this->plotArea->getYLogarithmic ()) {
      this->fixedMinY = LIMIT (yMinimumIn,  0.0, +1.0e23);
   } else {
      this->fixedMinY = LIMIT (yMinimumIn, -1.0e24, +1.0e23);
   }

   this->fixedMaxY = LIMIT (yMaximumIn, this->fixedMinY + 1.0e-20, +1.0e24);

   if (this->yScaleMode == QEPlotterNames::smDynamic) {
      this->yScaleMode = QEPlotterNames::smFixed;
   }
   this->pushState ();
}

//------------------------------------------------------------------------------
//
void QEPlotter::scaleSelect (const QPointF& start, const QPointF& finish)
{
   QPoint distance = this->plotArea->pixelDistance (start, finish);

   // The QEGraphic validates the selection, i.e. that user has un-ambiguously
   // selected x scaling or y scaling. Need only figure out which one.
   //
   if (ABS (distance.y ()) >=  ABS (distance.x ())) {
      // Makeing a Y scale adjustment.
      //
      this->setYRange (finish.y (), start.y ());

   } else {
      // Makeing a X scale adjustment.
      //
      this->setXRange (start.x (), finish.x ());

   }
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEPlotter::lineSelected (const QPointF&, const QPointF&)
{
   // no action per se - just request a replot (without the line).
   //
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
//
void QEPlotter::pvLabelModeChanged ()
{
   for (int slot = 0; slot < NUMBER_OF_SLOTS; slot++) {
      this->updateLabel (slot);
   }
}

//------------------------------------------------------------------------------
//
bool QEPlotter::eventFilter (QObject *obj, QEvent *event)
{
   const QEvent::Type type = event->type ();
   QMouseEvent* mouseEvent = NULL;
   int slot;

   switch (type) {

      case QEvent::MouseButtonPress:
         mouseEvent = static_cast<QMouseEvent *> (event);
         slot = this->findSlot (obj);
         if (slot > 0 && (mouseEvent->button () ==  Qt::LeftButton)) {
            // Leverage of menu handler
            this->menuSelected (QEPlotterNames::PLOTTER_DATA_SELECT, slot);
            return true;  // we have handled this mouse press
         }
         break;

      case QEvent::MouseButtonRelease:
         mouseEvent = static_cast<QMouseEvent *> (event);
         break;

      case QEvent::MouseMove:
         mouseEvent = static_cast<QMouseEvent *> (event);
         break;

      case QEvent::MouseButtonDblClick:
         slot = this->findSlot (obj);
         if (slot >= 0) {
            this->runDataDialog (slot, dynamic_cast <QWidget*> (obj));
            return true;  // we have handled double click
         }
         break;

      case QEvent::DragEnter:
         slot = this->findSlot (obj);
         if (slot >= 0) {
            QDragEnterEvent* dragEnterEvent = static_cast<QDragEnterEvent*> (event);

            // Can only drop if text and not in use.
            //
            if ((dragEnterEvent->mimeData()->hasText ()) &&
                (this->xy [slot].dataKind== NotInUse)) {
               dragEnterEvent->setDropAction (Qt::CopyAction);
               dragEnterEvent->accept ();
               this->highLight (slot, true);
            } else {
               dragEnterEvent->ignore ();
               this->highLight (slot, false);
            }
            return true;
         }
         break;

      case QEvent::DragLeave:
         slot = this->findSlot (obj);
         if (slot >= 0) {
            this->highLight (slot, false);
            return true;
         }
         break;

      case QEvent::Drop:
         slot = this->findSlot (obj);
         if (slot >= 0) {
            QDropEvent* dropEvent = static_cast<QDropEvent*> (event);
            this->pvNameDropEvent (slot, dropEvent);
            this->highLight (slot, false);
            return true;
         }
         break;

      default:
         // Just fall through
         break;
   }

   return false;
}

//------------------------------------------------------------------------------
//
QString QEPlotter::copyVariable ()
{
   QString result;

   for (int slot = 0; slot < ARRAY_LENGTH (this->xy); slot++) {
      QString strData = this->getXYExpandedDataPV (slot);
      QString strSize = this->getXYExpandedSizePV (slot);

      if (!strData.isEmpty()) {
         if( !result.isEmpty() ) {
            result.append( " " );
         }
         result.append (strData);
      }

      if (!strSize.isEmpty()) {
         if( !result.isEmpty() ) {
            result.append( " " );
         }
         result.append (strSize);
      }

   }

   return result;
}

//------------------------------------------------------------------------------
// Copy all data
//
QVariant QEPlotter::copyData ()
{
   const int fw = 12;   // field width
   DataSets* xs;
   DataSets* ys;
   QString result = "none";
   int nx;
   int ny [ARRAY_LENGTH (this->xy)];

   xs = &this->xy [0];
   if (!xs->isInUse()) {
      // no data
      return QVariant (result);
   }

   result = "\n";

   // First to headers.
   //
   result.append (QString ("%1").arg (xs->letter, fw));
   for (int slot = 1; slot < ARRAY_LENGTH (this->xy); slot++) {
      ys = &this->xy [slot];
      if (ys->isInUse ()) {
         result.append (QString ("\t%1").arg (ys->letter, fw));
         ny [slot] = ys->effectiveSize ();
      } else {
         ny [slot] = 0;
      }
   }
   result.append (QString ("\n"));

   nx = xs->effectiveSize ();
   for (int j = 0; j < nx; j++) {
      // Do x data
      //
      result.append (QString ("%1").arg (xs->data[j], fw));
      for (int slot = 1; slot < ARRAY_LENGTH (this->xy); slot++) {
         ys = &this->xy [slot];
         if (ys->isInUse ()) {
            if (j < ny [slot]) {
               result.append (QString ("\t%1").arg (ys->data [j], fw));
            } else {
               result.append (QString ("\t%1").arg ("nul", fw));
            }
         }
      }
      result.append (QString ("\n"));
   }

   return QVariant (result);
}

//------------------------------------------------------------------------------
//
void QEPlotter::saveConfiguration (PersistanceManager* pm)
{
   const QString formName = this->getPersistantName ();

   PMElement formElement = pm->addNamedConfiguration (formName);

   // Save each active PV.
   //
   PMElement pvListElement = formElement.addElement ("PV_List");

   for (int slot = 0; slot < ARRAY_LENGTH (this->xy); slot++) {
      QString strData = this->getXYExpandedDataPV (slot);
      QString strSize = this->getXYExpandedSizePV (slot);
      QString strAlias = this->getXYAlias (slot);

      // If at least one sub-item is defined then create the PV element.
      //
      if ((!strData.isEmpty ()) || (!strSize.isEmpty ()) || (!strAlias.isEmpty ()) ) {
         PMElement pvElement = pvListElement.addElement ("PV");
         pvElement.addAttribute ("id", slot);

         if (!strData.isEmpty ()) {
            pvElement.addValue ("Data", strData);
         }

         if (!strSize.isEmpty ()) {
            pvElement.addValue ("Size", strSize);
         }

         if (!strAlias.isEmpty ()) {
            pvElement.addValue ("Alias", strAlias);
         }
      }
   }
}

//------------------------------------------------------------------------------
//
void QEPlotter::restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase)
{
   if (restorePhase != FRAMEWORK) return;

   const QString formName = this->getPersistantName ();

   PMElement formElement = pm->getNamedConfiguration (formName);

   // Restore each PV.
   //
   PMElement pvListElement = formElement.getElement ("PV_List");

   for (int slot = 0; slot < ARRAY_LENGTH (this->xy); slot++) {
      PMElement pvElement = pvListElement.getElement ("PV", "id", slot);
      QString strValue;
      bool status;

      if (pvElement.isNull ()) continue;

      // Attempt to extract a PV names
      //
      status = pvElement.getValue ("Data", strValue);
      if (status) {
         this->setXYDataPV (slot, strValue);
      }

      status = pvElement.getValue ("Size", strValue);
      if (status) {
         this->setXYSizePV (slot, strValue);
      }

      status = pvElement.getValue ("Alias", strValue);
      if (status) {
         this->setXYAlias(slot, strValue);
      }
   }
}

//------------------------------------------------------------------------------
//
int QEPlotter::addPvName (const QString& pvName)
{
   int result = -1;

   // Paste to Y variables only.
   //
   for (int slot = 1; slot < ARRAY_LENGTH (this->xy); slot++) {
      DataSets* ds = &(this->xy [slot]);
      if (ds->isInUse () == false) {
         // Found an empty slot.
         //
         this->setXYDataPV (slot, pvName);
         result = slot;
         break;
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEPlotter::clearAllPvNames ()
{
   for (int slot = 0; slot < NUMBER_OF_SLOTS; slot++) {
      this->setXYDataPV (slot, "");
      this->setXYSizePV (slot, "");
   }
}

//------------------------------------------------------------------------------
//
void QEPlotter::setToolTipSummary ()
{
   int connected = 0;
   int disconnected = 0;
   int total;
   bool no_disconnects;
   QString customText;

   for (int slot = 0; slot < ARRAY_LENGTH (this->xy); slot++) {
      DataSets* ds = &this->xy [slot];

      if (ds->dataKind == DataPVPlot) {
         if (ds->dataIsConnected) {
            connected++;
         } else {
            disconnected++;
         }
      }

      if (ds->sizeKind == SizePVName) {
         if (ds->sizeIsConnected) {
            connected++;
         } else {
            disconnected++;
         }
      }
   }

   total = connected + disconnected;

   // Only disconnected when ALL, if any, are disconnected.
   //
   no_disconnects = ((connected > 0) || (total == 0));

   this->processConnectionInfo (no_disconnects);   // Is this sensible?

   if (total > 0) {
      if (connected == 0) {
         customText = "\nAll PVs disconnected";
      } else if (connected == total) {
         customText = "\nAll PVs connected";
      } else {
         customText = QString ("\n%1 out of %2 PVs connected").arg (connected).arg (total);
      }
   } else {
      customText = "";
   }

   this->updateToolTipCustom (customText);
}


//------------------------------------------------------------------------------
// Slots receiving PV data
//------------------------------------------------------------------------------
//
void QEPlotter::dataConnectionChanged (QCaConnectionInfo& connectionInfo,
                                       const unsigned int &variableIndex)
{
   const int slot = this->slotOf (variableIndex);

   SLOT_CHECK (slot,);

   DataSets* ds = &this->xy [slot];

   ds->dataIsConnected = connectionInfo.isChannelConnected ();
   this->updateToolTipConnection (ds->dataIsConnected, variableIndex);
   this->replotIsRequired = true;
   this->setToolTipSummary ();

   if (ds->dataIsConnected) {
      // We have a channel connect.
      //
      qcaobject::QCaObject* qca = this->getQcaItem(variableIndex);
      if (qca) {
         ds->description = qca->getDescription();
      }
   }
}

//------------------------------------------------------------------------------
//
void QEPlotter::dataArrayChanged (const QVector<double>& values,
                                  QCaAlarmInfo& alarmInfo,
                                  QCaDateTime&,
                                  const unsigned int& variableIndex)
{
   const int slot = this->slotOf (variableIndex);

   SLOT_CHECK (slot,);
   if (this->isPaused) return;
   if (alarmInfo.isInvalid ()) return;   // don't attempt to plot invalid data
   this->xy [slot].data = QEFloatingArray (values);
   this->replotIsRequired = true;
   this->processAlarmInfo (alarmInfo, variableIndex);
   this->setToolTipSummary ();
}

//------------------------------------------------------------------------------
//
void QEPlotter::sizeConnectionChanged (QCaConnectionInfo& connectionInfo,
                                       const unsigned int &variableIndex)
{
   const int slot = this->slotOf (variableIndex);

   SLOT_CHECK (slot,);
   this->xy [slot].sizeIsConnected = connectionInfo.isChannelConnected ();
   this->updateToolTipConnection (this->xy [slot].sizeIsConnected, variableIndex);
   this->replotIsRequired = true;
   this->setToolTipSummary ();
}

//------------------------------------------------------------------------------
//
void QEPlotter::sizeValueChanged (const long& value,
                                  QCaAlarmInfo& alarmInfo,
                                  QCaDateTime&,
                                  const unsigned int& variableIndex)
{
   const int slot = this->slotOf (variableIndex);

   SLOT_CHECK (slot,);
   if (this->isPaused) return;
   if (alarmInfo.isInvalid ()) return;   // don't attempt to plot invalid data
   this->xy [slot].dbSize = value;
   this->replotIsRequired = true;
   this->processAlarmInfo (alarmInfo, variableIndex);
   this->setToolTipSummary ();
}

//------------------------------------------------------------------------------
//
void QEPlotter::markerConnectionChanged (QCaConnectionInfo& connectionInfo,
                                         const unsigned int& variableIndex)
{
   QEGraphicNames::Markups markup;

   if (variableIndex == NONE_SLOT_VI_BASE + 0) {
      markup = QEGraphicNames::VerticalMarker_1;
   } else if (variableIndex == NONE_SLOT_VI_BASE + 1) {
      markup = QEGraphicNames::HorizontalMarker_1;
   } else {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   const bool isConnected = connectionInfo.isChannelConnected();
   if (!isConnected) {
      QEGraphic* g = this->getGraphic ();
      g->setMarkupVisible (markup, false);
   }

   this->updateToolTipConnection (isConnected, variableIndex);
   this->replotIsRequired = true;
// this->setToolTipSummary ();
}

//------------------------------------------------------------------------------
//
void QEPlotter::markerValueChanged (const double& value,
                                    QCaAlarmInfo&,
                                    QCaDateTime&,
                                    const unsigned int& variableIndex)
{
   QEGraphicNames::Markups markup;

   if (variableIndex == NONE_SLOT_VI_BASE + 0) {
      markup = QEGraphicNames::VerticalMarker_1;
   } else if (variableIndex == NONE_SLOT_VI_BASE + 1) {
      markup = QEGraphicNames::HorizontalMarker_1;
   } else {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   QEGraphic* g = this->getGraphic ();
   g->setMarkupVisible (markup, true);
   g->setMarkupPosition (markup, QPointF (value, value));
   this->replotIsRequired = true;
}

//------------------------------------------------------------------------------
// Plot and plot related functions
//------------------------------------------------------------------------------
//
void QEPlotter::plot ()
{
   QColor background;
   QColor grid;
   QPen pen;
   int slot;
   DataSets* xs;
   DataSets* ys;
   QEFloatingArray xdata;
   QEFloatingArray ydata;
   int effectiveXSize;
   int effectiveYSize;
   int number;
   double xMin, xMax;
   double yMin, yMax;
   bool xMinMaxDefined;
   bool yMinMaxDefined;

   // Prequel
   //
   this->doAnyCalculations ();

   // First release any/all previously allocated curves.
   //
   this->plotArea->releaseCurves ();

   // Set up brackground and grid.
   //
   if (this->isReverse) {
      background = clBlack;
      grid = clReverseGridLine;
   } else {
      background = clWhite;
      grid = clGridLine;
   }

   this->plotArea->setBackgroundColour (background);

   pen.setColor (grid);
   pen.setWidth (1);
   pen.setStyle (Qt::DashLine);
   this->plotArea->setGridPen (pen);

   xMinMaxDefined = false;
   xMin = 0.0;   // defaults when no values.
   xMax = 1.0;

   yMinMaxDefined = false;
   yMin = 0.0;
   yMax = 1.0;

   xs = &this->xy [0];
   effectiveXSize = xs->effectiveSize ();

   // The effectiveXSize cannot be more than the actual number of
   // array elements in the X PV.
   //
   effectiveXSize = MIN (effectiveXSize,  xs->data.size());

   for (slot = 1; slot < ARRAY_LENGTH (this->xy); slot++) {
      ys = &this->xy [slot];

      // Only display if requested.
      //
      if (!ys->isDisplayed) {
         continue;
      }

      // Ignore not in use or invalid expression
      //
      if ((ys->dataKind == NotInUse) ||
          (ys->dataKind == CalculationPlot && ! ys->expressionIsValid)) {
         continue;
      }

      effectiveYSize = ys->effectiveSize ();

      // The effectiveYSize cannot be more than the actual number of
      // array elements in the Y PV.
      //
      effectiveYSize = MIN (effectiveYSize, ys->data.size());

      // Calculate actual number of points to plot.
      // Skip if none or only a single point.
      // We need at least 2 points to plot anything.
      //
      number = MIN (effectiveXSize, effectiveYSize);

      if (number < 2)  {
         continue;
      }

      // Truncate both data sets to the same length.
      //
      xdata = QEFloatingArray (xs->data.mid (0, number));
      ydata = QEFloatingArray (ys->data.mid (0, number));

      // Gather, save and aggregate minimun and maximum values.
      // We ignore +/- inf values.
      //
      if (xMinMaxDefined) {
         // merge
         xMin = MIN (xMin, xdata.minimumValue (0.0, false));
         xMax = MAX (xMax, xdata.maximumValue (0.0, false));
      } else {
         xMin = xdata.minimumValue ();
         xMax = xdata.maximumValue ();
         xMinMaxDefined = true;
      }

      ys->plottedMin = ydata.minimumValue ();
      ys->plottedMax = ydata.maximumValue ();

      if (yMinMaxDefined) {
         // merge
         yMin = MIN (yMin, ys->plottedMin);
         yMax = MAX (yMax, ys->plottedMax);
      } else {
         yMin = ys->plottedMin;
         yMax = ys->plottedMax;
         yMinMaxDefined = true;
      }

      // This this item is the selected item, then calculate and display item attributes.
      //
      if (slot == this->selectedDataSet) {
         processSelectedItem (xdata, ydata, ys->plottedMin, ys->plottedMax);
      }

      // Check for NaN and +/-infinites.
      // NOTE: It would probably best not to plot NaN/Inf values at all,
      //       but for now set unplotable values to 0.0
      //
      for (int j = 0; j < number; j++) {
         double y = ydata [j];
         if (QEPlatform::isNaN(y) || QEPlatform::isInf(y)) {
            ydata [j] = 0.0;
         }
      }

      // Scale the y data as required.
      //
      if ((this->yScaleMode == QEPlotterNames::smNormalised) ||
          (this->yScaleMode == QEPlotterNames::smFractional)) {
         double m;
         double c;

         if (this->yScaleMode == QEPlotterNames::smNormalised) {
            m = 1.0 / MAX (ys->plottedMax - ys->plottedMin, 1.0e-6);
            c = -m * ys->plottedMin;
         } else {
            m = 1.0 / MAX (ys->plottedMax, 1.0e-6);
            c = 0.0;
         }

         for (int j = 0; j < number; j++) {
            double t = ydata [j];
            ydata [j] = m*t + c;
         }
      }

      // Lastly plot the data.
      //
      pen.setColor (ys->colour);
      if (ys->isBold) {
         pen.setWidth (2);
      } else {
         pen.setWidth (1);
      }

      if (ys->isDashed) {
         pen.setStyle (Qt::DashLine);
      } else {
         pen.setStyle (Qt::SolidLine);
      }

      this->plotArea->setCurvePen (pen);
      this->plotArea->setCurveRenderHint (QwtPlotItem::RenderAntialiased, false);
      this->plotArea->setCurveStyle (QwtPlotCurve::Lines);

      if ((ys->median > 1) && (number >= ys->median)) {
         // Apply median filter
         QEFloatingArray tdata = ydata.medianFilter (ys->median);
         ydata = tdata;
      }

      this->plotArea->plotCurveData (xdata, ydata);

      if (ys->showDots) {
         if (ys->isBold) {
            pen.setWidth (5);
         } else {
            pen.setWidth (3);
         }
         this->plotArea->setCurvePen (pen);
         this->plotArea->setCurveStyle (QwtPlotCurve::Dots);
         this->plotArea->plotCurveData (xdata, ydata);
      }
   }

   // Save current min/max values.
   //
   this->currentMinX = xMin;
   this->currentMaxX = xMax;
   this->currentMinY = yMin;
   this->currentMaxY = yMax;

   // Determine plot x and y range to use.
   // If not dynamic, use the fixed values.
   //
   if (this->xScaleMode != QEPlotterNames::smDynamic) {
      xMin = this->fixedMinX;
      xMax = this->fixedMaxX;
   }

   // Repeat for y.
   //
   if (this->yScaleMode != QEPlotterNames::smDynamic) {
      yMin = this->fixedMinY;
      yMax = this->fixedMaxY;
   }

   this->plotArea->setXRange (xMin, xMax, QEGraphicNames::SelectBySize, 40, false);
   this->plotArea->setYRange (yMin, yMax, QEGraphicNames::SelectBySize, 40, false);

   this->plotArea->replot ();

   // Ensure next timer tick only invokes plot if needs be.
   //
   this->replotIsRequired = false;
}

//------------------------------------------------------------------------------
//
int QEPlotter::maxActualYSizes () const
{
   int result = 0;

   for (int j = 1; j < ARRAY_LENGTH (this->xy); j++) {
      result = MAX (result, this->xy [j].actualSize ());
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEPlotter::doAnyCalculations ()
{
   const int x = QEExpressionEvaluation::indexOf ('X');
   const int s = QEExpressionEvaluation::indexOf ('S');

   QEExpressionEvaluation::CalculateArguments userArguments;
   DataSets* xs;
   DataSets* ys;
   int effectiveXSize;
   int effectiveYSize;
   int n;
   int j;
   int slot;
   int tols;
   double value;
   bool okay;
   bool dataIsAvailable;

   xs = &this->xy [0];  // use a alias pointer for brevity
   effectiveXSize = xs->effectiveSize ();

   switch (xs->dataKind) {

      case NotInUse:
         xs->data.clear ();

         // Use default calculation which is just x = index position 0 .. (n-1)
         //
         for (j = 0; j < effectiveXSize; j++) {
            xs->data.append ((double) j);
         }
         break;

      case DataPVPlot:
         // Leave as the data, if any, as supplied by the specified PV.
         break;

      case CalculationPlot:
         xs->data.clear ();
         if (xs->expressionIsValid) {
            for (j = 0; j < effectiveXSize; j++) {
               QEExpressionEvaluation::clear (userArguments);
               userArguments [Normal][s] = (double) j;

               value = xs->calculator->evaluate (userArguments, &okay);
               if (!okay || QEPlatform::isNaN (value) || QEPlatform::isInf (value) ) {
                  // Plot zero as opposed to some "crazy" value.
                  //
                  value = 0.0;
               }
               xs->data.append (value);
            }
         }
   }

   // Next calc slope of actual y fdta values.
   //
   for (slot = 1; slot < ARRAY_LENGTH (this->xy); slot++) {
      ys = &this->xy [slot];
      if (ys->dataKind == DataPVPlot) {
         ys->dyByDx = ys->data.calcDyByDx (xs->data);
      }
   }

   for (slot = 1; slot < ARRAY_LENGTH (this->xy); slot++) {
      ys = &this->xy [slot];
      if (ys->dataKind == CalculationPlot) {

         ys->data.clear ();
         effectiveYSize = ys->effectiveSize ();

         n = MIN (effectiveXSize, effectiveYSize);

         dataIsAvailable = true;
         for (j = 0; j < n; j++) {
            QEExpressionEvaluation::clear (userArguments);

            // Pre-defined values: S and X
            //
            userArguments [Normal][s] = (double) j;
            userArguments [Normal][x] = xs->data.value (j);
            userArguments [Primed][x] = 1.0;    // by defitions.

            for (tols = 1; tols < slot; tols++) {
               DataSets* ts = &this->xy [tols];

               // TODO: determine if this arg used?
               userArguments [Normal] [tols - 1] = ts->data.value (j, 0.0);
               userArguments [Primed] [tols - 1] = ts->dyByDx.value (j, 0.0);
            }

            if (!dataIsAvailable) break;

            value = ys->calculator->evaluate (userArguments, &okay);
            if (!okay || QEPlatform::isNaN (value) || QEPlatform::isInf (value) ) {
               // Plot zero as opposed to some "crazy" value.
               //
               value = 0.0;
            }
            ys->data.append (value);
         }

         // Calculate slope of calculated plot.
         //
         ys->dyByDx = ys->data.calcDyByDx (xs->data);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEPlotter::processSelectedItem (const QEFloatingArray& xdata,
                                     const QEFloatingArray& ydata,
                                     const double yMin, const double yMax)
{
   const int number = ydata.count ();
   QString image;
   double value;
   int jAtMax;
   int j;
   double limit;
   int lower;
   int upper;
   double sxy, sy;

   image = QString ("%1").arg (yMin);
   this->minValue->setText (image);

   image = QString ("%1").arg (yMax);
   this->maxValue->setText (image);

   value = 0.0;
   jAtMax = 0;
   for (j = 0; j < number; j++) {
      if (ydata.value(j) == yMax) {
         value = xdata.value (j);
         jAtMax = j;
         break;
      }
   }

   image = QString ("%1").arg (value);
   this->maxAtValue->setText (image);

   // FWHM: half max ias relative to min value.
   //
   limit = (yMax + yMin) / 2.0;
   for (lower = jAtMax; lower > 0          && ydata.value (lower) >= limit; lower--);
   for (upper = jAtMax; upper < number - 1 && ydata.value (upper) >= limit; upper++);

   value = xdata.value (upper) - xdata.value (lower);
   image = QString ("%1").arg (ABS(value));
   this->fwhmValue->setText (image);

   sxy = 0.0;
   sy = 0.0;
   for (j = 0; j < number; j++) {
      sxy += xdata.value (j)* ydata.value (j);
      sy  += ydata.value (j);
   }

   value = sxy / sy;
   image = QString ("%1").arg (value);
   this->comValue->setText (image);
}

//------------------------------------------------------------------------------
//
void QEPlotter::tickTimeout ()
{
   this->tickTimerCount = (this->tickTimerCount + 1) % 20;

   if ((this->tickTimerCount % 20) == 0) {
      // 20th update, i.e. 1 second has passed - must replot.
      this->replotIsRequired = true;
   }

   if (this->replotIsRequired) {
      // Refesh (description) labels.
      //
      for (int slot = 0; slot < NUMBER_OF_SLOTS; slot++) {
         this->updateLabel (slot);
      }

      this->plot ();   // clears replotIsRequired
   }
}

//------------------------------------------------------------------------------
// Property functions.
//------------------------------------------------------------------------------
//
void QEPlotter::setVariableSubstitutions (QString defaultSubstitutions)
{
   int slot;

   // Use same default subsitutions for all PVs used by this widget.
   //
   for (slot = 0 ; slot < ARRAY_LENGTH (this->xy); slot++) {
      this->xy [slot].dataVariableNameManager.setSubstitutionsProperty (defaultSubstitutions);
      this->xy [slot].sizeVariableNameManager.setSubstitutionsProperty (defaultSubstitutions);
   }

   this->xMarkerVariableNameManager.setSubstitutionsProperty (defaultSubstitutions);
   this->yMarkerVariableNameManager.setSubstitutionsProperty (defaultSubstitutions);
}

//------------------------------------------------------------------------------
//
QString QEPlotter::getVariableSubstitutions () const
{
   // Any one of these name managers can provide the subsitutions.
   //
   return this->xy [0].dataVariableNameManager.getSubstitutionsProperty ();
}

//------------------------------------------------------------------------------
//
void QEPlotter::setXMarkerPV (const QString& pvName)
{
   this->xMarkerVariableNameManager.setVariableNameProperty(pvName);
}

//------------------------------------------------------------------------------
//
QString QEPlotter::getXMarkerPV () const
{
   return this->xMarkerVariableNameManager.getVariableNameProperty();
}

//------------------------------------------------------------------------------
//
void QEPlotter::setYMarkerPV (const QString& pvName)
{
   this->yMarkerVariableNameManager.setVariableNameProperty(pvName);
}

//------------------------------------------------------------------------------
//
QString QEPlotter::getYMarkerPV () const
{
   return this->yMarkerVariableNameManager.getVariableNameProperty();
}

//------------------------------------------------------------------------------
//
void QEPlotter::setXYDataPV (const int slot, const QString& pvName)
{
   SLOT_CHECK (slot,);
   this->xy [slot].dataVariableNameManager.setVariableNameProperty (pvName);

   // Ensure we always subscribe (activate) irrespective of the profile DontActivateYet state.
   //
   const unsigned int vi = this->viOfDataSlot (slot);
   this->establishConnection (vi);
}

//------------------------------------------------------------------------------
//
QString QEPlotter::getXYDataPV (const int slot) const
{
   SLOT_CHECK (slot, "");
   return this->xy [slot].dataVariableNameManager.getVariableNameProperty ();
}

//------------------------------------------------------------------------------
//
void QEPlotter::setXYSizePV (const int slot, const QString& pvName)
{
   SLOT_CHECK (slot,);
   this->xy [slot].sizeVariableNameManager.setVariableNameProperty (pvName);

   // Ensure we always subscribe(activate) irrespective of the profile DontActivateYet state.
   //
   const unsigned int vi = this->viOfSizeSlot (slot);
   this->establishConnection (vi);
}

//------------------------------------------------------------------------------
//
QString QEPlotter::getXYSizePV (const int slot) const
{
   SLOT_CHECK (slot, "");
   return this->xy [slot].sizeVariableNameManager.getVariableNameProperty ();
}

//------------------------------------------------------------------------------
//
void QEPlotter::setXYAlias (const int slot, const QString& aliasName)
{
   SLOT_CHECK (slot,);
   this->xy[slot].aliasName = aliasName;
   this->updateLabel (slot);

   // Set guard to avoid signal slot loops.
   //
   this->alaisSetChangeInhibited = true;
   emit this->alaisNameSetChanged (this->getAliasNameSet ());
   this->alaisSetChangeInhibited = false;
}

//------------------------------------------------------------------------------
//
QString QEPlotter::getXYAlias (const int slot) const
{
   SLOT_CHECK (slot, "");
   return this->xy[slot].aliasName;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setXYColour (const int slot, const QColor& colour)
{
   SLOT_CHECK (slot,);

   // Slot 0 (X) has a fixed colour.
   //
   if (slot != 0) {
      this->xy [slot].colour = colour;
      if (this->xy [slot].itemName) {
         this->xy [slot].itemName->setStyleSheet (QEUtilities::colourToStyle (colour));
      }
   }
}

//------------------------------------------------------------------------------
//
QColor QEPlotter::getXYColour (const int slot) const
{
   SLOT_CHECK (slot, QColor (0,0,0,0));
   return this->xy[slot].colour;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setXYLineVisible (const int slot, const bool isVisible)
{
   SLOT_CHECK (slot,);

   // Slot 0 (X) has no visibility per se.
   //
   if (slot != 0) {
      this->xy[slot].isDisplayed = isVisible;
      this->xy [slot].checkBox->setChecked (isVisible);
      this->replotIsRequired = true;
   }
}

//------------------------------------------------------------------------------
//
bool QEPlotter::getXYLineVisible (const int slot) const
{
   SLOT_CHECK (slot, false);
   return this->xy[slot].isDisplayed;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setXYLineBold (const int slot, const bool isBold)
{
   SLOT_CHECK (slot,);

   // Slot 0 (X) has no boldness per se.
   //
   if (slot != 0) {
      this->xy[slot].isBold = isBold;
      this->replotIsRequired = true;
   }
}

//------------------------------------------------------------------------------
//
bool QEPlotter::getXYLineBold (const int slot) const
{
   SLOT_CHECK (slot, false);
   return this->xy[slot].isBold;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setXYLineDashed (const int slot, const bool isDashed)
{
   SLOT_CHECK (slot,);

   // Slot 0 (X) has no dashed attribute per se.
   //
   if (slot != 0) {
      this->xy[slot].isDashed = isDashed;
      this->replotIsRequired = true;
   }
}

//------------------------------------------------------------------------------
//
bool QEPlotter::getXYLineDashed (const int slot) const
{
   SLOT_CHECK (slot, false);
   return this->xy[slot].isDashed;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setXYLineHasDots (const int slot, const bool hasDots)
{
   SLOT_CHECK (slot,);

   // Slot 0 (X) has no dots to show per se.
   //
   if (slot != 0) {
      this->xy[slot].showDots = hasDots;
      this->replotIsRequired = true;
   }
}

//------------------------------------------------------------------------------
//
bool QEPlotter::getXYLineHasDots (const int slot) const
{
   SLOT_CHECK (slot, false);
   return this->xy[slot].showDots;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setEnableConextMenu (bool enable)
{
   this->enableConextMenu = enable;
}

bool QEPlotter::getEnableConextMenu () const
{
   return this->enableConextMenu;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setMenuEmitText  (const QString& text)
{
   this->contextMenuEmitLegend = text;
}

QString QEPlotter::getMenuEmitText () const
{
   return this->contextMenuEmitLegend;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setToolBarVisible (bool visible)
{
   this->toolBarIsVisible = visible;
   this->toolBarResize->setVisible (visible);
}

bool QEPlotter::getToolBarVisible () const
{
   return this->toolBarIsVisible;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setPvItemsVisible (bool visible)
{
   this->pvItemsIsVisible = visible;
   this->itemResize->setVisible (visible);
}

bool QEPlotter::getPvItemsVisible () const
{
   return this->pvItemsIsVisible;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setStatusVisible (bool visible)
{
   this->statusIsVisible = visible;
   this->statusFrame->setVisible (visible);
}

bool QEPlotter::getStatusVisible () const
{
   return this->statusIsVisible;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setAxisEnableX (bool axisEnableX)
{
   this->xAxisIsEnabled = axisEnableX;
   this->plotArea->setAxisEnableX (axisEnableX);
}

bool QEPlotter::getAxisEnableX () const
{
   return this->xAxisIsEnabled;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setAxisEnableY (bool axisEnableY)
{
   this->yAxisIsEnabled = axisEnableY;
   this->plotArea->setAxisEnableY (axisEnableY);
}

bool QEPlotter::getAxisEnableY () const
{
   return this->yAxisIsEnabled;
}


//------------------------------------------------------------------------------
//
void QEPlotter::setVideoMode (const VideoModes mode)
{
   switch (mode) {
      case normal:
         this->isReverse = false;
         this->setXYColour (NUMBER_OF_PLOTS, clBlack);
         this->replotIsRequired = true;
         break;

      case reverse:
         this->isReverse = true;
         this->setXYColour (NUMBER_OF_PLOTS, clWhite);
         this->replotIsRequired = true;
         break;
   }
}

QEPlotter::VideoModes QEPlotter::getVideoMode () const
{
   return this->isReverse ? reverse : normal;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setXLogarithmic (bool isLog)
{
   this->plotArea->setXLogarithmic (isLog);
   this->replotIsRequired = true;
}

bool QEPlotter::getXLogarithmic () const
{
   return this->plotArea->getXLogarithmic ();
}


//------------------------------------------------------------------------------
//
void QEPlotter::setYLogarithmic (bool isLog)
{
   this->plotArea->setYLogarithmic (isLog);
   this->replotIsRequired = true;
}


bool QEPlotter::getYLogarithmic () const
{
   return this->plotArea->getYLogarithmic ();
}

//------------------------------------------------------------------------------
//
void QEPlotter::setFullLengthArraySubscriptions (const bool useFullLengthArraySubscriptionsIn)
{
   this->useFullLengthArraySubscriptions = useFullLengthArraySubscriptionsIn;
}

//------------------------------------------------------------------------------
//
bool QEPlotter::getFullLengthArraySubscriptions() const
{
   return this->useFullLengthArraySubscriptions;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setIsPaused (bool paused)
{
   this->isPaused = paused;
   if (!this->isPaused) this->replotIsRequired = true;
}

bool QEPlotter::getIsPaused () const
{
   return this->isPaused;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setXMinimum (const double xMinimumIn)
{
   this->setXRange (xMinimumIn, this->fixedMaxX);
}

double QEPlotter::getXMinimum () const
{
   return this->fixedMinX;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setXMaximum (const double xMaximumIn)
{
   this->setXRange (this->fixedMinX, xMaximumIn);
}

double QEPlotter::getXMaximum () const
{
   return this->fixedMaxX;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setYMinimum (const double yMinimumIn)
{
   this->setYRange (yMinimumIn, this->fixedMaxY);
}

double QEPlotter::getYMinimum () const
{
   return this->fixedMinY;
}

//------------------------------------------------------------------------------
//
void QEPlotter::setYMaximum (const double yMaximumIn)
{
   this->setYRange (this->fixedMinY, yMaximumIn);
}

double QEPlotter::getYMaximum () const
{
   return this->fixedMaxY;
}

// end
