/*  imageDataFormats.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2018 Australian Synchrotron
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
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QE_IMAGE_DATA_FORMATS_H
#define QE_IMAGE_DATA_FORMATS_H

#include <QString>

// Video format options
/// \enum formatOptions
/// Video format options
class imageDataFormats
{
public:
   enum formatOptions {
      MONO,              ///< Grey scale
      BAYERGB,           ///< Colour (Bayer Green Blue)
      BAYERBG,           ///< Colour (Bayer Blue Green)
      BAYERGR,           ///< Colour (Bayer Green Red)
      BAYERRG,           ///< Colour (Bayer Red Green)
      RGB1,              ///< Colour (RGB ???)
      RGB2,              ///< Colour (RGB ???)
      RGB3,              ///< Colour (RGB ???)
      YUV444,            ///< Colour (???)
      YUV422,            ///< Colour (???)
      YUV421,            ///< Colour (???)
      NUMBER_OF_FORMATS  // Must be last
   };

   // Returns false if given text does not match one of the expected formats
   //
   static bool convertToFormatOption (const QString& text,
                                      formatOptions& format);

   static QString getFormatInformation (const formatOptions format);
};

#endif // QE_IMAGE_DATA_FORMATS_H
