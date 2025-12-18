/*  imageInfo.h
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

#ifndef QE_IMAGE_INFO_H
#define QE_IMAGE_INFO_H

#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QDateTime>
#include <QEFrameworkLibraryGlobal.h>

#define UPDATE_INDICATOR_SIZE 20
#define UPDATE_INDICATOR_STEPS 32

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT imageUpdateIndicator : public QWidget
{
public:
    imageUpdateIndicator();
    ~imageUpdateIndicator();
    void freshImage();
    void paintEvent ( QPaintEvent * );

private:
    int imageCount;
    QList<QRect> lines;
};

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT imageInfo
{
public:
    imageInfo();                                            // Initialise the information area
    void showInfo( bool show );

    QLayout* getInfoWidget();                               // Return the layout of the infomation area for insertion into the main QEImage widget
    void infoShow( const bool show );                       // Display or hide the contents of the information area

    void infoUpdateTarget();                                // Clear the target information
    void infoUpdateTarget( const int x, const int y );      // Update the target information

    void infoUpdateBeam();                                  // Clear the beam information
    void infoUpdateBeam( const int x, const int y );        // Update the beam information

    void infoUpdateVertProfile();                                                                   // Clear the vertical profile information
    void infoUpdateVertProfile( const int x, const unsigned int thickness );                        // Update the vertical profile information

    void infoUpdateHozProfile();                                                                    // Clear the horizontal profile information
    void infoUpdateHozProfile( const int y, const unsigned int thickness );                         // Update the horizontal profile information

    void infoUpdateProfile();                                                                       // Clear the arbitrarty profile information
    void infoUpdateProfile( const QPoint start, const QPoint end, const unsigned int thickness );   // Update the arbitrary profile information

    void infoUpdateRegion( const unsigned int region );                                             // Clear the region information
    void infoUpdateRegion( const unsigned int region, const int x1, const int y1, const int x2, const int y2 );// Update the region information

    void infoUpdatePixel();                                 // Clear the current pixel information
    void infoUpdatePixel( const QPoint pos, int value );    // Update the current pixel information

    void infoUpdateZoom();                                                             // Clear the zoom level information
    void infoUpdateZoom( int value, const double XStretch, const double YStretch );    // Update the zoom level information

    void infoUpdatePaused();                                 // Clear the 'paused' information
    void infoUpdatePaused( bool paused );                    // Update the 'paused' information

    void setBriefInfoArea( const bool briefIn );            // Set if displaying all info, or a brief summary
    bool getBriefInfoArea();                                // Report if displaying all info, or a brief summary

    void freshImage( QDateTime& time );                     // Indicate another image has arrived

private:
    bool show;
    bool brief;

    QGridLayout* infoLayout;
    QLabel* currentCursorPixelLabel;
    QLabel* currentVertPixelLabel;
    QLabel* currentHozPixelLabel;
    QLabel* currentLineLabel;
    QLabel* currentArea1Label;
    QLabel* currentArea2Label;
    QLabel* currentArea3Label;
    QLabel* currentArea4Label;
    QLabel* currentTargetLabel;
    QLabel* currentBeamLabel;
    QLabel* currentPausedLabel;
    QLabel* currentZoomLabel;

    imageUpdateIndicator* updateIndicator;
};

#endif // QE_IMAGE_INFO_H
