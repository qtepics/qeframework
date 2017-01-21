/*  QBitStatus.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at
 *  the Australian Synchrotron.
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
 *  Copyright (c) 2011  Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

//  Bit wise display of integer values.

/*
  This class is a BitStatus widget based on directly QWidget.
  It provides similar functionality to that provided by the edm/medm/dephi
  widgets of the same name.
 */

#include <QBitStatus.h>
#include <QECommon.h>

//------------------------------------------------------------------------------
//
QBitStatus::QBitStatus( QWidget *parent ) : QWidget (parent)
{
   // Set up data
   //
   mBorderColour  = QColor (  0,   0,  32);   // dark dark blue
   mOffColour     = QColor (255,   0,   0);   // red
   mOnColour      = QColor (0,   255,   0);   // green
   mInvalidColour = QColor (255, 182, 128);   // orange
   mClearColour   = QColor (192, 192, 192, 0);   // gray, but clear

   mDrawBorder = true;
   mNumberOfBits = 8;      // 1 .. 32
   mGap = 0;               // 0 .. 80
   mShift = 0;             // 0 .. 32
   mIsValid = true;
   mIsActive = true;
   mValue = 0;
   mOrientation = LSB_On_Right;
   mShape = Rectangle;
   mOnClearMask = 0x00000000;
   mOffClearMask = 0x00000000;
   mReversePolarityMask = 0x00000000;

   // Do this only once, not in paintEvent as it causes another paint event.
   //
   setAutoFillBackground (false);
   setBackgroundRole (QPalette::NoRole);
}


//------------------------------------------------------------------------------
//  Define default size for this widget class.
//
QSize QBitStatus::sizeHint () const {
   return QSize (48, 16);
}

//------------------------------------------------------------------------------
//
QColor QBitStatus::getBorderPaintColour ()
{
   return this->getIsActive() ? this->mBorderColour : QColor (160, 160, 160);
}

//------------------------------------------------------------------------------
//
QColor QBitStatus::getOffPaintColour ()
{
   return this->getIsActive () ? this->mOffColour : QColor (220, 220, 220);
}

//------------------------------------------------------------------------------
//
QColor QBitStatus::getOnPaintColour ()
{
   return this->getIsActive () ? this->mOnColour : QColor (240, 240, 240);
}

//------------------------------------------------------------------------------
//
QColor QBitStatus::getInvalidPaintColour ()
{
   return this->getIsActive () ? this->mInvalidColour : QColor (200, 200, 200);
}


//------------------------------------------------------------------------------
// Note: drawItem adds pen width on right and bottom, from the help info:
// "A stroked rectangle has a size of rectangle.size() plus the pen width."
//
// This function un-does this "helpful feature".
// The drawn rectangle/ellipse IS bounded by the specified rect.
//
void QBitStatus::drawItem (QPainter & painter, const QRect & rect)
{
   // Round down top-left offset, round up botton-right offset.
   //
   const int pen_width =  painter.pen ().width ();
   const int tl = pen_width / 2;
   const int br = pen_width - tl;

   QRect r;
   int d;
   int c1, c2;

   r.setTop    (rect.top ()    + tl);
   r.setLeft   (rect.left ()   + tl);
   r.setRight  (rect.right ()  - br);
   r.setBottom (rect.bottom () - br);

   switch (this->mShape) {
      case Rectangle:
         painter.drawRect (r);
         break;

      case Circle:
         d = rect.width () - rect.height ();
         if (d > 0) {
            c1 = d / 2;
            c2 = d - c1;
            r.setLeft   (rect.left ()  + c1);
            r.setRight  (rect.right () - c2);
         } else if (d < 0) {
            d = -d;
            c1 = d / 2;
            c2 = d - c1;
            r.setTop    (rect.top ()    + c1);
            r.setBottom (rect.bottom () - c2);
         }
         painter.drawEllipse (r);
         break;

      default:
         break;
   }

}

/* ---------------------------------------------------------------------------
 *  Draw the bit status.
 */
void QBitStatus::paintEvent (QPaintEvent *)
{
   QPainter painter (this);
   QPen pen;
   QBrush brush;
   QRect bit_area;
   int j;
   int bottom;
   int left;
   int right;
   double length;
   double bitSpacing;
   double offset;
   int useGap;
   int work;
   int onApplies;
   int offApplies;

   pen.setWidth (1);
   brush.setStyle (Qt::SolidPattern);

   // Draw everything with antialiasing off.
   //
   painter.setRenderHint (QPainter::Antialiasing, false);

   // Set right and bottom; and also apply translation and rotation
   // dependent upon widget orientation.
   //
   switch (mOrientation) {

      case LSB_On_Right:
         // Note: Pixels are  in range (0 .. size - 1).
         //
         right = this->width () - 1;
         bottom = this->height () - 1;
         painter.translate (0.0, 0.0);
         painter.rotate (0.0);
         break;

      case LSB_On_Bottom:
         right = this->height () - 1;
         bottom = this->width () - 1;
         painter.translate (this->width () - 1, 0.0);
         painter.rotate (90.0);    // clock wise (degrees)
         break;

      case LSB_On_Left:
         right = this->width () - 1;
         bottom = this->height () - 1;
         painter.translate (this->width () - 1, this->height () - 1);
         painter.rotate (180.0);    // clock wise (degrees)
         break;

      case LSB_On_Top:
         right = this->height () - 1;
         bottom = this->width () - 1;
         painter.translate (0.0, this->height () - 1);
         painter.rotate (270.0);    // clock wise (degrees)
         break;

      default:
         // report an error??
         //
         right = this->width () - 1;
         bottom = this->height () - 1;
         break;
   }

   left = 0;

   // We do the basic draw from right to left, i.e. LSB_On_Right.
   // The previously set translation and rotation looks after the rest.
   //
   bit_area.setTop (0);
   bit_area.setBottom (bottom);

   // Copy the specified gap
   //
   useGap = this->mGap;

   // Don't allow gap to overwhelm the actual drawn bits.
   //
   while ((useGap > 0) && ((this->mNumberOfBits * useGap) > (3 * right / 4))) {
       useGap--;
   }

   //
   if (this->mDrawBorder) {
      useGap -= 1;  // subtract pen width, i.e. allow boaders to overlap.
   }

   // Calculate the available (floating) pixel length.
   //
   length = double (right - left + 1);

   // Calculate the (floating) pixel size per bit.
   //
   bitSpacing = length / this->mNumberOfBits;

   // Calucate of edge offset from the centre.
   // - 1 for the pen width.
   //
   offset = (bitSpacing - 1.0 - useGap) / 2.0;

   work = (mValue >> mShift) ^ mReversePolarityMask;
   onApplies  = (-1) ^ mOnClearMask;
   offApplies = (-1) ^ mOffClearMask;

   for (j = mNumberOfBits - 1; j >= 0;  j--) {
      double centre;
      QColor bitColour;

      // Calculate the centre, and then and set left and right of this bit.
      //
      centre = left - 0.5 + (j + 0.5)*bitSpacing;

      // Calucalte size, but constrain to be within left to right.
      //
      bit_area.setLeft  (MAX (left,  QEUtilities::roundToInt (centre - offset)));
      bit_area.setRight (MIN (right, QEUtilities::roundToInt (centre + offset)));

      if (mIsValid) {

         if ((work & 1) == 1) {
            // Bit is on
            if ((onApplies & 1) == 1) {
               bitColour = this->getOnPaintColour ();
            }  else {
               bitColour = this->mClearColour;
            }
         } else {
            // Bit is off
            if ((offApplies & 1) == 1) {
               bitColour = this->getOffPaintColour ();
            }  else {
               bitColour = this->mClearColour;
            }
         }

      } else {
         bitColour = this->getInvalidPaintColour ();
      }

      brush.setColor (bitColour);
      painter.setBrush (brush);

      // Set up the pen and brush (color, thickness, etc.)
      //
      if (this->mDrawBorder) {
         pen.setColor (this->getBorderPaintColour ());
      } else {
         pen.setColor (bitColour);
      }
      painter.setPen (pen);

      // Do the actual draw.
      //
      this->drawItem (painter, bit_area);

      // Pre-pare for next iteration through the loop.
      // We don't worry about checking for last time through the loop.
      //
      work = work >> 1;
      onApplies = onApplies >> 1;
      offApplies = offApplies >> 1;
   }
}

//=============================================================================
// Property functions
//=============================================================================
//
// Standard propery access macro.
//
#define PROPERTY_ACCESS(type, name, convert)                 \
                                                             \
void QBitStatus::set##name (const type value)  {             \
   type temp;                                                \
   temp = convert;                                           \
   if (this->m##name != temp) {                              \
      this->m##name = temp;                                  \
      this->update ();                                       \
   }                                                         \
 }                                                           \
                                                             \
type QBitStatus::get##name () {                              \
   return this->m##name;                                     \
}


// NOTE: we have to qualify function return type here.
//
PROPERTY_ACCESS (QBitStatus::Orientations, Orientation, value)

PROPERTY_ACCESS (QBitStatus::Shapes, Shape, value)

PROPERTY_ACCESS (bool, IsValid, value)

PROPERTY_ACCESS (bool, IsActive, value)

PROPERTY_ACCESS (bool, DrawBorder, value)

PROPERTY_ACCESS (int, Value, value)

PROPERTY_ACCESS (int, NumberOfBits, LIMIT (value, 1, 32))

PROPERTY_ACCESS (int, Gap, LIMIT (value, 0, 80))

PROPERTY_ACCESS (int, Shift, LIMIT (value, 0, 31))

PROPERTY_ACCESS (QColor, BorderColour, value)

PROPERTY_ACCESS (QColor, OnColour, value)

PROPERTY_ACCESS (QColor, OffColour, value)

PROPERTY_ACCESS (QColor, InvalidColour, value)

PROPERTY_ACCESS (QColor, ClearColour, value)

#undef PROPERTY_ACCESS


//=============================================================================
// Non-standard propery access macro.
//=============================================================================
//
void QBitStatus::setOnClearMask (const QString value)
{
   int temp;

   temp = this->maskToInt (value);

   if (this->mOnClearMask != temp) {
      this->mOnClearMask = temp;
      this->update ();
   }
}

QString QBitStatus::getOnClearMask ()
{
   return this->intToMask (this->mOnClearMask);
}

//=============================================================================
//
void QBitStatus::setOffClearMask (const QString value)
{
   int temp;

   temp = this->maskToInt (value);

   if (this->mOffClearMask != temp) {
      this->mOffClearMask = temp;
      this->update ();
   }
}

QString QBitStatus::getOffClearMask ()
{
   return this->intToMask (this->mOffClearMask);
}

//=============================================================================
//
void QBitStatus::setReversePolarityMask (const QString value)
{
   int temp;

   temp = this->maskToInt (value);

   if (this->mReversePolarityMask != temp) {
      this->mReversePolarityMask = temp;
      this->update ();
   }
}

QString QBitStatus::getReversePolarityMask ()
{
   return this->intToMask (this->mReversePolarityMask);
}



//=============================================================================
// Private static functions
//=============================================================================
//
/* ---------------------------------------------------------------------------
 *  Converts integer to a binary mask of hexadecimal characters.
 */
QString QBitStatus::intToMask (int n)
{
   QString result ("");
   result.sprintf ("%02X-%02X-%02X-%02X", (n>>24)&255, (n>>16)&255, (n>>8)&255, (n>>0)&255);
   return result;
}

/* ---------------------------------------------------------------------------
 *  Converts a hexadecimal character mask to an integer;
 */
int QBitStatus::maskToInt (const QString mask)
{
   int result;
   int j;
   char c;

   result = 0;
   for (j = 0; j < mask.length(); j++) {
      c = mask.at (j).toLatin1 ();

      switch (c) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
         result = (result << 4) + int (c) - int ('0');
         break;

      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
         result = (result << 4) + int (c) - int ('A') + 10;
         break;

      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
         result = (result << 4) + int (c) - int ('a') + 10;
         break;

      default:
         // not a hex digit - don't care - just ignore.
         break;
      }
   }
   return result;
}

// end
