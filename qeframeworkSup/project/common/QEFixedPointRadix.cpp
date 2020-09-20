/*  QEFixedPointRadix.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2020 Australian Synchrotron.
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

#include "QEFixedPointRadix.h"
#include <math.h>
#include <QDebug>
#include <QECommon.h>
#include <QEPlatform.h>

#define DEBUG qDebug () << "QEFixedPointRadix" << __LINE__ << __FUNCTION__ << "  "

#define NUMBER_OF_RADICES  4

//-----------------------------------------------------------------------------
// Provides the radix values.
// Must be consistant with enum Radicies specification.
//
const static int radix_value_list [NUMBER_OF_RADICES] = { 10, 16, 8, 2 };

// Must be consistant with enum Separators specification.
//
const static char separatorChars [] = " ,_ ";


//-----------------------------------------------------------------------------
//
QEFixedPointRadix::QEFixedPointRadix (QObject* parent) : QObject (parent)
{
   this->radix = Decimal;
   this->separator = None;
}

//-----------------------------------------------------------------------------
//
QEFixedPointRadix::QEFixedPointRadix (const Radicies radixIn,
                                      const Separators separatorIn,
                                      QObject* parent) : QObject (parent)
{
   this->radix = radixIn;
   this->separator = separatorIn;
}

//-----------------------------------------------------------------------------
//
QEFixedPointRadix::~QEFixedPointRadix ()
{
   // place holder
}

//-----------------------------------------------------------------------------
//
void QEFixedPointRadix::setRadix (const Radicies radixIn)
{
   this->radix = radixIn;
}

//-----------------------------------------------------------------------------
//
QEFixedPointRadix::Radicies QEFixedPointRadix::getRadix () const
{
   return this->radix;
}

//-----------------------------------------------------------------------------
//
void QEFixedPointRadix::setSeparator (const Separators separatorIn)
{
   this->separator = separatorIn;
}

//-----------------------------------------------------------------------------
//
QEFixedPointRadix::Separators QEFixedPointRadix::getSeparator () const
{
   return this->separator;
}

//-----------------------------------------------------------------------------
//
int QEFixedPointRadix::getRadixValue () const
{
   return radix_value_list [this->radix];
}

//------------------------------------------------------------------------------
//
bool QEFixedPointRadix::isRadixDigit (const QChar qc) const
{
   bool result;
   char c = qc.toLatin1 ();

   switch (this->getRadix()) {
      case Decimal:
         result = ((c >= '0') && (c <= '9'));
         break;

      case Hexadecimal:
         result = ((c >= '0') && (c <= '9')) ||
                  ((c >= 'A') && (c <= 'F')) ||
                  ((c >= 'a') && (c <= 'f'));
         break;

      case Octal:
         result = ((c >= '0') && (c <= '7'));
         break;

      case Binary:
         result = ((c >= '0') && (c <= '1'));
         break;

      default:
         result = false;
         break;
   }
   return result;
}


//-----------------------------------------------------------------------------
//
QString QEFixedPointRadix::toString (const double value,
                                     const bool sign,
                                     const int zeros,
                                     const int precision) const
{
   static const char radixChars [] = "0123456789ABCDEF";
   static const int  separatorSizes [NUMBER_OF_RADICES] = { 3, 4, 3, 4 };

   const double dblRadix = double (radix_value_list [this->radix]);

   // Sanity checks/specials.
   //
   if (QEPlatform::isNaN (value)) {
      return "nan";
   }

   if (QEPlatform::isInf (value)) {
      if (value >= 0.0) {
         if (sign) {
            return "+inf";
         } else {
            return "inf";
         }
      } else {
         return "-inf";
      }
   }

   QString result;
   double work;
   int mostSig;
   int s;
   double prs;
   int t;

   result = "";

   // Do leading sign if needed or requested.
   //
   if (value >= 0.0) {
      if (sign) {
          result.append ('+');
      }
   } else {
       result.append ('-');
   }

   work = ABS (value);
   mostSig = 0;
   while (pow (dblRadix, int (mostSig + 1)) < work) {
      mostSig++;
   }

   // Alternative:
   // for (t = r; t < work; t = t*r) {
   //    mostSig++;
   // }

   mostSig = MAX (mostSig, zeros - 1);

   // Round up by half the value of the least significant digit.
   //
   work = work + (pow ((1.0/dblRadix), precision) * 0.499999999);

   for (s = mostSig; s >= -precision; s--) {

      prs = pow (dblRadix, s);
      t = int (floor (work / prs));
      work = work - t*prs;

      result.append (radixChars [t]);

      // All done?
      //
      if (s <= -precision) break;

      if (s == 0) {
         result.append ('.');
      } else if (ABS (s) % separatorSizes [this->radix] == 0) {
         if (separator != None) {
            result.append (separatorChars [this->separator]);
         }
      }
   }

   return result;
}

//-----------------------------------------------------------------------------
//
double QEFixedPointRadix::toValue (const QString& image,
                                   bool& okay) const
{
   const int intRadix = radix_value_list [this->radix];
   const double dblRadix = double (intRadix);

   double result;
   bool isNegative;
   bool isPoint;
   bool signIsAllowed;
   int scale;
   int j;
   char c;
   int d;

   okay = false;   // ensure not erroneous
   result = 0.0;

   isNegative = false;
   isPoint = false;
   scale = 0;

   // We could, and prob should, be more strict with the syntax checking.
   //
   signIsAllowed = true;

   for (j = 0; j < image.length (); j++) {
      c = image [j].toLatin1 ();
      d = 0;

      switch (c) {
         case ' ':
         case ',':
         case '_':
             // null
             break;

         case '+':
            if (!signIsAllowed) {
               return 0.0;
            }
            signIsAllowed = false;
            break;

         case '-':
            if (!signIsAllowed) {
               return 0.0;
            }
            isNegative = true;
            signIsAllowed = false;
            break;

         case '.':
          if (isPoint) {
             return 0.0;
          }
          isPoint = true;
          break;

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
            d = int (c) - int ('0');
            result = (result*intRadix) + d;
            if (isPoint) scale--;
            break;

         case 'A':
         case 'B':
         case 'C':
         case 'D':
         case 'E':
         case 'F':
            d = int (c) - int ('A') + 10;
            result = (result*intRadix) + d;
            if (isPoint) scale--;
            break;

         case 'a':
         case 'b':
         case 'c':
         case 'd':
         case 'e':
         case 'f':
            d = int (c) - int ('a') + 10;
            result = (result*intRadix) + d;
            if (isPoint) scale--;
            break;

         default:
            return 0.0;
            break;
      }
      if (d >= intRadix) {
         // digit is out of range
         return 0.0;
      }
   }

   // Scale result.
   //
   result = result * pow (dblRadix, scale);

   // Apply sign
   //
   if (isNegative) result = -result;

   // All good - indicate this to caller.
   //
   okay = true;
   return result;
}

// end
