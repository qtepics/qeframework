/*  QSimpleShape.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2022 Australian Synchrotron.
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

#include "QSimpleShape.h"
#include <QDebug>
#include <QPainter>
#include <QECommon.h>

#define DEBUG qDebug () << "QSimpleShape" << __LINE__ << __FUNCTION__ << "  "

//-----------------------------------------------------------------------------
//
QSimpleShape::QSimpleShape (QWidget* parent) : QWidget (parent)
{
   // This class properties.
   //
   this->value = 0;
   this->modulus = NUMBER_OF_STATES;
   this->shape = rectangle;
   this->textFormat = FixedText;
   this->fixedText = "";
   this->alignment = Qt::AlignHCenter | Qt::AlignVCenter;
   this->indent = 6;
   this->isActive = true;
   this->edgeWidth = 1;
   this->edgeStyle = Qt::SolidLine;
   this->semiCycles = 8;
   this->percentSize = 10;
   this->centreAngle = 0;
   this->spanAngle = 90;
   this->flashStateIsOn = false;

   this->edgeColour     = QColor (0,   0,   0);        // black
   this->flashOffColour = QColor (200, 200, 200,  0);  // clear, alpha = 0

   this->flashRate = QEScanTimers::Medium;
   QEScanTimers::attach (this, SLOT (flashTimeout (const bool)), this->flashRate);

   this->stateSet.clear ();
   for (int j = 0; j < NUMBER_OF_STATES; j++) {
      this->colourList[j] = QColor (200, 200, 200, 255);
      this->flashList [j] = false;
      this->stateSet.append ("");
   }
}

//-----------------------------------------------------------------------------
// Reduce width or height of rectangle so that it becomes square, and offset
// to keep it centred.
//
void QSimpleShape::equaliseRect (QRect& rect)
{
   const int diff = rect.width () - rect.height ();
   const int ew = this->getEdgeWidth ();

   if (diff > 0) {
      rect.setWidth (rect.height ());
      rect.moveLeft ((+diff) / 2 + ew / 2);  // Note: moves left edge, not to the left.

   } else if (diff < 0) {
      rect.setHeight (rect.width ());
      rect.moveTop  ((-diff) / 2 + ew / 2);
   }
}

//-----------------------------------------------------------------------------
//
void QSimpleShape::paintEvent (QPaintEvent*)
{
   QPainter painter (this);
   QPen pen;
   QBrush brush;
   QRect rect;
   QPoint polygon [128];  // 30 snake points
   QColor colour;
   QColor boarderColour;
   bool washedOut = false;
   QString text;
   int f = 0;             // fraction
   int g = 0;             // co-fraction
   int sum;

   // work variables
   int numPoints, ds;
   int p, q;
   int xc, x0, x1, x2, x3;
   int yc, y0, y1, y2, y3;
   double dx, dy;

   // Get basic colour property.
   // NOTE: This is a dispatching call.
   //
   colour = this->getItemColour ();

   // flash the colour, but not the boarder.
   //
   const int mv = this->getValue ();
   if (this->flashList [mv] && !this->flashStateIsOn) {
      colour = this->flashOffColour;
   }

   boarderColour = this->getEdgeColour ();

   // Draw as grayed out if disabled or inactive.
   //
   washedOut = !(this->isEnabled () && this->getIsActive());
   if (washedOut) {
      // Disconnected or disabled - grey out colours.
      //
      colour = QEUtilities::blandColour (colour);
      boarderColour = QEUtilities::blandColour (boarderColour);
   }

   int ew = this->getEdgeWidth ();
   if (ew == 0) {
      // We can't actually set a pen width of zero, so we go with 1 and use
      // same colour for both pen and brush.
      //
      ew = 1;
      pen.setColor (colour);
   } else {
      pen.setColor (boarderColour);
   }

   pen.setWidth (ew);
   pen.setStyle (this->edgeStyle);
   painter.setPen (pen);

   brush.setStyle (Qt::SolidPattern);
   brush.setColor (colour);
   painter.setBrush (brush);

   rect = this->geometry ();
   rect.moveTo (ew / 2, ew / 2);

   rect.setWidth (rect.width () - ew);
   rect.setHeight (rect.height () - ew);

   switch (this->getShape ()) {

      case circle:
         this->equaliseRect (rect);
         // fall through
      case ellipse:
         painter.drawEllipse (rect);
         break;


      case square:
         this->equaliseRect (rect);
         // fall through
      case rectangle:
         painter.drawRect (rect);
         break;


      case roundSquare:
         this->equaliseRect (rect);
         // fall through
      case roundRectangle:
         {
            // Normalise corner radii.
            //
            double size = MIN(rect.width(), rect.height());
            double radius = 0.25 * size;
            painter.drawRoundedRect (rect, radius, radius);
         }
         break;


      case equalDiamond:
         this->equaliseRect (rect);
         // fall through
      case diamond:
         polygon[0] = QPoint ((rect.left () + rect.right ()) / 2, rect.top ());
         polygon[1] = QPoint (rect.right (), (rect.top () + rect.bottom ()) / 2);
         polygon[2] = QPoint ((rect.left () + rect.right ()) / 2, rect.bottom ());
         polygon[3] = QPoint (rect.left (), (rect.top () + rect.bottom ()) / 2);
         polygon[4] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 5);
         break;


      case triangleUp:
         polygon[0] = QPoint ((rect.left () + rect.right ()) / 2, rect.top ());
         polygon[1] = QPoint (rect.right (), rect.bottom ());
         polygon[2] = QPoint (rect.left (), rect.bottom ());
         polygon[3] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case triangleDown:
         polygon[0] = QPoint ((rect.left () + rect.right ()) / 2, rect.bottom ());
         polygon[1] = QPoint (rect.right (), rect.top ());
         polygon[2] = QPoint (rect.left (), rect.top ());
         polygon[3] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case triangleLeft:
         polygon[0] = QPoint (rect.left (), (rect.top () + rect.bottom ()) / 2);
         polygon[1] = QPoint (rect.right (), rect.top ());
         polygon[2] = QPoint (rect.right (), rect.bottom ());
         polygon[3] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case triangleRight:
         polygon[0] = QPoint (rect.right (), (rect.top () + rect.bottom ()) / 2);
         polygon[1] = QPoint (rect.left (), rect.top ());
         polygon[2] = QPoint (rect.left (), rect.bottom ());
         polygon[3] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case triangleTopRight:
         polygon[0] = QPoint (rect.right (), rect.top ());
         polygon[1] = QPoint (rect.right (), rect.bottom ());
         polygon[2] = QPoint (rect.left (), rect.top ());
         polygon[3] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case triangleBottomRight:
         polygon[0] = QPoint (rect.right (), rect.bottom ());
         polygon[1] = QPoint (rect.left (), rect.bottom ());
         polygon[2] = QPoint (rect.right (), rect.top ());
         polygon[3] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case triangleBottomLeft:
         polygon[0] = QPoint (rect.left (), rect.bottom ());
         polygon[1] = QPoint (rect.left (), rect.top ());
         polygon[2] = QPoint (rect.right (), rect.bottom ());
         polygon[3] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case triangleTopLeft:
         polygon[0] = QPoint (rect.left (), rect.top ());
         polygon[1] = QPoint (rect.right (), rect.top ());
         polygon[2] = QPoint (rect.left (), rect.bottom ());
         polygon[3] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case arrowUp:
         // calculate some intermediate values.
         //
         y0 = rect.top () + rect.width () / 2;
         x0 = (rect.left () + rect.right ()) / 2;
         x1 = (2 * rect.left () + rect.right ()) / 3;
         x2 = (rect.left () + 2 * rect.right ()) / 3;

         polygon[0] = QPoint (x0, rect.top ());
         polygon[1] = QPoint (rect.right (), y0);
         polygon[2] = QPoint (x2, y0);
         polygon[3] = QPoint (x2, rect.bottom ());
         polygon[4] = QPoint (x1, rect.bottom ());
         polygon[5] = QPoint (x1, y0);
         polygon[6] = QPoint (rect.left (), y0);
         polygon[7] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 8);
         break;

      case arrowDown:
         // calculate some intermediate values.
         //
         y0 = rect.bottom () - rect.width () / 2;
         x0 = (rect.left () + rect.right ()) / 2;
         x1 = (2 * rect.left () + rect.right ()) / 3;
         x2 = (rect.left () + 2 * rect.right ()) / 3;

         polygon[0] = QPoint (x0, rect.bottom ());
         polygon[1] = QPoint (rect.right (), y0);
         polygon[2] = QPoint (x2, y0);
         polygon[3] = QPoint (x2, rect.top ());
         polygon[4] = QPoint (x1, rect.top ());
         polygon[5] = QPoint (x1, y0);
         polygon[6] = QPoint (rect.left (), y0);
         polygon[7] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 8);
         break;

      case arrowLeft:
         // calculate some intermediate values.
         //
         x0 = rect.left () + rect.height () / 2;
         y0 = (rect.top () + rect.bottom ()) / 2;
         y1 = (2 * rect.top () + rect.bottom ()) / 3;
         y2 = (rect.top () + 2 * rect.bottom ()) / 3;

         polygon[0] = QPoint (rect.left (),y0);
         polygon[1] = QPoint (x0, rect.top ());
         polygon[2] = QPoint (x0, y1);
         polygon[3] = QPoint (rect.right (), y1);
         polygon[4] = QPoint (rect.right (), y2);
         polygon[5] = QPoint (x0, y2);
         polygon[6] = QPoint (x0, rect.bottom ());
         polygon[7] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 8);
         break;

      case arrowRight:
         // calculate some intermediate values.
         //
         x0 = rect.right () - rect.height () / 2;
         y0 = (rect.top () + rect.bottom ()) / 2;
         y1 = (2 * rect.top () + rect.bottom ()) / 3;
         y2 = (rect.top () + 2 * rect.bottom ()) / 3;

         polygon[0] = QPoint (rect.right (), y0);
         polygon[1] = QPoint (x0, rect.top ());
         polygon[2] = QPoint (x0, y1);
         polygon[3] = QPoint (rect.left (), y1);
         polygon[4] = QPoint (rect.left (), y2);
         polygon[5] = QPoint (x0, y2);
         polygon[6] = QPoint (x0, rect.bottom ());
         polygon[7] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 8);
         break;

      case crossHorizontal:
         polygon[0] = QPoint (rect.left (), rect.top ());
         polygon[1] = QPoint (rect.right (), rect.top ());
         polygon[2] = QPoint (rect.left (), rect.bottom ());
         polygon[3] = QPoint (rect.right (), rect.bottom ());
         polygon[4] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case crossVertical:
         polygon[0] = QPoint (rect.left (), rect.top ());
         polygon[1] = QPoint (rect.left (), rect.bottom ());
         polygon[2] = QPoint (rect.right (), rect.top ());
         polygon[3] = QPoint (rect.right (), rect.bottom ());
         polygon[4] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 4);
         break;

      case hexagon:
         sum = 4;
         f = 3;
         g = sum - f;
         x1 = (f * rect.left () + g * rect.right ()) / sum;
         x2 = (g * rect.left () + f * rect.right ()) / sum;
         y0 = (rect.top () + rect.bottom ()) / 2;

         polygon[0] = QPoint (rect.left (),  y0);
         polygon[1] = QPoint (x1,            rect.top ());
         polygon[2] = QPoint (x2,            rect.top ());
         polygon[3] = QPoint (rect.right (), y0);
         polygon[4] = QPoint (x2,            rect.bottom ());
         polygon[5] = QPoint (x1,            rect.bottom ());
         polygon[6] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 7);
         break;

      case octogon:
         // sum/f is a very good approximation of sqrt (2) - the rest is high-school geometry.
         //
         sum = 99;
         f = 70;
         g = sum - f;
         x1 = (f * rect.left () + g * rect.right ()) / sum;
         x2 = (g * rect.left () + f * rect.right ()) / sum;
         y1 = (f * rect.top ()  + g * rect.bottom ()) / sum;
         y2 = (g * rect.top ()  + f * rect.bottom ()) / sum;
         polygon[0] = QPoint (rect.left (),  y1);
         polygon[1] = QPoint (x1,            rect.top ());
         polygon[2] = QPoint (x2,            rect.top ());
         polygon[3] = QPoint (rect.right (), y1);
         polygon[4] = QPoint (rect.right (), y2);
         polygon[5] = QPoint (x2,            rect.bottom ());
         polygon[6] = QPoint (x1,            rect.bottom ());
         polygon[7] = QPoint (rect.left (),  y2);
         polygon[8] = polygon[0];       // close loop
         painter.drawPolygon (polygon, 9);
         break;

      case snakeHorizontal:
         ds = (rect.height () * this->percentSize + 49) / 100;

         // calculate pitch
         dx = double (rect.width () - ds) / double (this->semiCycles);
         if (dx <= 0.0) break;

         y0 = rect.top () + ds;
         y1 = (rect.top () + rect.bottom ()) / 2;
         y2 = rect.bottom () - ds;

         numPoints = this->semiCycles*4 + 4;

         // First and last point indices, which count up/down respectively.
         //
         p = 0;
         q = numPoints - 1;

         polygon[p++] = QPoint (rect.left (),       y1);
         polygon[q--] = QPoint (rect.left () + ds,  y1);

         x2 = x3 = 0;  // avoid warning
         for (int j = 0; j < this->semiCycles; j++) {

            x0 = rect.left () + int ((j + 0)*dx);
            x1 = x0 + ds;

            x2 = rect.left () + int ((j + 1)*dx);
            x3 = x2 + ds;

            if (j%2 == 0) {
               polygon[p++] =  QPoint (x0, rect.top ());
               polygon[q--] =  QPoint (x1, y0);

               polygon[p++] =  QPoint (x3, rect.top ());
               polygon[q--] =  QPoint (x2, y0);
            } else {
               polygon[p++] =  QPoint (x1, y2);
               polygon[q--] =  QPoint (x0, rect.bottom ());

               polygon[p++] =  QPoint (x2, y2);
               polygon[q--] =  QPoint (x3, rect.bottom ());
            }
         }

         if (this->semiCycles%2 == 0) {
            polygon[p++] = QPoint (x2, y1);
            polygon[q--] = QPoint (x3, y1);
         } else {
            polygon[p++] = QPoint (x3, y1);
            polygon[q--] = QPoint (x2, y1);
         }

         polygon[numPoints] = polygon[0];       // close loop
         painter.drawPolygon (polygon, numPoints + 1);
         break;

      case snakeVertical:
         // Same but x/y swapped.

         ds = (rect.width () * this->percentSize + 49) / 100;
         // calculate pitch
         dy = double (rect.height () - ds) / double (this->semiCycles);
         if (dy <= 0.0) break;

         x0 = rect.left ()  + ds;
         x1 = (rect.left () + rect.right ()) / 2;
         x2 = rect.right () - ds;

         numPoints = this->semiCycles*4 + 4;

         // First and last point indices, which count up/down respectively.
         //
         p = 0;
         q = numPoints - 1;

         polygon[p++] = QPoint (x1, rect.top () + ds);
         polygon[q--] = QPoint (x1, rect.top ()     );

         y2 = y3 = 0;  // avoid warning
         for (int j = 0; j < this->semiCycles; j++) {
            y0 = rect.top () + int ((j + 0)*dy);
            y1 = y0 + ds;

            y2 = rect.top () + int ((j + 1)*dy);
            y3 = y2 + ds;

            if (j%2 == 1) {
               polygon[p++] =  QPoint (rect.left (), y0);
               polygon[q--] =  QPoint (x0, y1);

               polygon[p++] =  QPoint (rect.left (), y3);
               polygon[q--] =  QPoint (x0, y2);
            } else {
               polygon[p++] =  QPoint (x2, y1);
               polygon[q--] =  QPoint (rect.right (), y0);

               polygon[p++] =  QPoint (x2, y2);
               polygon[q--] =  QPoint (rect.right (), y3);
            }
         }

         if (this->semiCycles%2 == 1) {
            polygon[p++] = QPoint (x1, y2);
            polygon[q--] = QPoint (x1, y3);
         } else {
            polygon[p++] = QPoint (x1, y3);
            polygon[q--] = QPoint (x1, y2);
         }

         polygon[numPoints] = polygon[0];       // close loop
         painter.drawPolygon (polygon, numPoints + 1);
         break;

      case pentagon:
         // 5 point polygon. Vertices lie on corresponding ellipse.
         //
         dx = double (rect.right ()  - rect.left ())/1000.0;
         dy = double (rect.bottom () - rect.top ())/1000.0;
         x0 = rect.left ();
         y0 = rect.top ();

         p = 0;
         polygon[p++]  = QPoint (x0 + 500*dx, y0 +   0*dy);
         polygon[p++]  = QPoint (x0 + 976*dx, y0 + 345*dy);
         polygon[p++]  = QPoint (x0 + 794*dx, y0 + 905*dy);
         polygon[p++]  = QPoint (x0 + 206*dx, y0 + 905*dy);
         polygon[p++]  = QPoint (x0 +  24*dx, y0 + 345*dy);
         polygon[p++]  = polygon[0];            // close loop
         painter.drawPolygon (polygon, p);
         break;

      case star:
         // 10 point star. Outer vertices lie on corresponding ellipse.
         //
         dx = double (rect.right ()  - rect.left ())/1000.0;
         dy = double (rect.bottom () - rect.top ())/1000.0;
         x0 = rect.left ();
         y0 = rect.top ();

         p = 0;
         polygon[p++]  = QPoint (x0 + 500*dx, y0 +   0*dy);
         polygon[p++]  = QPoint (x0 + 612*dx, y0 + 345*dy);
         polygon[p++]  = QPoint (x0 + 976*dx, y0 + 345*dy);
         polygon[p++]  = QPoint (x0 + 682*dx, y0 + 559*dy);
         polygon[p++]  = QPoint (x0 + 794*dx, y0 + 905*dy);
         polygon[p++]  = QPoint (x0 + 500*dx, y0 + 691*dy);
         polygon[p++]  = QPoint (x0 + 206*dx, y0 + 905*dy);
         polygon[p++]  = QPoint (x0 + 318*dx, y0 + 559*dy);
         polygon[p++]  = QPoint (x0 +  24*dx, y0 + 345*dy);
         polygon[p++]  = QPoint (x0 + 388*dx, y0 + 345*dy);
         polygon[p++]  = polygon[0];            // close loop
         painter.drawPolygon (polygon, p);
         break;

      case plus:
         sum = 100;
         f = 50 - this->percentSize;
         g = sum - f;
         x1 = (g * rect.left () + f * rect.right ()) / sum;
         x2 = (f * rect.left () + g * rect.right ()) / sum;
         y1 = (g * rect.top ()  + f * rect.bottom ()) / sum;
         y2 = (f * rect.top ()  + g * rect.bottom ()) / sum;

         p = 0;
         polygon[p++] = QPoint (x1,            rect.top ());
         polygon[p++] = QPoint (x2,            rect.top ());
         polygon[p++] = QPoint (x2,            y1);
         polygon[p++] = QPoint (rect.right (), y1);
         polygon[p++] = QPoint (rect.right (), y2);
         polygon[p++] = QPoint (x2,            y2);
         polygon[p++] = QPoint (x2,            rect.bottom ());
         polygon[p++] = QPoint (x1,            rect.bottom ());
         polygon[p++] = QPoint (x1,            y2);
         polygon[p++] = QPoint (rect.left (),  y2);
         polygon[p++] = QPoint (rect.left (),  y1);
         polygon[p++] = QPoint (x1,            y1);
         polygon[p++] = polygon[0];            // close loop
         painter.drawPolygon (polygon, p);
         break;

      case cross:
         xc = (rect.left () + rect.right ())  / 2;
         yc = (rect.top ()  + rect.bottom ()) / 2;

         sum = 100;
         f = this->percentSize;
         g = sum - f;
         x0 = (g * rect.left () + f * rect.right ()) / sum;
         x3 = (f * rect.left () + g * rect.right ()) / sum;
         y0 = (g * rect.top ()  + f * rect.bottom ()) / sum;
         y3 = (f * rect.top ()  + g * rect.bottom ()) / sum;

         f = 50 - this->percentSize;
         g = sum - f;
         x1 = (g * rect.left () + f * rect.right ()) / sum;
         x2 = (f * rect.left () + g * rect.right ()) / sum;
         y1 = (g * rect.top ()  + f * rect.bottom ()) / sum;
         y2 = (f * rect.top ()  + g * rect.bottom ()) / sum;

         p = 0;
         polygon[p++] = QPoint (rect.left (),  rect.top ());
         polygon[p++] = QPoint (x0,            rect.top ());
         polygon[p++] = QPoint (xc,            y1);
         polygon[p++] = QPoint (x3,            rect.top ());
         polygon[p++] = QPoint (rect.right (), rect.top ());
         polygon[p++] = QPoint (rect.right (),  y0);
         polygon[p++] = QPoint (x2,            yc);
         polygon[p++] = QPoint (rect.right (), y3);
         polygon[p++] = QPoint (rect.right (), rect.bottom ());
         polygon[p++] = QPoint (x3,            rect.bottom ());
         polygon[p++] = QPoint (xc,            y2);
         polygon[p++] = QPoint (x0,            rect.bottom ());
         polygon[p++] = QPoint (rect.left (),  rect.bottom ());
         polygon[p++] = QPoint (rect.left (),  y3);
         polygon[p++] = QPoint (x1,            yc);
         polygon[p++] = QPoint (rect.left (),  y0);
         polygon[p++] = polygon[0];            // close loop
         painter.drawPolygon (polygon, p);
         break;

      case roundpie:
         this->equaliseRect (rect);
         // fall through
      case pie:
         // The startAngle and spanAngle must be specified in 1/16th of a degree.
         // We increment clockwise about 0, drawPie increments anti-clockwise from 90
         //
         f = 8 * (180 - (2*this->centreAngle - this->spanAngle));
         g = - 16 * this->spanAngle;
         painter.drawPie (rect, f, g);
         break;

     case heart:
         // For heart beat
         // Note: we round down the sizes (dx, dy) and
         // effectively round up centre (x0, y0).
         //
         dx = double (rect.right () - rect.left () - 1)/2.0;
         dy = double (rect.top () - rect.bottom () + 1)/2.0;  // Note: the sign flip

         x0 = double (rect.right () + rect.left () + 1)/2.0;
         y0 = double (rect.top () + rect.bottom () + 1)/2.0;

         p = 0;
         polygon[p++] = QPoint (x0,            y0 - 1.000*dy);
         polygon[p++] = QPoint (x0 - 0.900*dx, y0 + 0.200*dy);
         polygon[p++] = QPoint (x0 - 0.933*dx, y0 + 0.250*dy);
         polygon[p++] = QPoint (x0 - 0.983*dx, y0 + 0.371*dy);
         polygon[p++] = QPoint (x0 - 1.000*dx, y0 + 0.500*dy);
         polygon[p++] = QPoint (x0 - 0.983*dx, y0 + 0.629*dy);
         polygon[p++] = QPoint (x0 - 0.933*dx, y0 + 0.750*dy);
         polygon[p++] = QPoint (x0 - 0.854*dx, y0 + 0.854*dy);
         polygon[p++] = QPoint (x0 - 0.750*dx, y0 + 0.933*dy);
         polygon[p++] = QPoint (x0 - 0.629*dx, y0 + 0.983*dy);
         polygon[p++] = QPoint (x0 - 0.500*dx, y0 + 1.000*dy);
         polygon[p++] = QPoint (x0 - 0.371*dx, y0 + 0.983*dy);
         polygon[p++] = QPoint (x0 - 0.250*dx, y0 + 0.933*dy);
         polygon[p++] = QPoint (x0 - 0.146*dx, y0 + 0.854*dy);
         polygon[p++] = QPoint (x0 - 0.067*dx, y0 + 0.750*dy);
         polygon[p++] = QPoint (x0 - 0.017*dx, y0 + 0.629*dy);

         polygon[p++] = QPoint (x0 - 0.000*dx, y0 + 0.500*dy);

         polygon[p++] = QPoint (x0 + 0.017*dx, y0 + 0.629*dy);
         polygon[p++] = QPoint (x0 + 0.067*dx, y0 + 0.750*dy);
         polygon[p++] = QPoint (x0 + 0.146*dx, y0 + 0.854*dy);
         polygon[p++] = QPoint (x0 + 0.250*dx, y0 + 0.933*dy);
         polygon[p++] = QPoint (x0 + 0.371*dx, y0 + 0.983*dy);
         polygon[p++] = QPoint (x0 + 0.500*dx, y0 + 1.000*dy);
         polygon[p++] = QPoint (x0 + 0.629*dx, y0 + 0.983*dy);
         polygon[p++] = QPoint (x0 + 0.750*dx, y0 + 0.933*dy);
         polygon[p++] = QPoint (x0 + 0.854*dx, y0 + 0.854*dy);
         polygon[p++] = QPoint (x0 + 0.933*dx, y0 + 0.750*dy);
         polygon[p++] = QPoint (x0 + 0.983*dx, y0 + 0.629*dy);
         polygon[p++] = QPoint (x0 + 1.000*dx, y0 + 0.500*dy);
         polygon[p++] = QPoint (x0 + 0.983*dx, y0 + 0.371*dy);
         polygon[p++] = QPoint (x0 + 0.933*dx, y0 + 0.250*dy);
         polygon[p++] = QPoint (x0 + 0.900*dx, y0 + 0.200*dy);

         painter.drawPolygon (polygon, p);
         break;

     default:
         // And just for fun ....
         dx = double (rect.right ()  - rect.left ())/100.0;
         dy = double (rect.bottom () - rect.top ())/100.0;
         x0 = rect.left ();
         y0 = rect.top ();

         // Mainland map
         p = 0;
         polygon[p++]  = QPoint (x0 + 10*dx,  y0 + 74*dy);
         polygon[p++]  = QPoint (x0 +  1*dx,  y0 + 37*dy);
         polygon[p++]  = QPoint (x0 + 35*dx,  y0 +  6*dy);
         polygon[p++]  = QPoint (x0 + 55*dx,  y0 +  6*dy);
         polygon[p++]  = QPoint (x0 + 55*dx,  y0 + 13*dy);
         polygon[p++]  = QPoint (x0 + 68*dx,  y0 + 20*dy);
         polygon[p++]  = QPoint (x0 + 74*dx,  y0 +  2*dy);
         polygon[p++]  = QPoint (x0 + 98*dx,  y0 + 38*dy);
         polygon[p++]  = QPoint (x0 + 98*dx,  y0 + 58*dy);
         polygon[p++]  = QPoint (x0 + 84*dx,  y0 + 81*dy);
         polygon[p++]  = QPoint (x0 + 69*dx,  y0 + 81*dy);
         polygon[p++]  = QPoint (x0 + 42*dx,  y0 + 59*dy);
         polygon[p++]  = polygon[0];            // close loop
         painter.drawPolygon (polygon, p);

         // Now do the map of tassie
         p = 0;
         polygon[p++]  = QPoint (x0 + 70*dx,  y0 + 86*dy);
         polygon[p++]  = QPoint (x0 + 83*dx,  y0 + 86*dy);
         polygon[p++]  = QPoint (x0 + 76*dx,  y0 + 98*dy);
         polygon[p++]  = polygon[0];            // close loop
         painter.drawPolygon (polygon, p);
         break;
   }

   // Get the required text (if any).
   //
   text = this->calcTextImage ();
   if (!text.isEmpty ()) {

      QFont pf (this->font ());
      painter.setFont (pf);
      QFontMetrics fm = painter.fontMetrics ();

      const int textWidth  = fm.horizontalAdvance (text);
      const int textHeight = fm.height ();

      // baseLineOffset is the difference between the bottom of an 'normal' char
      // and the bottom of a "g", "j", "p", "q" and/or "y" char.
      //
      const int baseLineOffset = ((textHeight * 6) + 19) / 38;  /// TODO: fine tune this ratio.

      int xpos;      // holds the text bottom-left x position.
      int ypos;      // holds the text bottom-left y position.

      // Set the default position to the centre of the widget.
      //
      xpos = (this->width () - textWidth) / 2;
      ypos = (this->height() + textHeight) / 2 - baseLineOffset;

      // Calculate the indents - we do this whether required or not.
      //
      int xIndent = this->indent;  // pixel indent.
      int yIndent = this->indent;

      if (this->indent < 0) {
         // Indent -ve, use the x standard (as per QLabel).
         //
         xIndent = fm.horizontalAdvance ("x") / 2;
         yIndent = baseLineOffset;
      }

      // Take acount of the edgeWidth - usually black and will obsure text.
      // But this also makes QShape behave like QLabel where edgeWidth is
      // equivilent to QFrame lineWidth.
      //
      xIndent += this->edgeWidth;
      yIndent += this->edgeWidth;

      // Modify the postion to reflected alignment and indent.
      //
      if (this->alignment & Qt::AlignTop) {
         ypos = yIndent + textHeight - baseLineOffset;
      } else if (this->alignment & Qt::AlignBottom) {
         ypos = this->height () - yIndent - baseLineOffset;
      }

      if (this->alignment & Qt::AlignLeft) {
         xpos = xIndent;
      } else if (this->alignment & Qt::AlignRight) {
         xpos = (this->width () - (xIndent + textWidth));
      }

      if (!washedOut) {
         pen.setColor (QEUtilities::fontColour (colour));
      } else {
         pen.setColor (QColor (140, 140, 140, 255));   // gray
      }

      painter.setPen (pen);

      // If text too wide, then ensure we show most significant part.
      //
      painter.drawText (MAX (this->edgeWidth, xpos), ypos, text);
   }
}

//------------------------------------------------------------------------------
//
void QSimpleShape::flashTimeout (const bool isOn)
{   
   this->flashStateIsOn = isOn;  // as opposed to off.

   bool anyFlash = false;
   for (int j = 0; j < ARRAY_LENGTH (this->flashList); j++) {
      if (this->flashList [j]) {
         anyFlash = true;
         break;
      }
   }

   // Only call if at least one state is marked as flashing.
   //
   if (anyFlash) {
      this->update ();
   }
}

//------------------------------------------------------------------------------
//
QString QSimpleShape::calcTextImage ()
{
   QString result;

   switch (this->getTextFormat ()) {

      case QSimpleShape::FixedText:
         result = this->getFixedText ();
         break;

      case QSimpleShape::StateSet:
         result = this->stateSet.value (this->getValue (), "");
         break;

      case QSimpleShape::PvText:
      case QSimpleShape::LocalEnumeration:
         result = this->getItemText ();   // NOTE: This is a dispatching call.
         break;

      default:
         result = "?";
   }

   return result;
}

//------------------------------------------------------------------------------
//
QString QSimpleShape::getItemText ()
{
   return "";
}

//------------------------------------------------------------------------------
//
QColor QSimpleShape::getItemColour ()
{
   const int mv = this->getValue ();
   return this->getColourProperty (mv);
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setValue (const bool value)
{
   int n = value ? 1 : 0;
   this->setValue (n);
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setValue (const int valueIn)
{
   // We want the modulo value
   // Note: % operator is remainder not modulo, so need to be smarter.
   //
   this->value = valueIn % this->modulus;
   if (this->value < 0) {
      this->value += this->modulus;
   }
   this->update ();
}

//------------------------------------------------------------------------------
//
int QSimpleShape::getValue () const
{
   return this->value % this->modulus;   // pedantic.
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setModulus (const int value)
{
   this->modulus = LIMIT (value, 2, NUMBER_OF_STATES);

   // Ensure value within the new range.
   // Both operands are postive, rem (%) will do.
   //
   this->value =  this->value % this->modulus;
   this->update ();
}

//------------------------------------------------------------------------------
//
int QSimpleShape::getModulus () const
{
   return this->modulus;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setEdgeWidth (const int edgeWidthIn)
{
   this->edgeWidth = LIMIT (edgeWidthIn, 0, 20);
   this->update ();
}

//------------------------------------------------------------------------------
//
int QSimpleShape::getEdgeWidth () const
{
   return this->edgeWidth;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setSemiCycles (const int semiCyclesIn)
{
   this->semiCycles = LIMIT (semiCyclesIn, 1, 30);
   if ((this->shape == snakeHorizontal) || (this->shape == snakeVertical)) {
       this->update ();
   }
}

//------------------------------------------------------------------------------
//
int QSimpleShape::getSemiCycles () const
{
   return this->semiCycles;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setPercentSize (const int percentSizeIn)
{
   this->percentSize = LIMIT (percentSizeIn, 1, 50);
    if ((this->shape == snakeHorizontal) ||
        (this->shape == snakeVertical) ||
        (this->shape == plus) ||
        (this->shape == cross)) {
       this->update ();
   }
}

//------------------------------------------------------------------------------
//
int QSimpleShape::getPercentSize () const
{
   return this->percentSize;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setCentreAngle (const int angleIn)
{
   this->centreAngle = angleIn;
   while (this->centreAngle > +180) this->centreAngle -= 360;
   while (this->centreAngle < -180) this->centreAngle += 360;
   if ((this->shape == roundpie) || (this->shape == pie)) {
      this->update ();
   }
}

//------------------------------------------------------------------------------
//
int QSimpleShape::getCentreAngle () const
{
   return this->centreAngle;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setSpanAngle (const int spanIn)
{
   this->spanAngle = LIMIT (spanIn, 0, 360);
   if ((this->shape == roundpie) || (this->shape == pie)) {
       this->update ();
   }
}

//------------------------------------------------------------------------------
//
int QSimpleShape::getSpanAngle () const
{
   return this->spanAngle;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setEdgeStyle (const Qt::PenStyle penStyleIn)
{
   this->edgeStyle = penStyleIn;
   this->update ();
}

//------------------------------------------------------------------------------
//
Qt::PenStyle QSimpleShape::getEdgeStyle () const
{
   return this->edgeStyle;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setEdgeColour (const QColor edgeColourIn)
{
   this->edgeColour = edgeColourIn;
   this->update ();
}

//------------------------------------------------------------------------------
//
QColor QSimpleShape::getEdgeColour () const
{
   return this->edgeColour;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setStateSet (const QStringList& stateSetIn)
{
   this->stateSet = stateSetIn;

   // Pad/truncate as required.
   //
   while (this->stateSet.count () > NUMBER_OF_STATES) {
      this->stateSet.removeLast ();
   }
   while (this->stateSet.count () < NUMBER_OF_STATES) {
      this->stateSet.append ("");
   }

   if (this->getTextFormat () == StateSet) {
      this->update ();
   }
}

//------------------------------------------------------------------------------
//
QStringList QSimpleShape::getStateSet () const
{
   return this->stateSet;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setShape (const int value)
{
   this->setShape (Shapes (value));
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setShape (const Shapes shapeIn)
{
   if (this->shape != shapeIn) {
      this->shape = shapeIn;
      this->update ();
   }
}

//------------------------------------------------------------------------------
//
QSimpleShape::Shapes QSimpleShape::getShape () const
{
   return this->shape;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setTextFormat (const TextFormats textFormatIn)
{
   if (this->textFormat != textFormatIn) {
      this->textFormat = textFormatIn;
      this->update ();
   }
}

//------------------------------------------------------------------------------
//
QSimpleShape::TextFormats QSimpleShape::getTextFormat () const
{
   return this->textFormat;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setFixedText (const QString& value)
{
   if (this->fixedText != value) {
      this->fixedText = value;
      if (this->getTextFormat () == FixedText) {
         this->update ();
      }
   }
}

//------------------------------------------------------------------------------
//
QString QSimpleShape::getFixedText () const
{
   return this->fixedText;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setAlignment (const Qt::Alignment alignmentIn)
{
   if (this->alignment != alignmentIn) {
      this->alignment = alignmentIn;
      this->update ();
   }
}

//------------------------------------------------------------------------------
//
Qt::Alignment QSimpleShape::getAlignment () const
{
   return this->alignment;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setIndent (const int indentIn)
{
   const int tmp = MAX (-1, indentIn);
   if (this->indent != tmp) {
      this->indent = tmp;
      this->update ();
   }
}

//------------------------------------------------------------------------------
//
int QSimpleShape::getIndent () const
{
   return this->indent;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setFlashRate (const QEScanTimers::ScanRates flashRateIn)
{
   const char* member = SLOT (flashTimeout (const bool));

   if (this->flashRate != flashRateIn) {
      // Disconnect old flash rate signal and connect new flash rate signal.
      //
      QEScanTimers::detach (this, member);
      this->flashRate = flashRateIn;
      QEScanTimers::attach (this, member, this->flashRate);
   }
}

//------------------------------------------------------------------------------
//
QEScanTimers::ScanRates QSimpleShape::getFlashRate () const
{
   return this->flashRate;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setFlashOffColour (const QColor flashOffColourIn)
{
   this->flashOffColour = flashOffColourIn;
   this->update ();
}

//------------------------------------------------------------------------------
//
QColor QSimpleShape::getFlashOffColour () const
{
   return this->flashOffColour;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setIsActive (const bool isActiveIn)
{
   this->isActive = isActiveIn;
   this->update ();
}

//------------------------------------------------------------------------------
//
bool QSimpleShape::getIsActive () const
{
   return this->isActive;
}

//------------------------------------------------------------------------------
//
void QSimpleShape::setColourProperty (const int slot, const QColor colour)
{
   if ((slot >= 0) && (slot < NUMBER_OF_STATES)) {
      if (this->colourList[slot] != colour) {
         this->colourList[slot] = colour;
         if (this->getValue () == slot) {
            this->update ();
         }
      }
   }
}

//------------------------------------------------------------------------------
//
QColor QSimpleShape::getColourProperty (const int slot) const
{
   QColor result;

   if ((slot >= 0) && (slot < NUMBER_OF_STATES)) {
      result = this->colourList[slot];
   } else {
      result = QColor (0, 0, 0, 255);
   }
   return result;
}


//------------------------------------------------------------------------------
//
void QSimpleShape::setFlashProperty (const int slot, const bool isFlashing)
{
   if ((slot >= 0) && (slot < NUMBER_OF_STATES)) {
      if (this->flashList [slot] != isFlashing) {
         this->flashList [slot] = isFlashing;
         this->update ();
      }
   }
}

//------------------------------------------------------------------------------
//
bool QSimpleShape::getFlashProperty (const int slot) const
{
   bool result = false;

   if ((slot >= 0) && (slot < NUMBER_OF_STATES)) {
      result = this->flashList [slot];
   }
   return result;
}

// end
