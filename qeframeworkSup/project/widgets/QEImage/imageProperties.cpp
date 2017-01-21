/*  imageProperties.cpp
 *
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
 *  Copyright (c) 2015 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 This class manages the image attributes required for generating a QImage from a QByteArray holding CA image data.
 It is used as the base class for the imageProcessor class.
 */

#include "imageProperties.h"


/// Construction. Set all image attributes to sensible defaults
imageProperties::imageProperties()
{
    rotation = ROTATION_0;
    flipVert = false;
    flipHoz = false;

    formatOption = imageDataFormats::MONO;
    bitDepth = 8;

    imageDataSize = 0;
    elementsPerPixel = 1;
    bytesPerPixel = 0;

    clippingOn = false;
    clippingLow = 0;
    clippingHigh = 0;

    pixelLookupValid = false;

    receivedImageSize = 0;

    // Set image size to zero
    // Image will not be presented until size is available
    imageBuffWidth = 0;
    imageBuffHeight = 0;

    numDimensions = 0;
    imageDimension0 = 0;
    imageDimension1 = 0;
    imageDimension2 = 0;

    imageDisplayProps = NULL;
}

// Update the image dimensions (width and height) from the area detector dimension variables.
// If an area detector dimension is available, then set up the width and height from the
// appropriate area detector dimension variables if available.
// This function is called when any area detector dimension related variable changes
// Width and height will not be touched untill the number of dimensions is avaliable, and
// will only be altered if there is a valid dimension.
void imageProperties::setWidthHeightFromDimensions()
{
    switch( numDimensions )
    {
        // 2 dimensions - one data element per pixel, dimensions are width x height
        case 2:
            if( imageDimension0 )
            {
                imageBuffWidth = imageDimension0;
            }
            if( imageDimension1 )
            {
                imageBuffHeight = imageDimension1;
            }
            break;

        // 3 dimensions - multiple data elements per pixel, dimensions are pixel x width x height
        case 3:
            if( imageDimension1 )
            {
                imageBuffWidth = imageDimension1;
            }
            if( imageDimension2 )
            {
                imageBuffHeight = imageDimension2;
            }
            break;
    }
}

// Present information about the image.
// This is usefull when trying to determine why an image is not displaying well.
QString imageProperties::getInfoText()
{
    // Build the image information string
    QString about;

    about.append( QString( "\nSize (bytes) of CA data array: %1" ).arg( imageData.count() ));
    about.append( QString( "\nSize (bytes) of CA data elements: %1" ).arg( imageDataSize ));
    about.append( QString( "\nWidth (pixels) taken from dimension variables or width variable: %1" ).arg( imageBuffWidth ));
    about.append( QString( "\nHeight (pixels) taken from dimension variables or height variable: %1" ).arg( imageBuffHeight ));
    about.append( QString( "\nPixel depth taken from data type variable, bit depth variable or bit depth property: %1" ).arg( bitDepth ));

    QString name;
    switch( formatOption )
    {
        case imageDataFormats::MONO:        name = "Monochrome";         break;
        case imageDataFormats::BAYERGB:     name = "Bayer (Green/Blue)"; break;
        case imageDataFormats::BAYERBG:     name = "Bayer (Blue/Green)"; break;
        case imageDataFormats::BAYERGR:     name = "Bayer (Green/Red)";  break;
        case imageDataFormats::BAYERRG:     name = "Bayer (red/Green)";  break;
        case imageDataFormats::RGB1:        name = "8 bit RGB";          break;
        case imageDataFormats::RGB2:        name = "RGB2???";            break;
        case imageDataFormats::RGB3:        name = "RGB3???";            break;
        case imageDataFormats::YUV444:      name = "???bit YUV444";      break;
        case imageDataFormats::YUV422:      name = "???bit YUV422";      break;
        case imageDataFormats::YUV421:      name = "???bit YUV421";      break;
    }

    about.append( QString( "\nExpected format: " ).append( name ));

    about.append( "\n\nFirst bytes of raw image data:\n   ");
    if( imageData.isEmpty() )
    {
        about.append( "No data yet." );
    }
    else
    {
        int count = 20;
        if( imageData.count() < count )
        {
            count = imageData.count() ;
        }
        for( int i = 0; i < count; i++ )
        {
            about.append( QString( " %1" ).arg( (unsigned char)(imageData[i]) ));
        }
    }

    about.append( "\n\nFirst pixels of first row of image (after flipping, rotating and clipping:");
    if( image.isNull() )
    {
        about.append( "\n   No image yet." );
    }
    else
    {
        int count = std::min(image.width(),20);
        for( int i = 0; i < count; i ++ )
        {
            QRgb pixel = image.pixel( i, 0 );
            about.append( QString( "\n   [%1, %2, %3]" ).arg( pixel&0xff )
                                                        .arg( (pixel>>8)&0xff )
                                                        .arg( (pixel>>16)&0xff ));
        }
    }

    return about;
}

// Return the current image format
imageDataFormats::formatOptions imageProperties::getFormat()
{
    return formatOption;
}

// Set the current image format
void imageProperties::setFormat( imageDataFormats::formatOptions formatIn )
{
    // Invalidate any pixel lookup information held
    if( formatOption != formatIn )
    {
        pixelLookupValid = false;
    }

    // Save the option
    formatOption = formatIn;
}

// Set the format based on the area detector format text
// Return true if identified (and the format is set) or false if the text is not recognised.
bool imageProperties::setFormat( const QString& text )
{
    imageDataFormats::formatOptions newFormatOption;

    // Interpret Area detector formats
    if     ( !text.compare( "Mono" ) )         newFormatOption = imageDataFormats::MONO;
    else if( !text.compare( "Bayer" ) )        newFormatOption = imageDataFormats::BAYERRG;
    else if( !text.compare( "BayerGB" ) )      newFormatOption = imageDataFormats::BAYERGB;
    else if( !text.compare( "BayerBG" ) )      newFormatOption = imageDataFormats::BAYERBG;
    else if( !text.compare( "BayerGR" ) )      newFormatOption = imageDataFormats::BAYERGR;
    else if( !text.compare( "BayerRG" ) )      newFormatOption = imageDataFormats::BAYERRG;
    else if( !text.compare( "RGB1" ) )         newFormatOption = imageDataFormats::RGB1;
    else if( !text.compare( "RGB2" ) )         newFormatOption = imageDataFormats::RGB2;
    else if( !text.compare( "RGB3" ) )         newFormatOption = imageDataFormats::RGB3;
    else if( !text.compare( "YUV444" ) )       newFormatOption = imageDataFormats::YUV444;
    else if( !text.compare( "YUV422" ) )       newFormatOption = imageDataFormats::YUV422;
    else if( !text.compare( "YUV421" ) )       newFormatOption = imageDataFormats::YUV421;

    // Unknown format text
    else
    {
        // !!! warn unexpected format
        return false;
    }

    // Format text recognozed, use it
    formatOption = newFormatOption;
    return true;
}
