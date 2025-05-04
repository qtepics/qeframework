/*  imageProperties.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2015-2024 Australian Synchrotron
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

/*
 This class manages the image attributes required for generating a QImage from a QByteArray holding CA image data.
 It is used as the base class for the imageProcessor class.
 */

#include "imageProperties.h"


/// Construction. Set all image attributes to sensible defaults
imageProperties::imageProperties()
{
    rotation = QE::NoRotation;
    flipVert = false;
    flipHoz = false;

    formatOption = QE::Mono;
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

void imageProperties::setBitDepth( unsigned int bitDepthIn )
{
    // Invalidate pixel look up table if bit depth changes (it will be regenerated with the new depth when next needed)
    if( bitDepth != bitDepthIn )
    {
        pixelLookupValid = false;
    }
    bitDepth = bitDepthIn;
}


// Present information about the image.
// This is usefull when trying to determine why an image is not displaying well.
QString imageProperties::getInfoText() const
{
    // Build the image information string
    QString about;

    about.append( QObject::tr( "\nSize (bytes) of CA data array: %1" ).arg( imageData.size() ));
    about.append( QObject::tr( "\nSize (bytes) of CA data elements: %1" ).arg( imageDataSize ));
    about.append( QObject::tr( "\nWidth (pixels) taken from dimension variables or width variable: %1" ).arg( imageBuffWidth ));
    about.append( QObject::tr( "\nHeight (pixels) taken from dimension variables or height variable: %1" ).arg( imageBuffHeight ));
    about.append( QObject::tr( "\nPixel depth taken from data type variable, bit depth variable or bit depth property: %1" ).arg( bitDepth ));

    QString name = imageDataFormats::getFormatInformation (formatOption);
    about.append( QObject::tr( "\nExpected format: " ).append( name ));

    about.append( QObject::tr("\n\nFirst bytes of raw image data:\n   "));
    if( imageData.isEmpty() )
    {
        about.append( QObject::tr("No data yet.") );
    }
    else
    {
        int count = 20;
        if( count > imageData.size() )
        {
            count = imageData.size() ;
        }
        for( int i = 0; i < count; i++ )
        {
            about.append( QString( " %1" ).arg( (unsigned char)(imageData[i]) ));
        }
    }

    about.append( QObject::tr("\n\nFirst pixels of first row of image (after flipping, rotating and clipping:"));
    if( image.isNull() )
    {
        about.append( QObject::tr("\n   No image yet.") );
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
QE::ImageFormatOptions imageProperties::getFormat() const
{
    return formatOption;
}

// Set the current image format
void imageProperties::setFormat( QE::ImageFormatOptions formatIn )
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
    QE::ImageFormatOptions newFormatOption;
    const bool result = imageDataFormats::convertToFormatOption (text, newFormatOption);

    if( result ) {
        // Format text recognozed, use it
        formatOption = newFormatOption;
    }

    return result;  // return success or otherwise to caller
}

// end
