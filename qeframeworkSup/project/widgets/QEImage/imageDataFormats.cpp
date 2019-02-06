/*  imageDataFormats.cpp
 * 
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2018 Australian Synchrotron
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

static const char* images [imageDataFormats::NUMBER_OF_FORMATS] [2] = {
    { "Mono",     "Monochrome" },           // MONO
    { "BayerGB",  "Bayer (Green/Blue)" },   // BAYERGB;
    { "BayerBG",  "Bayer (Blue/Green)" },   // BAYERBG;
    { "BayerGR",  "Bayer (Green/Red)" },    // BAYERGR;
    { "BayerRG",  "Bayer (Red/Green)" },    // BAYERRG;
    { "RGB1",     "8 bit RGB" },            // RGB1;
    { "RGB2",     "RGB2???" },              // RGB2;
    { "RGB3",     "RGB3???" },              // RGB3;
    { "YUV444",   "???bit YUV444" },        // YUV444;
    { "YUV422",   "???bit YUV422" },        // YUV422;
    { "YUV421"    "???bit YUV421" }         // YUV421;
};

//------------------------------------------------------------------------------
// static
bool imageDataFormats::convertToFormatOption (const QString& text,
                                              formatOptions& format)
{
    bool result = false;   // hypothesize no match

    // Interpret Area detector formats
    //
    for (int j = 0 ; j < NUMBER_OF_FORMATS; j++) {
        if (!text.compare (images [j][0])) {
            format = formatOptions (j);
            result = true;
        }
    }

    if (!result) {
        // Do a Bayer special
        //
        if (!text.compare ("Bayer")) {
            format = imageDataFormats::BAYERRG;
            result = true;
        }
    }

    return result;
}

//------------------------------------------------------------------------------
// static
QString imageDataFormats::getFormatInformation (const formatOptions format)
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
