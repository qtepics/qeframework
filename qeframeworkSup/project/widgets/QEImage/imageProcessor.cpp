/*  imageProcessor.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2015-2019 Australian Synchrotron
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

// This class generates images for presentation from raw image data and formatting
// information such as brightness, contrast, flip, rotate, canvas size, etc.
// The work is performed in a dedicated thread .

#include <QDebug>
#include <QMutexLocker>
#include "imageProcessor.h"
#include "imageDataFormats.h"
#include <colourConversion.h>
#include <math.h>

#define DEBUG qDebug () << "imageProcessor" << __LINE__ << __FUNCTION__ << " "

// Constructor
imageProcessor::imageProcessor()
{
    // Initialise
    next = NULL;
    finishNow = false;

    // Manage image processing thread
    start();
}

// Destructor
imageProcessor::~imageProcessor()
{
    // If any outstanding images to process, get rid of it
    finishNow = true;
    {// set scope of QMutexLocker
        QMutexLocker locker( &imageLock );
        if( next )
        {
            delete next;
            next = NULL;
        }
    }

    // Ask the image processing thread to exit.
    // imageWait lock is acquired to ensure the image processing thread is waiting for image data
    // as it is only in this state that it will notice the 'wakeOne' call.
    // (the lock is normally held by the imaging processing thread and released while waiting)
    {// set scope of QMutexLocker
        QMutexLocker locker( &imageWait );
        imageSync.wakeOne();
    }

    // Wait for the thread to exit
    wait();
}

// Image processing thread
void imageProcessor::run()
{
    // Lock mutex that QEimage will be waiting on before signling this thread to finish
    QMutexLocker locker1( &imageWait );

    // Snapshot of all information required for image processing
    imagePropertiesCore* core = NULL;

    // Process images as the the data arrives
    while( true )
    {
        // Wait for a image data
        imageSync.wait( &imageWait );

        // Processing new image data until there is none.
        // There will be none if we process the data faster than it arrives.
        while( true )
        {
            // If asked to finish, then finish
            if( finishNow )
            {
                return;
            }

            // Get the next snapshot of image data and all the related image information
            {// set scope of QMutexLocker
                QMutexLocker locker2( &imageLock );
                core = next;
                next = NULL;
            }

            // If any image data, process it
            if( core )
            {
                // Build the image
                image = core->buildImageCore();

                // Deliver the image to the widget
                emit imageBuilt( image, "" );

                // Discard the image information
                delete core;
                core = NULL;
            }

            // If no image data, stop processing and loop back to wait
            else
            {
                break;
            }
        }
    }
}

// Save the image data for analysis, processing and display
void imageProcessor::setImage( const QByteArray& imageIn, unsigned long dataSize )
{
    // Save the current image
    imageData = imageIn;
    receivedImageSize = (unsigned long) imageData.size ();
    imageDataSize = dataSize;

    // Calculate the number of bytes per pixel.
    // If the number of elements per pixel is known (derived from the image dimension zero if there are three dimensions)
    // then it is the image data element size * the number of elements per pixel
    // If the number of elements per pixel is not known (number of dimensions is not know or not three or dimension zero is not present)
    // then the elements per pixel will default to 1.
    bytesPerPixel = imageDataSize * elementsPerPixel;
}

// Generate a new image.
// This is the first part of generating an image from new data.
// most of the processing will occur in a seperate thread in imagePropertiesCore::buildImageCore()
void imageProcessor::buildImage()
{
    // Initially no errors
    QString errorText;

    // Do nothing if there is no image, or are no image dimensions yet
    if( imageData.isEmpty() || !imageBuffWidth || !imageBuffHeight )
    {
        emit imageBuilt( QImage(), errorText );
        return;
    }

    // Do we have enough (or any) data
    //
    const unsigned long required_size = imageBuffWidth * imageBuffHeight * bytesPerPixel;
    if( required_size > (unsigned int)(imageData.size()) )
    {
        // Do nothing if no image data.
        //
        if( receivedImageSize == 0 )
        {
            emit imageBuilt( QImage(), errorText );
            return;
        }

        QString messageText;

        messageText = QString( "Image too small (")
                .append( QString( "available image size: %1, " )    .arg( receivedImageSize ))
                .append( QString( "required size: %1, " )           .arg( required_size ))
                .append( QString( "width: %1, " )                   .arg( imageBuffWidth ))
                .append( QString( "height: %1, " )                  .arg( imageBuffHeight ))
                .append( QString( "data element size: %1, " )       .arg( imageDataSize ))
                .append( QString( "data elements per pixel: %1, " ) .arg( elementsPerPixel ))
                .append( QString( "bytes per pixel: %1)" )          .arg( bytesPerPixel ));

        // Skip if messageText same as last message.
        if( messageText != previousMessageText )
        {
            errorText = messageText;
            previousMessageText = messageText;
        }

        // If not enough image data for the expected size then zero extend.
        // Part image better than no image at all.
        int extra = (int)required_size - imageData.size();
        QByteArray zero_extend ( extra, '\0' );
        imageData.append( zero_extend );
    }

    // Determine the number of pixels to process
    // If something is wrong, do nothing
    unsigned long pixelCount = imageBuffWidth*imageBuffHeight;
    if( pixelCount * bytesPerPixel > (unsigned long)imageData.size() )
    {
        emit imageBuilt( QImage(), errorText ); // !!! should clear the image by delivering non null blank image???
        return;
    }

    // Get the pixel lookup table to convert raw pixel values to display pixel values taking into
    // account input pixel size, clipping, contrast reversal, and local brightness and contrast.
    if( !pixelLookupValid )
    {
        getPixelTranslation();
        pixelLookupValid = true;
    }

    { // set scope of QMutexLocker
        QMutexLocker locker( &imageLock );

        // If there is earlier image data that is yet to be processed, discard it.
        if( next )
        {
            delete next;
            next = NULL;
        }

        // Package up the current image data and all related information
        next = new imagePropertiesCore( imageData,
                                        imageBuffWidth,
                                        imageBuffHeight,
                                        getScanOption(),
                                        bytesPerPixel,
                                        pixelLow,
                                        pixelHigh,
                                        bitDepth,
                                        pixelLookup,
                                        formatOption,
                                        imageDataSize,
                                        imageDisplayProps,
                                        rotatedImageBuffWidth(),
                                        rotatedImageBuffHeight() );
    }

// For testing you can include the following two lines to skip processing
// in a seperate thread and call processing in this thread instead:
//    emit imageBuilt( next->buildImageCore(), "" );
//    return;

    // Wake up the image processing thread if required to process the next lot of image data
    imageSync.wakeOne();
}

// Package up image data along with all the information
// needed to process it and generate a QImage.
imagePropertiesCore::imagePropertiesCore( QByteArray imageDataIn,
                                          unsigned long imageBuffWidthIn,
                                          unsigned long imageBuffHeightIn,
                                          int scanOptionIn,
                                          unsigned long bytesPerPixelIn,
                                          int pixelLowIn,
                                          int pixelHighIn,
                                          unsigned int bitDepthIn,
                                          imageDisplayProperties::rgbPixel* pixelLookupIn,
                                          imageDataFormats::formatOptions formatOptionIn,
                                          unsigned long imageDataSizeIn,
                                          imageDisplayProperties* imageDisplayPropsIn,
                                          unsigned int rotatedImageBuffWidthIn,
                                          unsigned int rotatedImageBuffHeightIn )
{
    imageData = imageDataIn;
    imageBuffWidth = imageBuffWidthIn;
    imageBuffHeight = imageBuffHeightIn;
    scanOption = scanOptionIn;
    bytesPerPixel = bytesPerPixelIn;
    pixelLow = pixelLowIn;
    pixelHigh = pixelHighIn;
    bitDepth = bitDepthIn;
    pixelLookup = pixelLookupIn;
    formatOption = formatOptionIn;
    imageDataSize = imageDataSizeIn;
    imageDisplayProps = imageDisplayPropsIn;
    rotatedImageBuffWidth = rotatedImageBuffWidthIn;
    rotatedImageBuffHeight = rotatedImageBuffHeightIn;
}

// Generate a new image.
// This is the second part of generating an image from new data.
// The image is generated in a seperate thread after preperation by imageProcessor::buildImage()
QImage imagePropertiesCore::buildImageCore()
{
    // Create image ready for building the image data
    QImage image( rotatedImageBuffWidth, rotatedImageBuffHeight, QImage::Format_RGB32 );

    // Set up input and output pointers and counters ready to process each pixel
    // Note, must be constData() - not data() - to avoid a reallocation of the data
    const unsigned char* dataIn = (unsigned char*)imageData.constData();
    // constBits is 4.8 or later. We want the read/write bits anyway.
    imageDisplayProperties::rgbPixel* dataOut = (imageDisplayProperties::rgbPixel*)(image.bits());
    unsigned long buffIndex = 0;
    unsigned long dataIndex = 0;

    // Depending on the flipping and rotating options pixel drawing can start in any of
    // the four corners and start scanning either vertically or horizontally.
    // See getScanOption() comments for more details on how the rotate and flip
    // options are used to generate one of 8 scan options.
    // The 8 scanning options are shown numbered here:
    //
    //    o----->1         2<-----o
    //    |                       |
    //    |                       |
    //    |                       |
    //    v                       v
    //    5                       6
    //
    //
    //
    //    7                       8
    //    ^                       ^
    //    |                       |
    //    |                       |
    //    |                       |
    //    o----->3         4<-----o
    //

    // Drawing is performed in two nested loops, one for height and one for width.
    // Depending on the scan option, however, the outer may be height or width.
    // The input buffer is read consecutivly from first pixel to last and written to the
    // output buffer, which is moved to the next pixel by both the inner and outer
    // loops to where ever that next pixel is according to the rotation and flipping.
    // The following defines parameters driving the loops:
    //
    // opt      = scan option
    // outCount = outer loop count (width or height);
    // inCount  = inner loop count (height or width)
    // start    = output buffer start pixel (one of the four corners)
    // outInc   = outer loop increment to output buffer
    // inInc    = inner loop increment to output buffer
    // w        = image width
    // h        = image height
    //
    // opt outCount inCount start    outInc   inInc
    //  1      h       w      0         0       1
    //  2      h       w      w-1       w      -1
    //  3      h       w    w*(h-1)    -2*w     1
    //  4      h       w    (w*h)-1     0      -1
    //  5      w       h      0     -w*(h-1)+1   w
    //  6      w       h      w-1   -w*(h-1)-1   w
    //  7      w       h    w*(h-1)  w*(h-1)+1  -w
    //  8      w       h    (w*h)-1  w*(h-1)-1  -w


    int outCount;   // Outer loop count (width or height);
    int inCount;    // Inner loop count (height or width)
    int start;      // Output buffer start pixel (one of the four corners)
    int outInc;     // Outer loop increment to output buffer
    int inInc;      // Inner loop increment to output buffer
    int h = imageBuffHeight;
    int w = imageBuffWidth;

    // Set the loop parameters according to the scan option
    switch( scanOption )
    {
        default:  // Sanity check. default to 1
        case 1: outCount = h; inCount = w; start = 0;       outInc =  0;     inInc =  1; break;
        case 2: outCount = h; inCount = w; start = w-1;     outInc =  2*w;   inInc = -1; break;
        case 3: outCount = h; inCount = w; start = w*(h-1); outInc = -2*w;   inInc =  1; break;
        case 4: outCount = h; inCount = w; start = (w*h)-1; outInc =  0;     inInc = -1; break;
        case 5: outCount = w; inCount = h; start = 0;       outInc = -w*h+1; inInc =  w; break;
        case 6: outCount = w; inCount = h; start = w-1;     outInc = -w*h-1; inInc =  w; break;
        case 7: outCount = w; inCount = h; start = w*(h-1); outInc =  w*h+1; inInc = -w; break;
        case 8: outCount = w; inCount = h; start = (w*h)-1; outInc =  w*h-1; inInc = -w; break;
    }



    // Draw the input pixels into the image buffer.
    // Drawing is performed in two nested loops, one for height and one for width.
    // Depending on the scan option, however, the outer may be height or width.
    // The input buffer is read consecutively from first pixel to last and written to the
    // output buffer, which is moved to the next pixel by both the inner and outer
    // loops to where ever that next pixel is according to the rotation and flipping.
    dataIndex = start;

    unsigned int pixelRange = pixelHigh-pixelLow;
    if( !pixelRange )
    {
        pixelRange = 1;
    }

    unsigned int mask = ((unsigned long)(1)<<bitDepth)-1;

    // Prepare for building image stats while processing image data
    unsigned int maxP = 0;
    unsigned int minP = UINT_MAX;
    unsigned int valP;
    unsigned int binShift = (bitDepth<8)?0:bitDepth-8;
    unsigned int bin;
    unsigned int bins[HISTOGRAM_BINS]; // Bins used for generating a pixel histogram
    for( int i = 0; i < HISTOGRAM_BINS; i++ )
    {
        bins[i]=0;
    }
#define BUILD_STATS \
    bin = valP>>binShift; \
    bins[bin] = bins[bin]+1; \
    if( valP < minP ) minP = valP; \
    else if( valP > maxP ) maxP = valP;

// For speed, the format switch statement is outside the pixel loop.
// An identical(ish) loop is used for each format
#define LOOP_START                          \
    for( int i = 0; i < outCount; i++ )     \
    {                                       \
        for( int j = 0; j < inCount; j++ )  \
        {

#define LOOP_END                            \
            dataIndex += inInc;             \
            buffIndex++;                    \
        }                                   \
        dataIndex += outInc;                \
    }

    // Format each pixel ready for use in an RGB32 QImage.
    // Note, for speed, the switch on format is outside the loop. The loop is duplicated in each case using macros.
    switch( formatOption )
    {
        case imageDataFormats::MONO:
        {
            LOOP_START
                unsigned int inPixel;

                // Extract pixel
                inPixel =  (*(unsigned int*) (&dataIn[dataIndex*bytesPerPixel]))&mask;

                // Accumulate pixel statistics
                valP = inPixel;
                BUILD_STATS

                // Scale pixel for local brightness and contrast
                ( (int)inPixel < pixelLow ) ? inPixel = 0 : ( (int)inPixel > pixelHigh ) ? inPixel = 255 : inPixel = ((int)inPixel-pixelLow)*255/pixelRange;

                // Select displayed pixel
                dataOut[buffIndex] = pixelLookup[inPixel];
            LOOP_END
            break;
        }

        case imageDataFormats::BAYERGB:
        case imageDataFormats::BAYERBG:
        case imageDataFormats::BAYERGR:
        case imageDataFormats::BAYERRG:
        {
            // Pre-calculate offsets in the data to neighbouring pixels
            int TLOffset = (-(int)(imageBuffWidth)-1)*(int)(bytesPerPixel);
            int  TOffset = -(int)(imageBuffWidth)*(int)(bytesPerPixel);
            int TROffset = (-(int)(imageBuffWidth)+1)*(int)(bytesPerPixel);
            int  LOffset = -(int)(bytesPerPixel);
            int  ROffset = (int)(bytesPerPixel);
            int BLOffset = ((int)(imageBuffWidth)-1)*(int)(bytesPerPixel);
            int  BOffset = imageBuffWidth*(int)(bytesPerPixel);
            int BROffset = ((int)(imageBuffWidth)+1)*(int)(bytesPerPixel);

            // Define regions in the image where different calculations occur.
            // Over most of the image four neighbouring cells are available.
            // On the sides five neighbours are present.
            // On the corners three neighbours are present.
            enum regions {REG_TL, REG_T, REG_TR, REG_L, REG_C, REG_R, REG_BL, REG_B, REG_BR};

            // Values for all cells that may be involved in generating a pixel
            quint32 g1; // Green above blue or red
            quint32 g2; // Green below of blue or red
            quint32 g3; // Green left of blue or red
            quint32 g4; // Green right of blue or red

            quint32 d1; // Red or blue diagonally above-left of blue or red
            quint32 d2; // Red or blue diagonally above-right of blue or red
            quint32 d3; // Red or blue diagonally below-left of blue or red
            quint32 d4; // Red or blue diagonally below-right of blue or red

            quint32 d;  // Sum of reds or blues diagonally sourounding current blue or red

            quint32 rb; // Red or blue from current cell (depending on pattern)

            quint32 h1; // Left of green (may be red or blue depending on pattern)
            quint32 h2; // Right of green (may be red or blue depending on pattern)
            quint32 v1; // Above green (may be red or blue depending on pattern)
            quint32 v2; // Below green (may be red or blue depending on pattern)

            quint32 h;  // Sum of left and right or green (horizontal) (may be red or blue depending on pattern)
            quint32 v;  // Sum of above and below green (vertical) (may be red or blue depending on pattern)

            quint32 g12;// Green (either Green1 or Green 2)

            quint32* g1r; // Pointer to red sum for Green1 (may be h (left and right) or v (above and below) depending on pattern)
            quint32* g2r; // Pointer to red sum for Green2 (may be h (left and right) or v (above and below) depending on pattern)
            quint32* g1b; // Pointer to blue sum for Green1 (may be h (left and right) or v (above and below) depending on pattern)
            quint32* g2b; // Pointer to blue sum for Green2 (may be h (left and right) or v (above and below) depending on pattern)

            // Pixel RGB values
            quint32 r;
            quint32 g;
            quint32 b;

            // Each Bayer cluster of four image cells contain one red, one blue, and two green values.
            // There are four combinations for each cluster and no standard :(
            // Preconfigure a table to translate from cluster cell index to color.
            enum CELL_COLOURS {CC_G1, CC_G2,CC_R,CC_B};
            CELL_COLOURS cellColours[4];
            switch( formatOption )
            {
                default:    // Should never hit the default case. Include to avoid compilation errors
                case imageDataFormats::BAYERGB: cellColours[0] = CC_G1; cellColours[1] = CC_B;  cellColours[2] = CC_R;  cellColours[3] = CC_G2; break;
                case imageDataFormats::BAYERBG: cellColours[0] = CC_B;  cellColours[1] = CC_G1; cellColours[2] = CC_G2; cellColours[3] = CC_R;  break;
                case imageDataFormats::BAYERGR: cellColours[0] = CC_G1; cellColours[1] = CC_R;  cellColours[2] = CC_B;  cellColours[3] = CC_G2; break;
                case imageDataFormats::BAYERRG: cellColours[0] = CC_R;  cellColours[1] = CC_G1; cellColours[2] = CC_G2; cellColours[3] = CC_B;  break;
            }

            // Preconfigure red and blue positions relative to green. Depending on the Bayer pattern
            // red can be left and right, and blue above and below, or the other way round
            switch( formatOption )
            {
                default:    // Should never hit the default case. Include to avoid compilation errors
                case imageDataFormats::BAYERGB:
                case imageDataFormats::BAYERBG:
                    g1r = &v; // Use vertical (v) for reds associated with Green1
                    g1b = &h; // Use horizontal (h) for blues associated with Green1
                    g2r = &h; // Use horizontal (h) for reds associated with Green2
                    g2b = &v; // Use vertical (v) for blues associated with Green2
                    break;

                case imageDataFormats::BAYERGR:
                case imageDataFormats::BAYERRG:
                    g1r = &h; // Use horizontal (h) for reds associated with Green1
                    g1b = &v; // Use vertical (v) for blues associated with Green1
                    g2r = &v; // Use vertical (v) for reds associated with Green2
                    g2b = &h; // Use horizontal (h) for blues associated with Green2
                    break;
            }

            // Pre-calculate last cell for inner and outer loops
            int outLast = outCount-1;
            int inLast = inCount-1;

            // Pre-calculate pixel index values for corners
            unsigned int TLPixel = 0;
            unsigned int TRPixel = imageBuffWidth-1;
            unsigned int BLPixel = (imageBuffHeight-1)*imageBuffWidth;
            unsigned int BRPixel = (imageBuffHeight*imageBuffWidth)-1;

            // Processing region (corners, edges, or central)
            regions region;

            // Pre-calculate data shift and mask nessesary to obtain most significant 8 bits
            int shift = (bitDepth<=8)?0:bitDepth-8;
            quint32 mask = ((unsigned long)(1)<<bitDepth)-1;

            // Loop through the input data
            // The loop order is based on current flip and rotation and so will not nessesarily
            // move linearly through input data. No matter what the order of processing neighbouring
            // cells are referenced the same way.
            LOOP_START

                // Get a reference to the current 'pixel'
                unsigned char* inPixel  = (unsigned char*)(&dataIn[dataIndex*bytesPerPixel]);

                // Calculate the current Bayer cell. (one of four as follows)
                //   01010101010101010101...
                //   23232323232323232323...
                //   01010101010101010101...
                //   23232323232323232323...
                //   01010101010101010101...
                //   23232323232323232323...
                //   .......................
                //   .......................
                //   .......................
                //
                unsigned int color = (dataIndex&1)|(((dataIndex/imageBuffWidth)&1)<<1);

                // Translate the bayer cell to a color
                // Depending on the specific bayer pattern the color number is interpreted as follows
                //
                // BayerBG = 01 = BG
                //           23   GR
                //
                // BayerGB = 01 = GB
                //           23   RG
                //
                // BayerRG = 01 = RG
                //           23   GB
                //
                // BayerGR = 01 = GR
                //           23   BG
                //
                // So, for example, color = 1 and Bayer pattern is BayerRG, then the current color is G1 (the first green)
                CELL_COLOURS cellColour = cellColours[color];

                // Calculate the processing region.
                // This is used to determine what neighbouring cells are available.

                // Assume Central region.
                region = REG_C;

                // If on an edge...
                if( i == 0 || j == 0 || i == outLast || j == inLast )
                {
                    // Determine where on edge
                    // (this will be simpler if we loop through source data rather than output image)

                    // If on top edge...
                    if( dataIndex < imageBuffWidth )
                    {
                        if     ( dataIndex == TLPixel ) region = REG_TL;
                        else if( dataIndex == TRPixel ) region = REG_TR;
                        else                            region = REG_T;
                    }

                    // If on bottom edge...
                    else if( dataIndex >= BLPixel)
                    {
                        if     ( dataIndex == BLPixel ) region = REG_BL;
                        else if( dataIndex == BRPixel ) region = REG_BR;
                        else                            region = REG_B;
                    }

                    // if on left or right edge...
                    else if( !(dataIndex % imageBuffWidth) ) region = REG_L;
                    else                                     region = REG_R;
                }


                // Process the cell
                switch( cellColour )
                {
                    case CC_R: // red
                    case CC_B: // blue

                        // Extract the value
                        rb = (*((quint32*)inPixel))&mask;

                        // Based on the region, use available neighbouring cells to supply green and red or blue (diagonal) values
                        switch( region )
                        {
                            case REG_C:
                                g1 = (*((quint32*)(&inPixel[TOffset])))&mask;
                                g2 = (*((quint32*)(&inPixel[BOffset])))&mask;
                                g3 = (*((quint32*)(&inPixel[LOffset])))&mask;
                                g4 = (*((quint32*)(&inPixel[ROffset])))&mask;
                                d1 = (*((quint32*)(&inPixel[TLOffset])))&mask;
                                d2 = (*((quint32*)(&inPixel[TROffset])))&mask;
                                d3 = (*((quint32*)(&inPixel[BLOffset])))&mask;
                                d4 = (*((quint32*)(&inPixel[BROffset])))&mask;
                                break;

                            case REG_TL:
                                g2 = (*((quint32*)(&inPixel[BOffset])))&mask;
                                g4 = (*((quint32*)(&inPixel[ROffset])))&mask;
                                g1 = g2;
                                g3 = g4;
                                d4 = (*((quint32*)(&inPixel[BROffset])))&mask;
                                d1 = d4;
                                d2 = d4;
                                d3 = d4;
                                break;

                            case REG_T:
                                g2 = (*((quint32*)(&inPixel[BOffset])))&mask;
                                g3 = (*((quint32*)(&inPixel[LOffset])))&mask;
                                g4 = (*((quint32*)(&inPixel[ROffset])))&mask;
                                g1 = (g2+g3+g4)/3;
                                d3 = (*((quint32*)(&inPixel[BLOffset])))&mask;
                                d4 = (*((quint32*)(&inPixel[BROffset])))&mask;
                                d1 = d3;
                                d2 = d4;
                                break;

                            case REG_TR:
                                g2 = (*((quint32*)(&inPixel[BOffset])))&mask;
                                g3 = (*((quint32*)(&inPixel[LOffset])))&mask;
                                g1 = g2;
                                g4 = g3;
                                d3 = (*((quint32*)(&inPixel[BLOffset])))&mask;
                                d1 = d3;
                                d2 = d3;
                                d4 = d3;
                                break;

                            case REG_L:
                                g1 = (*((quint32*)(&inPixel[TOffset])))&mask;
                                g2 = (*((quint32*)(&inPixel[BOffset])))&mask;
                                g4 = (*((quint32*)(&inPixel[ROffset])))&mask;
                                g3 = g4;
                                d2 = (*((quint32*)(&inPixel[TROffset])))&mask;
                                d4 = (*((quint32*)(&inPixel[BROffset])))&mask;
                                d1 = d2;
                                d3 = d4;
                                break;

                            case REG_R:
                                g1 = (*((quint32*)(&inPixel[TOffset])))&mask;
                                g2 = (*((quint32*)(&inPixel[BOffset])))&mask;
                                g3 = (*((quint32*)(&inPixel[LOffset])))&mask;
                                g4 = (g1+g2+g3)/3;
                                d1 = (*((quint32*)(&inPixel[TLOffset])))&mask;
                                d3 = (*((quint32*)(&inPixel[BLOffset])))&mask;
                                d2 = d1;
                                d4 = d3;
                                break;

                            case REG_BL:
                                g1 = (*((quint32*)(&inPixel[TOffset])))&mask;
                                g4 = (*((quint32*)(&inPixel[ROffset])))&mask;
                                g2 = g1;
                                g3 = g4;
                                d2 = (*((quint32*)(&inPixel[TROffset])))&mask;
                                d1 = d2;
                                d3 = d2;
                                d4 = d2;
                                break;

                            case REG_B:
                                g1 = (*((quint32*)(&inPixel[TOffset])))&mask;
                                g3 = (*((quint32*)(&inPixel[LOffset])))&mask;
                                g4 = (*((quint32*)(&inPixel[ROffset])))&mask;
                                g2 = (g1+g3+g4)/3;
                                d1 = (*((quint32*)(&inPixel[TLOffset])))&mask;
                                d2 = (*((quint32*)(&inPixel[TROffset])))&mask;
                                d3 = d1;
                                d4 = d2;
                                break;

                            case REG_BR:
                                g1 = (*((quint32*)(&inPixel[TOffset])))&mask;
                                g3 = (*((quint32*)(&inPixel[LOffset])))&mask;
                                g2 = g1;
                                g4 = g3;
                                d1 = (*((quint32*)(&inPixel[TLOffset])))&mask;
                                d2 = d1;
                                d3 = d1;
                                d4 = d1;
                                break;

                        }

                        // Calculate the diagonal sum (red or blue depending on the pattern)
                        d = (d1+d2+d3+d4)>>(shift+2);

                        // Calculate the Green value from the green cells
                        g = (g1+g2+g3+g4)>>(shift+2);

                        // Take the red and blue from the current cell and the diagonals, or the other way round depending on the pattern
                        switch( cellColour )
                        {
                            default:    // Should never hit the default case. Include to avoid compilation errors
                            case CC_R: // red
                                r = rb>>shift;
                                b = d;
                                break;
                            case CC_B: // blue
                                r = d;
                                b = rb>>shift;
                                break;
                        }

                        break;

                    case CC_G1: // green 1
                    case CC_G2: // green 2

                        // Extract the green value
                        g12 = (*((quint32*)inPixel))&mask;

                        // Based on the region, use available neighbouring cells to supply red and blue values
                        // Depending on the pattern top and bottom might be red and left and right blue,
                        // or the other way round, so for the time being, just refer to them by their
                        // orientation (v or h), rather than colour
                        switch( region )
                        {
                            case REG_C:
                                h1 = (*((quint32*)(&inPixel[LOffset])))&mask;
                                h2 = (*((quint32*)(&inPixel[ROffset])))&mask;
                                v1 = (*((quint32*)(&inPixel[TOffset])))&mask;
                                v2 = (*((quint32*)(&inPixel[BOffset])))&mask;
                                break;

                            case REG_T:
                                h1 = (*((quint32*)(&inPixel[LOffset])))&mask;
                                h2 = (*((quint32*)(&inPixel[ROffset])))&mask;
                                v2 = (*((quint32*)(&inPixel[BOffset])))&mask;
                                v1 = v2;
                                break;

                            case REG_TR:
                                h1 = (*((quint32*)(&inPixel[LOffset])))&mask;
                                h2 = h1;
                                v2 = (*((quint32*)(&inPixel[BOffset])))&mask;
                                v1 = v2;
                                break;

                            case REG_R:
                                h1 = (*((quint32*)(&inPixel[LOffset])))&mask;
                                h2 = h1;
                                v1 = (*((quint32*)(&inPixel[TOffset])))&mask;
                                v2 = (*((quint32*)(&inPixel[BOffset])))&mask;
                                break;

                            case REG_B:
                                h1 = (*((quint32*)(&inPixel[LOffset])))&mask;
                                h2 = (*((quint32*)(&inPixel[ROffset])))&mask;
                                v1 = (*((quint32*)(&inPixel[TOffset])))&mask;
                                v2 = v1;
                                break;

                            case REG_BR:
                                h1 = (*((quint32*)(&inPixel[LOffset])))&mask;
                                h2 = h1;
                                v1 = (*((quint32*)(&inPixel[TOffset])))&mask;
                                v2 = v1;
                                break;

                            case REG_TL:
                                h2 = (*((quint32*)(&inPixel[ROffset])))&mask;
                                h1 = h2;
                                v2 = (*((quint32*)(&inPixel[BOffset])))&mask;
                                v1 = v2;
                                break;

                            case REG_L:
                                h2 = (*((quint32*)(&inPixel[ROffset])))&mask;
                                h1 = h2;
                                v1 = (*((quint32*)(&inPixel[TOffset])))&mask;
                                v2 = (*((quint32*)(&inPixel[BOffset])))&mask;
                                break;

                            case REG_BL:
                                h2 = (*((quint32*)(&inPixel[ROffset])))&mask;
                                h1 = h2;
                                v1 = (*((quint32*)(&inPixel[TOffset])))&mask;
                                v2 = v1;
                                break;
                        }

                        // Calculate the vertical and horizontal sums (one is red, the other is blue depending on the pattern)
                        h = (h1+h2)>>(shift+1);
                        v = (v1+v2)>>(shift+1);

                        // Calculate the Green value from the green cell
                        g = g12>>shift;

                        // Take the red and blue from the vertical or horizontal sums depending on the pattern
                        switch( cellColour )
                        {
                            default:    // Should never hit the default case. Include to avoid compilation errors
                            case CC_G1: // green 1
                                r = *g1r;
                                b = *g1b;
                                break;
                            case CC_G2: // green 2
                                r = *g2r;
                                b = *g2b;
                                break;
                        }

                        break;

                    // Should never get here.
                    // Included to avoid compilation warnings on some compilers
                    default:
                        r = 0;
                        b = 0;
                        g = 0;
                        break;
                }



                // Accumulate pixel statistics
                valP = g; // use all three colors!!!
                BUILD_STATS

                // Scale pixel for local brightness and contrast
                // !!! This will introduce some hue issues. Should convert to HSV to manipulate brightness and contrast????
                ( (int)r < pixelLow ) ? r = 0 : ( (int)r > pixelHigh ) ? r = 255 : r = (r-pixelLow)*255/pixelRange;
                ( (int)g < pixelLow ) ? g = 0 : ( (int)g > pixelHigh ) ? g = 255 : g = (g-pixelLow)*255/pixelRange;
                ( (int)b < pixelLow ) ? b = 0 : ( (int)b > pixelHigh ) ? b = 255 : b = (b-pixelLow)*255/pixelRange;

                // Select displayed pixel
                dataOut[buffIndex].p[0] = pixelLookup[b].p[0];
                dataOut[buffIndex].p[1] = pixelLookup[g].p[0];
                dataOut[buffIndex].p[2] = pixelLookup[r].p[0];
                dataOut[buffIndex].p[3] = 0xff;

            LOOP_END
            break;
        }

        case imageDataFormats::RGB1:
        case imageDataFormats::RGB2: //!!! not done yet - just do the same as RGB1 for the time being and hope
        case imageDataFormats::RGB3: //!!! not done yet - just do the same as RGB1 for the time being and hope
        {
            //unsigned int rOffset = 0*imageDataSize;
            unsigned int gOffset = imageDataSize;
            unsigned int bOffset = 2*imageDataSize;
            LOOP_START

                // Extract pixel
                unsigned char* inPixel  = (unsigned char*)(&dataIn[dataIndex*bytesPerPixel]);
                unsigned int r = *inPixel;
                unsigned int g = inPixel[gOffset];
                unsigned int b = inPixel[bOffset];

                // Accumulate pixel statistics
                valP = g; // use all three colors!!!
                BUILD_STATS

                // Scale pixel for local brightness and contrast
                // !!! This will introduce some hue issues. Should convert to HSV to manipulate brightness and contrast????
                ( (int)r < pixelLow ) ? r = 0 : ( (int)r > pixelHigh ) ? r = 255 : r = (r-pixelLow)*255/pixelRange;
                ( (int)g < pixelLow ) ? g = 0 : ( (int)g > pixelHigh ) ? g = 255 : g = (g-pixelLow)*255/pixelRange;
                ( (int)b < pixelLow ) ? b = 0 : ( (int)b > pixelHigh ) ? b = 255 : b = (b-pixelLow)*255/pixelRange;

                // Select displayed pixel
                dataOut[buffIndex].p[0] = pixelLookup[b].p[0];
                dataOut[buffIndex].p[1] = pixelLookup[g].p[0];
                dataOut[buffIndex].p[2] = pixelLookup[r].p[0];
                dataOut[buffIndex].p[3] = 0xff;

            LOOP_END
            break;
        }

        case imageDataFormats::YUV421: //!!! not done yet. do the same as for YUV422
        case imageDataFormats::YUV422:
        case imageDataFormats::YUV444: //!!! not done yet. do the same as for YUV422
        {
            LOOP_START
                    // Extract pixel
                    // 4 values are used to generate 2 pixels as follows:

                    // u  = yuv[0];
                    // y1 = yuv[1];
                    // v  = yuv[2];
                    // y2 = yuv[3];

                    // rgb1 = YUVtoRGB(y1, u, v);
                    // rgb2 = YUVtoRGB(y2, u, v);

                    unsigned int pairIndex = dataIndex&1;               // Generating first or second pixel? extract lowest bit
                    unsigned long dataPairIndex = dataIndex-pairIndex;  // Create data index to base of both pixels - clear lowest bit

                    // Get base of both pixels.
                    // This loop is running through the output pixels, so depending on flip and rotate the last (or next)
                    // pixel processed may not be from the same pixel pair represented by this YUV quad.
                    // That's OK. As each pixel is processed, the correct parts from the appropriate quad is used.
                    unsigned char* yuv422Base = (unsigned char*)(&dataIn[dataPairIndex*bytesPerPixel]);

                    // Get the correct YUV values for this pixel
                    unsigned char* y;
                    if( pairIndex == 0 )
                    {
                        y  = yuv422Base+bytesPerPixel;                  // yuv[1]
                    }
                    else
                    {
                        y  = yuv422Base+(3*bytesPerPixel);              // yuv[3]
                    }
                    unsigned char* u   = yuv422Base;                    // yuv[0]
                    unsigned char* v   = yuv422Base+(2*bytesPerPixel);  // yuv[2]

                    // Extract pixel
                    unsigned int r = YUV2R(*y, *u, *v);
                    unsigned int g = YUV2G(*y, *u, *v);
                    unsigned int b = YUV2B(*y, *u, *v);

                    // Accumulate pixel statistics
                    valP = g; // use all three colors!!!
                    BUILD_STATS

                    // Scale pixel for local brightness and contrast
                    // !!! This will introduce some hue issues. Should convert to HSV to manipulate brightness and contrast????
                    ( (int)r < pixelLow ) ? r = 0 : ( (int)r > pixelHigh ) ? r = 255 : r = (r-pixelLow)*255/pixelRange;
                    ( (int)g < pixelLow ) ? g = 0 : ( (int)g > pixelHigh ) ? g = 255 : g = (g-pixelLow)*255/pixelRange;
                    ( (int)b < pixelLow ) ? b = 0 : ( (int)b > pixelHigh ) ? b = 255 : b = (b-pixelLow)*255/pixelRange;

                    // Select displayed pixel
                    dataOut[buffIndex].p[0] = pixelLookup[b].p[0];
                    dataOut[buffIndex].p[1] = pixelLookup[g].p[0];
                    dataOut[buffIndex].p[2] = pixelLookup[r].p[0];
                    dataOut[buffIndex].p[3] = 0xff;

            LOOP_END
            break;            
        }

        default:  // avoid  compilation warning for NUMBER_OF_FORMATS
            break;
    }

    // Update the image display properties controls if present
    if( imageDisplayProps )
    {
        imageDisplayProps->setStatistics( minP, maxP, bitDepth, bins, pixelLookup );
    }

    // Return the image
    return image;
}

// Set the image width
// Return true of the width changes as a result.
bool imageProcessor::setWidth( unsigned long uValue )
{
    if( imageBuffWidth != uValue )
    {
        imageBuffWidth = uValue;
        return true;
    }
    else
    {
        return false;
    }
}

// Set the image height
// Return true of the height changes as a result.
bool imageProcessor::setHeight( unsigned long uValue )
{
    if( imageBuffHeight != uValue )
    {
        imageBuffHeight = uValue;
        return true;
    }
    else
    {
        return false;
    }
}

// Set the number of dimensions.
// This is an area detector concept and is used to determine how to treat dimenstions 0, 1, and 2
bool imageProcessor::setNumDimensions( unsigned long uValue )
{
    if( numDimensions != uValue )
    {
        switch( uValue )
        {
            case 0:
                numDimensions = uValue;
                break;

            case 2:
            case 3:
                numDimensions = uValue;
                setWidthHeightFromDimensions();
                break;
        }
        return true;
    }
    else
    {
        return false;
    }
}

// Set the first dimension (width if two dimenstions, bytes per element if three dimensions)
bool imageProcessor::setDimension0( unsigned long uValue )
{
    if( imageDimension0 != uValue )
    {
        imageDimension0 = uValue;
        setWidthHeightFromDimensions();
        return true;
    }
    else
    {
        return false;
    }
}

// Set the second dimension (height if two dimensions, width if three dimensions)
bool imageProcessor::setDimension1( unsigned long uValue )
{
    if( imageDimension1 != uValue )
    {
        imageDimension1 = uValue;
        setWidthHeightFromDimensions();
        return true;
    }
    else
    {
        return false;
    }
}

// Set the third dimension (unused if two dimensions, height if three dimensions)
bool imageProcessor::setDimension2( unsigned long uValue )
{
    if( imageDimension2 != uValue )
    {
        imageDimension2 = uValue;
        setWidthHeightFromDimensions();
        return true;
    }
    else
    {
        return false;
    }
}

// Set clipping flag. If true, setClippingLow() and setClippingHigh() are used to set clipping values
void imageProcessor::setClippingOn( bool clippingOnIn )
{
    if( clippingOn != clippingOnIn )
    {
        clippingOn = clippingOnIn;
        pixelLookupValid = false;
    }
}

// Set pixel value below which low clip colour is displayed
void imageProcessor::setClippingLow( unsigned int value )
{
    if( clippingLow != (unsigned int)value )
    {
        clippingLow = value;
        pixelLookupValid = false;
    }
}

// Set pixel value above which high clip colour is displayed
void imageProcessor::setClippingHigh( unsigned int value )
{
    if( clippingHigh != (unsigned int)value )
    {
        clippingHigh = value;
        pixelLookupValid = false;
    }
}

// Determine the way the input pixel data must be scanned to accommodate the required
// rotate and flip options. This is used when generating the image data, and also when
// transforming points in the image back to references in the original pixel data.
int imageProcessor::getScanOption()
{
    // Depending on the flipping and rotating options pixel drawing can start in any of
    // the four corners and start scanning either vertically or horizontally.
    // The 8 scanning options are shown numbered here:
    //
    //    o----->1         2<-----o
    //    |                       |
    //    |                       |
    //    |                       |
    //    v                       v
    //    5                       6
    //
    //
    //
    //    7                       8
    //    ^                       ^
    //    |                       |
    //    |                       |
    //    |                       |
    //    o----->3         4<-----o
    //
    //
    // The rotation and flip properties can be set in 16 combinations, but these 16
    // options can only specify the 8 possible scan options as follows:
    // (for example rotating 180 degrees, then flipping both vertically and horizontally
    // is the same as doing no rotation or flipping at all - scan option 1)
    //
    //  rot vflip hflip scan_option
    //    0   0     0      1
    //    0   0     1      2
    //    0   1     0      3
    //    0   1     1      4
    //  R90   0     0      7
    //  R90   0     1      5
    //  R90   1     0      8
    //  R90   1     1      6
    //  L90   0     0      6
    //  L90   0     1      8
    //  L90   1     0      5
    //  L90   1     1      7
    //  180   0     0      4
    //  180   0     1      3
    //  180   1     0      2
    //  180   1     1      1
    //
    // Determine the scan option as shown in the above diagram
    switch( rotation )
    {                                               // vh v!h     !vh !v!h
        case ROTATION_0:        return flipVert?flipHoz?4:3:flipHoz?2:1;
        case ROTATION_90_RIGHT: return flipVert?flipHoz?6:8:flipHoz?5:7;
        case ROTATION_90_LEFT:  return flipVert?flipHoz?7:5:flipHoz?8:6;
        case ROTATION_180:      return flipVert?flipHoz?1:2:flipHoz?3:4;
        default:                return 1; // Sanity check
    }
}

// Generate a lookup table to convert raw pixel values to display pixel values taking into
// account, clipping, and contrast reversal.
// Note, the table will be used to translate each colour in an RGB format.
//
void imageProcessor::getPixelTranslation()
{
    // Maximum pixel value for 8 bit
    #define MAX_VALUE 255

    // If there is an image options control, get the relevent options
    bool contrastReversal;
    bool logBrightness;

    if( imageDisplayProps )
    {
        contrastReversal = imageDisplayProps->getContrastReversal();
        logBrightness = imageDisplayProps->getLog();
    }
    else
    {
        contrastReversal = false;
        logBrightness = false;
    }

    // If there is an image options control, and we have retrieved high and low pixels from an image, get the relevent options
    if( imageDisplayProps && imageDisplayProps->statisticsValid() )
    {
        pixelLow = imageDisplayProps->getLowPixel();
        pixelHigh = imageDisplayProps->getHighPixel();
    }
    else
    {
        pixelLow = 0;
        pixelHigh = maxPixelValue();
    }

    // Loop populating table with pixel translations for every pixel value
    unsigned int value = 0;
    for( value = 0; value <= MAX_VALUE; value++ )
    {
        // Alpha always 100%
        pixelLookup[value].p[3] = 0xff;

        // Assume no clipping
        bool clipped = false;
        if( clippingOn && (clippingHigh > 0 || clippingLow > 0 ))
        {
            // If clipping high, set pixel to solid 'clip high' color
            if( clippingHigh > 0 && value >= clippingHigh )
            {
                pixelLookup[value].p[0] = 0x80;
                pixelLookup[value].p[1] = 0x80;
                pixelLookup[value].p[2] = 0xff;
                clipped = true;
            }
            // If clipping low, set pixel to solid 'clip low' color
            else if( clippingLow > 0 && value <= clippingLow )
            {
                pixelLookup[value].p[0] = 0xff;
                pixelLookup[value].p[1] = 0x80;
                pixelLookup[value].p[2] = 0x80;
                clipped = true;
            }
        }

        // Translate pixel value if not clipped
        if( !clipped )
        {
            // Start with original value
            int translatedValue = value;

            // Logarithmic brightness if required
            if( logBrightness )
            {
                translatedValue = int( log10( value+1 ) * 105.8864 );
            }

            // Reverse contrast if required
            if( contrastReversal )
            {
                translatedValue = MAX_VALUE - translatedValue;
            }

            // Save translated pixel
            if( imageDisplayProps->getFalseColour() )
            {
                pixelLookup[value] = getFalseColor ((unsigned char)translatedValue);
            }
            else
            {
                pixelLookup[value].p[0] = (unsigned char)translatedValue;
                pixelLookup[value].p[1] = (unsigned char)translatedValue;
                pixelLookup[value].p[2] = (unsigned char)translatedValue;
            }
        }

    }

    return;
}

// Determine the maximum pixel value for the current format
unsigned int imageProcessor::maxPixelValue()
{
    double result = 0;

    switch( formatOption )
    {
        case imageDataFormats::BAYERGB:
        case imageDataFormats::BAYERBG:
        case imageDataFormats::BAYERGR:
        case imageDataFormats::BAYERRG:
        case imageDataFormats::MONO:
            result = ((unsigned long)(1)<<bitDepth)-1;
            break;

        case imageDataFormats::RGB1:
        case imageDataFormats::RGB2:
        case imageDataFormats::RGB3:
            result = (1<<8)-1; //???!!! not done yet probably correct
            break;

        case imageDataFormats::YUV444:
        case imageDataFormats::YUV422:
        case imageDataFormats::YUV421:
            result = (1<<8)-1; //???!!! not done yet probably correct
            break;

        default:  // avoid  compilation warning for NUMBER_OF_FORMATS
            result = 0;
            break;
    }

    if( result == 0 )
    {
        result = 255;
    }

    return result;
}

// Return the image width following any rotation
unsigned int imageProcessor::rotatedImageBuffWidth()
{
    switch( rotation)
    {
        default:
        case ROTATION_0:
        case ROTATION_180:
            return imageBuffWidth;

        case ROTATION_90_RIGHT:
        case ROTATION_90_LEFT:
            return imageBuffHeight;
    }
}

// Return the image height following any rotation
unsigned int imageProcessor::rotatedImageBuffHeight()
{
    switch( rotation)
    {
        default:
        case ROTATION_0:
        case ROTATION_180:
            return imageBuffHeight;

        case ROTATION_90_RIGHT:
        case ROTATION_90_LEFT:
            return imageBuffWidth;
    }
}

// Get a false color representation for an entry from the color lookup table
imageDisplayProperties::rgbPixel imageProcessor::getFalseColor (const unsigned char value) {

    const int max = 0xFF;
    const int half = 0x80;
    const int lightness_slope = 4;
    const int low_hue = 240;    // blue.
    const int high_hue = 0;     // red

    int bp1;
    int bp2;
    imageDisplayProperties::rgbPixel result;
    int h, l;
    QColor c;

    // Range of inputs broken into three bands:
    // [0 .. bp1], [bp1 .. bp2] and [bp2 .. max]
    //
    bp1 = half / lightness_slope;
    bp2 = max - (max - half) / lightness_slope;

    if( value < bp1 ){
        // Constant hue (blue), lightness ramps up to 128
        h = low_hue;
        l = lightness_slope*value;
    } else if( value > bp2 ){
        // Constant hue (red), lightness ramps up from 128 to 255
        h = high_hue;
        l = max - lightness_slope*(max-value);
    } else {
        // The bit in the middle.
        // Contant lightness, hue varies blue to red.
        h = ((value - bp1)*high_hue + (bp2 - value)*low_hue) / (bp2 - bp1);
        l = half;
    }

    c.setHsl( h, max, l );   // Saturation always 100%

    result.p[0] = (unsigned char) c.blue();
    result.p[1] = (unsigned char) c.green();
    result.p[2] = (unsigned char) c.red();
    result.p[3] = (unsigned char) max; // Alpha always 100%

    return result;
}


// Determine the element count expected based on the available dimensions
int imageProcessor::getElementCount()
{
    // If we already have the image dimensions (and the elements per pixel if required), update the image
    // size we need here before the subscription.
    // (we should have image dimensions as a connection is only established once these have been read)
    if( imageBuffWidth && imageBuffHeight && ( numDimensions !=3 || imageDimension0))
    {
        // element count is at least width x height
        unsigned int elementCount = imageBuffWidth * imageBuffHeight;

        // Regardless of the souce of the width and height (either from width and height variables or from
        // the appropriate area detector dimension variables), if the number of area detector dimensions
        // is 3, then the first dimension is the number or elements per pixel so the element count needs to
        // be multiplied by the first area detector dimension.

        // It is possible for the image dimensions to change dynamically. For example to change from
        // 3 dimensions to 2. In this example, the first dimension may change from being the data elements
        // per pixel to being the image width before the 'number of dimensions' variable changes. This results
        // in a window where the first dimension is assumed to be the data elements per pixel (num dimensions is 3)
        // but it is actually the image width (much larger) this can result in crashes where a huge number of bytes
        // per pixel is assumed and data arrays are overrun. If the dimensions appear odd, 32 was chosen as being large enough to cater for the
        // largest number of elements per pixel. It is reasonable for image widths to be less than 32, so code must
        // still handle invalid bytes per pixel calculations.
        if( numDimensions == 3 && imageDimension0 && imageDimension0 <= 32 )
        {
            elementCount = elementCount * elementsPerPixel;
        }

        return elementCount;
    }

    // We can't determine the element count yet.
    else
    {
        return 0;
    }
}


// Determine if the image dimensional information is valid.
// A side effect of this method is to set elementsPerPixel.
// If an image dimensions change dynamically we may pass through a period where a set of dimensions that are nonsense. For example,
// if the number of dimensions is changing from 3 to 2, this means the first dimension will change from being the data elements
// per pixel to the image width. If the update for the first dimension arrives first, the number of dimensions will still be 3 (implying the
// first dimension is the number of data elements per pixel, but the the first dimension will be the image width.
// If the dimensions appear nonsense, then don't force an image update. Note, this won't stop an image update from occuring, so
// the image update must cope with odd dimensions, but just no point forcing it here.
// The test for good dimensions is to check if a width and height is present, and (if the first dimension is expected to be the number
// of data elements per pixel, then is is less than 32. 32 was chosen as being large enough for any pixel format (for example 32 bits
// per color for 4 Bayer RGBG colours) but less than most image widths. This test doesn't have to be perfect since the image update must
// be able to cope with an invalid set of dimensions as mentioned above.
bool imageProcessor::validateDimensions()
{
    unsigned long pixelCount = imageBuffWidth * imageBuffHeight;
    if( pixelCount && (( numDimensions != 3 ) || ( imageDimension0 < 32 ) ) )
    {
        if( numDimensions == 3 )
        {
            elementsPerPixel = imageDimension0;
        }
        else
        {
            elementsPerPixel = 1;
        }

        return true;
    }
    else
    {
        return false;
    }
}

// Determine the range of pixel values an area of the image
void imageProcessor::getPixelRange( const QRect& area, unsigned int* min, unsigned int* max )
{
    // If the area selected was the the entire image, and the image was not presented at 100%, rounding areas while scaling
    // may result in area dimensions outside than the actual image by a pixel or so, so limit the area to within the image.
    unsigned int areaX = (area.topLeft().x()>=0)?area.topLeft().x():0;
    unsigned int areaY = (area.topLeft().y()>=0)?area.topLeft().y():0;
    unsigned int areaW = (area.width() <=(int)rotatedImageBuffWidth() )?area.width() :rotatedImageBuffWidth();
    unsigned int areaH = (area.height()<=(int)rotatedImageBuffHeight())?area.height():rotatedImageBuffHeight();

    // Set up to step pixel by pixel through the area
    const unsigned char* data = (unsigned char*)imageData.constData();
    unsigned int index = (areaY*rotatedImageBuffWidth()+areaX)*bytesPerPixel;

    // This function is called as the user drags region handles around the
    // screen. Recalculating min and max pixels for large areas
    // for each mouse movement event needs to be efficient so speed loop by
    // extracting width and height. (Compiler can't assume QRect width
    // and height stays constant so it is evaluated each iteration of for
    // loop if it was in the form   'for( int i = 0; i < area.height(); i++ )'
    unsigned int stepW = bytesPerPixel;

    // Calculate the step to the start of the next row in the area selected.
    unsigned int stepH = (rotatedImageBuffWidth()-areaW)*bytesPerPixel;

    unsigned int maxP = 0;
    unsigned int minP = UINT_MAX;

    // Determine the maximum and minimum pixel values in the area
    for( unsigned int i = 0; i < areaH; i++ )
    {
        for( unsigned int j = 0; j < areaW; j++ )
        {
            unsigned int p = getPixelValueFromData( &(data[index]) );
            if( p < minP ) minP = p;
            if( p > maxP ) maxP = p;

            index += stepW;
        }
        index += stepH;
    }

    // Return results
    *min = minP;
    *max = maxP;
}

// Return a pointer to pixel data in the original image data.
// The position parameter is scaled to the original image size but reflects
// the displayed rotation and flip options, so it must be transformed first.
// Return NULL, if there is no image data, or point is beyond end of image data
const unsigned char* imageProcessor::getImageDataPtr( QPoint& pos )
{
    QPoint posTr;

    // Transform the position to reflect the original unrotated or flipped data
    posTr = rotateFlipToDataPoint( pos );

    // Set up reference to start of the data, and the index to the required pixel
    const unsigned char* data = (unsigned char*)imageData.constData();
    int index = (posTr.x()+posTr.y()*imageBuffWidth)*bytesPerPixel;

    // Return a pointer to the pixel data if possible
    if( !imageData.isEmpty() && index < imageData.size() )
    {
        return &(data[index]);
    }
    else
    {
        return NULL;
    }
}

// Return a number representing a pixel intensity given a pointer into an image data buffer.
// Note, the pointer is indexed according to the pixel data size which will be at least
// big enough for the data format.
int imageProcessor::getPixelValueFromData( const unsigned char* ptr )
{
    // Sanity check
    if( !ptr )
        return 0;

    // Case the data to the correct size, then return the data as a floating point number.
    switch( formatOption )
    {
        case imageDataFormats::BAYERGB:
        case imageDataFormats::BAYERBG:
        case imageDataFormats::BAYERGR:
        case imageDataFormats::BAYERRG:
        case imageDataFormats::MONO:
            {
                unsigned int usableDepth = bitDepth;
                if( bitDepth > (imageDataSize*8) )
                {
                    usableDepth = imageDataSize*8;
                }

                quint32 mask = ((unsigned long)(1)<<usableDepth)-1;

                return (*((quint32*)ptr))&mask;
            }

        case imageDataFormats::RGB1:
            {
                // for RGB, average all colors
                unsigned int pixel = *(unsigned int*)ptr;
                return ((pixel&0xff0000>>16) + (pixel&0x00ff00>>8) + (pixel&0x0000ff)) / 3;
            }

        case imageDataFormats::RGB2:
            //!!! not done - copy of RGB1
            {
                // for RGB, average all colors
                unsigned int pixel = *(unsigned int*)ptr;
                return ((pixel&0xff0000>>16) + (pixel&0x00ff00>>8) + (pixel&0x0000ff)) / 3;
            }

        case imageDataFormats::RGB3:
            //!!! not done - copy of RGB1
            {
                // for RGB, average all colors
                unsigned int pixel = *(unsigned int*)ptr;
                return ((pixel&0xff0000>>16) + (pixel&0x00ff00>>8) + (pixel&0x0000ff)) / 3;
            }

        case imageDataFormats::YUV444:
            //!!! not done - copy of RGB1
            {
                // for RGB, average all colors
                unsigned int pixel = *(unsigned int*)ptr;
                return ((pixel&0xff0000>>16) + (pixel&0x00ff00>>8) + (pixel&0x0000ff)) / 3;
            }

        case imageDataFormats::YUV422:
            //!!! not done - copy of RGB1
            {
                // for RGB, average all colors
                unsigned int pixel = *(unsigned int*)ptr;
                return ((pixel&0xff0000>>16) + (pixel&0x00ff00>>8) + (pixel&0x0000ff)) / 3;
            }

        case imageDataFormats::YUV421:
            //!!! not done - copy of RGB1
            {
                // for RGB, average all colors
                unsigned int pixel = *(unsigned int*)ptr;
                return ((pixel&0xff0000>>16) + (pixel&0x00ff00>>8) + (pixel&0x0000ff)) / 3;
            }

        default:   // avoid  compilation warning for NUMBER_OF_FORMATS
            break;
    }

    // Avoid compilation warning (not sure why this is required as all cases are handled in switch statements.
    return *ptr;
}

// Return a floating point number representing a pixel intensity given a pointer into an image data buffer.
double imageProcessor::getFloatingPixelValueFromData( const unsigned char* ptr )
{
    return getPixelValueFromData( ptr );
}

// Return a QImage based on the current image
QImage imageProcessor::copyImage()
{
    return image;
}

// Generate a profile along a line down an image at a given X position
// Input ordinates are scaled to the source image data.
// The profile contains values for each pixel intersected by the line.
void imageProcessor::generateVSliceData( QVector<QPointF>& vSliceData, int x, unsigned int thickness )
{
    // Ensure the buffer is the correct size
    if( vSliceData.size() != (int)rotatedImageBuffHeight() )
        vSliceData.resize( rotatedImageBuffHeight() );

    // Set up to step pixel by pixel through the image data along the line
    const unsigned char* data = (unsigned char*)imageData.constData();
    const unsigned char* dataPtr = &(data[x*bytesPerPixel]);
    int dataPtrStep = rotatedImageBuffWidth()*bytesPerPixel;

    // Set up to step through the line thickness
    unsigned int halfThickness = thickness/2;
    int xMin = x-halfThickness;
    if( xMin < 0 ) xMin = 0;
    int xMax =  xMin+thickness;
    if( xMax >= (int)rotatedImageBuffWidth() ) xMax = rotatedImageBuffWidth();

    // Accumulate data for each pixel in the thickness
    bool firstPass = true;
    for( int nextX = xMin; nextX < xMax; nextX++ )
    {
        // Accumulate the image data value at each pixel.
        // The buffer is filled backwards so the plot, which sits on its side beside the image is drawn correctly
        QPoint pos;
        pos.setX( nextX );
        for( int i = rotatedImageBuffHeight()-1; i >= 0; i-- )
        {
            pos.setY( i );
            QPointF* dataPoint = &vSliceData[i];
            double value = getFloatingPixelValueFromData( getImageDataPtr( pos ) );

            // On first pass, set up X and Y
            if( firstPass )
            {
                dataPoint->setY( i );
                dataPoint->setX( value );
            }

            // On subsequent passes (when thickness is greater than 1), accumulate X
            else
            {
                dataPoint->setX( dataPoint->x() + value );
            }

            dataPtr += dataPtrStep;
        }

        firstPass = false;
    }

    // Calculate average pixel values if more than one pixel thick
    if( thickness > 1 )
    {
        for( int i = rotatedImageBuffHeight()-1; i >= 0; i-- )
        {
            QPointF* dataPoint = &vSliceData[i];
            dataPoint->setX( dataPoint->x()/thickness );
        }
    }
}

// Generate a profile along a line across an image at a given Y position
// Input ordinates are at the resolution of the source image data
// The profile contains values for each pixel intersected by the line.
void imageProcessor::generateHSliceData( QVector<QPointF>& hSliceData, int y, unsigned int thickness )
{
    // Ensure the buffer is the correct size
    if( hSliceData.size() != (int)rotatedImageBuffWidth() )
        hSliceData.resize( rotatedImageBuffWidth() );

    // Set up to step pixel by pixel through the image data along the line
    const unsigned char* data = (unsigned char*)imageData.constData();
    const unsigned char* dataPtr = &(data[y*rotatedImageBuffWidth()*bytesPerPixel]);
    int dataPtrStep = bytesPerPixel;

    // Set up to step through the line thickness
    unsigned int halfThickness = thickness/2;
    int yMin = y-halfThickness;
    if( yMin < 0 ) yMin = 0;
    int yMax =  yMin+thickness;
    if( yMax >= (int)rotatedImageBuffHeight() ) yMax = rotatedImageBuffHeight();

    // Accumulate data for each pixel in the thickness
    bool firstPass = true;
    for( int nextY = yMin; nextY < yMax; nextY++ )
    {
        // Accumulate the image data value at each pixel.
        QPoint pos;
        pos.setY( nextY );
        for( unsigned int i = 0; i < rotatedImageBuffWidth(); i++ )
        {
            pos.setX( i );
            QPointF* dataPoint = &hSliceData[i];
            double value = getFloatingPixelValueFromData( getImageDataPtr( pos ) );

            // On first pass, set up X and Y
            if( firstPass )
            {
                dataPoint->setX( i );
                dataPoint->setY( value );
            }

            // On subsequent passes (when thickness is greater than 1), accumulate X
            else
            {
                dataPoint->setY( dataPoint->y() + value );
            }

            dataPtr += dataPtrStep;
        }

        firstPass = false;
    }


    // Calculate average pixel values if more than one pixel thick
    if( thickness > 1 )
    {
        for( unsigned int i = 0; i < rotatedImageBuffWidth(); i++ )
        {
            QPointF* dataPoint = &hSliceData[i];
            dataPoint->setY( dataPoint->y()/thickness );
        }
    }
}

// Generate a profile along an arbitrary line through an image.
// Input ordinates are scaled to the source image data.
// The profile contains values one pixel length along the line.
// Except where the line is vertical or horizontal points one pixel
// length along the line will not line up with actual pixels.
// The values returned are a weighted average of the four actual pixels
// containing a notional pixel drawn around the each point on the line.
//
// In the example below, a line was drawn from pixels (1,1) to (3,3).
//
// The starting and ending points are the center of the start and end
// pixels: (1.5,1.5)  (3.5,3.5)
//
// The points along the line one pixel length apart are roughly at points
// (1.5,1.5) (2.2,2.2) (2.9,2.9) (3.6,3.6)
//
// The points are marked in the example with an 'x'.
//
//     0       1       2       3       4
//   +-------+-------+-------+-------+-------+
//   |       |       |       |       |       |
// 0 |       |       |       |       |       |
//   |       |       |       |       |       |
//   +-------+-------+-------+-------+-------+
//   |       |       |       |       |       |
// 1 |       |   x ......... |       |       |
//   |       |     . |     . |       |       |
//   +-------+-----.-+-----.-+-------+-------+
//   |       |     . | x   . |       |       |
// 2 |       |     . |     . |       |       |
//   |       |     .........x|       |       |
//   +-------+-------+-------+-------+-------+
//   |       |       |       |       |       |
// 3 |       |       |       |   x   |       |
//   |       |       |       |       |       |
//   +-------+-------+-------+-------+-------+
//   |       |       |       |       |       |
// 4 |       |       |       |       |       |
//   |       |       |       |       |       |
//   +-------+-------+-------+-------+-------+
//
// The second point has a notional pixel drawn around it like so:
//      .........
//      .       .
//      .       .
//      .   x   .
//      .       .
//      .........
//
// This notional pixel overlaps pixels (1,1) (1,2) (2,1) and (2,2).
//
// The notional pixel overlaps about 10% of pixel (1,1),
// 20% of pixels (1,2) and (2,1) and 50% of pixel (2,2).
//
// A value for the second point will be the sum of the four pixels
// overlayed by the notional pixel weighted by these values.
//
// The line has a notional thickness. The above processing for a single
// pixel width is repeated with the start and end points moved at right
// angles to the line by a 'pixel' distance up to the line thickness.
// The results are then averaged.
//
void imageProcessor::generateProfileData( QVector<QPointF>& profileData, QPoint point1, QPoint point2, unsigned int thickness )
{
    // X and Y components of line drawn
    double dX = point2.x()-point1.x();
    double dY = point2.y()-point1.y();

    // Line length
    double len = sqrt( dX*dX+dY*dY );

    // Step on each axis to move one 'pixel' length
    double xStep = dX/len;
    double yStep = dY/len;

    // Starting point in center of start pixel
    double initX = point1.x()+0.5;
    double initY = point1.y()+0.5;

    // Ensure output buffer is the correct size
    if( profileData.size() != len )
    {
       profileData.resize( int( len ) );
    }

    // Integer pixel length
    int intLen = (int)len;

    // Parrallel passes will be made one 'pixel' away from each other up to the thickness required.
    // Determine the offset for the first pass.
    // Note, this will not add an offset for a thickness of 1 pixel
    initX -= yStep * (double)(thickness-1) / 2;
    initY += xStep * (double)(thickness-1) / 2;

    // Accumulate a set of values for each pixel width up to the thickness required
    bool firstPass = true;
    for( unsigned int j = 0; j < thickness; j++ )
    {
        // Starting point for this pass
        double x = initX;
        double y = initY;

        // Calculate a value for each pixel length along the selected line
        for( int i = 0; i < intLen; i++ )
        {
            // Calculate the value if the point is within the image (user can drag outside the image)
            double value;
            if( x >= 0 && x < rotatedImageBuffWidth() && y >= 0 && y < rotatedImageBuffHeight() )
            {

                // Determine the top left of the notional pixel that will be measured
                // The notional pixel is one pixel length both dimensions and will not
                // nessesarily overlay a single real pixel
                double xTL = x-0.5;
                double yTL = y-0.5;

                // Determine the top left actual pixel of the four actual pixels that
                // the notional pixel overlays, and the fractional part of a pixel that
                // the notional pixel is offset by.
                double xTLi, xTLf; // i = integer part, f = fractional part
                double yTLi, yTLf; // i = integer part, f = fractional part

                xTLf = modf( xTL, & xTLi );
                yTLf = modf( yTL, & yTLi );

                // For each of the four actual pixels that the notional pixel overlays,
                // determine the proportion of the actual pixel covered by the notional pixel
                double propTL = (1.0-xTLf)*(1-yTLf);
                double propTR = (xTLf)*(1-yTLf);
                double propBL = (1.0-xTLf)*(yTLf);
                double propBR = (xTLf)*(yTLf);

                // Determine a pointer into the image data for each of the four actual pixels overlayed by the notional pixel
                int actualXTL = (int)xTLi;
                int actualYTL = (int)yTLi;
                QPoint posTL( actualXTL,   actualYTL );
                QPoint posTR( actualXTL+1, actualYTL );
                QPoint posBL( actualXTL,   actualYTL+1 );
                QPoint posBR( actualXTL+1, actualYTL+1 );

                const unsigned char* dataPtrTL = getImageDataPtr( posTL );
                const unsigned char* dataPtrTR = getImageDataPtr( posTR );
                const unsigned char* dataPtrBL = getImageDataPtr( posBL );
                const unsigned char* dataPtrBR = getImageDataPtr( posBR );

                // Determine the value of the notional pixel from a weighted average of the four real pixels it overlays.
                // The larger the proportion of the real pixel overlayed, the greated the weight.
                // (Ignore pixels outside the image)
                int pixelsInValue = 0;
                value = 0;
                if( xTLi >= 0 && yTLi >= 0 )
                {
                    value += propTL * getFloatingPixelValueFromData( dataPtrTL );
                    pixelsInValue++;
                }

                if( xTLi+1 < rotatedImageBuffWidth() && yTLi >= 0 )
                {
                    value += propTR * getFloatingPixelValueFromData( dataPtrTR );
                    pixelsInValue++;
                }

                if( xTLi >= 0 && yTLi+1 < rotatedImageBuffHeight() )
                {

                    value += propBL * getFloatingPixelValueFromData( dataPtrBL );
                    pixelsInValue++;
                }
                if( xTLi+1 < rotatedImageBuffWidth() && yTLi+1 < rotatedImageBuffHeight() )
                {
                    value += propBR * getFloatingPixelValueFromData( dataPtrBR );
                    pixelsInValue++;
                }


                // Calculate the weighted value
                value = value / pixelsInValue * 4;

                // Move on to the next 'point'
                x+=xStep;
                y+=yStep;
            }

            // Use a value of zero if the point is not within the image (user can drag outside the image)
            else
            {
                value = 0.0;
            }

            // Get a reference to the current data point
            QPointF* data = &profileData[i];

            // If the first pass, set the X axis and the initial data value
            if( firstPass )
            {
                data->setX( i );
                data->setY( value );
            }

            // On consequent passes, accumulate the data value
            else
            {
                data->setY( data->y() + value );
            }
        }

        initX += yStep;
        initY -= xStep;

        firstPass = false;

    }

    // Average the values
    for( int i = 0; i < intLen; i++ )
    {
        QPointF* data = &profileData[i];
        data->setY( data->y() / thickness );
    }
}

// Transform a rectangle in the displayed image to a rectangle in the
// original data according to current rotation and flip options.
QRect imageProcessor::rotateFlipToDataRectangle( const QRect& rect )
{
    QPoint pos1 = rect.topLeft();
    QPoint pos2 = rect.bottomRight();
    return rotateFlipToDataRectangle( pos1, pos2 );
}

// Transform a rectangle (defined by two points) in the displayed image to
// a rectangle in the original data according to current rotation and flip options.
QRect imageProcessor::rotateFlipToDataRectangle( const QPoint& pos1, const QPoint& pos2 )
{
    QPoint trPos1 = rotateFlipToDataPoint( pos1 );
    QPoint trPos2 = rotateFlipToDataPoint( pos2 );

    QRect trRect( trPos1, trPos2 );
    trRect = trRect.normalized();

    return trRect;
}

// Transform a point in the displayed image to a point in the original
// data according to current rotation and flip options.
QPoint imageProcessor::rotateFlipToDataPoint( const QPoint& pos )
{
    // Transform the point according to current rotation and flip options.
    // Depending on the flipping and rotating options pixel drawing can start in any of
    // the four corners and start scanning either vertically or horizontally.
    // The 8 scanning options are shown numbered here:
    //
    //    o----->1         2<-----o
    //    |                       |
    //    |                       |
    //    |                       |
    //    v                       v
    //    5                       6
    //
    //
    //
    //    7                       8
    //    ^                       ^
    //    |                       |
    //    |                       |
    //    |                       |
    //    o----->3         4<-----o
    //
    // A point from a rotated and fliped image needs to be transformed to be able to
    // reference pixel data in the original data buffer.
    // Base the transformation on the scanning option used when building the image
    int w = (int)imageBuffWidth-1;
    int h = (int)imageBuffHeight-1;
    QPoint posTr;
    int scanOption = getScanOption();
    switch( scanOption )
    {
        default:
        case 1: posTr = pos;                                      break;
        case 2: posTr.setX( w-pos.x() ); posTr.setY( pos.y() );   break;
        case 3: posTr.setX( pos.x() );   posTr.setY( h-pos.y() ); break;
        case 4: posTr.setX( w-pos.x() ); posTr.setY( h-pos.y() ); break;
        case 5: posTr.setX( pos.y() );   posTr.setY( pos.x() );   break;
        case 6: posTr.setX( w-pos.y() ); posTr.setY( pos.x() );   break;
        case 7: posTr.setX( pos.y() );   posTr.setY( h-pos.x() ); break;
        case 8: posTr.setX( w-pos.y() ); posTr.setY( h-pos.x() ); break;
    }

    return posTr;
}

// Transform a rectangle in the original data to a rectangle in the
// displayed image according to current rotation and flip options.
QRect imageProcessor::rotateFlipToImageRectangle( const QRect& rect )
{
    QPoint pos1 = rect.topLeft();
    QPoint pos2 = rect.bottomRight();
    return rotateFlipToImageRectangle( pos1, pos2 );
}

// Transform a rectangle (defined by two points) in the original data to a rectangle in the
// displayed image according to current rotation and flip options.
QRect imageProcessor::rotateFlipToImageRectangle( const QPoint& pos1, const QPoint& pos2 )
{
    QPoint trPos1 = rotateFlipToImagePoint( pos1 );
    QPoint trPos2 = rotateFlipToImagePoint( pos2 );

    QRect trRect( trPos1, trPos2 );
    trRect = trRect.normalized();

    return trRect;
}

// Transform a point in the original data to a point in the image
// according to current rotation and flip options.
QPoint imageProcessor::rotateFlipToImagePoint( const QPoint& pos )
{
    // Transform the point according to current rotation and flip options.
    // Depending on the flipping and rotating options pixel drawing can start in any of
    // the four corners and start scanning either vertically or horizontally.
    // The 8 scanning options are shown numbered here:
    //
    //    o----->1         2<-----o
    //    |                       |
    //    |                       |
    //    |                       |
    //    v                       v
    //    5                       6
    //
    //
    //
    //    7                       8
    //    ^                       ^
    //    |                       |
    //    |                       |
    //    |                       |
    //    o----->3         4<-----o
    //
    // A point from a rotated and fliped image needs to be transformed to be able to
    // reference pixel data in the original data buffer.
    // Base the transformation on the scanning option used when building the image
    int w = (int)imageBuffWidth-1;
    int h = (int)imageBuffHeight-1;
    QPoint posTr;
    int scanOption = getScanOption();
    switch( scanOption )
    {
        default:
        case 1: posTr = pos;                                      break;
        case 2: posTr.setX( w-pos.x() ); posTr.setY( pos.y() );   break;
        case 3: posTr.setX( pos.x() );   posTr.setY( h-pos.y() ); break;
        case 4: posTr.setX( w-pos.x() ); posTr.setY( h-pos.y() ); break;
        case 5: posTr.setX( pos.y() );   posTr.setY( pos.x() );   break;
        case 6: posTr.setX( pos.y() );   posTr.setY( w-pos.x() ); break;
        case 7: posTr.setX( h-pos.y() ); posTr.setY( pos.x() );   break;
        case 8: posTr.setX( h-pos.y() ); posTr.setY( w-pos.x() ); break;
    }

    return posTr;
}

// end
