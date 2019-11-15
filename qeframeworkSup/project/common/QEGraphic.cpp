/*  QEGraphic.cpp
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

#include <math.h>

#include <QDebug>
#include <QPainter>
#include <QPalette>

#include <qevent.h>       // QEvent maps to qcoreevent.h, not qevent.h
#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>

#include <QECommon.h>
#include <QEScaling.h>
#include <QEPlatform.h>
#include <QEGraphicMarkup.h>
#include <QEGraphic.h>

#define DEBUG qDebug () << "QEGraphic" <<  __LINE__ << __FUNCTION__  << "  "

// These should be consistant with QEDisplayRanges::adjustMinMax functions.
//
#define MINIMUM_SPAN              (1.0e-12)
#define MAXIMUM_SPAN              (1.0e+100)

#define NUMBER_TRANISTION_STEPS   6


//==============================================================================
// QEGraphicOwnPlot class
//==============================================================================
// Derived class so that we can override drawCanvas and use this to draw
// additional artefacts
//
class QEGraphic::OwnPlot : public QwtPlot {
public:
   explicit OwnPlot (QEGraphic* parent = NULL);
   explicit OwnPlot (const QwtText &title, QEGraphic* parent = NULL);
   ~OwnPlot ();
protected:
   void drawCanvas (QPainter* painter);
private:
   QEGraphic* owner;
};


//==============================================================================
//
QEGraphic::OwnPlot::OwnPlot (QEGraphic* parent) :
   QwtPlot (parent)
{
   this->owner = parent;
}

//------------------------------------------------------------------------------
//
QEGraphic::OwnPlot::OwnPlot (const QwtText& title, QEGraphic* parent) :
   QwtPlot (title, parent)
{
   this->owner = parent;
}

//------------------------------------------------------------------------------
//
QEGraphic::OwnPlot::~OwnPlot () { }

//------------------------------------------------------------------------------
//
void QEGraphic::OwnPlot::drawCanvas (QPainter* painter)
{
   QwtPlot::drawCanvas (painter); // call super class function first.
   if (this->owner) {
      this->owner->drawTexts (painter);
   }
}

//==============================================================================
// QEGraphic::Axis class
//==============================================================================
//
QEGraphic::Axis::Axis (QwtPlot* plotIn, const int axisIdIn)
{
   this->plot = plotIn;
   this->axisId = axisIdIn;
   this->plot->setAxisScaleEngine (this->axisId, new QwtLinearScaleEngine);

   // Set defaults.
   //
   this->isLogarithmic = false;
   this->scale = 1.0;
   this->offset = 0.0;
   this->axisEnabled = true;

   // Set 'current' ranges.
   //
   this->current.setRange (0.0, 1.0);
   this->source = this->current;
   this->target = this->current;
   this->transitionCount = 0;
   this->intervalMode = QEGraphicNames::SelectByValue;
   this->intervalValue = 8.0;
   this->determineAxisScale ();
}

//------------------------------------------------------------------------------
//
QEGraphic::Axis::~Axis ()
{
   this->plot->setAxisScaleEngine (this->axisId, NULL);
}

//------------------------------------------------------------------------------
//
void QEGraphic::Axis::setRange (const double minIn, const double maxIn,
                                const QEGraphicNames::AxisMajorIntervalModes modeIn,
                                const double valueIn,
                                const bool immediate)
{
   QEDisplayRanges newTarget;
   bool rescaleIsRequired;

   newTarget.setRange (minIn, LIMIT (maxIn, minIn + MINIMUM_SPAN, minIn + MAXIMUM_SPAN));

   // Is this a significant change? Hypothosize not.
   //
   rescaleIsRequired = false;

   // Avoid rescaling for trivial changes.
   //
   if (!this->target.isSimilar (newTarget, 0.001)) {
      this->target = newTarget;
      if (immediate) {
         // Immediate - no animation.
         //
         this->source = this->target;
         this->current = this->target;
         this->transitionCount = 0;
      } else {
         // Not immediate - provide an animated transition.
         // New source is where we currently are.
         // Set up transition count down.
         //
         this->source = this->current;
         this->transitionCount = NUMBER_TRANISTION_STEPS;
      }
      rescaleIsRequired = true;
   }

   if (this->intervalMode != modeIn) {
      this->intervalMode = modeIn;
      rescaleIsRequired = true;
   }

   if (this->intervalValue != valueIn) {
      this->intervalValue = valueIn;
      rescaleIsRequired = true;
   }

   // Something changed  - re do the scaling.
   //
   if (rescaleIsRequired) {
      this->determineAxisScale ();
   }
}

//------------------------------------------------------------------------------
//
void QEGraphic::Axis::getRange (double& min, double& max)
{
   // Use use apply reverse scaling/offset here to get real-world coordinates.
   //
   min = (this->useMin - this->offset) / this->scale;
   max = (this->useMax - this->offset) / this->scale;
}

//------------------------------------------------------------------------------
//
bool QEGraphic::Axis::doDynamicRescaling ()
{
   bool result = false;

   if (this->transitionCount > 0) {
      this->transitionCount--;

      // Calulate the new current point and re set axis scale.
      //
      this->current = QEGraphic::calcTransitionPoint (this->source, this->target,
                                                      this->transitionCount);
      this->determineAxisScale ();
      result = true;
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEGraphic::Axis::determineAxisScale ()
{
   int canvasSize;
   int number;

   if (this->isLogarithmic) {
      this->current.adjustLogMinMax (this->useMin, this->useMax, this->useStep);
   } else {
      //
      switch (this->intervalMode) {

         case QEGraphicNames::UserInterval:
            // Use knows what he/she is doing.
            //
            this->useMin = current.getMinimum ();
            this->useMax = current.getMaximum ();
            this->useStep = this->intervalValue;
            break;

         case QEGraphicNames::SelectByValue:
            number = this->intervalValue;
            current.adjustMinMax (number, false, this->useMin, this->useMax, this->useStep);
            break;

         case QEGraphicNames::SelectBySize:
            // Set size determined based on the pixel size of the widget.
            //
            switch (this->axisId) {
               case QwtPlot::xTop:
               case QwtPlot::xBottom:
                  canvasSize = this->plot->canvas()->width ();
                  break;

               case QwtPlot::yLeft:
               case QwtPlot::yRight:
                  canvasSize = this->plot->canvas()->height ();
                  break;

               default:
                  canvasSize = 800;   // avoid compiler warning
                  break;
            }

            number = canvasSize / MAX (1, this->intervalValue);
            current.adjustMinMax (number, false, this->useMin, this->useMax, this->useStep);
            break;
      }

      // Subtract/add tolerance as Qwt Axis ploting of minor ticks a bit slack.
      //
      this->useMin = this->useMin - (0.01 * this->useStep);
      this->useMax = this->useMax + (0.01 * this->useStep);
   }

   // This is the only place we set the actual axis scale.
   //
   this->plot->setAxisScale (this->axisId, this->useMin, this->useMax, this->useStep);
}

//------------------------------------------------------------------------------
//
double QEGraphic::Axis::pointToReal (const int pos) const
{
   double x;

   // Perform basic inverse transformation - pixel to axis coordinates.
   //
   x = this->plot->invTransform (this->axisId, pos);

   // Scale from axis to real world units.
   //
   x = (x - this->offset) / this->scale;

   return x;
}

//------------------------------------------------------------------------------
//
int QEGraphic::Axis::realToPoint (const double pos) const
{
   int x;
   double useX;

   // Do linear scaling (if any) followed by log scaling if required.
   //
   useX = this->scale * (double) pos + this->offset;

   if (this->isLogarithmic) {
      useX = MAX (1.0E-20, useX);   // avoid going out of range
   }

   // Perform basic plot transformation.
   //
   x = this->plot->transform (this->axisId, useX);

   return x;
}

//------------------------------------------------------------------------------
//
double QEGraphic::Axis::scaleValue (const double coordinate) const
{
   double x;

   x = this->scale * coordinate + this->offset;
   if (this->isLogarithmic) {
      x = MAX (1.0E-20, x);   // avoid going out of range
   }

   return x;
}

//------------------------------------------------------------------------------
//
void QEGraphic::Axis::setAxisEnable (const bool axisEnable)
{
   this->axisEnabled = axisEnable;
   this->plot->enableAxis (this->axisId, this->axisEnabled);
}

//------------------------------------------------------------------------------
//
bool QEGraphic::Axis::getAxisEnable () const
{
   return this->axisEnabled;
}

//------------------------------------------------------------------------------
//
void QEGraphic::Axis::setScale (const double scaleIn)
{
   this->scale = scaleIn;
}

//------------------------------------------------------------------------------
//
double QEGraphic::Axis::getScale () const
{
   return this->scale;
}

//------------------------------------------------------------------------------
//
void QEGraphic::Axis::setAxisColor (const QColor axisColor) const
{
   QwtScaleWidget* scaleWidget = this->plot->axisWidget(this->axisId);
   QPalette palette = scaleWidget->palette();
   palette.setColor (QPalette::WindowText, axisColor);
   palette.setColor (QPalette::Text, axisColor);
   scaleWidget->setPalette(palette);
}

//------------------------------------------------------------------------------
//
void QEGraphic::Axis::setOffset (const double offsetIn)
{
   this->offset = offsetIn;
}

//------------------------------------------------------------------------------
//
double QEGraphic::Axis::getOffset () const
{
   return this->offset;
}

//------------------------------------------------------------------------------
//
void QEGraphic::Axis::setLogarithmic (const bool isLogarithmicIn)
{
   if (this->isLogarithmic != isLogarithmicIn) {
      this->isLogarithmic = isLogarithmicIn;

      if (this->isLogarithmic) {
#if QWT_VERSION >= 0x060100
         this->plot->setAxisScaleEngine (this->axisId, new QwtLogScaleEngine);
#else
         this->plot->setAxisScaleEngine (this->axisId, new QwtLog10ScaleEngine);
#endif
      } else {
         this->plot->setAxisScaleEngine (this->axisId, new QwtLinearScaleEngine);
      }

      // Do immediate trasition and reset
      //
      this->determineAxisScale ();
      this->transitionCount = 0;
   }
}

//------------------------------------------------------------------------------
//
bool QEGraphic::Axis::getLogarithmic () const
{
   return this->isLogarithmic;
}

//==============================================================================
// QEGraphic class
//==============================================================================
//
QEGraphic::QEGraphic (QWidget* parent) : QWidget (parent)
{
   this->plot = new QEGraphic::OwnPlot (this);
   this->construct ();
}

//------------------------------------------------------------------------------
//
QEGraphic::QEGraphic (const QString& title, QWidget* parent) : QWidget (parent)
{
   this->plot = new QEGraphic::OwnPlot (title, this);
   this->construct ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::construct ()
{
   // Set min size, not needed Qt4 but sometimes needed in Qt5
   // Applying same to this->plot appears to have same effect.
   //
   this->setMinimumSize (20, 20);

   // Create a layout within the containing widget.
   //
   this->layout = new QHBoxLayout (this);
   this->layout->setContentsMargins (0, 0, 0, 0);
   this->layout->setSpacing (0);
   this->layout->addWidget (this->plot);

   this->plotGrid = new QwtPlotGrid ();
   this->plotGrid->attach (this->plot);

   this->xAxis = new Axis (this->plot, QwtPlot::xBottom);
   this->yAxisLeft = new Axis (this->plot, QwtPlot::yLeft);
   this->yAxisRight = new Axis (this->plot, QwtPlot::yRight);

   // Construct markups set.
   //
   this->graphicMarkupsSet = QEGraphicNames::createGraphicMarkupsSet (this);

   this->setAvailableMarkups (QEGraphicNames::None);  // default availability

   // Set defaults.
   //
   this->rightButtonIsPressed = false;

   this->pen = QPen (QColor (0, 0, 0, 255));  // black
   // go with default brush for now.

   this->textFont = this->font ();  // use parent font as default font.

   // Turning this on gives relatively fuzzy lines.
   //
   this->hint = QwtPlotItem::RenderAntialiased;
   this->hintOn = false;

   this->style = QwtPlotCurve::Lines;

#if QWT_VERSION < 0x060100
   this->plot->setCanvasLineWidth (1);
#endif
   this->plot->setLineWidth (1);

   this->plot->canvas()->setMouseTracking (true);
   this->plot->canvas()->installEventFilter (this);

   // Refresh Dynamic Rescaling the stip chart at 20Hz.
   //
   this->tickTimer = new QTimer (this);
   connect (this->tickTimer, SIGNAL (timeout ()), this, SLOT (tickTimeout ()));
   this->tickTimer->start (50);  // mSec = 0.05 s
}

//------------------------------------------------------------------------------
//
QEGraphic::~QEGraphic ()
{
   // Note: must detach curves and grids, otherwise some (older) versions of qwt
   // cause a segmentation fault when the associated QwtPolot object is deleted.
   //
   this->releaseCurves ();

   if (this->plotGrid) {
      this->plotGrid->detach();
      delete this->plotGrid;
      this->plotGrid  = NULL;
   }

   delete this->xAxis;
   delete this->yAxisLeft;
   delete this->yAxisRight;

   QEGraphicNames::cleanGraphicMarkupsSet (*this->graphicMarkupsSet);
   delete this->graphicMarkupsSet;
}

//------------------------------------------------------------------------------
//
void QEGraphic::setTitle (const QString& title)
{
   this->plot->setTitle (title);
}

//------------------------------------------------------------------------------
//
QString QEGraphic::getTitle () const
{
   return this->plot->title ().text ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::enableAxis (int axisId, bool tf)
{
   this->plot->enableAxis (axisId, tf);
}

//------------------------------------------------------------------------------
//
void QEGraphic::setAxisScale (int axisId, double min, double max, double step)
{
   this->plot->setAxisScale (axisId, min, max, step);
}

//------------------------------------------------------------------------------
//
void QEGraphic::installCanvasEventFilter (QObject* eventFilter)
{
   this->plot->canvas()->installEventFilter (eventFilter);
}

//------------------------------------------------------------------------------
//
bool QEGraphic::isCanvasObject (QObject* obj) const
{
   return (obj == this->plot->canvas());
}

//------------------------------------------------------------------------------
//
QRect QEGraphic::getEmbeddedCanvasGeometry () const
{
   return this->plot->canvas()->geometry();
}

//------------------------------------------------------------------------------
//
QwtPlot* QEGraphic::getEmbeddedQwtPlot () const
{
   return this->plot;
}

//------------------------------------------------------------------------------
//
void QEGraphic::saveConfiguration (PMElement& parentElement)
{
   QEGraphicNames::saveConfiguration (*this->graphicMarkupsSet, parentElement);
}

//------------------------------------------------------------------------------
//
void QEGraphic::restoreConfiguration (PMElement & parentElement)
{
   QEGraphicNames::restoreConfiguration (*this->graphicMarkupsSet, parentElement);
}

//------------------------------------------------------------------------------
//
bool QEGraphic::doDynamicRescaling (const QwtPlot::Axis selectedYAxis)
{
   bool result;
   bool a, b;

   a = this->xAxis->doDynamicRescaling ();
   b = axisFromPosition(selectedYAxis)->doDynamicRescaling ();

   result = a||b;
   if (result) {
      this->graphicReplot ();
   }

   return result;
}

//------------------------------------------------------------------------------
//
void  QEGraphic::tickTimeout ()
{
   this->doDynamicRescaling ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::setBackgroundColour (const QColor colour)
{
#if QWT_VERSION >= 0x060000
   this->plot->setCanvasBackground (QBrush (colour));
#else
   this->plot->setCanvasBackground (colour);
#endif
}

//------------------------------------------------------------------------------
//
void QEGraphic::setGridPen (const QPen& pen)
{
   this->plotGrid->setPen (pen);
}

//------------------------------------------------------------------------------
//
void QEGraphic::setGridPens (const QPen& majorPen, const QPen& minorPen,
                             const bool enableMajorX, const bool enableMajorY,
                             const bool enableMinorX, const bool enableMinorY)
{
#if QWT_VERSION >= 0x060100
   this->plotGrid->setMajorPen (majorPen);
   this->plotGrid->setMinorPen (minorPen);
#else
   this->plotGrid->setMajPen (majorPen);
   this->plotGrid->setMinPen (minorPen);
#endif
   this->plotGrid->enableX (enableMajorX);
   this->plotGrid->enableY (enableMajorY);
   this->plotGrid->enableXMin (enableMinorX);
   this->plotGrid->enableYMin (enableMinorY);
   // If all distabled - detach grid??
}

//------------------------------------------------------------------------------
//
void QEGraphic::setAvailableMarkups (const QEGraphicNames::MarkupFlags markupFlag)
{
   const QEGraphicNames::MarkupLists keys = this->graphicMarkupsSet->keys ();
   for (int j = 0; j < keys.count (); j++) {
      const QEGraphicNames::Markups markup = keys.value (j);
      QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (markup, NULL);
      if (graphicMarkup) {
         graphicMarkup->setInUse (markup & markupFlag);
      }
   }
}

//------------------------------------------------------------------------------
//
QEGraphicNames::MarkupFlags QEGraphic::getAvailableMarkups () const
{
   QEGraphicNames::MarkupFlags result = QEGraphicNames::None;

   const QEGraphicNames::MarkupLists keys = this->graphicMarkupsSet->keys ();
   for (int j = 0; j < keys.count (); j++) {
      const QEGraphicNames::Markups markup = keys.value (j);
      QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (markup, NULL);
      if (graphicMarkup) {
         if (graphicMarkup->isInUse ()) {
            result |= markup;
         }
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEGraphic::setMarkupVisible (const QEGraphicNames::Markups markup, const bool isVisible)
{
   QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (markup, NULL);
   if (graphicMarkup && graphicMarkup->isInUse ()) {
      graphicMarkup->setVisible (isVisible);
   }
}

//------------------------------------------------------------------------------
//
bool QEGraphic::getMarkupVisible (const QEGraphicNames::Markups markup) const
{
   bool result = false;
   QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (markup, NULL);
   if (graphicMarkup) {
      result = graphicMarkup->isVisible ();
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEGraphic::setMarkupEnabled (const QEGraphicNames::Markups markup, const bool isEnabled)
{
   QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (markup, NULL);
   if (graphicMarkup && graphicMarkup->isInUse ()) {
      graphicMarkup->setEnabled (isEnabled);
   }
}

//------------------------------------------------------------------------------
//
bool QEGraphic::getMarkupEnabled (const QEGraphicNames::Markups markup) const
{
   bool result = false;
   QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (markup, NULL);
   if (graphicMarkup) {
      result = graphicMarkup->isEnabled ();
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEGraphic::setMarkupSelected (const QEGraphicNames::Markups markup, const bool selected)
{
   QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (markup, NULL);
   if (graphicMarkup && graphicMarkup->isInUse ()) {
      graphicMarkup->setSelected (selected);
   }
}

//------------------------------------------------------------------------------
//
bool QEGraphic::getMarkupIsSelected (const QEGraphicNames::Markups markup) const
{
   bool result = false;
   QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (markup, NULL);
   if (graphicMarkup) {
      result = graphicMarkup->isSelected ();
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEGraphic::setMarkupPosition (const QEGraphicNames::Markups markup, const QPointF& position)
{
   QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (markup, NULL);
   if (graphicMarkup && graphicMarkup->isInUse ()) {
      graphicMarkup->setCurrentPosition (position);
   }
}

//------------------------------------------------------------------------------
//
QPointF QEGraphic::getMarkupPosition (const QEGraphicNames::Markups markup) const
{
   QPointF result (0.0, 0.0);
   QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (markup, NULL);
   if (graphicMarkup && graphicMarkup->isInUse ()) {
      result = graphicMarkup->getCurrentPosition ();
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEGraphic::setMarkupData (const QEGraphicNames::Markups markup, const QVariant& data)
{
   QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (markup, NULL);
   if (graphicMarkup && graphicMarkup->isInUse ()) {
      graphicMarkup->setData (data);
   }
}

//------------------------------------------------------------------------------
//
QVariant QEGraphic::getMarkupData (const QEGraphicNames::Markups markup) const
{
   QVariant result = QVariant (QVariant::Invalid);
   QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (markup, NULL);
   if (graphicMarkup && graphicMarkup->isInUse ()) {
      result = graphicMarkup->getData ();
   }
   return result;
}

//------------------------------------------------------------------------------
// Depreciated
void QEGraphic::setCrosshairsVisible (const bool isVisible)
{
   this->setMarkupVisible (QEGraphicNames::CrossHair, isVisible);
}

//------------------------------------------------------------------------------
// Depreciated
void QEGraphic::setCrosshairsVisible (const bool isVisible, const QPointF& position)
{
   this->setMarkupVisible (QEGraphicNames::CrossHair, isVisible);
   this->setMarkupPosition (QEGraphicNames::CrossHair, position);
}

//------------------------------------------------------------------------------
// Depreciated
bool QEGraphic::getCrosshairsVisible () const
{
   return this->getMarkupVisible (QEGraphicNames::CrossHair);
}

//------------------------------------------------------------------------------
//
QPointF QEGraphic::pointToReal (const QPoint& pos, const QwtPlot::Axis selectedYAxis) const
{
   double x, y;

   x = this->xAxis->pointToReal (pos.x ());
   y = axisFromPosition(selectedYAxis)->pointToReal (pos.y ());

   return QPointF (x, y);
}

//------------------------------------------------------------------------------
//
QPointF QEGraphic::pointToReal (const QPointF& pos, const QwtPlot::Axis selectedYAxis) const
{
   double x, y;

   x = this->xAxis->pointToReal ((int) pos.x ());
   y = axisFromPosition(selectedYAxis)->pointToReal ((int) pos.y ());

   return QPointF (x, y);
}

//------------------------------------------------------------------------------
//
QPoint QEGraphic::realToPoint (const QPointF& pos, const QwtPlot::Axis selectedYAxis) const
{
   int x, y;

   x = this->xAxis->realToPoint (pos.x ());
   y = axisFromPosition(selectedYAxis)->realToPoint (pos.y ());

   return QPoint (x, y);
}

//------------------------------------------------------------------------------
//
void QEGraphic::releaseCurveList (CurveLists& list)
{
   for (int j = 0; j < list.size (); j++) {
      QwtPlotCurve* curve = list.value (j);
      if (curve) {
         curve->detach ();
         delete curve;
      }
   }

   // This clears the list of (now) dangaling curve references.
   //
   list.clear ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::releaseTextItemList (TextItemLists& list)
{
   list.clear ();
}

//------------------------------------------------------------------------------
// Releases all curves
//
void QEGraphic::releaseCurves ()
{
   this->releaseCurveList (this->userCurveList);
   this->releaseCurveList (this->markupCurveList);
   this->releaseTextItemList (this->textItemList);
}

//------------------------------------------------------------------------------
//
void QEGraphic::attchOwnCurve (QwtPlotCurve* curve)
{
   if (curve) {
      curve->attach (this->plot);
      this->userCurveList.append (curve);
   }
}

//------------------------------------------------------------------------------
//
QwtPlotCurve* QEGraphic::createCurveData (const QEGraphicNames::DoubleVector& xData,
                                          const QEGraphicNames::DoubleVector& yData,
                                          const QwtPlot::Axis selectedYAxis)
{
   const int curveLength = MIN (xData.size (), yData.size ());

   if (curveLength <= 1) return NULL;  // sainity check

   QwtPlotCurve* curve;
   QEGraphicNames::DoubleVector useXData;
   QEGraphicNames::DoubleVector useYData;
   curve = new QwtPlotCurve ();

   // Set curve propeties using current curve attributes.
   //
   curve->setPen (this->getCurvePen ());
   curve->setBrush (this->getCurveBrush ());

   curve->setRenderHint (this->getCurveRenderHint (),
                         this->getCurveRenderHintOn ());
   curve->setStyle (this->getCurveStyle ());
   curve->setYAxis(selectedYAxis);

   // Scale data as need be. Underlying Qwr widget does basic transformation,
   // but we need to do any required real world/log scaling.
   //
   useXData.clear();
   useYData.clear();
   for (int j = 0; j < curveLength; j++) {
      double x, y;

      x = this->xAxis->scaleValue (xData.value (j));
      useXData.append (x);

      y = axisFromPosition(selectedYAxis)->scaleValue (yData.value (j));
      useYData.append (y);
   }

#if QWT_VERSION >= 0x060000
   curve->setSamples (useXData, useYData);
#else
   curve->setData (useXData, useYData);
#endif

   // Attach new curve to the plot object.
   // By defaut curves are plotted on the yLeft y axis.
   //
   curve->attach (this->plot);

   return curve;
}

//------------------------------------------------------------------------------
//
void QEGraphic::plotCurveData (const QEGraphicNames::DoubleVector& xData,
                               const QEGraphicNames::DoubleVector& yData,
                               const QwtPlot::Axis yAxis)
{
   QwtPlotCurve* curve;
   curve = this->createCurveData (xData, yData, yAxis);
   if (curve) this->userCurveList.append (curve);
}

//------------------------------------------------------------------------------
//
void QEGraphic::plotMarkupCurveData (const QEGraphicNames::DoubleVector& xData,
                                     const QEGraphicNames::DoubleVector& yData)
{
   QwtPlotCurve* curve;
   curve = this->createCurveData (xData, yData);
   if (curve) this->markupCurveList.append (curve);
}

//------------------------------------------------------------------------------
//
void QEGraphic::plotMarkups ()
{
   const QEGraphicNames::MarkupLists keys = this->graphicMarkupsSet->keys ();
   for (int j = 0; j < keys.count (); j++) {
      QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (keys.value (j), NULL);
      if (graphicMarkup) {
         graphicMarkup->relocate ();  // specials to avoid off screen
         graphicMarkup->plot ();
      }
   }
}

//------------------------------------------------------------------------------
//
void QEGraphic::graphicReplot ()
{
   this->releaseCurveList (this->markupCurveList);
   this->plotMarkups ();
   this->plot->replot ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::drawText (const QPointF& posn,
                          const QString& text,
                          const QEGraphicNames::TextPositions option,
                          bool isCentred)
{
   TextItems item;

   // We store real-world postions.
   if (option == QEGraphicNames::RealWorldPosition) {
      item.position = posn;
   } else {
      item.position = this->pointToReal (posn);
   }
   item.text = text;
   item.isCentred = isCentred;
   item.font = this->textFont;   // use current text font
   item.pen = this->pen;         // use current curve pen

   this->textItemList.append (item);
}

//------------------------------------------------------------------------------
//
void QEGraphic::drawText (const QPoint& posn,
                          const QString& text,
                          const QEGraphicNames::TextPositions option,
                          bool isCentred)
{
   this->drawText (QPointF (posn), text, option, isCentred);
}

//------------------------------------------------------------------------------
//
void QEGraphic::drawTexts (QPainter* painter)
{
   int n = this->textItemList.count ();
   for (int j = 0; j < n; j++) {
      const TextItems item = this->textItemList.value (j);
      const int ps = QEScaling::scale (item.font.pointSize ());

      // Sewt the reqiuired font point size..
      //
      QFont font = item.font;
      font.setPointSize (ps);
      painter->setFont (font);

      // Do last minute conversion.
      //
      const QPoint pixelPos = this->realToPoint (item.position);

      int x = pixelPos.x ();
      int y = pixelPos.y ();

      if (item.isCentred) {
         QFontMetrics fm = painter->fontMetrics ();
         x -= fm.width (item.text)/2;
         y += (ps + 1)/2;
      }

      painter->setPen (item.pen);
      painter->drawText (x, y, item.text);
   }
}

//------------------------------------------------------------------------------
//
QEGraphic::Axis* QEGraphic::axisFromPosition (const QwtPlot::Axis axisPosition) const
{
   if (axisPosition == QwtPlot::yLeft) {
      return this->yAxisLeft;
   } else {
      return this->yAxisRight;
   }
}

//------------------------------------------------------------------------------
//
bool QEGraphic::rightButtonPressed () const
{
   return this->rightButtonIsPressed;
}

//------------------------------------------------------------------------------
//
bool QEGraphic::getSlopeIsDefined (QPointF& slope) const
{
   bool result;
   QEGraphicLineMarkup* markup =
         static_cast <QEGraphicLineMarkup*> (this->graphicMarkupsSet->value (QEGraphicNames::Line, NULL));

   if (markup && markup->isVisible ()) {
      slope = markup->getSlope ();
      result = true;
   } else {
      slope = QPointF (0.0, 0.0);
      result = false;
   }
   return result;
}

//------------------------------------------------------------------------------
//
QPointF QEGraphic::getRealMousePosition () const
{
   return this->realMousePosition;
}

//------------------------------------------------------------------------------
//
bool QEGraphic::globalPosIsOverCanvas (const QPoint& golbalPos) const
{
   QPoint canvasPos;
   QRect canvasGeo;

   canvasPos = this->plot->canvas()->mapFromGlobal (golbalPos);
   canvasGeo = this->plot->canvas()->geometry ();

   return (canvasPos.x () >= 0) && (canvasPos.x () < canvasGeo.width ()) &&
          (canvasPos.y () >= 0) && (canvasPos.y () < canvasGeo.height ());
}

//------------------------------------------------------------------------------
//
QPoint QEGraphic::pixelDistance (const QPointF& from, const QPointF& to) const
{
   QPoint pointFrom = this->realToPoint (from);
   QPoint pointTo = this->realToPoint (to);
   return pointTo - pointFrom;
}

//------------------------------------------------------------------------------
//
QPointF QEGraphic::realOffset (const QPoint& offset, const QwtPlot::Axis selectedYAxis) const
{
   QPoint origin = QPoint (0,0);
   QPointF realOrigin = this->pointToReal (origin, selectedYAxis);
   QPointF realOffset = this->pointToReal (offset, selectedYAxis);
   return realOffset - realOrigin;
}

//------------------------------------------------------------------------------
//
QPointF QEGraphic::realOffset (const QPointF& offset, const QwtPlot::Axis selectedYAxis) const
{
   QPointF origin = QPointF (0.0, 0.0);
   QPointF realOrigin = this->pointToReal (origin, selectedYAxis);
   QPointF realOffset = this->pointToReal (offset, selectedYAxis);
   return realOffset - realOrigin;
}

//------------------------------------------------------------------------------
//
void QEGraphic::setAxisEnableX (const bool enable)
{
   this->xAxis->setAxisEnable (enable);
}

//------------------------------------------------------------------------------
//
bool QEGraphic::getAxisEnableX () const
{
   return this->xAxis->getAxisEnable ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::setAxisEnableY (const bool enable, const QwtPlot::Axis selectedYAxis)
{
   this->axisFromPosition(selectedYAxis)->setAxisEnable (enable);
}

//------------------------------------------------------------------------------
//
bool QEGraphic::getAxisEnableY (const QwtPlot::Axis selectedYAxis) const
{
   return this->axisFromPosition(selectedYAxis)->getAxisEnable ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::setXScale (const double scale)
{
   this->xAxis->setScale (scale);
}

//------------------------------------------------------------------------------
//
double QEGraphic::getXScale () const
{
   return this->xAxis->getScale ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::setXOffset (const double offset)
{
   this->xAxis->setOffset (offset);
}

//------------------------------------------------------------------------------
//
double QEGraphic::getXOffset () const
{
   return this->xAxis->getOffset ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::setXLogarithmic (const bool isLog)
{
   this->xAxis->setLogarithmic (isLog);
}

//------------------------------------------------------------------------------
//
bool QEGraphic::getXLogarithmic () const
{
   return this->xAxis->getLogarithmic ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::setYScale (const double scale, const QwtPlot::Axis selectedYAxis)
{
   this->axisFromPosition(selectedYAxis)->setScale (scale);
}

//------------------------------------------------------------------------------
//
double QEGraphic::getYScale (const QwtPlot::Axis selectedYAxis) const
{
   return this->axisFromPosition(selectedYAxis)->getScale ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::setYOffset (const double offset, const QwtPlot::Axis selectedYAxis)
{
   this->axisFromPosition(selectedYAxis)->setOffset (offset);
}

//------------------------------------------------------------------------------
//
double QEGraphic::getYOffset (const QwtPlot::Axis selectedYAxis) const
{
   return this->axisFromPosition(selectedYAxis)->getOffset ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::setYLogarithmic (const bool isLog, const QwtPlot::Axis selectedYAxis)
{
   this->axisFromPosition(selectedYAxis)->setLogarithmic (isLog);
}

//------------------------------------------------------------------------------
//
bool QEGraphic::getYLogarithmic (const QwtPlot::Axis selectedYAxis) const
{
   return this->axisFromPosition (selectedYAxis)->getLogarithmic ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::setYColor (const QColor color, const QwtPlot::Axis selectedYAxis)
{
   this->axisFromPosition (selectedYAxis)->setAxisColor (color);
}

//------------------------------------------------------------------------------
//
QEGraphicMarkup* QEGraphic::mouseIsOverMarkup ()
{
   QEGraphicMarkup* search;
   int minDistance;

   search = NULL;
   minDistance = 100000;  // some unfeasible large distance. A real distance much smaller.

   const QEGraphicNames::MarkupLists keys = this->graphicMarkupsSet->keys ();
   for (int j = 0; j < keys.count (); j++) {
      QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (keys.value (j), NULL);
      if (graphicMarkup) {
         int dist;
         if (graphicMarkup->isOver (this->realMousePosition, dist)) {
            // Note: <=  operator. All things being equal this means:
            // Last in, best dressed.  That is essentially the same as the
            // plotMarkups, i.e. we find the markup the user can see.
            //
            if (dist <= minDistance) {
               minDistance = dist;
               search = graphicMarkup;
            }
         }
      }
   }

   return search;
}

//------------------------------------------------------------------------------
//
void QEGraphic::canvasMousePress (QMouseEvent* mouseEvent)
{
   Qt::MouseButton button;
   QEGraphicMarkup* search;

   button = mouseEvent->button ();
   this->realMousePosition = this->pointToReal (mouseEvent->pos ());

   search = NULL;

   // We can always "find" the Area and Line markups.
   //
   if (button == Qt::LeftButton) {
      search = this->graphicMarkupsSet->value (QEGraphicNames::Area, NULL);
   } else if (button == MIDDLE_BUTTON) {
      search = this->graphicMarkupsSet->value (QEGraphicNames::Line, NULL);
   }

   // Is press over/closer an existing/visible markup?
   // Iff we found something, then replace search.
   //
   QEGraphicMarkup* target = this->mouseIsOverMarkup ();
   if (target) {
      // Don't allow box to override line.
      bool lineAndBox;
      lineAndBox = (search && search->getMarkup() == QEGraphicNames::Line) &&
                   (target && target->getMarkup() == QEGraphicNames::Box);

      if (!lineAndBox) {
         search = target;
      }
   }

   // Mark this markup as selected (if markup allows it)
   //
   if (search) {
      search->setSelected (true);
   }

   const QEGraphicNames::MarkupLists keys = this->graphicMarkupsSet->keys ();
   for (int j = 0; j < keys.count (); j++) {
      QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (keys.value (j), NULL);
      if (graphicMarkup && graphicMarkup->isSelected ()) {
         graphicMarkup->mousePress (this->realMousePosition, button);
      }
   }

   if (button == Qt::RightButton) {
      this->rightButtonIsPressed = true;
   }

   // Treat as a mouse move as well.
   //
   this->canvasMouseMove (mouseEvent, true);
}

//------------------------------------------------------------------------------
//
void QEGraphic::canvasMouseRelease (QMouseEvent* mouseEvent)
{
   Qt::MouseButton button;

   button = mouseEvent->button ();
   this->realMousePosition = this->pointToReal (mouseEvent->pos ());

   const QEGraphicNames::MarkupLists keys = this->graphicMarkupsSet->keys ();
   for (int j = 0; j < keys.count (); j++) {
      QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (keys.value (j), NULL);
      if (graphicMarkup && graphicMarkup->isSelected ()) {
         graphicMarkup->mouseRelease (this->realMousePosition, button);
         this->plot->canvas()->setCursor (Qt::CrossCursor);   // default cursor
      }
   }

   if (button == Qt::RightButton) {
      this->rightButtonIsPressed = false;
   }

   // Treat as a mouse move as well.
   this->canvasMouseMove (mouseEvent, true);
}

//------------------------------------------------------------------------------
//
void QEGraphic::canvasMouseMove (QMouseEvent* mouseEvent, const bool isButtonAction)
{
   bool replotIsRequired;

   this->realMousePosition = this->pointToReal (mouseEvent->pos ());

   replotIsRequired = false;
   QEGraphicNames::MarkupLists keys = this->graphicMarkupsSet->keys ();
   for (int j = 0; j < keys.count (); j++) {
      QEGraphicMarkup* graphicMarkup = this->graphicMarkupsSet->value (keys.value (j), NULL);
      if (graphicMarkup && graphicMarkup->isSelected ()) {
         graphicMarkup->mouseMove (this->realMousePosition);
         // A selected item will need replotted.
         //
         replotIsRequired = true;
      }
   }

   if (replotIsRequired | isButtonAction ) {
      this->graphicReplot ();
   }

   if (!replotIsRequired) {
      // Nothing selected. Is cursor over markup
      //
      QEGraphicMarkup* search;
      QCursor cursor;

      search = this->mouseIsOverMarkup ();
      if (search) {
         cursor = search->getCursor ();
      } else {
         cursor = Qt::CrossCursor;
      }
      this->plot->canvas()->setCursor (cursor);
   }

   emit mouseMove (this->realMousePosition);
}

//------------------------------------------------------------------------------
//
bool QEGraphic::eventFilter (QObject* obj, QEvent* event)
{
   const QEvent::Type type = event->type ();
   QMouseEvent* mouseEvent = NULL;
   QWheelEvent* wheelEvent = NULL;

   switch (type) {

      case QEvent::MouseButtonPress:
         mouseEvent = static_cast<QMouseEvent *> (event);
         if (obj == this->plot->canvas ()) {
            this->canvasMousePress (mouseEvent);
            return true;  // we have handled this mouse press
         }
         break;

      case QEvent::MouseButtonRelease:
         mouseEvent = static_cast<QMouseEvent *> (event);
         if (obj == this->plot->canvas ()) {
            this->canvasMouseRelease (mouseEvent);
            return true;  // we have handled this mouse press
         }
         break;

      case QEvent::MouseMove:
         mouseEvent = static_cast<QMouseEvent *> (event);
         if (obj == this->plot->canvas ()) {
            this->canvasMouseMove (mouseEvent, false);
            return true;  // we have handled move nouse event
         }
         break;

      case QEvent::Wheel:
         wheelEvent = static_cast<QWheelEvent *> (event);
         if (obj == this->plot->canvas ()) {

            emit wheelRotate (this->realMousePosition,
                              wheelEvent->delta ());

            return true;  // we have handled wheel event
         }
         break;

      case QEvent::Resize:
         if (obj == this->plot->canvas ()) {
            this->graphicReplot ();
         }
         break;

      default:
         break;
   }

   return false;
}

//------------------------------------------------------------------------------
//
void QEGraphic::setXRange (const double min, const double max,
                           const QEGraphicNames::AxisMajorIntervalModes mode,
                           const double value,
                           const bool immediate)
{
   this->xAxis->setRange (min, max, mode, value, immediate);
}


//------------------------------------------------------------------------------
//
void QEGraphic::getXRange (double& min, double& max) const
{
   this->xAxis->getRange (min, max);
}

//------------------------------------------------------------------------------
//
void QEGraphic::setYRange (const double min, const double max,
                           const QEGraphicNames::AxisMajorIntervalModes mode,
                           const double value,
                           const bool immediate, const QwtPlot::Axis selectedYAxis)
{
   this->axisFromPosition (selectedYAxis)->setRange (min, max, mode, value, immediate);
}

//------------------------------------------------------------------------------
//
void QEGraphic::getYRange (double& min, double& max, const QwtPlot::Axis selectedYAxis) const
{
   this->axisFromPosition(selectedYAxis)->getRange (min, max);
}

//------------------------------------------------------------------------------
//
void QEGraphic::setAxisTitle (const QwtPlot::Axis selectedAxis, const QString& title)
{
   this->plot->setAxisTitle (selectedAxis, title);
}

//------------------------------------------------------------------------------
//
QString QEGraphic::getAxisTitle (const QwtPlot::Axis selectedAxis) const
{
   return this->plot->axisTitle (selectedAxis).text();
}

//------------------------------------------------------------------------------
//
void QEGraphic::setAxisAutoScale (const QwtPlot::Axis selectedAxis, const bool enabled)
{
#if QWT_VERSION >= 0x060100
   this->plot->setAxisAutoScale (selectedAxis, enabled);
#else
   if (enabled)
      this->plot->setAxisAutoScale (selectedAxis);
#endif
}

//------------------------------------------------------------------------------
//
bool QEGraphic::getAxisAutoScale (const QwtPlot::Axis selectedAxis) const
{
   return this->plot->axisAutoScale (selectedAxis);
}

//------------------------------------------------------------------------------
//
void QEGraphic::replot ()
{
   // User artefacts already plotted - now do markup plots.
   //
   this->plotMarkups ();
   this->plot->replot ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::setCurvePen (const QPen& penIn)
{
   this->pen = penIn;
}

//------------------------------------------------------------------------------
//
QPen QEGraphic::getCurvePen () const {
   return this->pen;
}

//------------------------------------------------------------------------------
//
void QEGraphic::setCurveBrush (const QBrush& brushIn)
{
   this->brush = brushIn;
}

//------------------------------------------------------------------------------
//
QBrush QEGraphic::getCurveBrush () const
{
   return this->brush;
}

//------------------------------------------------------------------------------
//
void QEGraphic::setTextFont (const QFont& fontIn)
{
   this->textFont = fontIn;
}

//------------------------------------------------------------------------------
//
QFont QEGraphic::getTextFont () const
{
   return this->textFont;
}

//------------------------------------------------------------------------------
//
void QEGraphic::setTextPointSize (const int pointSize)
{
   this->textFont.setPointSize (pointSize);
}

//------------------------------------------------------------------------------
//
int QEGraphic::getTextPointSize () const
{
   return this->textFont.pointSize ();
}

//------------------------------------------------------------------------------
//
void QEGraphic::setCurveRenderHint (const QwtPlotItem::RenderHint hintIn,
                                    const bool on)
{
   this->hint = hintIn;
   this->hintOn = on;
}

//------------------------------------------------------------------------------
//
QwtPlotItem::RenderHint QEGraphic::getCurveRenderHint () const
{
   return this->hint;
}

//------------------------------------------------------------------------------
//
bool QEGraphic::getCurveRenderHintOn () const
{
    return this->hintOn;
}

//------------------------------------------------------------------------------
//
void QEGraphic::setCurveStyle (const QwtPlotCurve::CurveStyle styleIn)
{
   this->style = styleIn;
}

//------------------------------------------------------------------------------
//
QwtPlotCurve::CurveStyle QEGraphic::getCurveStyle ()
{
   return this->style;
}

//------------------------------------------------------------------------------
// static
//------------------------------------------------------------------------------
//
QEDisplayRanges QEGraphic::calcTransitionPoint (const QEDisplayRanges& start,
                                                const QEDisplayRanges& finish,
                                                const int step)
{
   QEDisplayRanges result;
   double minimum;
   double maximum;

   if (step <= 0) {
      result = finish;
   } else if (step >= NUMBER_TRANISTION_STEPS) {
      result = start;
   } else  {

      // Truely in transition - perform a linear interpolation.
      //
      const double s = double (step) / (double) NUMBER_TRANISTION_STEPS;
      const double f = 1.0 - s;

      minimum = (s * start.getMinimum ()) + (f * finish.getMinimum ());
      maximum = (s * start.getMaximum ()) + (f * finish.getMaximum ());

      result.setRange (minimum, maximum);
   }
   return result;
}

// end
