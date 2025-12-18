/*  QBitStatus.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2011-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

//  Bit wise display of integer values.

#include "QBitStatus.h"
#include <QDebug>
#include <QECommon.h>

#define DEBUG qDebug () << "QBitStatus" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QBitStatus::QBitStatus( QWidget *parent ) : QWidget (parent)
{
   // Set up data
   //
   this->mBorderColour  = QColor (  0,   0,  32);   // dark dark blue
   this->mOffColour     = QColor (255,   0,   0);   // red
   this->mOnColour      = QColor (0,   255,   0);   // green
   this->mInvalidColour = QColor (255, 182, 128);   // orange
   this->mClearColour   = QColor (192, 192, 192, 0);   // gray, but clear

   this->mDrawBorder = true;
   this->mNumberOfBits = 8;      // 1 .. 32
   this->mGap = 0;               // 0 .. 80
   this->mShift = 0;             // 0 .. 32
   this->mIsValid = true;
   this->mIsActive = true;
   this->mValue = 0;
   this->mOrientation = Qt::Horizontal;
   this->mInvertedAppearance = false;
   this->mShape = Rectangle;
   this->mOnClearMask = 0x00000000;
   this->mOffClearMask = 0x00000000;
   this->mReversePolarityMask = 0x00000000;

   // Do this only once, not in paintEvent as it causes another paint event.
   //
   this->setAutoFillBackground (false);
   this->setBackgroundRole (QPalette::NoRole);
}

//------------------------------------------------------------------------------
// place holder
QBitStatus::~QBitStatus () { }

//------------------------------------------------------------------------------
//  Define default size for this widget class.
//
QSize QBitStatus::sizeHint () const
{
   return QSize (48, 16);
}

//------------------------------------------------------------------------------
//
QColor QBitStatus::getBorderPaintColour () const
{
   return this->getIsActive()
         ? this->mBorderColour : QEUtilities::blandColour (this->mBorderColour);
}

//------------------------------------------------------------------------------
//
QColor QBitStatus::getOffPaintColour () const
{
   return this->getIsActive ()
         ? this->mOffColour : QEUtilities::blandColour (this->mOffColour);
}

//------------------------------------------------------------------------------
//
QColor QBitStatus::getOnPaintColour () const
{
   return this->getIsActive ()
         ? this->mOnColour : QEUtilities::blandColour (this->mOnColour);
}

//------------------------------------------------------------------------------
//
QColor QBitStatus::getInvalidPaintColour () const
{
   return this->getIsActive ()
         ? this->mInvalidColour : QEUtilities::blandColour (this->mInvalidColour);
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

//-----------------------------------------------------------------------------
// Draw the bit status.
//
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
   // dependent upon widget orientation and/or inverted appearance.
   //
   switch (this->mOrientation) {

      case Qt::Horizontal:
         if (!this->mInvertedAppearance) {
            // LSB On Right
            // Note: Pixels are  in range (0 .. size - 1).
            //
            right = this->width () - 1;
            bottom = this->height () - 1;
            painter.translate (0.0, 0.0);
            painter.rotate (0.0);
         } else {
            // LSB On Left
            right = this->width () - 1;
            bottom = this->height () - 1;
            painter.translate (this->width () - 1, this->height () - 1);
            painter.rotate (180.0);    // clock-wise (degrees)
         }
         break;

      case Qt::Vertical:
         if (!this->mInvertedAppearance) {
            // LSB On Bottom
            right = this->height () - 1;
            bottom = this->width () - 1;
            painter.translate (this->width () - 1, 0.0);
            painter.rotate (90.0);    // clock wise (degrees)
         } else {
            // LSB On Top
            right = this->height () - 1;
            bottom = this->width () - 1;
            painter.translate (0.0, this->height () - 1);
            painter.rotate (270.0);    // clock wise (degrees)
         }
         break;

      default:
         // report an error.
         //
         DEBUG << "Unexpected orientation: " << int (this->mOrientation);
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
type QBitStatus::get##name () const {                        \
   return this->m##name;                                     \
}


// NOTE: we have to qualify function return type here.
//
PROPERTY_ACCESS (Qt::Orientation, Orientation, value)

PROPERTY_ACCESS (bool, InvertedAppearance, value)

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

//-----------------------------------------------------------------------------
//
QString QBitStatus::getOnClearMask () const
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

//-----------------------------------------------------------------------------
//
QString QBitStatus::getOffClearMask () const
{
   return this->intToMask (this->mOffClearMask);
}

//=============================================================================
//
void QBitStatus::setReversePolarityMask (const QString value)
{
   const int temp = this->maskToInt (value);

   if (this->mReversePolarityMask != temp) {
      this->mReversePolarityMask = temp;
      this->update ();
   }
}

//-----------------------------------------------------------------------------
//
QString QBitStatus::getReversePolarityMask () const
{
   return this->intToMask (this->mReversePolarityMask);
}

//=============================================================================
// Private static functions
//=============================================================================
//
// Converts integer to a binary mask of hexadecimal characters.
//
QString QBitStatus::intToMask (const int n)
{
   QString result;

   const int x1 = (n>>24)&255;
   const int x2 = (n>>16)&255;
   const int x3 = (n>>8)&255;
   const int x4 = (n>>0)&255;

   result = QString::asprintf ("%02X-%02X-%02X-%02X", x1, x2, x3, x4);
   return result;
}

//-----------------------------------------------------------------------------
// Converts a hexadecimal character mask to an integer;
//
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
