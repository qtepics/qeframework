/*  imageProcessor.h
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

#ifndef QE_IMAGE_PROCESSOR_H
#define QE_IMAGE_PROCESSOR_H

#include <QByteArray>
#include <QString>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QReadWriteLock>
#include <imageProperties.h>

/*!
 This class generates images for presentation from raw image data and formatting
 information such as brightness, contrast, flip, rotate, canvas size, etc.
 The work is performed in a dedicated thread .
 */
class imageProcessor : public QThread, public imageProperties
{
    Q_OBJECT

public:
    // Construction
    imageProcessor();                                                   ///< Constructor
    ~imageProcessor();                                                  ///< Destructor

    // Image update
    void setImage( const QByteArray& imageIn, unsigned long dataSize ); ///< Save the image data for analysis processing and display
    void buildImage();                                                  ///< Generate a new image.

    // Set functions for dimensions and image attributes
    bool setWidth( unsigned long uValue );          ///< Set the image width
    bool setHeight( unsigned long uValue );         ///< Set the image height
    bool setNumDimensions( unsigned long uValue );  ///< Set the number of dimensions
    bool setDimension0( unsigned long uValue );     ///< Set the first dimension (width if two dimenstions, bytes per element if three dimensions)
    bool setDimension1( unsigned long uValue );     ///< Set the second dimension (height if two dimensions, width if three dimensions)
    bool setDimension2( unsigned long uValue );     ///< Set the third dimension (unused if two dimensions, height if three dimensions)

    void setClippingOn( bool clippingOnIn );        ///< Set clipping flag. If true, setClippingLow() and setClippingHigh() are used to set clipping values
    void setClippingLow( unsigned int value );      ///< Set pixel value below which low clip colour is displayed
    void setClippingHigh( unsigned int value );     ///< Set pixel value above which high clip colour is displayed

    // Image information
    int getScanOption();                            ///< Determine the way the input pixel data must be scanned to accommodate the required rotate and flip options.
    void getPixelTranslation();                     ///< Generate a lookup table to convert raw pixel values to display pixel values
    unsigned int maxPixelValue();                   ///< Determine the maximum pixel value for the current format
    unsigned int rotatedImageBuffWidth();           ///< Return the image width following any rotation
    unsigned int rotatedImageBuffHeight();          ///< Return the image height following any rotation
    imageDisplayProperties::rgbPixel getFalseColor (const unsigned char value);    ///< Get a false color representation for an entry fro the color lookup table
    int getElementCount();                                                         ///< Determine the element count expected based on the available dimensions
    bool validateDimensions();                                                     ///< Determine if the image dimensional information is valid.
    void getPixelRange( const QRect& area, unsigned int* min, unsigned int* max ); ///< Determine the range of pixel values an area of the image
    bool hasImage(){ return !imageData.isEmpty(); }                                ///< Return true if the current image is empty
    const unsigned char* getImageDataPtr( QPoint& pos );                           ///< Return a pointer to pixel data in the original image data.
    int getPixelValueFromData( const unsigned char* ptr );                         ///< Return a number representing a pixel intensity given a pointer into an image data buffer.
    double getFloatingPixelValueFromData( const unsigned char* ptr );              ///< Return a floating point number representing a pixel intensity given a pointer into an image data buffer.

    QImage copyImage();         ///< Return a QImage based on the current image

    void generateVSliceData( QVector<QPointF>& vSliceData, int x, unsigned int thickness );                          ///< Generate a series of pixel values from a vertical slice through the current image.
    void generateHSliceData( QVector<QPointF>& hSliceData, int y, unsigned int thickness );                          ///< Generate a series of pixel values from a horizontal slice through the current image.
    void generateProfileData( QVector<QPointF>& profileData, QPoint point1, QPoint point2, unsigned int thickness ); ///< Generate a series of pseudo pixel values from an arbitrary line between two pixels.

    // Transformations
    QRect rotateFlipToDataRectangle( const QRect& rect );                       ///< Transform a rectangle from the image to the original data according to current rotation and flip options
    QRect rotateFlipToDataRectangle( const QPoint& pos1, const QPoint& pos2 );  ///< Transform a rectangle from the image to the original data according to current rotation and flip options
    QPoint rotateFlipToDataPoint( const QPoint& pos );                          ///< Transform a point from the image to the original data according to current rotation and flip options.

    QRect rotateFlipToImageRectangle( const QRect& rect );                      ///< Transform a rectangle from the original data to the image according to current rotation and flip options
    QRect rotateFlipToImageRectangle( const QPoint& pos1, const QPoint& pos2 ); ///< Transform a rectangle from the original data to the image according to current rotation and flip options
    QPoint rotateFlipToImagePoint( const QPoint& pos );                         ///< Transform a point from the original data to the image according to current rotation and flip options.


    void                 run();     // Image processing thread starting point.
    QWaitCondition       imageSync; // Communication between QEImage thread and image processing thread ('new image data ready to process'  or 'please finish')
    QMutex               imageWait; // Lock used by imageSync
    QMutex               imageLock; // Locks access to image data 'next' shared between QEImage thread and image processing thread
    bool                 finishNow; // Flag to image processing thread that it should exit
    imagePropertiesCore* next;      // Image related information passed to image processing thread and protected by imageLock

signals:
    void imageBuilt( QImage image, QString error );                         ///< An image has been generated from image data and in now ready for presentation

private:
};

#endif // QE_IMAGE_PROCESSOR_H
