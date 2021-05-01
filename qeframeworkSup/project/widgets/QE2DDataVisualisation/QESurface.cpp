/*  QESurface.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2017-2021 Australian Synchrotron.
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

#include "QESurface.h"

#include <math.h>
#include <stdlib.h>
#include <QColor>
#include <QCursor>
#include <QDebug>
#include <QPainter>
#include <QTimer>
#include <QECommon.h>

#define DEBUG qDebug () << "QESurface" << __LINE__ << __FUNCTION__ << "  "

#define DEG2RAD(deg)    (TAU * (deg) / 360.0)

static const double paintScale = 0.01;

//------------------------------------------------------------------------------
//
QESurface:: QESurface (QWidget* parent) :
   QEAbstract2DData (parent)
{
   this->commonSetup();
}

//------------------------------------------------------------------------------
//
QESurface::QESurface (const QString& dataVariableName,
                          QWidget* parent):
   QEAbstract2DData (dataVariableName, parent)
{
   this->commonSetup();
}

//------------------------------------------------------------------------------
//
QESurface::QESurface (const QString& dataVariableName,
                          const QString& widthVariableName,
                          QWidget* parent) :
   QEAbstract2DData (dataVariableName, widthVariableName, parent)
{
   this->commonSetup();
}

//------------------------------------------------------------------------------
//
void QESurface::commonSetup ()
{
   // Set default values.
   //
   QFont font = this->font();
   font.setFamily ("Monospace");
   this->setFont (font);

   this->mShowGrid = false;
   this->mGridStyle = Qt::SolidLine;
   this->mAxisColour = QColor (120,120,120);
   this->mGridColour = QColor (0,0,0);
   this->mShowSurface = true;
   this->mSurfaceStyle = Qt::SolidPattern;
   this->mTheta = -30.0;
   this->mPhi = +72.0;
   this->mXScale = +100.0;
   this->mYScale = +100.0;
   this->mZScale = +100.0;
   this->mZoom = 1000.0;
   this->mClampData = false;
   this->mShowScaling = false;
   this->mTestDataEnabled = false;
   this->numberCols = 0;
   this->numberRows = 0;

   this->activeMarker = mkNone;
   this->setCursor (QCursor (Qt::CrossCursor));

   this->mouseIsDown = false;
   this->setMouseTracking (true);

   this->setMinimumSize (320, 320);
}

//---------------------------------------------------------------------------------
//
QESurface::~QESurface () { }

//---------------------------------------------------------------------------------
//
QSize QESurface::sizeHint () const
{
   return QSize (320, 320);
}

//------------------------------------------------------------------------------
//
void QESurface::paintAxis ()
{
   QPainter painter (this);
   QPen pen;
   QBrush brush;
}

//------------------------------------------------------------------------------
//
void QESurface::paintEvent (QPaintEvent* event)
{
   // static int dd = 1000000;

   QFrame::paintEvent (event);   // call parent function first

   // Both numbers have been sanitised <= max number row/cols
   //
   const int nr = this->numberRows;
   const int nc = this->numberCols;
   if ((nr < 1) || (nc < 1)) return;

   // DEBUG << "nr" << nr << "nc" << nc;

   QPainter painter (this);
   QPen pen;
   QBrush brush;

   Matrix rt;
   Matrix rp;
   Matrix rotMat;

   // Form theta and phi rotation matricies
   //
   const double cost = cos (DEG2RAD (this->mTheta));
   const double sint = sin (DEG2RAD (this->mTheta));
   const double cosp = cos (DEG2RAD (this->mPhi));
   const double sinp = sin (DEG2RAD (this->mPhi));

   rt [0][0] = cost;  rt [0][1] = sint;  rt [0][2] = 0.0;
   rt [1][0] = -sint; rt [1][1] = cost;  rt [1][2] = 0.0;
   rt [2][0] = 0.0;   rt [2][1] = 0.0;   rt [2][2] = 1.0;

   rp [0][0] = 1.0;   rp [0][1] = 0.0;   rp [0][2] = 0.0;
   rp [1][0] = 0.0;   rp [1][1] = cosp;  rp [1][2] = -sinp;
   rp [2][0] = 0.0;   rp [2][1] = sinp;  rp [2][2] = cosp;

   // Combine into a single matrix m.
   //
   this->mmult (rotMat, rp, rt);  // rotMat = rp * rt

   const double z0 = this->mZoom;    // controls the amount of perspective

   const double xScreenOrigin = this->width()/2.0;
   const double yScreenOrigin = this->height()/2.0;

   // data origin:
   //   x = +(nc -1)/2
   //   y = +(nr -1)/2
   //   z = (max - min) / 2

   const double xDataOrigin = (nc-1)/2.0;
   const double yDataOrigin = (nr-1)/2.0;

   double min;
   double max;
   this->getScaleModeMinMaxValues (min, max);

   const double zDataOrigin = (min + max) / 2.0;
   const double zDataHaSpan = (max - min) / 2.0;

   // Calulate the working z scale nbase on user scale selection, selected min/max
   // and widget height. The 0.4 is a bit arbitary and found empirically.
   //
   const double zSpan = MAX(max - min, 1.0);  // avod the divide by zero
   const double zScale = this->mZScale*paintScale * this->height () * 0.4 / zSpan;

   // r, c and v are 0 or 1
   for (int r = 0; r < 2; r++) {
      for (int c = 0; c < 2; c++) {
         for (int v = 0; v < 2; v++) {
            Vector d;
            Vector e;

            d [0] = (c == 0 ? -xDataOrigin : +xDataOrigin)*this->mXScale*paintScale;
            d [1] = (r == 0 ? -yDataOrigin : +yDataOrigin)*this->mYScale*paintScale;
            d [2] = (v == 0 ? -zDataHaSpan : +zDataHaSpan)*zScale;

            this->vmult (e, rotMat, d);   // e = rotMat*d;

            double f = z0 / (z0 - e[2]);  // perspective

            double x = f*e[0];
            double y = f*e[1];

            // Translate to relavtive to centre of screen.
            //
            x += xScreenOrigin;
            y += yScreenOrigin;

            this->transformedCorners [r][c][v] = QPointF (x, y);
            this->zinfoCorners [r][c][v] = e[2];
         }
      }
   }

   // Draw the axis
   //
   pen.setWidth (1);
   pen.setStyle (Qt::SolidLine);
   pen.setColor (this->mAxisColour);
   painter.setPen (pen);
   QPointF box [5];
   QPointF line [2];

   int s = 0;

   s = (this->zinfoCorners [0][0][0] < this->zinfoCorners [1][0][0]) ? 0 : 1;
   box [0] = this->transformedCorners [s][0][0];
   box [1] = this->transformedCorners [s][0][1];
   box [2] = this->transformedCorners [s][1][1];
   box [3] = this->transformedCorners [s][1][0];
   box [4] = box [0];
   painter.drawPolyline (box, 5);

   for (int f = 1; f <= 4; f++) {
      const double g = f / 5.0;
      line [0] = g * box [0] + (1.0 - g) * box [1];
      line [1] = g * box [3] + (1.0 - g) * box [2];
      painter.drawPolyline (line, 2);
   }

   s = (this->zinfoCorners [0][0][0] < this->zinfoCorners [0][1][0]) ? 0 : 1;
   box [0] = this->transformedCorners [0][s][0];
   box [1] = this->transformedCorners [0][s][1];
   box [2] = this->transformedCorners [1][s][1];
   box [3] = this->transformedCorners [1][s][0];
   box [4] = box [0];
   painter.drawPolyline (box, 5);

   for (int f = 1; f <= 4; f++) {
      const double g = f / 5.0;
      line [0] = g * box [0] + (1.0 - g) * box [1];
      line [1] = g * box [3] + (1.0 - g) * box [2];
      painter.drawPolyline (line, 2);
   }


   s = (this->zinfoCorners [0][0][0] < this->zinfoCorners [0][0][1]) ? 0 : 1;
   box [0] = this->transformedCorners [0][0][s];
   box [1] = this->transformedCorners [0][1][s];
   box [2] = this->transformedCorners [1][1][s];
   box [3] = this->transformedCorners [1][0][s];
   box [4] = box [0];
   painter.drawPolyline (box, 5);

// What is this ???
//   for (int f = 1; f <= 4; f++) {
//      const double g = f / 5.0;
//      line [0] = g * box [0] + (1.0 - g) * box [1];
//      line [1] = g * box [3] + (1.0 - g) * box [2];
//      painter.drawPolyline (line, 2);
//   }

   // Transform all grid points.
   //
   for (int r = 0; r < nr; r++) {
      for (int c = 0; c < nc; c++) {
         Vector d;
         Vector e;
         double v = this->surfaceData [r][c];

         if (this->mClampData) {
            v = LIMIT (v, min, max);
         }

         d [0] = (c - xDataOrigin)*this->mXScale*paintScale;
         d [1] = (r - yDataOrigin)*this->mYScale*paintScale;
         d [2] = (v - zDataOrigin)*zScale;

         this->vmult (e, rotMat, d);  // e = m*d;

         double f = z0 / (z0 - e [2]);  // perspective

         double x = f*e[0];
         double y = f*e[1];

         // Translate.
         x += xScreenOrigin;
         y += yScreenOrigin;

         this->transformed [r][c] = QPointF (x, y);
         this->zinfo [r][c] = e[2];
      }
   }


   // Draw the grid and/or surface.
   //
   pen.setWidth (1);
   pen.setStyle (this->mShowGrid ? this->mGridStyle : Qt::NoPen);
   pen.setColor (this->mGridColour);
   painter.setPen (pen);

   brush.setStyle (this->mShowSurface ? this->mSurfaceStyle : Qt::NoBrush);

   int row = 0;
   int col = 0;
   for (bool ok = this->firstPoint(row, col);
        ok;  ok = this->nextPoint (row, col)) {

      /// TODO: choose mean value for colour
      //
      double x = this->surfaceData [row][col];
      double f = 0.0;
      if (x >= max) {
         f = 1.0;
      } else if (x <= min) {
         f = 0.0;
      } else {
         f = (x - min) / (max - min);
      }

      QColor k;
      k.setHsl (int ((1.0 - f) * 240.0) % 360, 255, int (88.0 + f*80.0));

      //    registration colours
      //      if ((row <  4) && (col < 4)) {
      //         k = QColor ("#00c000");
      //      }
      //      if ((row <  4) && (col > nc - 6)) {
      //         k = QColor ("#ff0000");
      //      }
      //      if ((row > nr - 6) && (col < 4)) {
      //         k = QColor ("#ffff00");
      //      }
      //      if ((row > nr - 6) && (col > nc - 6)) {
      //         k = QColor ("#80c0ff");
      //      }

      brush.setColor (k);
      painter.setBrush (brush);

      QPointF quad [4];
      quad [0] = this->transformed [row  ][col  ];
      quad [1] = this->transformed [row+1][col  ];
      quad [2] = this->transformed [row+1][col+1];
      quad [3] = this->transformed [row  ][col+1];
      //    quad [4] = quad [0];

      painter.drawPolygon (quad, 4);
   }


   QPointF mkloc;     // marker location

   int t = 0;

   s = (this->zinfoCorners [0][0][0] < this->zinfoCorners [1][0][0]) ? 0 : 1;
   t = (this->zinfoCorners [0][0][0] < this->zinfoCorners [0][1][0]) ? 0 : 1;

   mkloc = this->transformedCorners [s][1-t][0];
   this->markerCorners [mkX] = mkloc;
   this->markerBoxes [mkX] = QRect (mkloc.x() - 4, mkloc.y() - 4, 9, 9);   // left top, width, height

   mkloc = this->transformedCorners [1-s][t][0];
   this->markerCorners [mkY] = mkloc;
   this->markerBoxes [mkY] = QRect (mkloc.x() - 4, mkloc.y() - 4, 9, 9);   // left top, width, height

   mkloc = this->transformedCorners [s][t][1];
   this->markerCorners [mkZ] = mkloc;
   this->markerBoxes [mkZ] = QRect (mkloc.x() - 4, mkloc.y() - 4, 9, 9);   // left top, width, height

   // The other end of each of the marker axis.
   //
   this->coMarker  = this->transformedCorners [s][t][0];

   pen.setColor (QColor ("#003060"));  // dark bleuish
   pen.setWidth (1);
   pen.setStyle (Qt::SolidLine);
   painter.setPen (pen);

   brush.setStyle (Qt::SolidPattern);

   for (int j = 0; j < ARRAY_LENGTH (this->markerBoxes); j++) {
      if (j == this->activeMarker) {
         brush.setColor (QColor ("#80c0ff"));
      } else {
         brush.setColor (QColor ("#c0c0c0"));
      }
      painter.setBrush (brush);
      painter.drawRect (this->markerBoxes [j]);
   }

   if (this->mShowScaling) {
      QString text;
      text.sprintf ("theta: %+06.1f  phi: %+06.1f"
                    "  x-scale: %.1f%%  y-scale: %.1f%%  z-scale: %.1f%%",
                    this->mTheta, this->mPhi,
                    this->mXScale, this->mYScale, this->mZScale);
      painter.drawText (QPoint (8, this->height () - 8), text);
   }
}

//------------------------------------------------------------------------------
//
void  QESurface::wheelEvent (QWheelEvent* event)
{
   // factor**48 == 100 - just because I can.
   //
   static const double factor = 1.1006941712522096;
   if (event->delta () < 0) {
      this->setXScale (this->getXScale() * factor);
      this->setYScale (this->getYScale() * factor);
      this->setZScale (this->getZScale() * factor);
   } else {
      this->setXScale (this->getXScale() / factor);
      this->setYScale (this->getYScale() / factor);
      this->setZScale (this->getZScale() / factor);
   }
}

//------------------------------------------------------------------------------
//
void QESurface::mousePressEvent (QMouseEvent* event)
{
   // Check that required button pressed.
   //
   if (event->button() != Qt::LeftButton) return;

   QPointF diff;

   this->mouseIsDown = true;
   this->mouseDownPoint = event->pos();

   switch (this->activeMarker) {
      case mkX:
      case mkY:
      case mkZ:
         this->scalePositionA = (this->coMarker + markerCorners [this->activeMarker])/2.0;  // mean - used as origin
         this->scalePositionB = markerCorners [this->activeMarker];                         // active corner
         diff = this->scalePositionB - this->scalePositionA;                                // difference
         this->referenceDistance = sqrt (diff.x()*diff.x() + diff.y()*diff.y());            // initial distance
         switch (this->activeMarker) {                                                      // initial scale
            case mkX: this->referenceScale = this->getXScale (); break;
            case mkY: this->referenceScale = this->getYScale (); break;
            case mkZ: this->referenceScale = this->getZScale (); break;
            default:  this->referenceScale = 100.0; break;
         }
         break;

      default: // avoid pesky warning
         break;
   }

   this->update ();
}

//------------------------------------------------------------------------------
//
void QESurface::mouseMoveEvent (QMouseEvent* event)
{
   const QPoint pos = event->pos();
   const QPointF realPos = pos;

   QPoint delta;
   QPointF intersectPostion;
   QPointF diff;
   double newDistance;
   double refDistance;
   double newScale;

   if (this->mouseIsDown) {

      switch (this->activeMarker) {
         case mkNone:
            delta = pos - this->mouseDownPoint;
            this->setTheta (this->getTheta() + delta.x() * 0.4);
            this->setPhi   (this->getPhi()   - delta.y() * 0.8);
            this->mouseDownPoint = pos;   // must always be relative
            break;

         case mkX:
         case mkY:
         case mkZ:
            // TODO: revist this
            // Calc change of scale with respect to the origin.
            //
            intersectPostion = this->calcPerpIntersect (this->scalePositionA, this->scalePositionB, realPos);
            diff = intersectPostion - this->scalePositionA;
            newDistance = sqrt (diff.x()*diff.x() + diff.y()*diff.y());
            newDistance = MAX(newDistance, 1.0);              // Avoid zero distance
            refDistance = MAX(this->referenceDistance, 1.0);  // Avoid the divide by zero
            newScale = (newDistance /refDistance) * MAX(this->referenceScale, 1.0);

            switch (this->activeMarker) {
               case mkX: this->setXScale (newScale); break;
               case mkY: this->setYScale (newScale); break;
               case mkZ: this->setZScale (newScale); break;
               default:  break;
            }
            break;

         default:
            break;
      }

   } else {
      // Save what we have so that we can detect any change.
      //
      Markers prevActiveMarker = this->activeMarker;

      this->activeMarker = mkNone;
      for (int j = 0; j < ARRAY_LENGTH (this->markerBoxes); j++) {
         if (this->markerBoxes[j].contains (pos)) {
            // found it
            //
            this->setCursor (QCursor (Qt::ArrowCursor));
            this->activeMarker = Markers (j);
            break;
         }
      }

      // Has the active marker selection changhed?
      //
      if (this->activeMarker != prevActiveMarker) {
         if (this->activeMarker == mkNone) {
            this->setCursor (QCursor (Qt::CrossCursor));
         } else {
            this->setCursor (QCursor (Qt::ArrowCursor));
         }
         this->update ();
      }
   }
}

//------------------------------------------------------------------------------
//
void QESurface::mouseReleaseEvent (QMouseEvent*)
{
   this->mouseIsDown = false;
   this->activeMarker = mkNone;
   this->update ();
}

//------------------------------------------------------------------------------
//
void QESurface::updateDataVisulation ()
{
   int numberAvailableCols;
   int numberAvailableRows;
   this->getNumberRowsAndCols (numberAvailableRows, numberAvailableCols);

   // Limit to what the widget can support
   //
   this->numberCols = MIN (numberAvailableCols, maxNumberOfCols);
   this->numberRows = MIN (numberAvailableRows, maxNumberOfRows);

   double min;   // default
   double max;   // unused here
   this->getScaleModeMinMaxValues (min, max);

   // Cache data locally - do we ready need this ??
   //
   for (int row = 0; row < this->numberRows; row++) {
      for (int col = 0; col < this->numberCols; col++) {
         double value = this->getValue (row, col, min);
         this->surfaceData [row][col] = value;
      }
   }
   this->update ();
}

//------------------------------------------------------------------------------
// Build the specific context menu
//
QMenu* QESurface::buildContextMenu ()
{
   QMenu* menu = ParentWidgetClass::buildContextMenu ();
   QAction* action;

   menu->addSeparator ();

   action = new QAction ("Show Grid", menu);
   action->setCheckable (true);
   action->setChecked (this->getShowGrid ());
   action->setData (QESURF_SHOW_GRID_FLIP);
   menu->addAction (action);

   action = new QAction ("Show Surface", menu);
   action->setCheckable (true);
   action->setChecked (this->getShowSurface ());
   action->setData (QESURF_SHOW_SURFACE_FLIP);
   menu->addAction (action);

   action = new QAction ("Clamp Data", menu);
   action->setCheckable (true);
   action->setChecked (this->getClampData());
   action->setData (QESURF_CLAMP_DATA_FLIP);
   menu->addAction (action);

   action = new QAction ("Show Scaling", menu);
   action->setCheckable (true);
   action->setChecked (this->getShowScaling ());
   action->setData (QESURF_SHOW_SCALING_FLIP);
   menu->addAction (action);

   return menu;

}

//------------------------------------------------------------------------------
// An action was selected from the context menu
//
void QESurface::contextMenuTriggered (int selectedItemNum)
{
   switch (selectedItemNum) {

      case QESURF_SHOW_GRID_FLIP:
         // flip the show grid state.
         //
         this->setShowGrid (!this->getShowGrid ());
         break;

      case QESURF_SHOW_SURFACE_FLIP:
         // flip the show surface state.
         //
         this->setShowSurface (!this->getShowSurface ());
         break;

      case QESURF_CLAMP_DATA_FLIP:
         // flip the clamp data state.
         //
         this->setClampData (!this->getClampData ());
         break;

      case QESURF_SHOW_SCALING_FLIP:
         // flip the show scaling state.
         //
         this->setShowScaling (!this->getShowScaling ());
         break;

      default:
         // Call parent class function.
         //
         ParentWidgetClass::contextMenuTriggered (selectedItemNum);
         break;
   }
}

//------------------------------------------------------------------------------
//
bool QESurface::firstPoint (int& row, int& col)
{
   this->iterationCount = 0;

   // Both numbers have been sanitised <= max number row/cols
   //
   const int nr = this->numberRows;
   const int nc = this->numberCols;
   if (nr <= 0) return false;
   if (nc <= 0) return false;

   // sort by zinfo OR just look at zinfo of each corner
   // Want to draw from back to front.
   // TODO Use corner zinfo - value independent
   //
   const double z00 = this->zinfo [0]     [0];
   const double z01 = this->zinfo [0]     [nc - 1];
   const double z10 = this->zinfo [nr - 1][0];
   const double z11 = this->zinfo [nr - 1][nc - 1];

   if ((z00 <= z01) && (z00 <= z10) && (z00) <= (z11)) {
      this->iterationRowFirst = 0;
      this->iterationColFirst = 0;
      this->iterationRowMajor = (z01 < z10);
   } else if ((z01 <= z10) && (z01) <= (z11)) {
      this->iterationRowFirst = 0;
      this->iterationColFirst = nc - 2;
      this->iterationRowMajor = (z00 < z11);
   } else if (z10 <= z11) {
      this->iterationRowFirst = nr - 2;
      this->iterationColFirst = 0;
      this->iterationRowMajor = (z11 < z00);
   } else {
      this->iterationRowFirst = nr - 2;
      this->iterationColFirst = nc - 2;
      this->iterationRowMajor = (z10 < z01);
   }

   // DEBUG << this->iterationRowFirst << this->iterationColFirst <<  this->iterationRowMajor;
   // DEBUG << "(" << z00 << z01 << ") (" << z10 << z11 << ")";
   return this->nextPoint (row, col);
}

//------------------------------------------------------------------------------
//
bool QESurface::nextPoint (int& row, int& col)
{
   // Both numbers have been sanitised <= max number row/cols
   //
   const int nr = this->numberRows;
   const int nc = this->numberCols;
   if (nr <= 0) return false;
   if (nc <= 0) return false;

   const int total = (nr - 1) * (nc - 1);
   if (this->iterationCount >= total) return false;

   if (this->iterationRowMajor) {
      row = this->iterationCount / (nc - 1);
      col = this->iterationCount % (nc - 1);
   } else {
      col = this->iterationCount / (nr - 1);
      row = this->iterationCount % (nr - 1);
   }

   if (this->iterationRowFirst > 0) row = (nr - 2) - row;
   if (this->iterationColFirst > 0) col = (nc - 2) - col;

   this->iterationCount++;
   return true;
}

//------------------------------------------------------------------------------
//
double QESurface::normaliseAngle (const double angle)
{
   double result = angle;
   while (result < 0.0) result += 360.0;

   // result is +ve, lock to multiple of 15 degree values
   // We do the locking with +result to ensure consistant int rounding
   //
   static const double delta = 0.8;
   const int m = int (result / 15.0 + 0.5);   // round up
   const double base = m * 15.0;
   double frac = result - base;               // frac is -7.5 .. +7.5
   if ((frac >= -delta) && (frac <= +delta)) frac = 0.0;
   result = base + frac;

   // allow result to be -ve.
   //
   while (result > +180.0) result -= 360.0;

   return result;
}

//==============================================================================
// Properties
//
void QESurface::setShowGrid (const bool showGridIn)
{
   this->mShowGrid = showGridIn;
   this->update ();
}

//------------------------------------------------------------------------------
//
bool QESurface::getShowGrid () const
{
   return this->mShowGrid;
}

//------------------------------------------------------------------------------
//
void QESurface::setGridStyle (const Qt::PenStyle penStyleIn)
{
   this->mGridStyle = penStyleIn;
   this->update ();
}

//------------------------------------------------------------------------------
//
Qt::PenStyle QESurface::getGridStyle () const
{
   return this->mGridStyle;
}

//------------------------------------------------------------------------------
//
void QESurface::setGridColour (const QColor& gridColourIn)
{
   this->mGridColour = gridColourIn;
   this->update ();
}

//------------------------------------------------------------------------------
//
QColor QESurface::getGridColour () const
{
   return this->mGridColour;
}

//------------------------------------------------------------------------------
//
void QESurface::setAxisColour (const QColor& axisColourIn)
{
   this->mAxisColour = axisColourIn;
   this->update ();
}

//------------------------------------------------------------------------------
//
QColor QESurface::getAxisColour () const
{
   return this->mAxisColour;
}

//------------------------------------------------------------------------------
//
void QESurface::setShowSurface (const bool showSurfaceIn)
{
   this->mShowSurface = showSurfaceIn;
   this->update ();
}

//------------------------------------------------------------------------------
//
bool QESurface::getShowSurface () const
{
   return this->mShowSurface;
}

//------------------------------------------------------------------------------
//
void QESurface::setSurfaceStyle (const Qt::BrushStyle brushStyleIn)
{
   this->mSurfaceStyle = brushStyleIn;
   this->update ();
}

//------------------------------------------------------------------------------
//
Qt::BrushStyle QESurface::getSurfaceStyle () const
{
   return this->mSurfaceStyle;
}

//------------------------------------------------------------------------------
//
void QESurface::setTheta (const double thetaIn)
{
   this->mTheta = this->normaliseAngle (thetaIn);
   this->update ();
}

//------------------------------------------------------------------------------
//
double QESurface::getTheta () const
{
   return this->mTheta;
}

//------------------------------------------------------------------------------
//
void QESurface::setPhi (const double phiIn)
{
   this->mPhi = this->normaliseAngle (phiIn);
   this->update ();
}

//------------------------------------------------------------------------------
//
double QESurface::getPhi () const
{
   return this->mPhi;
}

//------------------------------------------------------------------------------
//
void QESurface::setZoom (const double zoomIn)
{
   this->mZoom = LIMIT (zoomIn, 100.0, 10000.0);
   this->update ();
}

//------------------------------------------------------------------------------
//
double QESurface::getZoom () const
{
   return this->mZoom;
}

//------------------------------------------------------------------------------
//
void  QESurface::setXScale (const double xScaleIn)
{
   this->mXScale = LIMIT (xScaleIn, 5.0, 10000.0);
   this->update ();
}

//------------------------------------------------------------------------------
//
double QESurface::getXScale () const
{
   return this->mXScale;
}

//------------------------------------------------------------------------------
//
void  QESurface::setYScale (const double yScaleIn)
{
   this->mYScale = LIMIT (yScaleIn, 5.0, 10000.0);
   this->update ();
}

//------------------------------------------------------------------------------
//
double QESurface::getYScale () const
{
   return this->mYScale;
}
//------------------------------------------------------------------------------
//
void QESurface::setZScale (const double zScaleIn)
{
   this->mZScale = LIMIT (zScaleIn, 0.0, 10000.0);
   if (this->mZScale <= 0.5) this->mZScale = 0.0;
   this->update ();
}

//------------------------------------------------------------------------------
//
double QESurface::getZScale () const
{
   return this->mZScale;
}

//------------------------------------------------------------------------------
//
void QESurface::setClampData (const bool clampDataIn)
{
   this->mClampData = clampDataIn;
   this->update ();
}

//------------------------------------------------------------------------------
//
bool QESurface::getClampData () const
{
   return this->mClampData;
}

//------------------------------------------------------------------------------
//
void QESurface::setShowScaling (const bool showScalingIn)
{
   this->mShowScaling = showScalingIn;
   this->update ();
}

//------------------------------------------------------------------------------
//
bool QESurface::getShowScaling () const
{
   return this->mShowScaling;
}

//------------------------------------------------------------------------------
// returns a random float between a and b
//
static double randomFloat (const double a, const double b)
{
   double r = double (rand()) / double (RAND_MAX);
   r = a + (b-a)*r;
   return r;
}

//------------------------------------------------------------------------------
// Relocate to abstarct widget
void QESurface::setTestData (const bool testDataEnabledIn)
{
   if (this->mTestDataEnabled != testDataEnabledIn) {
      this->mTestDataEnabled = testDataEnabledIn;

      if (this->mTestDataEnabled) {

         QPointF centres [8];
         double amplitues [ARRAY_LENGTH (centres)];
         double decays [ARRAY_LENGTH (centres)];

         for (int q = 0; q < ARRAY_LENGTH (centres); q++) {
            centres [q] = QPointF (randomFloat (20, maxNumberOfRows - 20),
                                   randomFloat (20, maxNumberOfCols - 20));
            amplitues [q] = randomFloat (-100, +100);
            decays [q] = randomFloat (400, 2000);
         }

         for (int row = 0; row < maxNumberOfRows; row++) {
            QVector <double> rowData;
            for (int col = 0; col < maxNumberOfCols; col++) {
               QPointF p = QPointF (row, col);

               double v = 0.0;
               for (int q = 0; q < ARRAY_LENGTH (centres); q++) {
                  QPointF s = p - centres [q];

                  double r2;
                  double y;
                  r2 = s.x()*s.x() + s.y()*s.y();
                  y = amplitues[q]*exp (-r2 / decays [q]);
                  v += y;
               }
               this->surfaceData [row][col] = v;
            }
         }
      }
   }
}

//------------------------------------------------------------------------------
//
bool QESurface::getTestData () const
{
   return this->mTestDataEnabled;
}


//------------------------------------------------------------------------------
// static R = A.B
void QESurface::mmult (Matrix result, const Matrix a, const Matrix b)
{
   result [0][0] = a[0][0]*b[0][0] + a[0][1]*b[1][0] + a[0][2]*b[2][0];
   result [0][1] = a[0][0]*b[0][1] + a[0][1]*b[1][1] + a[0][2]*b[2][1];
   result [0][2] = a[0][0]*b[0][2] + a[0][1]*b[1][2] + a[0][2]*b[2][2];

   result [1][0] = a[1][0]*b[0][0] + a[1][1]*b[1][0] + a[1][2]*b[2][0];
   result [1][1] = a[1][0]*b[0][1] + a[1][1]*b[1][1] + a[1][2]*b[2][1];
   result [1][2] = a[1][0]*b[0][2] + a[1][1]*b[1][2] + a[1][2]*b[2][2];

   result [2][0] = a[2][0]*b[0][0] + a[2][1]*b[1][0] + a[2][2]*b[2][0];
   result [2][1] = a[2][0]*b[0][1] + a[2][1]*b[1][1] + a[2][2]*b[2][1];
   result [2][2] = a[2][0]*b[0][2] + a[2][1]*b[1][2] + a[2][2]*b[2][2];
}

//------------------------------------------------------------------------------
// static r = A.x
void QESurface::vmult (Vector result, const Matrix a, const Vector x)
{
   result [0] = a[0][0]*x[0] + a[0][1]*x[1] + a[0][2]*x[2];
   result [1] = a[1][0]*x[0] + a[1][1]*x[1] + a[1][2]*x[2];
   result [2] = a[2][0]*x[0] + a[2][1]*x[1] + a[2][2]*x[2];
}

//------------------------------------------------------------------------------
// static
QPointF QESurface::calcPerpIntersect (const QPointF p1, const QPointF p2,
                                      const QPointF p3)
{
   QPointF result;

   // A.x + B.y + C is a line through p1 and p2
   //
   const double A =  p2.y() - p1.y();
   const double B =  p1.x() - p2.x();
   const double C = -(A*p1.x() + B*p1.y());

   // Perpendicular line though p3 is of form:  B.x - A.y + D = 0
   //
   const double D = A*p3.y() - B*p3.x();

   // Let intersect point be (u,v), then
   // A.u + B.v + C = 0
   // B.u - A.v + D = 0
   //
   const double s = A*A + B*B;
   if (s <= 0) {
      result = p1;
   } else {
      const double u = -(A*C + B*D)/s;
      const double v = -(B*C - A*D)/s;
      result =  QPointF (u, v);
   }

   return result;
}

// end
