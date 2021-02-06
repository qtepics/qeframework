/*  QEWaterfall.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2020-2021 Australian Synchrotron
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
 *    andrews@ansto.gov.au
 */

#include "QEWaterfall.h"
#include <math.h>
#include <QBrush>
#include <QDebug>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QPolygonF>
#include <QECommon.h>
#include <QEDisplayRanges.h>

#define DEBUG qDebug () << "QEWaterfall" << __LINE__ << __FUNCTION__ << "  "

static const int axisIndents      = 20;
static const int nearLimit        = 10;
static const int nearLimitSquared = nearLimit * nearLimit;

//------------------------------------------------------------------------------
//
QEWaterfall:: QEWaterfall (QWidget* parent) :
   QEAbstract2DData (parent)
{
   this->commonSetup();
}

//------------------------------------------------------------------------------
//
QEWaterfall::QEWaterfall (const QString& dataVariableName,
                          QWidget* parent):
   QEAbstract2DData (dataVariableName, parent)
{
   this->commonSetup();
}

//------------------------------------------------------------------------------
//
QEWaterfall::QEWaterfall (const QString& dataVariableName,
                          const QString& widthVariableName,
                          QWidget* parent) :
   QEAbstract2DData (dataVariableName, widthVariableName, parent)
{
   this->commonSetup();
}

//------------------------------------------------------------------------------
//
void QEWaterfall::commonSetup ()
{
   QEWaterfall::PosToSrcMap::clear (this);   // should not be necessary

   // Create internal widget.
   //
   this->plotArea = new QWidget (NULL);
   this->layout = new QHBoxLayout (this);
   this->layout->setMargin (4);    // make a property?
   this->layout->setSpacing (0);
   this->layout->addWidget (this->plotArea);
   this->plotArea->setMouseTracking (true);
   this->plotArea->installEventFilter (this);

   // Set default properties.
   //
   this->setMinimumWidth  (200);
   this->setMinimumHeight (100);

   this->mAngle = 30;
   this->mTraceGap = 5;
   this->mTraceColour = QColor ("#0000c0");
   this->mMutableHue = false;
   this->mTraceWidth = 1;
   this->mBackgroundColour = QColor ("#ffffff");


   // NOTE: The axis objects are used as painters, not as widgets.
   //
   this->xAxis = new QEAxisPainter (NULL);
   this->yAxis = new QEAxisPainter (NULL);

   this->xAxis->setPrecision (0);
   this->xAxis->setMinorInterval (4.0);
   this->xAxis->setHasAxisLine (true);
   this->xAxis->setOrientation (QEAxisPainter::Left_To_Right);
   this->xAxis->setIndent (axisIndents, axisIndents);
   this->xAxis->setGap (0);

   this->yAxis->setPrecision (1);
   this->yAxis->setMinorInterval (1.0);
   this->yAxis->setHasAxisLine (true);
   this->yAxis->setOrientation (QEAxisPainter::Bottom_To_Top);
   this->yAxis->setIndent (axisIndents, axisIndents);
   this->yAxis->setGap (0);
}

//------------------------------------------------------------------------------
//
QEWaterfall::~QEWaterfall ()
{
   if (this->xAxis) delete this->xAxis;
   if (this->yAxis) delete this->yAxis;
   QEWaterfall::PosToSrcMap::clear (this);
}

//------------------------------------------------------------------------------
//
QSize QEWaterfall::sizeHint () const
{
   return QSize (200, 100);
}

//------------------------------------------------------------------------------
//
void QEWaterfall::updateDataVisulation ()
{
   this->plotArea->update ();  // trigger a paint event
}

//------------------------------------------------------------------------------
//
bool QEWaterfall::eventFilter (QObject* watched, QEvent* event)
{
   const QEvent::Type type = event->type ();

   bool result = false;

   switch (type) {
      case QEvent::MouseMove:
         if (watched == this->plotArea) {
            const QMouseEvent* mouseEvent = static_cast<QMouseEvent *> (event);
            const QPoint pos = mouseEvent->pos ();
            this->waterfallMouseMove (pos);
            result = true;   // event has been handled.
         }
         break;

      case QEvent::Paint:
         if (watched == this->plotArea) {
            this->paintWaterfall();
            result = true;  // event handled.
         }
         break;

      default:
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEWaterfall::paintWaterfall ()
{
   // If background is dark, choose white as oen colour.
   //
   const QColor penColour = QEUtilities::fontColour (this->mBackgroundColour);

   this->xAxis->setPenColour (penColour);
   this->yAxis->setPenColour (penColour);

   double min;
   double max;
   this->getScaleModeMinMaxValues (min, max);

   int numberCols;
   int numberRows;
   this->getNumberRowsAndCols (numberRows, numberCols);

   // Set up painter.
   //
   QPainter painter (this->plotArea);
   QRect rect = this->plotArea->geometry();

   // We want rect in the plot area, not plot area in QESpectrogram.
   //
   rect.moveTo (0, 0);

   // Do the geometry.
   //
   const double alpha = this->mAngle * TAU / 360.0;
   const double sinAlpha = sin (alpha);
   const double cosAlpha = cos (alpha);

   // Separation between plot lines.
   //
   const double ds = this->mTraceGap;

   const double dsx = ds * sinAlpha;      // x separation
   const double dsy = ds * cosAlpha;      // y separation

   // Vertical split into four regions sized ay, by, cy and dy (top to bottom)
   //
   const double ay = 20.0;                // gap at top
   const double by = numberRows * dsy;    // slope bit
   const double dy = 36.0;                // room for axis
   const double cy = rect.height() - (ay + by + dy);

   // Horizontal split into four regions sized ax, bx, cx and dx (right to left)
   //
   const double ax = 20.0;                // gap at left
   const double bx = numberRows * dsx;    // slope bit
   const double dx = 52.0;                // room for axis
   const double cx = rect.width() - (ax + bx + dx);

   // Now we can calculate scalin4g for form y = m.x + c
   //
   const double yScale = -cy / (max - min);      // -ve because +y is downwards.
   const double yOffset = (ay + by) - yScale*max;

   const double xScale = +cx / (numberCols - 1.0);
   const double xOffset = dx - xScale * 0;

   QPointF rowDelta = QPointF (+dsx, -dsy);      // y -ve because +y is downwards.

   // Rear points on the bounding cuboid.
   //
   //        3----------------------------------5
   //       /|                                  |
   //      / |                                  |
   //     /  |                                  |
   //    2   |                                  |
   //    |   4----------------------------------6
   //    |  /                                  /
   //    | /                                  /
   //    |/                                  /
   //    1----------------------------------7
   //
   const QPointF k1 = QPointF (dx, ay + by + cy);
   const QPointF k2 = QPointF (dx, ay + by);
   const QPointF k3 = QPointF (dx + bx, ay);
   const QPointF k4 = QPointF (dx + bx, ay + cy);
   const QPointF k5 = QPointF (dx + cx + bx, ay);
   const QPointF k6 = QPointF (dx + cx + bx, ay + cy);
   const QPointF k7 = QPointF (dx + cx, ay + by + cy);

   // Start painting and drawing.
   //
   // Fill background.
   //
   painter.fillRect (rect, this->mBackgroundColour);

   QPen pen;
   pen.setStyle (Qt::SolidLine);
   pen.setWidth (1);
   pen.setColor (penColour);
   painter.setPen (pen);

   // Draw rear edges of bounding cuboid.
   //
   // painter.drawLine (k1, k2);  /// axis
   painter.drawLine (k2, k3);
   painter.drawLine (k1, k4);
   painter.drawLine (k3, k4);
   painter.drawLine (k3, k5);
   painter.drawLine (k4, k6);
   // painter.drawLine (k1, k7);  /// axis
   painter.drawLine (k5, k6);
   painter.drawLine (k6, k7);

   const int pw = (this->mTraceWidth >= 1)
         ? this->mTraceWidth
         : MAX(1, int ((ds + 4) / 8));

   pen.setWidth (pw);
   pen.setColor (this->mTraceColour);
   pen.setStyle (Qt::SolidLine);

   QBrush brush;
   brush.setStyle (Qt::SolidPattern);
   brush.setColor (this->mBackgroundColour);

   QEWaterfall::PosToSrcMap::clear (this);

   for (int row = 0; row < numberRows; row++) {
      // row 0 is oldest row, row numberRows - 1 is latest row.
      //
      const int coRow = numberRows - 1 - row;

      if (this->mMutableHue) {
         int h, s, l;
         this->mTraceColour.getHsl (&h, &s, &l);

         // Modify hue - we add a constant 36000 (a somewhat arbitary offset),
         // becaue % is a remainder operator, not a modulo operator.
         // For 2D data use a fixed count, for 1D use update cout so that
         // same hue assoicated with the same data set.
         //
         int hueOffset = (this->getDataFormat () == array1D) ? this->getUpdateCount() : 0;
         h += (12 * (36000 + hueOffset - coRow)) % 360;

         QColor c;
         c.setHsl (h, s, l);
         pen.setColor (c);
      } else {
         pen.setColor (this->mTraceColour);
      }

      QPolygonF line;
      line.reserve (numberCols);

      // Calc the offset applies to each row.
      //
      const QPointF offset = (coRow + 0.5) * rowDelta;

      for (int col = 0; col < numberCols; col++) {

         double value = this->getValue (row, col, min);

         // col is the x coordinate, value is the y coordinate.
         //
         QPointF item = QPointF (col   * xScale + xOffset,
                                 value * yScale + yOffset);
         item += offset;
         line.append (item);

         // Insert plot info into look up structure.
         //
         QEWaterfall::PosToSrcMap lookUp (int (item.x()), int (item.y()), row, col);
         lookUp.insert (this);
      }

      painter.setPen (pen);
      painter.setBrush (brush);
      painter.drawPolyline (line);
   }

   // Re-draw the bounding edges that can get "zapped".
   //
   pen.setWidth (1);
   pen.setColor (penColour);
   painter.setPen (pen);
   painter.drawLine (k5, k6);
   painter.drawLine (k6, k7);

   // Now do the axis.
   //
   double minOut, maxOut, majorOut;

   QEDisplayRanges xRange (0.0, numberCols - 1);
   xRange.adjustMinMax (5, true, minOut, maxOut, majorOut);
   this->xAxis->setMinimum (0);
   this->xAxis->setMaximum (numberCols);
   this->xAxis->setMinorInterval (majorOut / 5.0); // Default major minor ratio is 5

   QRect xAxisArea (dx -   axisIndents, ay + by + cy,
                    cx + 2*axisIndents, dy);
   this->xAxis->paint (painter, 8, xAxisArea);

   QEDisplayRanges yRange (min, max);
   yRange.adjustMinMax (5, true, minOut, maxOut, majorOut);
   this->yAxis->setMinimum (minOut);
   this->yAxis->setMaximum (maxOut);
   this->yAxis->setMinorInterval (majorOut / 5.0); // Default major minor ratio is 5

   QRect yAxisArea (0,  ay + by - axisIndents,
                    dx, cy      + 2*axisIndents);
   this->yAxis->paint (painter, 8, yAxisArea);
}

//------------------------------------------------------------------------------
//
void QEWaterfall::waterfallMouseMove (const QPoint& pos)
{
   int row;
   int col;

   // Convert the mosue postion into a data element index - if we can.
   //
   bool found = QEWaterfall::PosToSrcMap::findNearest (this, pos.x(), pos.y(), row, col);
   if (found) {
      this->setMouseOverElement (row, col);
   } else {
      this->setMouseOverElement (-1, -1);
   }
}

//==============================================================================
// Property access functions
//==============================================================================
//
void QEWaterfall::setAngle (const int angleIn)
{
   this->mAngle = LIMIT (angleIn, 0, 90);
   this->plotArea->update ();
}

//------------------------------------------------------------------------------
//
int QEWaterfall::getAngle () const
{
   return this->mAngle;
}

//------------------------------------------------------------------------------
//
void QEWaterfall::setTraceGap (const int traceGapIn)
{
   this->mTraceGap = LIMIT (traceGapIn, 1, 40);
   this->plotArea->update ();
}

//------------------------------------------------------------------------------
//
int QEWaterfall::getTraceGap () const
{
   return this->mTraceGap;
}

//------------------------------------------------------------------------------
//
void QEWaterfall::setTraceWidth (const int traceWidthIn)
{
   this->mTraceWidth = LIMIT (traceWidthIn, 0, 10);  // 10 arbitrary but sufficient
   this->plotArea->update ();
}

//------------------------------------------------------------------------------
//
int QEWaterfall::getTraceWidth () const
{
   return this->mTraceWidth;
}

//------------------------------------------------------------------------------
//
void QEWaterfall::setTraceColour (const QColor& traceColourIn)
{
   this->mTraceColour = traceColourIn;
   this->plotArea->update ();
}

//------------------------------------------------------------------------------
//
QColor QEWaterfall::getTraceColour () const
{
   return this->mTraceColour;
}

//------------------------------------------------------------------------------
//
void QEWaterfall::setBackgroundColour (const QColor& traceBackgroundIn)
{
   this->mBackgroundColour = traceBackgroundIn;
   this->plotArea->update ();
}

//------------------------------------------------------------------------------
//
QColor QEWaterfall::getBackgroundColour () const
{
   return this->mBackgroundColour;
}

//------------------------------------------------------------------------------
//
void QEWaterfall::setMutableHue (const bool mutableHueIn)
{
   this->mMutableHue = mutableHueIn;
   this->plotArea->update ();
}

//------------------------------------------------------------------------------
//
bool QEWaterfall::getMutableHue () const
{
   return this->mMutableHue;
}

//==============================================================================
// PosToSrcMap inner class methods.
//==============================================================================
//
QEWaterfall::PosToSrcMap::PosToSrcMap ()
{
   this->posX = 0;
   this->posY = 0;
   this->dataRow = -1;
   this->dataCol = -1;
}

//------------------------------------------------------------------------------
//
QEWaterfall::PosToSrcMap::PosToSrcMap (const int posXIn, const int posYIn,
                           const int dataRowIn, const int dataColIn)
{
   this->posX = posXIn;
   this->posY = posYIn;
   this->dataRow = dataRowIn;
   this->dataCol = dataColIn;
}

//------------------------------------------------------------------------------
//
QEWaterfall::PosToSrcMap::~PosToSrcMap () { }

//------------------------------------------------------------------------------
// static
void QEWaterfall::PosToSrcMap::clear (QEWaterfall* owner)
{
   if (!owner) return; // sanity check

   for (int i = 0; i < ARRAY_LENGTH (owner->mapArrays); i++) {
      for (int j = 0; j < ARRAY_LENGTH (owner->mapArrays[i]); j++) {
         owner->mapArrays[i][j].clear();
      }
   }
}

//------------------------------------------------------------------------------
//
void QEWaterfall::PosToSrcMap::insert (QEWaterfall* owner) const
{
   if (!owner) return; // sanity check

   /// Keep this code snippet alligned with code out of PosToSrcMap::findNearest
   ///
   // If outside of the onewr display area - then forget it.
   //
   QRect plotRect = owner->plotArea->geometry();

   if ((this->posX < 0) || (this->posX) >= plotRect.width())  return;  // off-screen.
   if ((this->posY < 0) || (this->posY) >= plotRect.height()) return;  // off-screen.

   const int cellWidth  = (plotRect.width()  + NumberListCols - 1) / NumberListCols; // round up
   const int cellHeight = (plotRect.height() + NumberListRows - 1) / NumberListRows; // round up

   const int ci = this->posX / cellWidth;     // column index
   const int ri = this->posY / cellHeight;    // row index
   ///
   /// end of code snippet

   // Add 'this' to the list to be searched.
   //
   owner->mapArrays[ri][ci].append(*this);

   // Are we close to any of the neigbouring cells - examine the remainders.
   //
   const int cr = this->posX % cellWidth;     // column remainder
   const int rr = this->posY % cellHeight;    // row remainder

   // Dermine which, if any, edges the point is near provided an adjacent cell exists.
   //
   const bool nearLeft   = (cr < nearLimit)               && (ci >= 1);
   const bool nearRight  = (cr >= cellWidth  - nearLimit) && (ci < NumberListCols - 1);
   const bool nearTop    = (rr < nearLimit)               && (ri >= 1);
   const bool nearBottom = (rr >= cellHeight - nearLimit) && (ri < NumberListRows - 1);

   // Add 'this' to the search lists of the adjacent cells if needs be.
   //
   // Apply edge checks.
   //
   if (nearLeft)   owner->mapArrays[ri][ci-1].append(*this);
   if (nearRight)  owner->mapArrays[ri][ci+1].append(*this);
   if (nearTop)    owner->mapArrays[ri-1][ci].append(*this);
   if (nearBottom) owner->mapArrays[ri+1][ci].append(*this);

   // And do corner checks as well.
   //
   if (nearTop    & nearLeft)  owner->mapArrays[ri-1][ci-1].append(*this);
   if (nearTop    & nearRight) owner->mapArrays[ri-1][ci+1].append(*this);
   if (nearBottom & nearLeft)  owner->mapArrays[ri+1][ci-1].append(*this);
   if (nearBottom & nearRight) owner->mapArrays[ri+1][ci+1].append(*this);
}

//------------------------------------------------------------------------------
// static
bool QEWaterfall::PosToSrcMap::findNearest (QEWaterfall* owner,
                                            const int posX, const int posY,
                                            int& dataRow, int& dataCol)
{
   if (!owner) return false; // sanity check

   /// Keep this code snippet alligned with code out of PosToSrcMap::insert
   ///
   // If outside of the onewr display area - then forget it.
   //
   QRect plotRect = owner->plotArea->geometry();

   if ((posY < 0) || (posY) >= plotRect.height()) return false;  // off-screen.
   if ((posX < 0) || (posX) >= plotRect.width()) return false;   // off-screen.

   const int cellWidth  = (plotRect.width()  + NumberListCols - 1) / NumberListCols; // round up
   const int cellHeight = (plotRect.height() + NumberListRows - 1) / NumberListRows; // round up

   const int ci = posX / cellWidth;        // column index
   const int ri = posY / cellHeight;       // row index
   ///
   /// end of code snippet

   PosToSrcMapLists* map = &owner->mapArrays[ri][ci];

   bool result = false;
   int minDistanceSquared = 1e9;   // any large positive number will do.
   int bestk = 0;
   for (int k = 0; k < map->count(); k++) {
      const PosToSrcMap self = map->value (k);

      const int dx = posX - self.posX;
      const int dy = posY - self.posY;
      const int distanceSquared = dx*dx + dy*dy;

      // Are we within the limit and closer than any previously found point?
      //
      if ((distanceSquared <= nearLimitSquared) &&
          (distanceSquared < minDistanceSquared)) {
         // Yes - save new min distance and index within the list.
         //
         minDistanceSquared = distanceSquared;
         bestk = k;
         result = true;  // we have found a point.
      }
   }

   if (result) {
      const PosToSrcMap self = map->value (bestk);
      dataRow = self.dataRow;
      dataCol = self.dataCol;
   }

   return result;
}

// end
