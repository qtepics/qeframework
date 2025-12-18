/*  imageProperties.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2015-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_IMAGE_PROPERTIES_H
#define QE_IMAGE_PROPERTIES_H

#include "QCaDateTime.h"
#include <QEEnums.h>
#include "imageDataFormats.h"
#include <brightnessContrast.h> // Remove this, or extract the general definitions used (eg rgbPixel) into another include file


// Class to manage core image processing by a seperate thread.
//
// Much of the information required for processing an image can me
// modified by the user or by incomming data and if would be:
// 1) Dangerous to modify this on the fly while the thread
//    processing an image is doing its work.
// 2) Complicated and not robust to lock access to individual
//    items in this set of information. Even if each bit of information
//    was managed well, the entire set may be inconsistant.
//
// This class captures all the information required to completely process
// an image during construction, then processes that image in a class function
// that cannot see anything outside this class. This means that the processing
// cannot accidentially reference anything that has not been captured in the
// constructor of this class.
//
class imagePropertiesCore
{
public:
    imagePropertiesCore( QByteArray imageDataIn,
                         unsigned long imageBuffWidthIn,
                         unsigned long imageBuffHeightIn,
                         int scanOptionIn,
                         unsigned long bytesPerPixelIn,
                         int pixelLowIn,
                         int pixelHighIn,
                         unsigned int bitDepthIn,
                         imageDisplayProperties::rgbPixel* pixelLookupIn,
                         QE::ImageFormatOptions formatOptionIn,
                         unsigned long imageDataSizeIn,
                         imageDisplayProperties* imageDisplayPropsIn,
                         unsigned int rotatedImageBuffWidthIn,
                         unsigned int rotatedImageBuffHeightIn );

    QImage buildImageCore();
private:
    QByteArray imageData;             // Buffer to hold original image data.
    unsigned long imageBuffWidth;     // Original image width (may be generated directly from a width variable, or selected from the relevent dimension variable)
    unsigned long imageBuffHeight;    // Original image height (may be generated directly from a width variable, or selected from the relevent dimension variable)
    int scanOption;
    unsigned long bytesPerPixel;      // Bytes in input data per pixel (imageDataSize * elementsPerPixel)
    int pixelLow;
    int pixelHigh;
    unsigned int bitDepth;
    imageDisplayProperties::rgbPixel* pixelLookup;
    QE::ImageFormatOptions formatOption;
    unsigned long imageDataSize;      // Size of elements in image data (originating from CA data type)
    imageDisplayProperties* imageDisplayProps;
    unsigned int rotatedImageBuffWidth;
    unsigned int rotatedImageBuffHeight;
};

/*!
 This class manages the image attributes required for generating a QImage from a QByteArray holding CA image data.
 It is used as the base class for the imageProcessor class.
 Note, while this class holds and manages all the information needed to process an image, a snapshot
 of all the information required for processing an image in a seperate thread is made by the imagePropertiesCore class.
 */
class imageProperties
{
public:
    imageProperties(); ///< Constructor

    // Image attribut set and get functions
    void setRotation( QE::RotationOptions rotationIn ){ rotation = rotationIn; }
    QE::RotationOptions getRotation() const { return rotation; }

    void setFlipVert( bool flipVertIn ){ flipVert = flipVertIn; }
    bool getFlipVert() const { return flipVert; }

    void setFlipHoz( bool flipHozIn ){ flipHoz = flipHozIn; }
    bool getFlipHoz() const { return flipHoz; }

    void setImageBuffWidth( unsigned long imageBuffWidthIn ){ imageBuffWidth = imageBuffWidthIn; }
    void setImageBuffHeight( unsigned long imageBuffHeightIn ){ imageBuffHeight = imageBuffHeightIn; }

    unsigned long getImageBuffWidth() const { return imageBuffWidth; }
    unsigned long getImageBuffHeight() const { return imageBuffHeight; }

    QE::ImageFormatOptions getFormat() const;
    void setFormat( QE::ImageFormatOptions formatIn );
    bool setFormat( const QString& text );

    void setBitDepth( unsigned int bitDepth );
    unsigned int getBitDepth() const { return bitDepth; }

    void setElementsPerPixel( unsigned long elementsPerPixelIn ){ elementsPerPixel = elementsPerPixelIn; }  //LOCK ACCESS???
    unsigned long getElementsPerPixel () const { return elementsPerPixel; }

    void setImageDisplayProperties( imageDisplayProperties* imageDisplayPropsIn ){ imageDisplayProps = imageDisplayPropsIn; }

    // Methods to force reprocessing
    void setWidthHeightFromDimensions();  ///< // Update the image dimensions (width and height) from the area detector dimension variables.
    void invalidatePixelLookup(){ pixelLookupValid = false; } ///< recalculate (when next requried) pixel summary information
    QString getInfoText() const;         ///< Generate textual information regarding the current image

protected:

    imageDisplayProperties* imageDisplayProps;  // Dialog for user manuipulation (and storage of) brightness and contrast and related info

    // Options
    QE::ImageFormatOptions formatOption;
    unsigned int bitDepth;

    // Image and related information
    unsigned long imageDataSize;      // Size of elements in image data (originating from CA data type)
    unsigned long elementsPerPixel;   // Number of data elements per pixel. Derived from image dimension 0 (only when there are three dimensions)
    unsigned long bytesPerPixel;      // Bytes in input data per pixel (imageDataSize * elementsPerPixel)
    QByteArray imageData;                 // Buffer to hold original image data.
    unsigned long receivedImageSize;  // Size as received on last CA update.
    QString previousMessageText;      // Previous message text - avoid repeats.
    QImage image;                     // Last image generated. Kept as the widget may ask for it again. For example, if the user is saving it.
#define IMAGEBUFF_BYTES_PER_PIXEL 4   // 4 bytes for Format_RGB32
    unsigned long imageBuffWidth;     // Original image width (may be generated directly from a width variable, or selected from the relevent dimension variable)
    unsigned long imageBuffHeight;    // Original image height (may be generated directly from a width variable, or selected from the relevent dimension variable)

    unsigned long numDimensions;      // Image data dimensions. Expected values are 0 (all dimensions values are ignored), 2 (one data element per pixel, dimensions are width x height), 3 (multiple data elements per pixel, dimensions are pixel x width x height)
    unsigned long imageDimension0;    // Image data dimension 0. If two dimensions, this is the width, if three dimensions, this is the pixel depth (the elements used to represent each pixel)
    unsigned long imageDimension1;    // Image data dimension 1. If two dimensions, this is the height, if three dimensions, this is the width
    unsigned long imageDimension2;    // Image data dimension 2. If two dimensions, this is not used, if three dimensions, this is the height


    // Pixel information
    bool pixelLookupValid;            // pixelLookup is valid. It is invalid if anything that affects the translation changes, such as pixel format, local brigHtness, etc
    imageDisplayProperties::rgbPixel pixelLookup[256];
    int pixelLow;
    int pixelHigh;

    // Clipping info (determined from cliping variable data)
    bool clippingOn;
    unsigned int clippingLow;
    unsigned int clippingHigh;

    // Flip rotate options
    QE::RotationOptions rotation;   // Rotation option
    bool flipVert;              // True if vertical flip option set
    bool flipHoz;               // True if horizontal flip option set
};

#endif // QE_IMAGE_PROPERTIES_H
