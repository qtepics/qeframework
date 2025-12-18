/*  imageInfo.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/*
  This class manages the presentation of textual information about an image in a QEImage widget,
  such as what is the pixel value the cursor is over.
  It is tighly integrated with the QEImage widget. Refer to QEImage.cpp for to see how it is used.
 */

#include "imageInfo.h"
#include <QPainter>
#define _USE_MATH_DEFINES
#include <math.h>
#include <QDebug>
#include <QHBoxLayout>

// Initialise the information area
imageInfo::imageInfo()
{
    show = false;
    brief = false;

    currentCursorPixelLabel = new QLabel();
    currentVertPixelLabel = new QLabel();
    currentHozPixelLabel = new QLabel();
    currentLineLabel = new QLabel();
    currentArea1Label = new QLabel();
    currentArea2Label = new QLabel();
    currentArea3Label = new QLabel();
    currentArea4Label = new QLabel();
    currentTargetLabel = new QLabel();
    currentBeamLabel = new QLabel();
    currentPausedLabel = new QLabel();
    currentZoomLabel = new QLabel();

    updateIndicator = new imageUpdateIndicator();

    QHBoxLayout* imageLayout = new QHBoxLayout();
    imageLayout->addWidget( updateIndicator );
    imageLayout->addWidget( currentPausedLabel );

    infoLayout = new QGridLayout();
    infoLayout->addWidget( currentCursorPixelLabel, 0, 0 );
    infoLayout->addLayout( imageLayout, 0, 1 );
    infoLayout->addWidget( currentZoomLabel, 0, 2 );
    infoLayout->addWidget( currentVertPixelLabel, 1, 0 );
    infoLayout->addWidget( currentHozPixelLabel, 1, 1 );
    infoLayout->addWidget( currentLineLabel, 1, 2 );
    infoLayout->addWidget( currentArea1Label, 2, 0 );
    infoLayout->addWidget( currentArea2Label, 2, 1 );
    infoLayout->addWidget( currentArea3Label, 2, 2 );
    infoLayout->addWidget( currentArea4Label, 2, 3 );
    infoLayout->addWidget( currentTargetLabel, 3, 0 );
    infoLayout->addWidget( currentBeamLabel, 3, 1 );
}

// Return the layout of the infomation area for insertion into the main QEImage widget
QLayout* imageInfo::getInfoWidget()
{
    return infoLayout;
}

void imageInfo::setBriefInfoArea( const bool briefIn )
{
    // Save the state
    brief = briefIn;

    // Update the info, only if currently shown
    if( show )
    {
        showInfo( true );
    }
}

bool imageInfo::getBriefInfoArea()
{
    return brief;
}

// Display or hide the contents of the information area
void imageInfo::showInfo( const bool showIn )
{
    show = showIn;
    if( show )
    {
        currentCursorPixelLabel->show();
        currentPausedLabel->show();
        updateIndicator->show();
        currentZoomLabel->show();
        currentVertPixelLabel->setHidden( brief );
        currentHozPixelLabel->setHidden( brief );
        currentLineLabel->setHidden( brief );
        currentArea1Label->setHidden( brief );
        currentArea2Label->setHidden( brief );
        currentArea3Label->setHidden( brief );
        currentArea4Label->setHidden( brief );
        currentTargetLabel->setHidden( brief );
        currentBeamLabel->setHidden( brief );
    }
    else
    {
        currentCursorPixelLabel->hide();
        currentPausedLabel->hide();
        updateIndicator->hide();
        currentZoomLabel->hide();
        currentVertPixelLabel->hide();
        currentHozPixelLabel->hide();
        currentLineLabel->hide();
        currentArea1Label->hide();
        currentArea2Label->hide();
        currentArea3Label->hide();
        currentArea4Label->hide();
        currentTargetLabel->hide();
        currentBeamLabel->hide();
    }
}

// Clear the vertical profile information
void imageInfo::infoUpdateVertProfile()
{
    currentVertPixelLabel->clear();
}

// Clear the horizontal profile information
void imageInfo::infoUpdateHozProfile()
{
    currentHozPixelLabel->clear();
}

// Clear the arbitrarty profile information
void imageInfo::infoUpdateProfile()
{
    currentLineLabel->clear();
}

// Clear the beam information
void imageInfo::infoUpdateBeam()
{
    currentBeamLabel->clear();
}

// Clear the target information
void imageInfo::infoUpdateTarget()
{
    currentTargetLabel->clear();
}

// Clear the region information
void imageInfo::infoUpdateRegion( const unsigned int region )
{
    switch( region )
    {
    case 1:
        currentArea1Label->clear();
        break;

    case 2:
        currentArea2Label->clear();
        break;

    case 3:
        currentArea3Label->clear();
        break;

    case 4:
        currentArea4Label->clear();
        break;

    default:
        break;
    }
}

// Clear the current pixel information
void imageInfo::infoUpdatePixel()
{
    currentCursorPixelLabel->clear();
}

// Clear the current paused information
void imageInfo::infoUpdatePaused()
{
    currentPausedLabel->clear();
}

// Clear the current zoom information
void imageInfo::infoUpdateZoom()
{
    currentZoomLabel->clear();
}



// Update the target information
void imageInfo::infoUpdateTarget( const int x, const int y )
{
    // Display textual info
    currentTargetLabel->setText( QString( "T: (%1,%2)" ).arg( x ).arg( y ) );
}

// Update the beam information
void imageInfo::infoUpdateBeam( const int x, const int y )
{
    // Display textual info
    currentBeamLabel->setText( QString( "B: (%1,%2)" ).arg( x ).arg( y ) );
}

// Update the vertical profile information
void imageInfo::infoUpdateVertProfile( const int x, const unsigned int thickness )
{
    currentVertPixelLabel->setText( QString( "V1: %1 x %2" ).arg( x ).arg( thickness ) );
}

// Update the horizontal profile information
void imageInfo::infoUpdateHozProfile( const int y, const unsigned int thickness )
{
    currentHozPixelLabel->setText( QString( "H1: %1 x %2" ).arg( y ).arg( thickness ) );
}

// Update the arbitrary profile information
void imageInfo::infoUpdateProfile( QPoint start, QPoint end, const unsigned int thickness )
{
    currentLineLabel->setText( QString( "L: (%1,%2)(%3,%4)x%5" ).arg( start.x() ).arg( start.y() ).arg( end.x()).arg( end.y()).arg( thickness ) );
}


// Update the region information
void imageInfo::infoUpdateRegion( const unsigned int region, const int x1, const int y1, const int x2, const int y2 )
{
    switch( region )
    {
    case 1:
        currentArea1Label->setText( QString( "R1: (%1,%2)(%3,%4)" ).arg( x1 ).arg( y1 ).arg( x2 ).arg( y2 ) );
        break;

    case 2:
        currentArea2Label->setText( QString( "R2: (%1,%2)(%3,%4)" ).arg( x1 ).arg( y1 ).arg( x2 ).arg( y2 ) );
        break;

    case 3:
        currentArea3Label->setText( QString( "R3: (%1,%2)(%3,%4)" ).arg( x1 ).arg( y1 ).arg( x2 ).arg( y2 ) );
        break;

    case 4:
        currentArea4Label->setText( QString( "R4: (%1,%2)(%3,%4)" ).arg( x1 ).arg( y1 ).arg( x2 ).arg( y2 ) );
        break;

    default:
        break;
    }
}

// Update the current pixel information
void imageInfo::infoUpdatePixel( const QPoint pos, int value )
{
    currentCursorPixelLabel->setText( QString( "(%1,%2)=%3" ).arg( pos.x() ).arg( pos.y() ).arg( value ) );
}

// Update the paused information
void imageInfo::infoUpdatePaused( bool paused )
{
    currentPausedLabel->setText( paused?QString( "Paused" ):QString( "Waiting for an image" ));
}

// Update the zoom information
void imageInfo::infoUpdateZoom( const int zoom, const double XStretch, const double YStretch )
{
    QString zoomText;

    // Format the zoom
    zoomText = QString( "Zoom: %1%" ).arg( zoom );

    // Add X or Y stretch factors of present
    if( XStretch != 1.0 &&  YStretch != 1.0)
    {
        zoomText.append( QString( "(X %1, Y %2)").arg( XStretch ).arg( YStretch ));
    }
    else if( XStretch != 1.0 )
    {
        zoomText.append( QString( "(X %1)").arg( XStretch ));
    }
    else if( YStretch != 1.0 )
    {
        zoomText.append( QString( "(Y %1)").arg( YStretch ));
    }

    // Display the zoom text
    currentZoomLabel->setText( zoomText );
}

// Update the 'new image' indicator
void imageInfo::freshImage( QDateTime& time )
{
    currentPausedLabel->setText( time.toString( "hh:mm:ss.zzz" ));
    updateIndicator->freshImage();
}

//=============================================
// Class to give a visual indication of the image update rate (a rotating line as the image updates)

// Construction
imageUpdateIndicator::imageUpdateIndicator()
{
    // Fix the size
    setMinimumWidth( UPDATE_INDICATOR_SIZE );
    setMinimumHeight( UPDATE_INDICATOR_SIZE );
    setMaximumWidth( UPDATE_INDICATOR_SIZE );
    setMaximumHeight( UPDATE_INDICATOR_SIZE );

    // Initialise
    imageCount = 0;

    // Calculate a set of lines
    float angle = (float)(2*M_PI) / UPDATE_INDICATOR_STEPS;

    for( int i = 0; i < UPDATE_INDICATOR_STEPS; i++ )
    {
        QRect line;
        int cX = UPDATE_INDICATOR_SIZE/2;
        int cY = UPDATE_INDICATOR_SIZE/2;
        int len = UPDATE_INDICATOR_SIZE/2;

        float x = len * cos( angle * (float)i );
        float y = len * sin( angle * (float)i );

        line.setLeft( cX-x );
        line.setRight( cX+x );
        line.setTop( cY-y );
        line.setBottom( cY+y );

        lines.append( line );
    }

}

imageUpdateIndicator::~imageUpdateIndicator()
{

}

// Manage a paint event in image update indicator
void imageUpdateIndicator::paintEvent(QPaintEvent* )
{
    // Draw the indicator line
    QPainter painter(this);
    painter.drawLine( lines.at( imageCount ).topLeft(), lines.at( imageCount ).bottomRight() );
}

// Update the zoom information
void imageUpdateIndicator::freshImage()
{
    // step onto the next indicator line
    imageCount++;
    if(imageCount>=lines.count() )
    {
        imageCount = 0;
    }

    // Redraw with new indicator line
    update();
}
