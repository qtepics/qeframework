/*
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
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
 *  Copyright (c) 2014 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef IMAGEDATAFORMATS_H
#define IMAGEDATAFORMATS_H

// Video format options
/// \enum formatOptions
/// Video format options
namespace imageDataFormats
{
    enum formatOptions{ MONO,           ///< Grey scale
                        BAYERGB,        ///< Colour (Bayer Green Blue)
                        BAYERBG,        ///< Colour (Bayer Blue Green)
                        BAYERGR,        ///< Colour (Bayer Green Red)
                        BAYERRG,        ///< Colour (Bayer Red Green)
                        RGB1,           ///< Colour (RGB ???)
                        RGB2,           ///< Colour (RGB ???)
                        RGB3,           ///< Colour (RGB ???)
                        YUV444,         ///< Colour (???)
                        YUV422,         ///< Colour (???)
                        YUV421          ///< Colour (???)
                      };
}

#endif // IMAGEDATAFORMATS_H
