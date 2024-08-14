/*  imageDataFormats.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2018-2022 Australian Synchrotron
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

#include "imageDataFormats.h"
#include <QECommon.h>

// NOTE: The fore of this table must remain consistant with
// the QE::ImageFormatOptions definition out of QEEnums.h
//
static const char* images [] [2] = {
   { "Mono",     "Monochrome" },           // Mono
   { "XXXX",     "Bayer (Red/Green" },     // Bayer
   { "BayerGB",  "Bayer (Green/Blue)" },   // BayerGB
   { "BayerBG",  "Bayer (Blue/Green)" },   // BayerBG
   { "BayerGR",  "Bayer (Green/Red)" },    // BayerGR
   { "BayerRG",  "Bayer (Red/Green)" },    // BayerRG
   { "RGB1",     "8 bit RGB" },            // rgb1
   { "RGB2",     "RGB2???" },              // rgb2
   { "RGB3",     "RGB3???" },              // rgb3
   { "YUV444",   "???bit YUV444" },        // yuv444
   { "YUV422",   "???bit YUV422" },        // yuv422
   { "YUV421"    "???bit YUV421" }         // yuv421
};

#define NUMBER_OF_FORMATS  ARRAY_LENGTH(images)

//------------------------------------------------------------------------------
// static
bool imageDataFormats::convertToFormatOption (const QString& text,
                                              QE::ImageFormatOptions& format)
{
   bool result = false;   // hypothesize no match

   // Interpret Area detector formats
   //
   for (int j = 0 ; j < NUMBER_OF_FORMATS; j++) {
      if (!text.compare (images [j][0])) {
         format = QE::ImageFormatOptions (j);
         result = true;
      }
   }

   if (!result) {
      // Do a Bayer special
      //
      if (!text.compare ("Bayer")) {
         format = QE::BayerRG;
         result = true;
      }
   }

   return result;
}

//------------------------------------------------------------------------------
// static
QString imageDataFormats::getFormatInformation (const QE::ImageFormatOptions format)
{
   QString result;

   if ((format >= 0) && (format < NUMBER_OF_FORMATS)) {
      result = QString (images [format][1]);
   } else {
      result = QString ("Unknown format (%1)").arg (format);
   }

   return result;
}

// end
