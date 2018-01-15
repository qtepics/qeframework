/*  QEimage.h
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
 *  Copyright (c) 2012,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QE_IMAGE_H
#define QE_IMAGE_H

#include <QScrollArea>
#include <QEWidget.h>
#include <QEInteger.h>
#include <videowidget.h>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QToolBar>
#include <zoomMenu.h>
#include <flipRotateMenu.h>
#include <selectMenu.h>
#include <markupDisplayMenu.h>
#include <QEImageOptionsDialog.h>
#include <QCaVariableNamePropertyManager.h>
#include <imageInfo.h>
#include <brightnessContrast.h>
#include <applicationLauncher.h>
#include "imageDataFormats.h"

#include <QEStringFormatting.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QEIntegerFormatting.h>
#include <QEFloatingFormatting.h>
#include <fullScreenWindow.h>
#include <recording.h>
#include <imageProperties.h>
#include <imageProcessor.h>

#include <mpeg.h>

// Differed class declaration - no visible dependency on profilePlot.h (it is
// included from the .cpp file) and hence no visible dependency on qwt_plot.h
// and qwt_plot_curve.h.
//
class profilePlot;

// Class to keep track of a rectangular area such as region of interest or profile line information
// As data arrives, this class is used to record it.
class areaInfo
{
    public:
        // Construction
        areaInfo() { haveX1 = false; haveY1 = false; haveX2 = false; haveY2 = false; }

        // Set elements
        void setX1( long x ) { p1.setX( x ); haveX1 = true; }
        void setY1( long y ) { p1.setY( y ); haveY1 = true; }
        void setX2( long x ) { p2.setX( x ); haveX2 = true; }
        void setY2( long y ) { p2.setY( y ); haveY2 = true; }

        void setX( long x ) { int w = p2.x()-p1.x(); p1.setX( x ); p2.setX( x+w ); haveX1 = true; }
        void setY( long y ) { int h = p2.y()-p1.y(); p1.setY( y ); p2.setY( y+h ); haveY1 = true; }
        void setW( long w ) { p2.setX( p1.x()+w ); haveX2 = true; }
        void setH( long h ) { p2.setY( p1.y()+h ); haveY2 = true; }

        void setPoint1( QPoint p1In ) { p1 = p1In; haveX1 = true; haveY1 = true; }
        void setPoint2( QPoint p2In ) { p2 = p2In; haveX2 = true; haveY2 = true; }

        // Clear elements (invalid data)
        void clearX1() { haveX1 = false; }
        void clearY1() { haveY1 = false; }
        void clearX2() { haveX2 = false; }
        void clearY2() { haveY2 = false; }

        void clearX() { clearX1(); }
        void clearY() { clearY1(); }
        void clearW() { clearX2(); }
        void clearH() { clearY2(); }

        // Get ROI info
        bool getStatus() { return haveX1 && haveY1 && haveX2 && haveY2; }
        QRect getArea() { return QRect( p1, p2 ); }
        QPoint getPoint1() { return p1; }
        QPoint getPoint2() { return p2; }

    private:
        QPoint p1;
        QPoint p2;
        bool haveX1;
        bool haveY1;
        bool haveX2;
        bool haveY2;
};

// Class to keep track of a point such as beam or target information
// As data arrives, this class is used to record it.
class pointInfo
{
    public:
        // Construction
        pointInfo() { haveX = false; haveY = false; }

        // Set elements
        void setX( long x ) { p.setX( x ); haveX = true; }
        void setY( long y ) { p.setY( y ); haveY = true; }

        void setPoint( QPoint pIn ) { p = pIn; haveX = true; haveY = true; }

        // Clear elements (invalid data)
        void clearX() { haveX = false; }
        void clearY() { haveY = false; }

        // Get ROI info
        bool getStatus() { return haveX && haveY; }
        QPoint getPoint() { return p; }

    private:
        QPoint p;
        bool haveX;
        bool haveY;
};

/*!
  This class is a EPICS aware image widget.
  When image related variables are defined the image will be displayed.
  Many PVs may be defined to allow user interaction, such as selecting regions of interest.
  It is tighly integrated with the base class QEWidget which provides generic support such as macro substitutions, drag/drop, and standard properties.
 */
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEImage : public QFrame, public QEWidget, public imageInfo {
    Q_OBJECT
  public:
    /// Create without a variable.
    /// Use setVariableName'n'Property() - where 'n' is a number from 0 to 40 - and setSubstitutionsProperty() to define variables and, optionally, macro substitutions later.
    /// Note, each variable property is named by function (such as imageVariable and widthVariable) but given
    /// a numeric get and set property access function such as setVariableName22Property(). Refer to the
    /// property definitions to determine what 'set' and 'get' function is used for each varible, or use Qt library functions to set or get the variable names by name.
    QEImage( QWidget *parent = 0 );

    /// Create with a variable.
    /// A connection is automatically established.
    /// The variable is set up as the first variable.
    /// This is consistant with other widgets, but will not result in an updating image as the width and height variables are required as a minimum.
    QEImage( const QString &variableName, QWidget *parent = 0 );

    /// Destructor
    ~QEImage();

public:
    ///  \enum selectOptions.
    /// Internal use only. Selection options. What will happen when the user interacts with the image area
    enum selectOptions{ SO_NONE,                                ///< Do nothing
                        SO_PANNING,                             ///< User is panning
                        SO_VSLICE1,                             ///< Select the vertical slice 1 point
                        SO_VSLICE2,                             ///< Select the vertical slice 2 point
                        SO_VSLICE3,                             ///< Select the vertical slice 3 point
                        SO_VSLICE4,                             ///< Select the vertical slice 4 point
                        SO_VSLICE5,                             ///< Select the vertical slice 5 point
                        SO_HSLICE1,                             ///< Select the horizontal slice 1 point
                        SO_HSLICE2,                             ///< Select the horizontal slice 2 point
                        SO_HSLICE3,                             ///< Select the horizontal slice 3 point
                        SO_HSLICE4,                             ///< Select the horizontal slice 4 point
                        SO_HSLICE5,                             ///< Select the horizontal slice 5 point
                        SO_AREA1, SO_AREA2, SO_AREA3, SO_AREA4, ///< User is selecting an area (for region of interest)
                        SO_PROFILE,                             ///< Select an arbitrary line across the image (to determine a profile)
                        SO_TARGET,                              ///< Mark the target point
                        SO_BEAM                                 ///< Mark the current beam location
                       };

    selectOptions getSelectionOption();    // Get the current selection option

    enum imageUses { IMAGE_USE_DISPLAY, IMAGE_USE_SAVE, IMAGE_USE_DISPLAY_AND_SAVE };

public:
    // Property convenience functions

    void setBitDepth( unsigned int bitDepthIn );                        ///< Access function for #bitDepth property - refer to #bitDepth property for details
    unsigned int getBitDepth();                                         ///< Access function for #bitDepth property - refer to #bitDepth property for details

    void setFormatOption( imageDataFormats::formatOptions formatOption );                 ///< Access function for #formatOption property - refer to #formatOption property for details
    imageDataFormats::formatOptions getFormatOption();                                    ///< Access function for #formatOption property - refer to #formatOption property for details

    // Size options
    /// \enum resizeOptions
    /// Image resize options
    enum resizeOptions { RESIZE_OPTION_ZOOM,    ///< Zoom to selected percentage
                         RESIZE_OPTION_FIT      ///< Zoom to fit the current window size
                       };
    void setResizeOption( resizeOptions resizeOptionIn );               ///< Access function for #resizeOption property - refer to #resizeOption property for details
    resizeOptions getResizeOption();                                    ///< Access function for #resizeOption property - refer to #resizeOption property for details

    // Zoom
    void setZoom( int zoomIn );                                         ///< Access function for #zoom property - refer to #zoom property for details
    int getZoom();                                                      ///< Access function for #zoom property - refer to #zoom property for details


    // Stretch
    void setXStretch( double XStretchIn );                              ///< Access function for #XStretch property - refer to #XStretch property for details
    double getXStretch();                                               ///< Access function for #XStretch property - refer to #XStretch property for details
    void setYStretch( double YStretchIn );                              ///< Access function for #YStretch property - refer to #YStretch property for details
    double getYStretch();                                               ///< Access function for #YStretch property - refer to #YStretch property for details

    void setRotation( imageProperties::rotationOptions rotationIn );    ///< Access function for #rotation property - refer to #rotation property for details
    imageProperties::rotationOptions getRotation();                     ///< Access function for #rotation property - refer to #rotation property for details

    void setHorizontalFlip( bool flipHozIn );                           ///< Access function for #horizontalFlip property - refer to #horizontalFlip property for details
    bool getHorizontalFlip();                                           ///< Access function for #horizontalFlip property - refer to #horizontalFlip property for details

    void setVerticalFlip( bool flipVertIn );                            ///< Access function for #verticalFlip property - refer to #verticalFlip property for details
    bool getVerticalFlip();                                             ///< Access function for #verticalFlip property - refer to #verticalFlip property for details

    void setInitialHozScrollPos( int initialHosScrollPosIn );           ///< Access function for #initialHosScrollPos property - refer to #initialHosScrollPos property for details
    int getInitialHozScrollPos();                                       ///< Access function for #initialHosScrollPos property - refer to #initialHosScrollPos property for details

    void setInitialVertScrollPos( int initialVertScrollPosIn );         ///< Access function for #initialVertScrollPos property - refer to #initialVertScrollPos property for details
    int getInitialVertScrollPos();                                      ///< Access function for #initialVertScrollPos property - refer to #initialVertScrollPos property for details

    void setDisplayButtonBar( bool displayButtonBarIn );                ///< Access function for #displayButtonBar property - refer to #displayButtonBar property for details
    bool getDisplayButtonBar();                                         ///< Access function for #displayButtonBar property - refer to #displayButtonBar property for details

    void setShowTime(bool pValue);                                      ///< Access function for #showTime property - refer to #showTime property for details
    bool getShowTime();                                                 ///< Access function for #showTime property - refer to #showTime property for details

    void setUseFalseColour(bool pValue);                                ///< Access function for #useFalseColour property - refer to #useFalseColour property for details
    bool getUseFalseColour();                                           ///< Access function for #useFalseColour property - refer to #useFalseColour property for details

    void setVertSlice1MarkupColor(QColor pValue);                        ///< Access function for #vertSliceColor property - refer to #vertSliceColor property for details
    QColor getVertSlice1MarkupColor();                                   ///< Access function for #vertSliceColor property - refer to #vertSliceColor property for details

    void setVertSlice2MarkupColor(QColor pValue);                        ///< Access function for #vertSlice2Color property - refer to #vertSlice2Color property for details
    QColor getVertSlice2MarkupColor();                                   ///< Access function for #vertSlice2Color property - refer to #vertSlice2Color property for details

    void setVertSlice3MarkupColor(QColor pValue);                        ///< Access function for #vertSlice3Color property - refer to #vertSlice3Color property for details
    QColor getVertSlice3MarkupColor();                                   ///< Access function for #vertSlice3Color property - refer to #vertSlice3Color property for details

    void setVertSlice4MarkupColor(QColor pValue);                        ///< Access function for #vertSlice4Color property - refer to #vertSlice4Color property for details
    QColor getVertSlice4MarkupColor();                                   ///< Access function for #vertSlice4Color property - refer to #vertSlice4Color property for details

    void setVertSlice5MarkupColor(QColor pValue);                        ///< Access function for #vertSlice5Color property - refer to #vertSlice5Color property for details
    QColor getVertSlice5MarkupColor();                                   ///< Access function for #vertSlice5Color property - refer to #vertSlice5Color property for details

    void setHozSlice1MarkupColor(QColor pValue);                         ///< Access function for #hozSliceColor property - refer to #hozSliceColor property for details
    QColor getHozSlice1MarkupColor();                                    ///< Access function for #hozSliceColor property - refer to #hozSliceColor property for details

    void setHozSlice2MarkupColor(QColor pValue);                         ///< Access function for #hozSlice2Color property - refer to #hozSlice2Color property for details
    QColor getHozSlice2MarkupColor();                                    ///< Access function for #hozSlice2Color property - refer to #hozSlice2Color property for details

    void setHozSlice3MarkupColor(QColor pValue);                         ///< Access function for #hozSlice3Color property - refer to #hozSlice3Color property for details
    QColor getHozSlice3MarkupColor();                                    ///< Access function for #hozSlice3Color property - refer to #hozSlice3Color property for details

    void setHozSlice4MarkupColor(QColor pValue);                         ///< Access function for #hozSlice4Color property - refer to #hozSlice4Color property for details
    QColor getHozSlice4MarkupColor();                                    ///< Access function for #hozSlice4Color property - refer to #hozSlice4Color property for details

    void setHozSlice5MarkupColor(QColor pValue);                         ///< Access function for #hozSlice5Color property - refer to #hozSlice5Color property for details
    QColor getHozSlice5MarkupColor();                                    ///< Access function for #hozSlice5Color property - refer to #hozSlice5Color property for details

    void setProfileMarkupColor(QColor pValue);                          ///< Access function for #profileColor property - refer to #profileColor property for details
    QColor getProfileMarkupColor();                                     ///< Access function for #profileColor property - refer to #profileColor property for details

    void setAreaMarkupColor(QColor pValue);                             ///< Access function for #areaColor property - refer to #areaColor property for details
    QColor getAreaMarkupColor();                                        ///< Access function for #areaColor property - refer to #areaColor property for details

    void setTargetMarkupColor(QColor pValue);                           ///< Access function for #targetColor property - refer to #targetColor property for details
    QColor getTargetMarkupColor();                                      ///< Access function for #targetColor property - refer to #targetColor property for details

    void setBeamMarkupColor(QColor pValue);                             ///< Access function for #beamColor property - refer to #beamColor property for details
    QColor getBeamMarkupColor();                                        ///< Access function for #beamColor property - refer to #beamColor property for details

    void setTimeMarkupColor(QColor pValue);                             ///< Access function for #timeColor property - refer to #timeColor property for details
    QColor getTimeMarkupColor();                                        ///< Access function for #timeColor property - refer to #timeColor property for details

    void setEllipseMarkupColor(QColor markupColor );                    ///< Access function for ellipseColor property - refer to #ellipseColor property for details
    QColor getEllipseMarkupColor();                                     ///< Access function for #ellipseColor property - refer to #ellipseColor property for details

    void setDisplayCursorPixelInfo( bool displayCursorPixelInfo );      ///< Access function for #displayCursorPixelInfo property - refer to #displayCursorPixelInfo property for details
    bool getDisplayCursorPixelInfo();                                   ///< Access function for #displayCursorPixelInfo property - refer to #displayCursorPixelInfo property for details

    void setContrastReversal( bool contrastReversalIn );                ///< Access function for #contrastReversal property - refer to #contrastReversal property for details
    bool getContrastReversal();                                         ///< Access function for #contrastReversal property - refer to #contrastReversal property for details

    void setLog( bool log );                                            ///< Access function for #logBrightness property - refer to #logBrightness property for details
    bool getLog();                                                      ///< Access function for #logBrightness property - refer to #logBrightness property for details

    void setEnableVertSlice1Selection( bool enableVSliceSelection );     ///< Access function for #enableVertSlice1Selection property - refer to #enableVertSlice1Selection property for details
    bool getEnableVertSlice1Selection();                                 ///< Access function for #enableVertSlice1Selection property - refer to #enableVertSlice1Selection property for details

    void setEnableVertSlice2Selection( bool enableVSliceSelection );     ///< Access function for #enableVertSlice2Selection property - refer to #enableVertSlice2Selection property for details
    bool getEnableVertSlice2Selection();                                 ///< Access function for #enableVertSlice2Selection property - refer to #enableVertSlice2Selection property for details

    void setEnableVertSlice3Selection( bool enableVSliceSelection );     ///< Access function for #enableVertSlice3Selection property - refer to #enableVertSlice3Selection property for details
    bool getEnableVertSlice3Selection();                                 ///< Access function for #enableVertSlice3Selection property - refer to #enableVertSlice3Selection property for details

    void setEnableVertSlice4Selection( bool enableVSliceSelection );     ///< Access function for #enableVertSlice4Selection property - refer to #enableVertSlice4Selection property for details
    bool getEnableVertSlice4Selection();                                 ///< Access function for #enableVertSlice4Selection property - refer to #enableVertSlice4Selection property for details

    void setEnableVertSlice5Selection( bool enableVSliceSelection );     ///< Access function for #enableVertSlice5Selection property - refer to #enableVertSlice5Selection property for details
    bool getEnableVertSlice5Selection();                                 ///< Access function for #enableVertSlice5Selection property - refer to #enableVertSlice5Selection property for details

    void setEnableHozSlice1Selection( bool enableHSliceSelection );      ///< Access function for #enableHozSlice1Selection property - refer to #enableHozSlice1Selection property for details
    bool getEnableHozSlice1Selection();                                  ///< Access function for #enableHozSlice1Selection property - refer to #enableHozSlice1Selection property for details

    void setEnableHozSlice2Selection( bool enableHSliceSelection );      ///< Access function for #enableHozSlice2Selection property - refer to #enableHozSlice2Selection property for details
    bool getEnableHozSlice2Selection();                                  ///< Access function for #enableHozSlice2Selection property - refer to #enableHozSlice2Selection property for details

    void setEnableHozSlice3Selection( bool enableHSliceSelection );      ///< Access function for #enableHozSlice3Selection property - refer to #enableHozSlice3Selection property for details
    bool getEnableHozSlice3Selection();                                  ///< Access function for #enableHozSlice3Selection property - refer to #enableHozSlice3Selection property for details

    void setEnableHozSlice4Selection( bool enableHSliceSelection );      ///< Access function for #enableHozSlice4Selection property - refer to #enableHozSlice4Selection property for details
    bool getEnableHozSlice4Selection();                                  ///< Access function for #enableHozSlice4Selection property - refer to #enableHozSlice4Selection property for details

    void setEnableHozSlice5Selection( bool enableHSliceSelection );      ///< Access function for #enableHozSlice5Selection property - refer to #enableHozSlice5Selection property for details
    bool getEnableHozSlice5Selection();                                  ///< Access function for #enableHozSlice5Selection property - refer to #enableHozSlice5Selection property for details

    void setEnableArea1Selection( bool enableAreaSelectionIn );          ///< Access function for #enableArea1Selection property - refer to #enableArea1Selection property for details
    bool getEnableArea1Selection();                                      ///< Access function for #enableArea1Selection property - refer to #enableArea1Selection property for details

    void setEnableArea2Selection( bool enableAreaSelectionIn );          ///< Access function for #enableArea2Selection property - refer to #enableArea2Selection property for details
    bool getEnableArea2Selection();                                      ///< Access function for #enableArea2Selection property - refer to #enableArea2Selection property for details

    void setEnableArea3Selection( bool enableAreaSelectionIn );          ///< Access function for #enableArea3Selection property - refer to #enableArea3Selection property for details
    bool getEnableArea3Selection();                                      ///< Access function for #enableArea3Selection property - refer to #enableArea3Selection property for details

    void setEnableArea4Selection( bool enableAreaSelectionIn );          ///< Access function for #enableArea4Selection property - refer to #enableArea4Selection property for details
    bool getEnableArea4Selection();                                      ///< Access function for #enableArea4Selection property - refer to #enableArea4Selection property for details

    void setEnableProfileSelection( bool enableProfileSelectionIn );    ///< Access function for #enableProfileSelection property - refer to #enableProfileSelection property for details
    bool getEnableProfileSelection();                                   ///< Access function for #enableProfileSelection property - refer to #enableProfileSelection property for details

    void setEnableTargetSelection( bool enableTargetSelectionIn );      ///< Access function for #enableTargetSelection property - refer to #enableTargetSelection property for details
    bool getEnableTargetSelection();                                    ///< Access function for #enableTargetSelection property - refer to #enableTargetSelection property for details

    void setEnableBeamSelection( bool enableBeamSelectionIn );          ///< Access function for #enableBeamSelection property - refer to #enableBeamSelection property for details
    bool getEnableBeamSelection();                                      ///< Access function for #enableBeamSelection property - refer to #enableBeamSelection property for details

    void setEnableImageDisplayProperties( bool enableImageDisplayPropertiesIn );///< Access function for #enableImageDisplayProperties property - refer to #enableImageDisplayProperties property for details
    bool getEnableImageDisplayProperties();                                     ///< Access function for #enableImageDisplayProperties property - refer to #enableImageDisplayProperties property for details

    void setEnableRecording( bool enableRecordingIn );                  ///< Access function for #enableRecording property - refer to #enableRecording property for details
    bool getEnableRecording();                                          ///< Access function for #enableRecording property - refer to #enableRecording property for details

    void setAutoBrightnessContrast( bool autoBrightnessContrastIn );    ///< Access function for #autoBrightnessContrast property - refer to #autoBrightnessContrast property for details
    bool getAutoBrightnessContrast();                                   ///< Access function for #autoBrightnessContrast property - refer to #autoBrightnessContrast property for details

    void setExternalControls( bool externalControlsIn );                ///< Access function for #externalControls property - refer to #externalControls property for details
    bool getExternalControls();                                         ///< Access function for #externalControls property - refer to #externalControls property for details

    void setFullContextMenu( bool fullContextMenuIn );                  ///< Access function for #fullContextMenu property - refer to #fullContextMenu property for details
    bool getFullContextMenu();                                          ///< Access function for #fullContextMenu property - refer to #fullContextMenu property for details

    void setEnableProfilePresentation( bool enableProfilePresentationIn );     ///< Access function for #enableProfilePresentation property - refer to #enableProfilePresentation property for details
    bool getEnableProfilePresentation();                                       ///< Access function for #enableProfilePresentation property - refer to #enableProfilePresentation property for details

    void setEnableHozSlicePresentation( bool enableHozSlicePresentationIn );   ///< Access function for #enableHozSlicePresentation property - refer to #enableHozSlicePresentation property for details
    bool getEnableHozSlicePresentation();                                      ///< Access function for #enableHozSlicePresentation property - refer to #enableHozSlicePresentation property for details

    void setEnableVertSlicePresentation( bool enableVertSlicePresentationIn ); ///< Access function for #enableVertSlicePresentation property - refer to #enableVertSlicePresentation property for details
    bool getEnableVertSlicePresentation();                                     ///< Access function for #enableVertSlicePresentation property - refer to #enableVertSlicePresentation property for details

    void setDisplayVertSlice1Selection( bool displayVSliceSelection );         ///< Access function for #displayVertSlice1Selection property - refer to #displayVertSlice1Selection property for details
    bool getDisplayVertSlice1Selection();                                      ///< Access function for #displayVertSlice1Selection property - refer to #displayVertSlice1Selection property for details

    void setDisplayVertSlice2Selection( bool displayVSliceSelection );         ///< Access function for #displayVertSlice2Selection property - refer to #displayVertSlice2Selection property for details
    bool getDisplayVertSlice2Selection();                                      ///< Access function for #displayVertSlice2Selection property - refer to #displayVertSlice2Selection property for details

    void setDisplayVertSlice3Selection( bool displayVSliceSelection );         ///< Access function for #displayVertSlice3Selection property - refer to #displayVertSlice3Selection property for details
    bool getDisplayVertSlice3Selection();                                      ///< Access function for #displayVertSlice3Selection property - refer to #displayVertSlice3Selection property for details

    void setDisplayVertSlice4Selection( bool displayVSliceSelection );         ///< Access function for #displayVertSlice4Selection property - refer to #displayVertSlice4Selection property for details
    bool getDisplayVertSlice4Selection();                                      ///< Access function for #displayVertSlice4Selection property - refer to #displayVertSlice4Selection property for details

    void setDisplayVertSlice5Selection( bool displayVSliceSelection );         ///< Access function for #displayVertSlice5Selection property - refer to #displayVertSlice5Selection property for details
    bool getDisplayVertSlice5Selection();                                      ///< Access function for #displayVertSlice5Selection property - refer to #displayVertSlice5Selection property for details


    void setDisplayHozSlice1Selection( bool displayHSliceSelection );            ///< Access function for #displayHozSlice1Selection property - refer to #displayHozSlice1Selection property for details
    bool getDisplayHozSlice1Selection();                                         ///< Access function for #displayHozSlice1Selection property - refer to #displayHozSlice1Selection property for details

    void setDisplayHozSlice2Selection( bool displayHSliceSelection );            ///< Access function for #displayHozSlice2Selection property - refer to #displayHozSlice2Selection property for details
    bool getDisplayHozSlice2Selection();                                         ///< Access function for #displayHozSlice2Selection property - refer to #displayHozSlice2Selection property for details

    void setDisplayHozSlice3Selection( bool displayHSliceSelection );            ///< Access function for #displayHozSlice3Selection property - refer to #displayHozSlice3Selection property for details
    bool getDisplayHozSlice3Selection();                                         ///< Access function for #displayHozSlice3Selection property - refer to #displayHozSlice3Selection property for details

    void setDisplayHozSlice4Selection( bool displayHSliceSelection );            ///< Access function for #displayHozSlice4Selection property - refer to #displayHozSlice4Selection property for details
    bool getDisplayHozSlice4Selection();                                         ///< Access function for #displayHozSlice4Selection property - refer to #displayHozSlice4Selection property for details

    void setDisplayHozSlice5Selection( bool displayHSliceSelection );            ///< Access function for #displayHozSlice5Selection property - refer to #displayHozSlice5Selection property for details
    bool getDisplayHozSlice5Selection();                                         ///< Access function for #displayHozSlice5Selection property - refer to #displayHozSlice5Selection property for details


    void setDisplayArea1Selection( bool displayAreaSelection );                 ///< Access function for #displayArea1Selection property - refer to #displayArea1Selection property for details
    bool getDisplayArea1Selection();                                            ///< Access function for #displayArea1Selection property - refer to #displayArea1Selection property for details

    void setDisplayArea2Selection( bool displayAreaSelection );                 ///< Access function for #displayArea2Selection property - refer to #displayArea2Selection property for details
    bool getDisplayArea2Selection();                                            ///< Access function for #displayArea2Selection property - refer to #displayArea2Selection property for details

    void setDisplayArea3Selection( bool displayAreaSelection );                 ///< Access function for #displayArea3Selection property - refer to #displayArea3Selection property for details
    bool getDisplayArea3Selection();                                            ///< Access function for #displayArea3Selection property - refer to #displayArea3Selection property for details

    void setDisplayArea4Selection( bool displayAreaSelection );                 ///< Access function for #displayArea4Selection property - refer to #displayArea4Selection property for details
    bool getDisplayArea4Selection();                                            ///< Access function for #displayArea4Selection property - refer to #displayArea4Selection property for details

    void setDisplayProfileSelection( bool displayProfileSelection );            ///< Access function for #displayProfileSelection property - refer to #displayProfileSelection property for details
    bool getDisplayProfileSelection();                                          ///< Access function for #displayProfileSelection property - refer to #displayProfileSelection property for details

    void setDisplayTargetSelection( bool displayTargetSelection );              ///< Access function for #displayTargetSelection property - refer to #displayTargetSelection property for details
    bool getDisplayTargetSelection();                                           ///< Access function for #displayTargetSelection property - refer to #displayTargetSelection property for details

    void setDisplayBeamSelection( bool displayBeamSelection );                  ///< Access function for #displayBeamSelection property - refer to #displayBeamSelection property for details
    bool getDisplayBeamSelection();                                             ///< Access function for #displayBeamSelection property - refer to #displayBeamSelection property for details

    void setDisplayEllipse( bool displayEllipse );                              ///< Access function for #displayEllipse property - refer to #displayEllipse property for details
    bool getDisplayEllipse();                                                   ///< Access function for #displayEllipse property - refer to #displayEllipse property for details

    ///  \enum ellipseVariableDefinitions.
    /// Options for the use of ellipse markup variables.
    enum ellipseVariableDefinitions { BOUNDING_RECTANGLE,                       ///< Variables define bounding rectagle of ellipse
                                      CENTRE_AND_SIZE };                        ///< Variables define centre and size of ellipse
    ellipseVariableDefinitions getEllipseVariableDefinition();                  ///< Access function for #ellipseVariableDefinition property - refer to #ellipseVariableDefinition property for details
    void setEllipseVariableDefinition( ellipseVariableDefinitions def );        ///< Access function for #ellipseVariableDefinition property - refer to #ellipseVariableDefinition property for details

    void setDisplayMarkups( bool displayMarkupsIn );                    ///< Access function for #displayMarkups property - refer to #displayMarkups property for details
    bool getDisplayMarkups();                                           ///< Access function for #displayMarkups property - refer to #displayMarkups property for details

    void setName( QString nameIn );                            ///< Access function for name property - refer to #name property for details
    QString getName();                                         ///< Access function for name property - refer to #name property for details

    void setProgram1( QString program );                       ///< Access function for #program1 property - refer to #program1 property for details
    QString getProgram1();                                     ///< Access function for #program1 property - refer to #program1 property for details
    void setProgram2( QString program );                       ///< Access function for #program2 property - refer to #program2 property for details
    QString getProgram2();                                     ///< Access function for #program2 property - refer to #program2 property for details

    // Arguments String
    void setArguments1( QStringList arguments );                       ///< Access function for #arguments1 property - refer to #arguments1 property for details
    QStringList getArguments1();                                       ///< Access function for #arguments1 property - refer to #arguments1 property for details
    void setArguments2( QStringList arguments );                       ///< Access function for #arguments2 property - refer to #arguments2 property for details
    QStringList getArguments2();                                       ///< Access function for #arguments2 property - refer to #arguments2 property for details

    // Startup option
    void setProgramStartupOption1( applicationLauncher::programStartupOptions programStartupOption ); ///< Access function for #programStartupOption1 property - refer to #programStartupOption1 property for details
    applicationLauncher::programStartupOptions getProgramStartupOption1();                            ///< Access function for #programStartupOption1 property - refer to #programStartupOption1 property for details
    void setProgramStartupOption2( applicationLauncher::programStartupOptions programStartupOption ); ///< Access function for #programStartupOption2 property - refer to #programStartupOption2 property for details
    applicationLauncher::programStartupOptions getProgramStartupOption2();                            ///< Access function for #programStartupOption2 property - refer to #programStartupOption2 property for details


    QString getHozSlice1Legend();                       ///< Access function for #hozSlice1Legend property - refer to #hozSlice1Legend property for details
    void setHozSlice1Legend( QString legend );          ///< Access function for #hozSlice1Legend property - refer to #hozSlice1Legend property for details
    QString getHozSlice2Legend();                       ///< Access function for #hozSlice2Legend property - refer to #hozSlice2Legend property for details
    void setHozSlice2Legend( QString legend );          ///< Access function for #hozSlice2Legend property - refer to #hozSlice2Legend property for details
    QString getHozSlice3Legend();                       ///< Access function for #hozSlice3Legend property - refer to #hozSlice3Legend property for details
    void setHozSlice3Legend( QString legend );          ///< Access function for #hozSlice3Legend property - refer to #hozSlice3Legend property for details
    QString getHozSlice4Legend();                       ///< Access function for #hozSlice4Legend property - refer to #hozSlice4Legend property for details
    void setHozSlice4Legend( QString legend );          ///< Access function for #hozSlice4Legend property - refer to #hozSlice4Legend property for details
    QString getHozSlice5Legend();                       ///< Access function for #hozSlice5Legend property - refer to #hozSlice5Legend property for details
    void setHozSlice5Legend( QString legend );          ///< Access function for #hozSlice5Legend property - refer to #hozSlice5Legend property for details
    QString getVertSlice1Legend();                      ///< Access function for #vertSlice1Legend property - refer to #vertSlice1Legend property for details
    void setVertSlice1Legend( QString legend );         ///< Access function for #vertSlice1Legend property - refer to #vertSlice1Legend property for details
    QString getVertSlice2Legend();                      ///< Access function for #vertSlice2Legend property - refer to #vertSlice2Legend property for details
    void setVertSlice2Legend( QString legend );         ///< Access function for #vertSlice2Legend property - refer to #vertSlice2Legend property for details
    QString getVertSlice3Legend();                      ///< Access function for #vertSlice3Legend property - refer to #vertSlice3Legend property for details
    void setVertSlice3Legend( QString legend );         ///< Access function for #vertSlice3Legend property - refer to #vertSlice3Legend property for details
    QString getVertSlice4Legend();                      ///< Access function for #vertSlice4Legend property - refer to #vertSlice4Legend property for details
    void setVertSlice4Legend( QString legend );         ///< Access function for #vertSlice4Legend property - refer to #vertSlice4Legend property for details
    QString getVertSlice5Legend();                      ///< Access function for #vertSlice5Legend property - refer to #vertSlice5Legend property for details
    void setVertSlice5Legend( QString legend );         ///< Access function for #vertSlice5Legend property - refer to #vertSlice5Legend property for details
    QString getprofileLegend();                        ///< Access function for #profileLegend property - refer to #profileLegend property for details
    void setProfileLegend( QString legend );            ///< Access function for #profileLegend property - refer to #profileLegend property for details
    QString getAreaSelection1Legend();                 ///< Access function for #areaSelection1Legend property - refer to #areaSelection1Legend property for details
    void setAreaSelection1Legend( QString legend );     ///< Access function for #areaSelection1Legend property - refer to #areaSelection1Legend property for details
    QString getAreaSelection2Legend();                 ///< Access function for #areaSelection2Legend property - refer to #areaSelection2Legend property for details
    void setAreaSelection2Legend( QString legend );     ///< Access function for #areaSelection2Legend property - refer to #areaSelection2Legend property for details
    QString getAreaSelection3Legend();                 ///< Access function for #areaSelection3Legend property - refer to #areaSelection3Legend property for details
    void setAreaSelection3Legend( QString legend );     ///< Access function for #areaSelection3Legend property - refer to #areaSelection3Legend property for details
    QString getAreaSelection4Legend();                 ///< Access function for #areaSelection4Legend property - refer to #areaSelection4Legend property for details
    void setAreaSelection4Legend( QString legend );     ///< Access function for #areaSelection4Legend property - refer to #areaSelection4Legend property for details
    QString getTargetLegend();                         ///< Access function for #targetLegend property - refer to #targetLegend property for details
    void setTargetLegend( QString legend );             ///< Access function for #targetLegend property - refer to #targetLegend property for details
    QString getBeamLegend();                           ///< Access function for #beamLegend property - refer to #beamLegend property for details
    void setBeamLegend( QString legend );               ///< Access function for #beamLegend property - refer to #beamLegend property for details
    QString getEllipseLegend();                        ///< Access function for #ellipseLegend property - refer to #ellipseLegend property for details
    void setEllipseLegend( QString legend );            ///< Access function for #ellipseLegend property - refer to #ellipseLegend property for details

    bool getFullScreen();                               ///< Access function for #fullScreen property - refer to #fullScreen property for details
    void setFullScreen( bool fullScreenIn );            ///< Access function for #fullScreen property - refer to #fullScreen property for details

    void setSubstitutedUrl( QString urlIn );            ///< Access function for #URL property - refer to #URL property for deta
    QString getSubstitutedUrl();                        ///< Access function for #URL property - refer to #URL property for deta

  protected:
    QEStringFormatting stringFormatting;     // String formatting options.
    QEIntegerFormatting integerFormatting;   // Integer formatting options.
    QEFloatingFormatting floatingFormatting; // Floating formatting options.

    void establishConnection( unsigned int variableIndex );

    // Index for access to registered variables
    enum variableIndexes{ IMAGE_VARIABLE,
                          FORMAT_VARIABLE, BIT_DEPTH_VARIABLE, DATA_TYPE_VARIABLE,
                          WIDTH_VARIABLE, HEIGHT_VARIABLE,
                          NUM_DIMENSIONS_VARIABLE, DIMENSION_0_VARIABLE, DIMENSION_1_VARIABLE, DIMENSION_2_VARIABLE,
                          ROI1_X_VARIABLE, ROI1_Y_VARIABLE, ROI1_W_VARIABLE, ROI1_H_VARIABLE,
                          ROI2_X_VARIABLE, ROI2_Y_VARIABLE, ROI2_W_VARIABLE, ROI2_H_VARIABLE,
                          ROI3_X_VARIABLE, ROI3_Y_VARIABLE, ROI3_W_VARIABLE, ROI3_H_VARIABLE,
                          ROI4_X_VARIABLE, ROI4_Y_VARIABLE, ROI4_W_VARIABLE, ROI4_H_VARIABLE,
                          TARGET_X_VARIABLE, TARGET_Y_VARIABLE,
                          BEAM_X_VARIABLE, BEAM_Y_VARIABLE,
                          TARGET_TRIGGER_VARIABLE,
                          CLIPPING_ONOFF_VARIABLE, CLIPPING_LOW_VARIABLE, CLIPPING_HIGH_VARIABLE,
                          PROFILE_H1_VARIABLE, PROFILE_H1_THICKNESS_VARIABLE,
                          PROFILE_H2_VARIABLE, PROFILE_H2_THICKNESS_VARIABLE,
                          PROFILE_H3_VARIABLE, PROFILE_H3_THICKNESS_VARIABLE,
                          PROFILE_H4_VARIABLE, PROFILE_H4_THICKNESS_VARIABLE,
                          PROFILE_H5_VARIABLE, PROFILE_H5_THICKNESS_VARIABLE,
                          PROFILE_V1_VARIABLE, PROFILE_V1_THICKNESS_VARIABLE,
                          PROFILE_V2_VARIABLE, PROFILE_V2_THICKNESS_VARIABLE,
                          PROFILE_V3_VARIABLE, PROFILE_V3_THICKNESS_VARIABLE,
                          PROFILE_V4_VARIABLE, PROFILE_V4_THICKNESS_VARIABLE,
                          PROFILE_V5_VARIABLE, PROFILE_V5_THICKNESS_VARIABLE,
                          LINE_PROFILE_X1_VARIABLE, LINE_PROFILE_Y1_VARIABLE, LINE_PROFILE_X2_VARIABLE, LINE_PROFILE_Y2_VARIABLE, LINE_PROFILE_THICKNESS_VARIABLE,
                          PROFILE_H_ARRAY, PROFILE_V_ARRAY, PROFILE_LINE_ARRAY,
                          ELLIPSE_X_VARIABLE, ELLIPSE_Y_VARIABLE, ELLIPSE_W_VARIABLE, ELLIPSE_H_VARIABLE,

                          QEIMAGE_NUM_VARIABLES /*Must be last*/ };

    resizeOptions resizeOption; // Resize option. (zoom or fit)
    int zoom;                   // Zoom percentage
    double XStretch;            // X stretch factor. Used when determining canvas size of fully processed image (zoomed, flipped, etc)
    double YStretch;            // Y stretch factor. Used when determining canvas size of fully processed image (zoomed, flipped, etc)
    int initialHozScrollPos;    // Initial horizontal scroll bar position (for when starting zoomed)
    int initialVertScrollPos;   // Initial vertical scroll bar position (for when starting zoomed)

    bool displayButtonBar;      // True if button bar should be displayed

    void redisplayAllMarkups();

private slots:
    // MPEG data update slots (and maybe other non CA sources)
    void setDataImage( const QByteArray& imageIn,
                       unsigned long dataSize, unsigned long elements,
                       unsigned long width, unsigned long height,
                       imageDataFormats::formatOptions format, unsigned int depth );

    // QCa data update slots
    void connectionChanged( QCaConnectionInfo& connectionInfo );
    void setImage( const QByteArray& image, unsigned long dataSize, QCaAlarmInfo&, QCaDateTime&, const unsigned int& );
    void setFormat( const QString& text, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex);
    void setBitDepth( const long& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex);
    void setDataType( const QString& text, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex );
    void setDimension( const long& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex);
    void setClipping( const long& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex);
    void setROI( const long& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex);
    void setProfile( const long& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex);
    void setTargeting( const long& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex);
    void setEllipse( const long& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex);

    // Menu choice slots
    void vSlice1SelectModeClicked();
    void vSlice2SelectModeClicked();
    void vSlice3SelectModeClicked();
    void vSlice4SelectModeClicked();
    void vSlice5SelectModeClicked();
    void hSlice1SelectModeClicked();
    void hSlice2SelectModeClicked();
    void hSlice3SelectModeClicked();
    void hSlice4SelectModeClicked();
    void hSlice5SelectModeClicked();
    void area1SelectModeClicked();
    void area2SelectModeClicked();
    void area3SelectModeClicked();
    void area4SelectModeClicked();
    void profileSelectModeClicked();
    void targetSelectModeClicked();
    void beamSelectModeClicked();

    void panModeClicked();

    void imageDisplayPropertiesChanged();
    void brightnessContrastAutoImageRequest();

    // !! move this functionality into QEWidget???
    // !! needs one for single variables and one for multiple variables, or just the multiple variable one for all
    void useNewVariableNameProperty( QString variableNameIn, QString variableNameSubstitutionsIn, unsigned int variableIndex )// !! move into Standard Properties section??
    {
        setVariableNameAndSubstitutions(variableNameIn, variableNameSubstitutionsIn, variableIndex);
    }

    void userSelection( imageMarkup::markupIds mode, bool complete, bool clearing, QPoint point1, QPoint point2, unsigned int thickness );
    void zoomInOut( int zoomAmount );
    void currentPixelInfo( QPoint pos );
    void pan( QPoint pos );
    void redraw();
    void showImageContextMenuFullScreen( const QPoint& pos );
    void showImageContextMenu( const QPoint& );
    void selectMenuTriggered( QAction* selectedItem );
    void markupDisplayMenuTriggered( QAction* selectedItem );
    void zoomMenuTriggered( QAction* selectedItem );
    void flipRotateMenuTriggered( QAction* selectedItem );
    void showImageAboutDialog();
    void optionAction( imageContextMenu::imageContextMenuOptions option, bool checked );

    // Slots to make profile plots appear or disappear
    // They are used as timer events to ensure resize events (that happen as the controls are inserted or deleted)
    // don't cause a redraw of markups while handling a markup draw event
    void setVSliceControlsVisible();
    void setVSliceControlsNotVisible();
    void setHSliceControlsVisible();
    void setHSliceControlsNotVisible();
    void setLineProfileControlsVisible();
    void setLineProfileControlsNotVisible();

    void useAllMarkupData();
    void raiseFullScreen();         // Ensure the full screen main window is in front of the application.
    void resizeFullScreen();        // Resize full screen once it has been managed

    void playingBack( bool playing );

    void displayBuiltImage( QImage image, QString error );

public slots:
    void setImageFile( QString name );
    // Slots to allow external setting of selection menu options
    void setSelectPanMode()     { sMenu->setChecked(  QEImage::SO_PANNING ); panModeClicked(); }            ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectVSliceMode()  { sMenu->setChecked(  QEImage::SO_VSLICE1 );  vSlice1SelectModeClicked(); }   ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectHSliceMode()  { sMenu->setChecked(  QEImage::SO_HSLICE1 );  hSlice1SelectModeClicked(); }   ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectArea1Mode()   { sMenu->setChecked(  QEImage::SO_AREA1 );   area1SelectModeClicked(); }    ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectArea2Mode()   { sMenu->setChecked(  QEImage::SO_AREA2 );   area2SelectModeClicked(); }    ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectArea3Mode()   { sMenu->setChecked(  QEImage::SO_AREA3 );   area3SelectModeClicked(); }    ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectArea4Mode()   { sMenu->setChecked(  QEImage::SO_AREA4 );   area4SelectModeClicked(); }    ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectProfileMode() { sMenu->setChecked(  QEImage::SO_PROFILE ); profileSelectModeClicked(); }  ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectTargetMode()  { sMenu->setChecked(  QEImage::SO_TARGET );  targetSelectModeClicked(); }   ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectBeamMode()    { sMenu->setChecked(  QEImage::SO_BEAM );    beamSelectModeClicked(); }     ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectVSlice1Mode()  { sMenu->setChecked(  QEImage::SO_VSLICE1 );  vSlice1SelectModeClicked(); }   ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectVSlice2Mode()  { sMenu->setChecked(  QEImage::SO_VSLICE2 );  vSlice2SelectModeClicked(); }   ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectVSlice3Mode()  { sMenu->setChecked(  QEImage::SO_VSLICE3 );  vSlice3SelectModeClicked(); }   ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectVSlice4Mode()  { sMenu->setChecked(  QEImage::SO_VSLICE4 );  vSlice4SelectModeClicked(); }   ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectVSlice5Mode()  { sMenu->setChecked(  QEImage::SO_VSLICE5 );  vSlice5SelectModeClicked(); }   ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectHSlice1Mode()  { sMenu->setChecked(  QEImage::SO_HSLICE1 );  hSlice1SelectModeClicked(); }   ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectHSlice2Mode()  { sMenu->setChecked(  QEImage::SO_HSLICE2 );  hSlice2SelectModeClicked(); }   ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectHSlice3Mode()  { sMenu->setChecked(  QEImage::SO_HSLICE3 );  hSlice3SelectModeClicked(); }   ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectHSlice4Mode()  { sMenu->setChecked(  QEImage::SO_HSLICE4 );  hSlice4SelectModeClicked(); }   ///< Framework use only. Slot to allow external setting of selection menu options
    void setSelectHSlice5Mode()  { sMenu->setChecked(  QEImage::SO_HSLICE5 );  hSlice5SelectModeClicked(); }   ///< Framework use only. Slot to allow external setting of selection menu options

    // Slots to allow external operation of control buttons
    void pauseClicked();           ///< Framework use only. Slot to allow external setting of selection menu options

    void saveClicked();            ///< Framework use only. Slot to allow external setting of selection menu options

    void targetClicked();      ///< Framework use only. Slot to allow external setting of selection menu options

    void imageDisplayPropsDestroyed( QObject* ); ///< Framework use only. Slot to catch deletion of components (such as profile plots) that have been passed to the application for presentation
    void vSliceDisplayDestroyed( QObject* ); ///< Framework use only. Slot to catch deletion of components (such as profile plots) that have been passed to the application for presentation
    void hSliceDisplayDestroyed( QObject* ); ///< Framework use only. Slot to catch deletion of components (such as profile plots) that have been passed to the application for presentation
    void profileDisplayDestroyed( QObject* ); ///< Framework use only. Slot to catch deletion of components (such as profile plots) that have been passed to the application for presentation
    void recorderDestroyed( QObject* ); ///< Framework use only. Slot to catch deletion of components (such as profile plots) that have been passed to the application for presentation

    // Slots to enable external signals to show/hide markups.
    // Note that setDisplayMarkups(bool) complicates things. If it is called with 'true'
    // then markups that are PV controlled will still display (if the pv changes) even though the markup's setDisplay???Selection(false) has been called.
    // So if you want a PV controlled markup to really not show, you have to call setDisplayMarkups(false) and then setDisplay????Selection(false)
    // Note also that calling setDisplayMarkups(true) will show those previously hidden PV controlled markups but calling setDisplayMarkups(false)
    // won't hide them again. To do that you also have to call their setDisplay?????Selection(false) again.
    void showProfile()            { setDisplayProfileSelection( true ); }  ///< Show the arbitrary line (profile) markup - refer to #enableProfileSelection property and #displayMarkups property for details
    void showProfile( bool show ) { setDisplayProfileSelection( show ); }  ///< Show or hide the arbitrary line (profile) markup. Note that when hiding if its PV changes it will reshow unless DisplayMarkups has been set to off - refer to #enableProfileSelection property and #displayMarkups property for details
    void hideProfile()            { setDisplayProfileSelection( false ); } ///< Hide the arbitrary line (profile) markup but note that if its PV changes it will reshow unless DisplayMarkups has been set to off - refer to #enableProfileSelection property and #displayMarkups property for details

    void showArea1()           { setDisplayArea1Selection( true ); }       ///< Show the area1 markup - refer to #enableArea1Selection property and #displayMarkups property for details
    void showArea1( bool show) { setDisplayArea1Selection( show ); }       ///< show or hide the area1 markup. Note that when hiding if its PV changes it will reshow unless DisplayMarkups has been set to off - refer to #enableArea1Selection property and #displayMarkups property for details
    void hideArea1()           { setDisplayArea1Selection( false ); }      ///< Hide the area1 markup but note that if its PV changes it will reshow unless DisplayMarkups has been set to off - refer to #enableArea1Selection property and #displayMarkups property for details

    void setDisplayMarkupsOn()  { setDisplayMarkups( true ); }             ///< Set markup display to on to show all markups that change either due to user or PV activity, even if their setDisplay????Selection is off - refer to #displayMarkups property for details
    void setDisplayMarkupsOn( bool on )  { setDisplayMarkups( on ); }      ///< Set markup display to on to or off show all markups that change either due to user or PV activity, even if their setDisplay????Selection is off - refer to #displayMarkups property for details
    void setDisplayMarkupsOff() { setDisplayMarkups( false ); }            ///< Set markup display to off to stop PV controlled pvs from showing even if they change, unless their setDisplay????Selection is on - refer to #displayMarkups property for details

signals:
    // Note, the following signals are common to many QE widgets,
    // if changing the doxygen comments, ensure relevent changes are migrated to all instances
    /// Sent when the widget is updated following a data change
    /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
    /// For example a QList widget could log updates from this widget.
    void dbValueChanged( const QString& out );
    /// Internal use only. Used when changing a property value to force a re-display to reflect the new property value.
    void requestResend();

    void componentHostRequest( const QEActionRequests& request );

  private:
    imageUses imageUse;
    void useTargetingData();
//    void useAllMarkupData();
    void useROIData( const unsigned int& variableIndex );
    void useProfileData( const unsigned int& variableIndex );
    void useEllipseData();

    void roi1Changed();        ///< Framework use only. Slot to allow external setting of selection menu options
    void roi2Changed();        ///< Framework use only. Slot to allow external setting of selection menu options
    void roi3Changed();        ///< Framework use only. Slot to allow external setting of selection menu options
    void roi4Changed();        ///< Framework use only. Slot to allow external setting of selection menu options

    void lineProfileChanged();  ///< Framework use only. Slot to allow external setting of selection menu options

    void hozProfile1Changed();   ///< Framework use only. Slot to allow external setting of selection menu options
    void hozProfile2Changed();   ///< Framework use only. Slot to allow external setting of selection menu options
    void hozProfile3Changed();   ///< Framework use only. Slot to allow external setting of selection menu options
    void hozProfile4Changed();   ///< Framework use only. Slot to allow external setting of selection menu options
    void hozProfile5Changed();   ///< Framework use only. Slot to allow external setting of selection menu options

    void vertProfile1Changed();  ///< Framework use only. Slot to allow external setting of selection menu options
    void vertProfile2Changed();  ///< Framework use only. Slot to allow external setting of selection menu options
    void vertProfile3Changed();  ///< Framework use only. Slot to allow external setting of selection menu options
    void vertProfile4Changed();  ///< Framework use only. Slot to allow external setting of selection menu options
    void vertProfile5Changed();  ///< Framework use only. Slot to allow external setting of selection menu options


    void actionRequest( QString action, QStringList arguments, bool initialise, QAction* originator ); // Perform a named action


    void emitComponentHostRequest( const QEActionRequests& request ){ emit componentHostRequest( request ); }

    QSize getVideoDestinationSize();                // Get the size of the widget where the image is being displayed (either a scroll widget within the QEImage widget, or a full screen main window)
    void showImageContextMenuCommon( const QPoint& pos, const QPoint& globalPos );  // Common support for showImageContextMenu() and showImageContextMenuFullScreen()

    void saveConfiguration( PersistanceManager* pm );
    void restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase);

    void setup();
    qcaobject::QCaObject* createQcaItem( unsigned int variableIndex );

    QCAALARMINFO_SEVERITY lastSeverity;
    bool isConnected;

    bool imageSizeSet;      // Flag the video widget size has been set (setImageSize() has been called and done something)
    void setImageSize();    // Set the video widget size so it will match the processed image.

    QGridLayout* mainLayout;
    QGridLayout* graphicsLayout;

    QFrame *buttonGroup;
    QToolBar* toolBar;

    QScrollArea* scrollArea;
    bool initScrollPosSet;

    MpegSource*  mpegSource;
    VideoWidget* videoWidget;

    bool appHostsControls;
    bool hostingAppAvailable;

    bool fullContextMenu;

    void presentControls();

    bool displayMarkups;

    QString name;                              // Widget unique name for dislay and idetification purpose

    bool fullScreen;                            // True if in full screen mode
    fullScreenWindow* fullScreenMainWindow;     // Main window used to present image in full screen mode. Only present when in full screen mode

    // Button widgets
    QPushButton* pauseButton;
    QPushButton* saveButton;
    QPushButton* targetButton;
    QPushButton* zoomButton;
    QPushButton* selectModeButton;
    QPushButton* markupDisplayButton;
    QPushButton* flipRotateButton;

    // External actions optionally provided by the application creating this widget
    QAction* pauseExternalAction;

    // Profile graphic widgets
    QLabel* vSliceLabel;
    QLabel* hSliceLabel;
    QLabel* profileLabel;
    profilePlot* vSliceDisplay;
    profilePlot* hSliceDisplay;
    profilePlot* profileDisplay;

    imageDisplayProperties* imageDisplayProps;

    // Menus
    zoomMenu*       zMenu;
    flipRotateMenu* frMenu;
    selectMenu*     sMenu;
    markupDisplayMenu* mdMenu;

    QEImageOptionsDialog* optionsDialog;

    // Presentation
    bool paused;

    bool enableHozSlicePresentation;
    bool enableVertSlicePresentation;
    bool enableProfilePresentation;

    // Image and related information
    QCaDateTime imageTime;

    // Image history
    recording* recorder;

    // Region of interest information
    areaInfo roiInfo[4];

    // User selected information
    int vSlice1X;
    unsigned int vSlice1Thickness;
    int vSlice2X;
    unsigned int vSlice2Thickness;
    int vSlice3X;
    unsigned int vSlice3Thickness;
    int vSlice4X;
    unsigned int vSlice4Thickness;
    int vSlice5X;
    unsigned int vSlice5Thickness;

    int hSlice1Y;
    unsigned int hSlice1Thickness;
    int hSlice2Y;
    unsigned int hSlice2Thickness;
    int hSlice3Y;
    unsigned int hSlice3Thickness;
    int hSlice4Y;
    unsigned int hSlice4Thickness;
    int hSlice5Y;
    unsigned int hSlice5Thickness;

    areaInfo lineProfileInfo;
    areaInfo ellipseInfo;
    QPoint profileLineStart;
    QPoint profileLineEnd;
    unsigned int profileThickness;

    QPoint selectedArea1Point1;
    QPoint selectedArea1Point2;
    QPoint selectedArea2Point1;
    QPoint selectedArea2Point2;
    QPoint selectedArea3Point1;
    QPoint selectedArea3Point2;
    QPoint selectedArea4Point1;
    QPoint selectedArea4Point2;

    pointInfo targetInfo;
    pointInfo beamInfo;

    bool haveVSlice1X;
    bool haveVSlice2X;
    bool haveVSlice3X;
    bool haveVSlice4X;
    bool haveVSlice5X;
    bool haveHSlice1Y;
    bool haveHSlice2Y;
    bool haveHSlice3Y;
    bool haveHSlice4Y;
    bool haveHSlice5Y;
    bool haveProfileLine;
    bool haveSelectedArea1;
    bool haveSelectedArea2;
    bool haveSelectedArea3;
    bool haveSelectedArea4;


    // Private methods
    void generateVSlice( int x, unsigned int thickness );                           // Generate a profile along a line down an image at a given X position
    void generateHSlice( int y, unsigned int thickness );                           // Generate a profile along a line across an image at a given Y position
    void generateProfile( QPoint point1, QPoint point2, unsigned int thickness );   // Generate a profile along an arbitrary line through an image.
    void displaySelectedAreaInfo( const int region, const QPoint point1, const QPoint point2 );  // Display textual info about a selected area

    void updateMarkupData();                                               // Update markups if required. (For example, after image update)

    imageProcessor iProcessor;                              // Image processor. Generates images for presentation from raw image data and formatting information such as brightness, contrast, flip, rotate, canvas size, etc
    void displayImage();                                    // Display a new image.

    void zoomToArea();                                      // Zoom to the area selected on the image
    void setResizeOptionAndZoom( int zoomIn );              // Set the zoom percentage (and force zoom mode)

    // Data generated from pixel profiles
    // Kept for life of QEImage to avoid regenerating each time data is calculated
    QVector<QPointF> vSliceData;
    QVector<QPointF> hSliceData;
    QVector<QPointF> profileData;

    // Icons
    QIcon* pauseButtonIcon;
    QIcon* playButtonIcon;

    void setRegionAutoBrightnessContrast( QPoint point1, QPoint point2 );    // Update the brightness and contrast, if in auto, to match the recently selected region

    void doEnableButtonBar( bool enableButtonBar );
    void doEnableImageDisplayProperties( bool enableImageDisplayProperties );
    void doEnableRecording( bool enableRecording );
    void doContrastReversal( bool contrastReversal );
    void doEnableVertSlice1Selection( bool enableVSliceSelection );
    void doEnableVertSlice2Selection( bool enableVSliceSelection );
    void doEnableVertSlice3Selection( bool enableVSliceSelection );
    void doEnableVertSlice4Selection( bool enableVSliceSelection );
    void doEnableVertSlice5Selection( bool enableVSliceSelection );
    void doEnableHozSlice1Selection( bool enableHSliceSelection );
    void doEnableHozSlice2Selection( bool enableHSliceSelection );
    void doEnableHozSlice3Selection( bool enableHSliceSelection );
    void doEnableHozSlice4Selection( bool enableHSliceSelection );
    void doEnableHozSlice5Selection( bool enableHSliceSelection );
    void doEnableAreaSelection( /*imageContextMenu::imageContextMenuOptions area,*/ bool enableAreaSelection );
    void doEnableProfileSelection( bool enableProfileSelection );
    void doEnableTargetSelection( bool enableTargetSelection );
    void doEnableBeamSelection( bool enableBeamSelection );

    applicationLauncher programLauncher1;
    applicationLauncher programLauncher2;

    QString url;                            // URL (before macro substitutions) used when sourcing images from an MPEG stream

    ellipseVariableDefinitions ellipseVariableUsage;    // Determines how ellipse variables are used ( bounding rectangle, or centre and size)

    // Drag and Drop
protected:
    void dragEnterEvent(QDragEnterEvent *event) { qcaDragEnterEvent( event ); }
    void dropEvent(QDropEvent *event)           { qcaDropEvent( event ); }
    // Don't drag from interactive widget void mousePressEvent(QMouseEvent *event)    { qcaMousePressEvent( event ); }
    void setDrop( QVariant drop );
    QVariant getDrop();
    QString copyVariable();
    QVariant copyData();
    void paste( QVariant v );

    void resizeEvent(QResizeEvent* );

    //=================================================================================
    // Multiple Variable properties
    // These properties should be similar for every widget using multiple variables (The number of variables may vary).
    // WHEN MAKING CHANGES: search for MULTIPLEVARIABLEPROPERTIESBASE and change all occurances.
    private:
        QCaVariableNamePropertyManager variableNamePropertyManagers[QEIMAGE_NUM_VARIABLES];
    public:

    // Define a variable
    // Note, the QPROPERTY declaration itself can't be in this macro
#define VARIABLE_PROPERTY_ACCESS(VAR_INDEX) \
    void    setVariableName##VAR_INDEX##Property( QString variableName ){ variableNamePropertyManagers[VAR_INDEX].setVariableNameProperty( variableName ); } \
    QString getVariableName##VAR_INDEX##Property(){ return variableNamePropertyManagers[VAR_INDEX].getVariableNameProperty(); }

    VARIABLE_PROPERTY_ACCESS(0)
    /// EPICS variable name (CA PV).
    /// This variable is used as the source the image waveform.
    Q_PROPERTY(QString imageVariable READ getVariableName0Property WRITE setVariableName0Property)

    VARIABLE_PROPERTY_ACCESS(1)
    /// EPICS variable name (CA PV).
    /// This variable is used to read the format of the image.
    Q_PROPERTY(QString formatVariable READ getVariableName1Property WRITE setVariableName1Property)

    VARIABLE_PROPERTY_ACCESS(2)
    /// EPICS variable name (CA PV).
    /// This variable is used to read the bit depth of the image.
    Q_PROPERTY(QString bitDepthVariable READ getVariableName2Property WRITE setVariableName2Property)

    VARIABLE_PROPERTY_ACCESS(3)
    /// EPICS variable name (CA PV).
    /// This variable is used to infer the bit depth of the image.
    Q_PROPERTY(QString dataTypeVariable READ getVariableName3Property WRITE setVariableName3Property)

    VARIABLE_PROPERTY_ACCESS(4)
    /// EPICS variable name (CA PV).
    /// This variable is used to read the width of the image.
    Q_PROPERTY(QString widthVariable READ getVariableName4Property WRITE setVariableName4Property)

    VARIABLE_PROPERTY_ACCESS(5)
    /// EPICS variable name (CA PV).
    /// This variable is used to read the height of the image.
    Q_PROPERTY(QString heightVariable READ getVariableName5Property WRITE setVariableName5Property)

    VARIABLE_PROPERTY_ACCESS(6)
    /// EPICS variable name (CA PV).
    /// This variable is used to read the number of area detector dimensions of the image.
    /// If used, this will be 2 (one element per pixel arranged by width and height) or
    /// 3 (multiple elements per pixel arranged by pixel, width and height)
    Q_PROPERTY(QString dimensionsVariable READ getVariableName6Property WRITE setVariableName6Property)

    VARIABLE_PROPERTY_ACCESS(7)
    /// EPICS variable name (CA PV).
    /// This variable is used to read the first area detector dimension of the image.
    /// If there are 2 dimensions, this will be the image width.
    /// If there are 3 dimensions, this will be the number of elements per pixel.
    Q_PROPERTY(QString dimension1Variable READ getVariableName7Property WRITE setVariableName7Property)

    VARIABLE_PROPERTY_ACCESS(8)
    /// EPICS variable name (CA PV).
    /// This variable is used to read the second area detector dimension of the image.
    /// If there are 2 dimensions, this will be the image height.
    /// If there are 3 dimensions, this will be the image width.
    Q_PROPERTY(QString dimension2Variable READ getVariableName8Property WRITE setVariableName8Property)

    VARIABLE_PROPERTY_ACCESS(9)
    /// EPICS variable name (CA PV).
    /// This variable is used to read the third area detector dimension of the image.
    /// If there are 3 dimensions, this will be the image height.
    Q_PROPERTY(QString dimension3Variable READ getVariableName9Property WRITE setVariableName9Property)

    VARIABLE_PROPERTY_ACCESS(10)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the first region of interest X position.
    Q_PROPERTY(QString regionOfInterest1XVariable READ getVariableName10Property WRITE setVariableName10Property)

    VARIABLE_PROPERTY_ACCESS(11)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the first region of interest Y position.
    Q_PROPERTY(QString regionOfInterest1YVariable READ getVariableName11Property WRITE setVariableName11Property)

    VARIABLE_PROPERTY_ACCESS(12)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the first region of interest width.
    Q_PROPERTY(QString regionOfInterest1WVariable READ getVariableName12Property WRITE setVariableName12Property)

    VARIABLE_PROPERTY_ACCESS(13)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the first region of interest height.
    Q_PROPERTY(QString regionOfInterest1HVariable READ getVariableName13Property WRITE setVariableName13Property)

    VARIABLE_PROPERTY_ACCESS(14)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the second region of interest X position.
    Q_PROPERTY(QString regionOfInterest2XVariable READ getVariableName14Property WRITE setVariableName14Property)

    VARIABLE_PROPERTY_ACCESS(15)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the second region of interest Y position.
    Q_PROPERTY(QString regionOfInterest2YVariable READ getVariableName15Property WRITE setVariableName15Property)

    VARIABLE_PROPERTY_ACCESS(16)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the second region of interest width.
    Q_PROPERTY(QString regionOfInterest2WVariable READ getVariableName16Property WRITE setVariableName16Property)

    VARIABLE_PROPERTY_ACCESS(17)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the second region of interest height.
    Q_PROPERTY(QString regionOfInterest2HVariable READ getVariableName17Property WRITE setVariableName17Property)

    VARIABLE_PROPERTY_ACCESS(18)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the third region of interest X position.
    Q_PROPERTY(QString regionOfInterest3XVariable READ getVariableName18Property WRITE setVariableName18Property)

    VARIABLE_PROPERTY_ACCESS(19)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the third region of interest Y position.
    Q_PROPERTY(QString regionOfInterest3YVariable READ getVariableName19Property WRITE setVariableName19Property)

    VARIABLE_PROPERTY_ACCESS(20)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the third region of interest width.
    Q_PROPERTY(QString regionOfInterest3WVariable READ getVariableName20Property WRITE setVariableName20Property)

    VARIABLE_PROPERTY_ACCESS(21)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the third region of interest height.
    Q_PROPERTY(QString regionOfInterest3HVariable READ getVariableName21Property WRITE setVariableName21Property)

    VARIABLE_PROPERTY_ACCESS(22)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the fourth region of interest X position.
    Q_PROPERTY(QString regionOfInterest4XVariable READ getVariableName22Property WRITE setVariableName22Property)

    VARIABLE_PROPERTY_ACCESS(23)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the fourth region of interest Y position.
    Q_PROPERTY(QString regionOfInterest4YVariable READ getVariableName23Property WRITE setVariableName23Property)

    VARIABLE_PROPERTY_ACCESS(24)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the fourth region of interest width.
    Q_PROPERTY(QString regionOfInterest4WVariable READ getVariableName24Property WRITE setVariableName24Property)

    VARIABLE_PROPERTY_ACCESS(25)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the fourth region of interest height.
    Q_PROPERTY(QString regionOfInterest4HVariable READ getVariableName25Property WRITE setVariableName25Property)

    VARIABLE_PROPERTY_ACCESS(26)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the selected target X position.
    Q_PROPERTY(QString targetXVariable READ getVariableName26Property WRITE setVariableName26Property)

    VARIABLE_PROPERTY_ACCESS(27)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the selected target Y position.
    Q_PROPERTY(QString targetYVariable READ getVariableName27Property WRITE setVariableName27Property)

    VARIABLE_PROPERTY_ACCESS(28)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the selected beam X position.
    Q_PROPERTY(QString beamXVariable READ getVariableName28Property WRITE setVariableName28Property)

    VARIABLE_PROPERTY_ACCESS(29)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the selected beam Y position.
    Q_PROPERTY(QString beamYVariable READ getVariableName29Property WRITE setVariableName29Property)

    VARIABLE_PROPERTY_ACCESS(30)
    /// EPICS variable name (CA PV).
    /// This variable is used to write a 'trigger' to initiate movement of the target into the beam as defined by the target and beam X and Y positions.
    Q_PROPERTY(QString targetTriggerVariable READ getVariableName30Property WRITE setVariableName30Property)

    VARIABLE_PROPERTY_ACCESS(31)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector clipping on/off command.
    Q_PROPERTY(QString clippingOnOffVariable READ getVariableName31Property WRITE setVariableName31Property)

    VARIABLE_PROPERTY_ACCESS(32)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector clipping low level.
    Q_PROPERTY(QString clippingLowVariable READ getVariableName32Property WRITE setVariableName32Property)

    VARIABLE_PROPERTY_ACCESS(33)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector clipping high level.
    Q_PROPERTY(QString clippingHighVariable READ getVariableName33Property WRITE setVariableName33Property)

    VARIABLE_PROPERTY_ACCESS(34)
    // Deprecated. Use profileHoz1Variable instead
    Q_PROPERTY(QString profileHozVariable READ getVariableName34Property WRITE setVariableName34Property DESIGNABLE false)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector first horizontal profile.
    Q_PROPERTY(QString profileHoz1Variable READ getVariableName34Property WRITE setVariableName34Property)

    VARIABLE_PROPERTY_ACCESS(35)
    // Deprecated. Use profileHoz1ThicknessVariable instead
    Q_PROPERTY(QString profileHozThicknessVariable READ getVariableName35Property WRITE setVariableName35Property DESIGNABLE false)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector first horizontal profile thickness.
    Q_PROPERTY(QString profileHoz1ThicknessVariable READ getVariableName35Property WRITE setVariableName35Property)

    VARIABLE_PROPERTY_ACCESS(36)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector second horizontal profile.
    Q_PROPERTY(QString profileHoz2Variable READ getVariableName36Property WRITE setVariableName36Property)

    VARIABLE_PROPERTY_ACCESS(37)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector second horizontal profile thickness.
    Q_PROPERTY(QString profileHoz2ThicknessVariable READ getVariableName37Property WRITE setVariableName37Property)

    VARIABLE_PROPERTY_ACCESS(38)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector third horizontal profile.
    Q_PROPERTY(QString profileHoz3Variable READ getVariableName38Property WRITE setVariableName38Property)

    VARIABLE_PROPERTY_ACCESS(39)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector third horizontal profile thickness.
    Q_PROPERTY(QString profileHoz3ThicknessVariable READ getVariableName39Property WRITE setVariableName39Property)

    VARIABLE_PROPERTY_ACCESS(40)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector fourth horizontal profile.
    Q_PROPERTY(QString profileHoz4Variable READ getVariableName40Property WRITE setVariableName40Property)

    VARIABLE_PROPERTY_ACCESS(41)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector fourth horizontal profile thickness.
    Q_PROPERTY(QString profileHoz4ThicknessVariable READ getVariableName41Property WRITE setVariableName41Property)

    VARIABLE_PROPERTY_ACCESS(42)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector fifth horizontal profile.
    Q_PROPERTY(QString profileHoz5Variable READ getVariableName42Property WRITE setVariableName42Property)

    VARIABLE_PROPERTY_ACCESS(43)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector fifth horizontal profile thickness.
    Q_PROPERTY(QString profileHoz5ThicknessVariable READ getVariableName43Property WRITE setVariableName43Property)

    VARIABLE_PROPERTY_ACCESS(44)
    // Deprecated. Use profileVert1Variable instead
    Q_PROPERTY(QString profileVertVariable READ getVariableName44Property WRITE setVariableName44Property DESIGNABLE false)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector first vertical profile.
    Q_PROPERTY(QString profileVert1Variable READ getVariableName44Property WRITE setVariableName44Property)

    VARIABLE_PROPERTY_ACCESS(45)
    // Deprecated. Use profileVert1ThicknessVariable instead
    Q_PROPERTY(QString profileVertThicknessVariable READ getVariableName45Property WRITE setVariableName45Property DESIGNABLE false)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector first vertical profile.
    Q_PROPERTY(QString profileVert1ThicknessVariable READ getVariableName45Property WRITE setVariableName45Property)

    VARIABLE_PROPERTY_ACCESS(46)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector second vertical profile.
    Q_PROPERTY(QString profileVert2Variable READ getVariableName46Property WRITE setVariableName46Property)

    VARIABLE_PROPERTY_ACCESS(47)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector second vertical profile.
    Q_PROPERTY(QString profileVert2ThicknessVariable READ getVariableName47Property WRITE setVariableName47Property)

    VARIABLE_PROPERTY_ACCESS(48)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector third vertical profile.
    Q_PROPERTY(QString profileVert3Variable READ getVariableName48Property WRITE setVariableName48Property)

    VARIABLE_PROPERTY_ACCESS(49)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector third vertical profile.
    Q_PROPERTY(QString profileVert3ThicknessVariable READ getVariableName49Property WRITE setVariableName49Property)

    VARIABLE_PROPERTY_ACCESS(50)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector fourth vertical profile.
    Q_PROPERTY(QString profileVert4Variable READ getVariableName50Property WRITE setVariableName50Property)

    VARIABLE_PROPERTY_ACCESS(51)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector fourth vertical profile.
    Q_PROPERTY(QString profileVert4ThicknessVariable READ getVariableName51Property WRITE setVariableName51Property)

    VARIABLE_PROPERTY_ACCESS(52)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector fifth vertical profile.
    Q_PROPERTY(QString profileVert5Variable READ getVariableName52Property WRITE setVariableName52Property)

    VARIABLE_PROPERTY_ACCESS(53)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector fifth vertical profile.
    Q_PROPERTY(QString profileVert5ThicknessVariable READ getVariableName53Property WRITE setVariableName53Property)

    VARIABLE_PROPERTY_ACCESS(54)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector arbitrary line profile start X.
    Q_PROPERTY(QString lineProfileX1Variable READ getVariableName54Property WRITE setVariableName54Property)

    VARIABLE_PROPERTY_ACCESS(55)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector arbitrary line profile start Y.
    Q_PROPERTY(QString lineProfileY1Variable READ getVariableName55Property WRITE setVariableName55Property)

    VARIABLE_PROPERTY_ACCESS(56)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector arbitrary line profile end X.
    Q_PROPERTY(QString lineProfileX2Variable READ getVariableName56Property WRITE setVariableName56Property)

    VARIABLE_PROPERTY_ACCESS(57)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector arbitrary line profile end Y.
    Q_PROPERTY(QString lineProfileY2Variable READ getVariableName57Property WRITE setVariableName57Property)

    VARIABLE_PROPERTY_ACCESS(58)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector arbitrary line profile end Y.
    Q_PROPERTY(QString lineProfileThicknessVariable READ getVariableName58Property WRITE setVariableName58Property)

    VARIABLE_PROPERTY_ACCESS(59)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector horizontal profile array.
    Q_PROPERTY(QString profileHozArrayVariable READ getVariableName59Property WRITE setVariableName59Property)

    VARIABLE_PROPERTY_ACCESS(60)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector vertical profile array.
    Q_PROPERTY(QString profileVertArrayVariable READ getVariableName60Property WRITE setVariableName60Property)

    VARIABLE_PROPERTY_ACCESS(61)
    /// EPICS variable name (CA PV).
    /// This variable is used to write the areadetector arbitrary line profile array.
    Q_PROPERTY(QString lineProfileArrayVariable READ getVariableName61Property WRITE setVariableName61Property)

    VARIABLE_PROPERTY_ACCESS(62)
    /// EPICS variable name (CA PV).
    /// This variable is used to read an ellipse X (center or top left corner of bounding rectangle depending on property ellipseDefinition).
    Q_PROPERTY(QString ellipseXVariable READ getVariableName62Property WRITE setVariableName62Property)

    VARIABLE_PROPERTY_ACCESS(63)
    /// EPICS variable name (CA PV).
    /// This variable is used to read an ellipse Y (center or top left corner of bounding rectangle depending on property ellipseDefinition).
    Q_PROPERTY(QString ellipseYVariable READ getVariableName63Property WRITE setVariableName63Property)

    VARIABLE_PROPERTY_ACCESS(64)
    /// EPICS variable name (CA PV).
    /// This variable is used to read an ellipse width.
    Q_PROPERTY(QString ellipseWVariable READ getVariableName64Property WRITE setVariableName64Property)

    VARIABLE_PROPERTY_ACCESS(65)
    /// EPICS variable name (CA PV).
    /// This variable is used to read an ellipse height
    Q_PROPERTY(QString ellipseHVariable READ getVariableName65Property WRITE setVariableName65Property)

#undef VARIABLE_PROPERTY_ACCESS

    /// Macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2... Values may be quoted strings. For example, 'CAM=1, NAME = "Image 1"'
    /// These substitutions are applied to all the variable names.
    Q_PROPERTY(QString variableSubstitutions READ getVariableNameSubstitutionsProperty WRITE setVariableNameSubstitutionsProperty)

    /// Property access function for #variableSubstitutions property. This has special behaviour to work well within designer.
    void    setVariableNameSubstitutionsProperty( QString variableNameSubstitutions )
    {
        for( int i = 0; i < QEIMAGE_NUM_VARIABLES; i++ )
        {
            variableNamePropertyManagers[i].setSubstitutionsProperty( variableNameSubstitutions );
        }
    }
    /// Property access function for #variableSubstitutions property. This has special behaviour to work well within designer.
    QString getVariableNameSubstitutionsProperty()
    {
        return variableNamePropertyManagers[0].getSubstitutionsProperty();
    }
public:
    //=================================================================================


    // BEGIN-STANDARD-PROPERTIES ======================================================
    // Standard properties
    // These properties should be identical for every widget using them.
    // WHEN MAKING CHANGES: Use the update_widget_properties script in the
    // resources directory.
public slots:
    /// Slot to set the visibility of a QE widget, taking into account the user level.
    /// Widget will be hidden if hidden by a call this slot, by will only be made visible by a calll to this slot if the user level allows.
    void setManagedVisible( bool v ){ setRunVisible( v ); }
public:
    /// Use the variable as the tool tip. Default is true. Tool tip property will be overwritten by the variable name.
    ///
    Q_PROPERTY(bool variableAsToolTip READ getVariableAsToolTip WRITE setVariableAsToolTip)

    /// Allow drag/drops operations to this widget. Default is false. Any dropped text will be used as a new variable name.
    ///
    Q_PROPERTY(bool allowDrop READ getAllowDrop WRITE setAllowDrop)

    /// Display the widget. Default is true.
    /// Setting this property false is usefull if widget is only used to provide a signal - for example, when supplying data to a QELink widget.
    /// Note, when false the widget will still be visible in Qt Designer.
    Q_PROPERTY(bool visible READ getRunVisible WRITE setRunVisible)

    /// Set the ID used by the message filtering system. Default is zero.
    /// Widgets or applications that use messages from the framework have the option of filtering on this ID.
    /// For example, by using a unique message source ID a QELog widget may be set up to only log messages from a select set of widgets.
    Q_PROPERTY(unsigned int messageSourceId READ getMessageSourceId WRITE setMessageSourceId )

    /// Hide style sheet from designer as style calculation by the styleManager and not directly setable per se.
    /// This also stops transient styles being saved to the ui file.
    Q_PROPERTY(QString styleSheet   READ styleSheet       WRITE setStyleSheet  DESIGNABLE false)

    /// Style Sheet string to be applied before, i.e. lower priority than, any other style, e.g. alarm style and/or user level style.
    /// Default is an empty string.
    Q_PROPERTY(QString defaultStyle READ getStyleDefault  WRITE setStyleDefault)

    /// Style Sheet string to be applied when the widget is displayed in 'User' mode. Default is an empty string.
    /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
    /// This Style Sheet string will be applied by the styleManager class.
    /// Refer to the styleManager class for details about how this Style Sheet string will be merged with any pre-existing Style Sheet string
    /// and any Style Sheet strings generated during the display of data.
    Q_PROPERTY(QString userLevelUserStyle READ getStyleUser WRITE setStyleUser)

    /// Style Sheet string to be applied when the widget is displayed in 'Scientist' mode. Default is an empty string.
    /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
    /// This Style Sheet string will be applied by the styleManager class.
    /// Refer to the styleManager class for details about how this Style Sheet string will be merged with any pre-existing Style Sheet string
    /// and any Style Sheet strings generated during the display of data.
    Q_PROPERTY(QString userLevelScientistStyle READ getStyleScientist WRITE setStyleScientist)

    /// Style Sheet string to be applied when the widget is displayed in 'Engineer' mode. Default is an empty string.
    /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
    /// This Style Sheet string will be applied by the styleManager class.
    /// Refer to the styleManager class for details about how this Style Sheet string will be merged with any pre-existing Style Sheet string
    /// and any Style Sheet strings generated during the display of data.
    Q_PROPERTY(QString userLevelEngineerStyle READ getStyleEngineer WRITE setStyleEngineer)

    /// \enum UserLevels
    /// User friendly enumerations for #userLevelVisibility and #userLevelEnabled properties - refer to #userLevelVisibility and #userLevelEnabled properties and userLevel enumeration for details.
    enum UserLevels { User      = userLevelTypes::USERLEVEL_USER,          ///< Refer to USERLEVEL_USER for details
                      Scientist = userLevelTypes::USERLEVEL_SCIENTIST,     ///< Refer to USERLEVEL_SCIENTIST for details
                      Engineer  = userLevelTypes::USERLEVEL_ENGINEER       ///< Refer to USERLEVEL_ENGINEER for details
                              };
    Q_ENUMS(UserLevels)

    /// Lowest user level at which the widget is visible. Default is 'User'.
    /// Used when designing GUIs that display more and more detail according to the user mode.
    /// The user mode is set application wide through the QELogin widget, or programatically through setUserLevel()
    /// Widgets that are always visible should be visible at 'User'.
    /// Widgets that are only used by scientists managing the facility should be visible at 'Scientist'.
    /// Widgets that are only used by engineers maintaining the facility should be visible at 'Engineer'.
    Q_PROPERTY(UserLevels userLevelVisibility READ getUserLevelVisibilityProperty WRITE setUserLevelVisibilityProperty)

    /// Lowest user level at which the widget is enabled. Default is 'User'.
    /// Used when designing GUIs that allow access to more and more detail according to the user mode.
    /// The user mode is set application wide through the QELogin widget, or programatically through setUserLevel()
    /// Widgets that are always accessable should be visible at 'User'.
    /// Widgets that are only accessable to scientists managing the facility should be visible at 'Scientist'.
    /// Widgets that are only accessable to engineers maintaining the facility should be visible at 'Engineer'.
    Q_PROPERTY(UserLevels userLevelEnabled READ getUserLevelEnabledProperty WRITE setUserLevelEnabledProperty)

    UserLevels getUserLevelVisibilityProperty() { return (UserLevels)getUserLevelVisibility(); }            ///< Access function for #userLevelVisibility property - refer to #userLevelVisibility property for details
    void setUserLevelVisibilityProperty( UserLevels level ) { setUserLevelVisibility( (userLevelTypes::userLevels)level ); }///< Access function for #userLevelVisibility property - refer to #userLevelVisibility property for details
    UserLevels getUserLevelEnabledProperty() { return (UserLevels)getUserLevelEnabled(); }                  ///< Access function for #userLevelEnabled property - refer to #userLevelEnabled property for details
    void setUserLevelEnabledProperty( UserLevels level ) { setUserLevelEnabled( (userLevelTypes::userLevels)level ); }      ///< Access function for #userLevelEnabled property - refer to #userLevelEnabled property for details

    /// DEPRECATED. USE displayAlarmStateOption INSTEAD.
    /// If set (default) widget will indicate the alarm state of any variable data it is displaying.
    /// If clear widget will never indicate the alarm state of any variable data it is displaying.
    /// Typically the background colour is set to indicate the alarm state.
    /// Note, this property is included in the set of standard properties as it applies to most widgets. It
    /// will do nothing for widgets that don't display data.
    Q_PROPERTY(bool displayAlarmState READ getDisplayAlarmState WRITE setDisplayAlarmState DESIGNABLE false)

    /// \enum DisplayAlarmStateOptions
    /// User friendly enumerations for #displayAlarmStateOption property - refer to #displayAlarmStateOption property and displayAlarmStateOptions enumeration for details.
    enum DisplayAlarmStateOptions { Never       = standardProperties::DISPLAY_ALARM_STATE_NEVER,          ///< Refer to DISPLAY_ALARM_STATE_NEVER for details
                                    Always      = standardProperties::DISPLAY_ALARM_STATE_ALWAYS,         ///< Refer to DISPLAY_ALARM_STATE_ALWAYS for details
                                    WhenInAlarm = standardProperties::DISPLAY_ALARM_STATE_WHEN_IN_ALARM   ///< Refer to DISPLAY_ALARM_STATE_WHEN_IN_ALARM for details
                              };
    Q_ENUMS(DisplayAlarmStateOptions)
    /// If 'Always' (default) widget will indicate the alarm state of any variable data it is displaying, including 'No Alarm'
    /// If 'Never' widget will never indicate the alarm state of any variable data it is displaying.
    /// If 'WhenInAlarm' widget only indicate the alarm state of any variable data it is displaying if it is 'in alarm'.
    /// Typically the background colour is set to indicate the alarm state.
    /// Note, this property is included in the set of standard properties as it applies to most widgets. It
    /// will do nothing for widgets that don't display data.
    Q_PROPERTY(DisplayAlarmStateOptions displayAlarmStateOption READ getDisplayAlarmStateOptionProperty WRITE setDisplayAlarmStateOptionProperty)

    DisplayAlarmStateOptions getDisplayAlarmStateOptionProperty() { return (DisplayAlarmStateOptions)getDisplayAlarmStateOption(); }            ///< Access function for #displayAlarmStateOption property - refer to #displayAlarmStateOption property for details
    void setDisplayAlarmStateOptionProperty( DisplayAlarmStateOptions option ) { setDisplayAlarmStateOption( (displayAlarmStateOptions)option ); }///< Access function for #displayAlarmStateOption property - refer to #displayAlarmStateOption property for details

public:
    // END-STANDARD-PROPERTIES ========================================================

//==========================================================================
// Widget specific properties
public:

    // Format options (Mono, RGB, etc)
    Q_ENUMS(FormatOptions)
    /// Video format.
    /// EPICS data type size will typically be adequate for the number of bits required (one byte for 8 bits, 2 bytes for 12 and 16 bits), but can be larger (4 bytes for 24 bits.)
    Q_PROPERTY(FormatOptions formatOption READ getFormatOptionProperty WRITE setFormatOptionProperty)

    /// \enum FormatOptions
    /// User friendly enumerations for #formatOption property - refer to #formatOption property and #formatOptions enumeration for details.
    enum FormatOptions { Mono     = imageDataFormats::MONO,      ///< Grey scale
                         Bayer    = imageDataFormats::BAYERRG,     ///< Colour (Bayer Red Green)
                         BayerGB  = imageDataFormats::BAYERGB,     ///< Colour (Bayer Green Blue)
                         BayerBG  = imageDataFormats::BAYERBG,     ///< Colour (Bayer Blue Green)
                         BayerGR  = imageDataFormats::BAYERGR,     ///< Colour (Bayer Green Red)
                         BayerRG  = imageDataFormats::BAYERRG,     ///< Colour (Bayer Red Green)
                         rgb1     = imageDataFormats::RGB1,      ///< Colour (24 bit RGB)
                         rgb2     = imageDataFormats::RGB2,      ///< Colour (??? bit RGB)
                         rgb3     = imageDataFormats::RGB3,      ///< Colour (??? bit RGB)
                         yuv444   = imageDataFormats::YUV444,    ///< Colour (???)
                         yuv422   = imageDataFormats::YUV422,    ///< Colour (???)
                         yuv421   = imageDataFormats::YUV421 };  ///< Colour (???)

    void setFormatOptionProperty( FormatOptions formatOption ){ setFormatOption( (imageDataFormats::formatOptions)formatOption ); }  ///< Access function for #formatOption property - refer to #formatOption property for details
    FormatOptions getFormatOptionProperty(){ return (FormatOptions)getFormatOption(); }                                     ///< Access function for #formatOption property - refer to #formatOption property for details

    // Mono format option bit depths
    /// Bit depth.
    /// Note, EPICS data type size will typically be adequate for the number of bits required (one byte for up to 8 bits, 2 bytes for up to 16 bits, etc),
    /// but can be larger (for example, 4 bytes for 24 bits) and may be larger than nessesary (4 bytes for 8 bits).
    Q_PROPERTY(unsigned int bitDepth READ getBitDepthProperty WRITE setBitDepthProperty)

    void setBitDepthProperty( unsigned int bitDepth ){ setBitDepth( bitDepth ); }                                           ///< Access function for #bitDepth property - refer to #bitDepth property for details
    unsigned int getBitDepthProperty(){ return getBitDepth(); }                                                             ///< Access function for #bitDepth property - refer to #bitDepth property for details

    //=========

    // This property is deprecated. Use enableVertSlice1Selection instead.
    // If true, the option to select a vertical slice through the image will be available to the user.
    // This will be used to generate a horizontal pixel profile, and write the position of the slice to the optional variable specified by the #profileVert1Variable property.
    // The profile will only be presented to the user if enableVertSlicePresentation property is true.
    Q_PROPERTY(bool enableVertSliceSelection READ getEnableVertSlice1Selection WRITE setEnableVertSlice1Selection DESIGNABLE false)

    /// If true, the option to select a vertical slice through the image will be available to the user.
    /// This will be used to generate a horizontal pixel profile, and write the position of the slice to the optional variable specified by the #profileVert1Variable property.
    /// The profile will only be presented to the user if #enableVertSlicePresentation property is true.
    Q_PROPERTY(bool enableVertSlice1Selection READ getEnableVertSlice1Selection WRITE setEnableVertSlice1Selection)

    /// If true, the option to select a second vertical slice through the image will be available to the user.
    /// This will be used to write the position of the slice to the optional variable specified by the #profileVert2Variable property.
    Q_PROPERTY(bool enableVertSlice2Selection READ getEnableVertSlice2Selection WRITE setEnableVertSlice2Selection)

    /// If true, the option to select a third vertical slice through the image will be available to the user.
    /// This will be used to write the position of the slice to the optional variable specified by the #profileVert3Variable property.
    Q_PROPERTY(bool enableVertSlice3Selection READ getEnableVertSlice3Selection WRITE setEnableVertSlice3Selection)

    /// If true, the option to select a fourth vertical slice through the image will be available to the user.
    /// This will be used to write the position of the slice to the optional variable specified by the #profileVert4Variable property.
    Q_PROPERTY(bool enableVertSlice4Selection READ getEnableVertSlice4Selection WRITE setEnableVertSlice4Selection)

    /// If true, the option to select a fifth vertical slice through the image will be available to the user.
    /// This will be used to write the position of the slice to the optional variable specified by the #profileVert5Variable property.
    Q_PROPERTY(bool enableVertSlice5Selection READ getEnableVertSlice5Selection WRITE setEnableVertSlice5Selection)

    //=========

    // This property is deprecated. Use enableHozSlice1Selection instead.
    // If true, the option to select a horizontal slice through the image will be available to the user.
    // This will be used to generate a horizontal pixel profile, and write the position of the slice to the optional variable specified by the profileHoz1Variable property.
    // The profile will only be presented to the user if enableHozSlicePresentation property is true.
    Q_PROPERTY(bool enableHozSliceSelection READ getEnableHozSlice1Selection WRITE setEnableHozSlice1Selection  DESIGNABLE false)

    /// If true, the option to select a horizontal slice through the image will be available to the user.
    /// This will be used to generate a horizontal pixel profile, and write the position of the slice to the optional variable specified by the profileHoz1Variable property.
    /// The profile will only be presented to the user if enableHozSlicePresentation property is true.
    Q_PROPERTY(bool enableHozSlice1Selection READ getEnableHozSlice1Selection WRITE setEnableHozSlice1Selection)

    /// If true, the option to select a second horizontal slice through the image will be available to the user.
    /// This will be used to write the position of the slice to the optional variable specified by the #profileHoz2Variable property.
    Q_PROPERTY(bool enableHozSlice2Selection READ getEnableHozSlice2Selection WRITE setEnableHozSlice2Selection)

    /// If true, the option to select a third horizontal slice through the image will be available to the user.
    /// This will be used to write the position of the slice to the optional variable specified by the #profileHoz3Variable property.
    Q_PROPERTY(bool enableHozSlice3Selection READ getEnableHozSlice3Selection WRITE setEnableHozSlice3Selection)

    /// If true, the option to select a fourth horizontal slice through the image will be available to the user.
    /// This will be used to write the position of the slice to the optional variable specified by the #profileHoz4Variable property.
    Q_PROPERTY(bool enableHozSlice4Selection READ getEnableHozSlice4Selection WRITE setEnableHozSlice4Selection)

    /// If true, the option to select a fifth horizontal slice through the image will be available to the user.
    /// This will be used to write the position of the slice to the optional variable specified by the #profileHoz5Variable property.
    Q_PROPERTY(bool enableHozSlice5Selection READ getEnableHozSlice5Selection WRITE setEnableHozSlice5Selection)

    //=========

    /// If true, the option to select an arbitrary line through any part of the image will be available to the user.
    /// This will be used to generate a pixel profile.
    Q_PROPERTY(bool enableProfileSelection READ getEnableProfileSelection WRITE setEnableProfileSelection)

    //=========

    /// If true, the horizontal pixel profile plot will be presented to the user when a horizontal slice is selected.
    /// If false, the profile plot will not be presented to the user. False is used when a variable has been specified
    /// in the #profileHozVariable property and the presentation or use of the slice information is being managed elsewhere.
    Q_PROPERTY(bool enableHozSlicePresentation READ getEnableHozSlicePresentation WRITE setEnableHozSlicePresentation)

    /// If true, the vertical pixel profile plot will be presented to the user when a vertical slice is selected.
    /// If false, the profile plot will not be presented to the user. False is used when a variable has been specified
    /// in the #profileVertVariable property and the presentation or use of the slice information is being managed elsewhere.
    Q_PROPERTY(bool enableVertSlicePresentation READ getEnableVertSlicePresentation WRITE setEnableVertSlicePresentation)

    /// If true, the vertical pixel profile plot will be presented to the user when a vertical slice is selected.
    /// If false, the profile plot will not be presented to the user. False is used when a variable has been specified
    /// in the #profileVertVariable property and the presentation or use of the slice information is being managed elsewhere.
    Q_PROPERTY(bool enableProfilePresentation READ getEnableProfilePresentation WRITE setEnableProfilePresentation)

    //=========

    /// If true, the user will be able to select area 1. These are used for selection of Region of Interests,
    /// and for zooming to area 1
    Q_PROPERTY(bool enableArea1Selection READ getEnableArea1Selection WRITE setEnableArea1Selection)

    /// If true, the user will be able to select area 2. These are used for selection of Region of Interests,
    /// and for zooming to area 2
    Q_PROPERTY(bool enableArea2Selection READ getEnableArea2Selection WRITE setEnableArea2Selection)

    /// If true, the user will be able to select area 3. These are used for selection of Region of Interests,
    /// and for zooming to area 3
    Q_PROPERTY(bool enableArea3Selection READ getEnableArea3Selection WRITE setEnableArea3Selection)

    /// If true, the user will be able to select area 4. These are used for selection of Region of Interests,
    /// and for zooming to area 4
    Q_PROPERTY(bool enableArea4Selection READ getEnableArea4Selection WRITE setEnableArea4Selection)

    /// If true, the user will be able to select points on the image to mark a target position.
    /// This can be used for automatic beam positioning.
    Q_PROPERTY(bool enableTargetSelection READ getEnableTargetSelection WRITE setEnableTargetSelection)

    /// If true, the user will be able to select points on the image to mark a beam position.
    /// This can be used for automatic beam positioning.
    Q_PROPERTY(bool enableBeamSelection READ getEnableBeamSelection WRITE setEnableBeamSelection)

    //=========

    // Deprecated. Name of horizontal slice profile markup
    Q_PROPERTY(QString hozSliceLegend READ getHozSlice1Legend WRITE setHozSlice1Legend DESIGNABLE false)
    /// Name of horizontal slice 1 markup
    Q_PROPERTY(QString hozSlice1Legend READ getHozSlice1Legend WRITE setHozSlice1Legend)

    /// Name of horizontal slice 2 markup
    Q_PROPERTY(QString hozSlice2Legend READ getHozSlice2Legend WRITE setHozSlice2Legend)

    /// Name of horizontal slice 3 markup
    Q_PROPERTY(QString hozSlice3Legend READ getHozSlice3Legend WRITE setHozSlice3Legend)

    /// Name of horizontal slice 4 markup
    Q_PROPERTY(QString hozSlice4Legend READ getHozSlice4Legend WRITE setHozSlice4Legend)

    /// Name of horizontal slice 5 markup
    Q_PROPERTY(QString hozSlice5Legend READ getHozSlice5Legend WRITE setHozSlice5Legend)

    // Deprecated. Name of vertical slice profile markup
    Q_PROPERTY(QString vertSliceLegend READ getVertSlice1Legend WRITE setVertSlice1Legend DESIGNABLE false)
    /// Name of vertical slice 1 markup
    Q_PROPERTY(QString vertSlice1Legend READ getVertSlice1Legend WRITE setVertSlice1Legend)

    /// Name of vertical slice 2 markup
    Q_PROPERTY(QString vertSlice2Legend READ getVertSlice2Legend WRITE setVertSlice2Legend)

    /// Name of vertical slice 3 markup
    Q_PROPERTY(QString vertSlice3Legend READ getVertSlice3Legend WRITE setVertSlice3Legend)

    /// Name of vertical slice 4 markup
    Q_PROPERTY(QString vertSlice4Legend READ getVertSlice4Legend WRITE setVertSlice4Legend)

    /// Name of vertical slice 5 markup
    Q_PROPERTY(QString vertSlice5Legend READ getVertSlice5Legend WRITE setVertSlice5Legend)

    /// Name of arbitrary profile markup
    Q_PROPERTY(QString profileLegend READ getprofileLegend WRITE setProfileLegend)

    /// Name of area selection 1 markup
    Q_PROPERTY(QString areaSelection1Legend READ getAreaSelection1Legend WRITE setAreaSelection1Legend)

    /// Name of area selection 2 markup
    Q_PROPERTY(QString areaSelection2Legend READ getAreaSelection2Legend WRITE setAreaSelection2Legend)

    /// Name of area selection 3 markup
    Q_PROPERTY(QString areaSelection3Legend READ getAreaSelection3Legend WRITE setAreaSelection3Legend)

    /// Name of area selection 4 markup
    Q_PROPERTY(QString areaSelection4Legend READ getAreaSelection4Legend WRITE setAreaSelection4Legend)

    /// Name of target markup
    Q_PROPERTY(QString targetLegend READ getTargetLegend WRITE setTargetLegend)

    /// Name of beam markup
    Q_PROPERTY(QString beamLegend READ getBeamLegend WRITE setBeamLegend)

    /// Name of ellipse markup
    Q_PROPERTY(QString ellipseLegend READ getEllipseLegend WRITE setEllipseLegend)

    //=========

    //=========

    // Deprecated. If true, the selected vertical slice will be displayed on the image. Note, this property is ignored unless the #enableVertSliceSelection property is true.
    Q_PROPERTY(bool displayVertSliceSelection READ getDisplayVertSlice1Selection WRITE setDisplayVertSlice1Selection DESIGNABLE false)

    /// If true, the selected vertical slice 1 will be displayed on the image. Note, this property is ignored unless the #enableVertSlice1Selection property is true.
    ///
    Q_PROPERTY(bool displayVertSlice1Selection READ getDisplayVertSlice1Selection WRITE setDisplayVertSlice1Selection)

    /// If true, the selected vertical slice 2 will be displayed on the image. Note, this property is ignored unless the #enableVertSlice2Selection property is true.
    ///
    Q_PROPERTY(bool displayVertSlice2Selection READ getDisplayVertSlice2Selection WRITE setDisplayVertSlice2Selection)

    /// If true, the selected vertical slice 3 will be displayed on the image. Note, this property is ignored unless the #enableVertSlice3Selection property is true.
    ///
    Q_PROPERTY(bool displayVertSlice3Selection READ getDisplayVertSlice3Selection WRITE setDisplayVertSlice3Selection)

    /// If true, the selected vertical slice 4 will be displayed on the image. Note, this property is ignored unless the #enableVertSlice4Selection property is true.
    ///
    Q_PROPERTY(bool displayVertSlice4Selection READ getDisplayVertSlice4Selection WRITE setDisplayVertSlice4Selection)

    /// If true, the selected vertical slice 5 will be displayed on the image. Note, this property is ignored unless the #enableVertSlice5Selection property is true.
    ///
    Q_PROPERTY(bool displayVertSlice5Selection READ getDisplayVertSlice5Selection WRITE setDisplayVertSlice5Selection)

    //=========

    // Deprecated. If true, the selected horizontal slice will be displayed on the image. Note, this property is ignored unless the #enableHozSliceSelection property is true.
    Q_PROPERTY(bool displayHozSliceSelection READ getDisplayHozSlice1Selection WRITE setDisplayHozSlice1Selection DESIGNABLE false)

    /// If true, the selected horizontal slice will be displayed on the image. Note, this property is ignored unless the #enableHozSlice1Selection property is true.
    ///
    Q_PROPERTY(bool displayHozSlice1Selection READ getDisplayHozSlice1Selection WRITE setDisplayHozSlice1Selection)

    /// If true, the selected horizontal slice will be displayed on the image. Note, this property is ignored unless the #enableHozSlice2Selection property is true.
    ///
    Q_PROPERTY(bool displayHozSlice2Selection READ getDisplayHozSlice2Selection WRITE setDisplayHozSlice2Selection)

    /// If true, the selected horizontal slice will be displayed on the image. Note, this property is ignored unless the #enableHozSlice3Selection property is true.
    ///
    Q_PROPERTY(bool displayHozSlice3Selection READ getDisplayHozSlice3Selection WRITE setDisplayHozSlice3Selection)

    /// If true, the selected horizontal slice will be displayed on the image. Note, this property is ignored unless the #enableHozSlice4Selection property is true.
    ///
    Q_PROPERTY(bool displayHozSlice4Selection READ getDisplayHozSlice4Selection WRITE setDisplayHozSlice4Selection)

    /// If true, the selected horizontal slice will be displayed on the image. Note, this property is ignored unless the #enableHozSlice5Selection property is true.
    ///
    Q_PROPERTY(bool displayHozSlice5Selection READ getDisplayHozSlice5Selection WRITE setDisplayHozSlice5Selection)

    //=========

    /// If true, the selected arbirtary line will be displayed on the image. Note, this property is ignored unless the #enableProfileSelection property is true.
    ///
    Q_PROPERTY(bool displayProfileSelection READ getDisplayProfileSelection WRITE setDisplayProfileSelection)

    /// If true, selected area 1 will be displayed on the image. Note, this property is ignored unless the #enableArea1Selection property is true.
    ///
    Q_PROPERTY(bool displayArea1Selection READ getDisplayArea1Selection WRITE setDisplayArea1Selection)

    /// If true, selected area 2 will be displayed on the image. Note, this property is ignored unless the #enableArea2Selection property is true.
    ///
    Q_PROPERTY(bool displayArea2Selection READ getDisplayArea2Selection WRITE setDisplayArea2Selection)

    /// If true, selected area 3 will be displayed on the image. Note, this property is ignored unless the #enableArea3Selection property is true.
    ///
    Q_PROPERTY(bool displayArea3Selection READ getDisplayArea3Selection WRITE setDisplayArea3Selection)

    /// If true, selected area 4 will be displayed on the image. Note, this property is ignored unless the #enableArea4Selection property is true.
    ///
    Q_PROPERTY(bool displayArea4Selection READ getDisplayArea4Selection WRITE setDisplayArea4Selection)

    /// If true, target selection will be displayed on the image. Note, this property is ignored unless the #enableTargetSelection property is true.
    ///
    Q_PROPERTY(bool displayTargetSelection READ getDisplayTargetSelection WRITE setDisplayTargetSelection)

    /// If true, beam selection will be displayed on the image. Note, this property is ignored unless the #enableBeamSelection property is true.
    ///
    Q_PROPERTY(bool displayBeamSelection READ getDisplayBeamSelection WRITE setDisplayBeamSelection)

    /// If true, the ellipse markup will be displayed on the image.
    ///
    Q_PROPERTY(bool displayEllipse READ getDisplayEllipse WRITE setDisplayEllipse)

    Q_ENUMS(EllipseVariableDefinitions)
    /// \enum EllipseVariableDefinitions
    /// User friendly enumerations for #ellipseVariableDefinition property - refer to #ellipseVariableDefinition property for details.
    enum EllipseVariableDefinitions { BoundingRectangle = BOUNDING_RECTANGLE,       ///< Refer to BOUNDING_RECTANGLE for details
                                      CenterAndSize     = CENTRE_AND_SIZE           ///< Refer to CENTRE_AND_SIZE for details
                                    };

    /// Definition of how ellipse variables are to be used.
    Q_PROPERTY(EllipseVariableDefinitions ellipseVariableDefinition READ getEllipseVariableDefinitionProperty WRITE setEllipseVariableDefinitionProperty)
            EllipseVariableDefinitions getEllipseVariableDefinitionProperty() { return (EllipseVariableDefinitions)getEllipseVariableDefinition(); }            ///< Access function for #EllipseVariableDefinition property - refer to #EllipseVariableDefinition property for details
            void setEllipseVariableDefinitionProperty( EllipseVariableDefinitions variableUsage ) { setEllipseVariableDefinition( (ellipseVariableDefinitions)variableUsage ); }///< Access function for #EllipseVariableDefinitions property - refer to #EllipseVariableDefinitions property for details


    Q_ENUMS(TargetOptions)
    /// \enum TargetOptions
    /// User friendly enumerations for #targetOptions property - refer to #targetOptions property for details.
            enum TargetOptions { DottedFullCrosshair = VideoWidget::CROSSHAIR1,       ///< Refer to CROSSHAIR1 for details
                                 SolidSmallCrosshair = VideoWidget::CROSSHAIR2        ///< Refer to CROSSHAIR2 for details
                                    };

    /// Definition of target markup options.
    Q_PROPERTY(TargetOptions targetOption READ getTargetOptionProperty WRITE setTargetOptionProperty)
            TargetOptions getTargetOptionProperty() { return (TargetOptions)videoWidget->getTargetOption(); }            ///< Access function for #targetOption property - refer to #targetOption property for details
            void setTargetOptionProperty( TargetOptions option ) { videoWidget->setTargetOption( (VideoWidget::beamAndTargetOptions)option ); }///< Access function for #targetOption property - refer to #targetOption property for details

    /// Definition of beam markup options.
    Q_PROPERTY(TargetOptions beamOption READ getBeamOptionProperty WRITE setBeamOptionProperty)
            TargetOptions getBeamOptionProperty() { return (TargetOptions)videoWidget->getBeamOption(); }            ///< Access function for #beamOption property - refer to #beamOption property for details
            void setBeamOptionProperty( TargetOptions option ) { videoWidget->setBeamOption( (VideoWidget::beamAndTargetOptions)option ); }///< Access function for #beamOption property - refer to #beamOption property for details

            //=========

    /// If true, an area will be presented under the image with textual information about the pixel under
    /// the cursor, and for other selections such as selected areas.
    Q_PROPERTY(bool displayCursorPixelInfo READ getDisplayCursorPixelInfo WRITE setDisplayCursorPixelInfo)

    /// If true, the image will undergo contrast reversal.
    ///
    Q_PROPERTY(bool contrastReversal READ getContrastReversal WRITE setContrastReversal)

    /// If true, the image will be displayed using a logarithmic brightness scale.
    ///
    Q_PROPERTY(bool logBrightness READ getLog WRITE setLog)

    /// If true, a button bar will be displayed above the image.
    /// If not displayed, all buttons in the button bar are still available in the right click menu.
    Q_PROPERTY(bool displayButtonBar READ getDisplayButtonBar WRITE setDisplayButtonBar)

    /// If true, the image timestamp will be written in the top left of the image.
    ///
    Q_PROPERTY(bool showTime READ getShowTime WRITE setShowTime)

    /// If true, the apply false colour to the image.
    ///
    Q_PROPERTY(bool useFalseColour READ getUseFalseColour WRITE setUseFalseColour)

    // Deprecated. Used to select the color of the vertical slice markup.
    Q_PROPERTY(QColor vertSliceColor READ getVertSlice1MarkupColor WRITE setVertSlice1MarkupColor DESIGNABLE false)

    /// Used to select the color of the vertical slice 1 markup.
    ///
    Q_PROPERTY(QColor vertSlice1Color READ getVertSlice1MarkupColor WRITE setVertSlice1MarkupColor)

    /// Used to select the color of the vertical slice 2 markup.
    ///
    Q_PROPERTY(QColor vertSlice2Color READ getVertSlice2MarkupColor WRITE setVertSlice2MarkupColor)

    /// Used to select the color of the vertical slice 3 markup.
    ///
    Q_PROPERTY(QColor vertSlice3Color READ getVertSlice3MarkupColor WRITE setVertSlice3MarkupColor)

    /// Used to select the color of the vertical slice 4 markup.
    ///
    Q_PROPERTY(QColor vertSlice4Color READ getVertSlice4MarkupColor WRITE setVertSlice4MarkupColor)

    /// Used to select the color of the vertical slice 5 markup.
    ///
    Q_PROPERTY(QColor vertSlice5Color READ getVertSlice5MarkupColor WRITE setVertSlice5MarkupColor)

    // Deprecated. Used to select the color of the horizontal slice markup.
    Q_PROPERTY(QColor hozSliceColor READ getHozSlice1MarkupColor WRITE setHozSlice1MarkupColor)

    /// Used to select the color of the horizontal slice 1 markup.
    ///
    Q_PROPERTY(QColor hozSlice1Color READ getHozSlice1MarkupColor WRITE setHozSlice1MarkupColor)

    /// Used to select the color of the horizontal slice 2 markup.
    ///
    Q_PROPERTY(QColor hozSlice2Color READ getHozSlice2MarkupColor WRITE setHozSlice2MarkupColor)

    /// Used to select the color of the horizontal slice 3 markup.
    ///
    Q_PROPERTY(QColor hozSlice3Color READ getHozSlice3MarkupColor WRITE setHozSlice3MarkupColor)

    /// Used to select the color of the horizontal slice 4 markup.
    ///
    Q_PROPERTY(QColor hozSlice4Color READ getHozSlice4MarkupColor WRITE setHozSlice4MarkupColor)

    /// Used to select the color of the horizontal slice 5 markup.
    ///
    Q_PROPERTY(QColor hozSlice5Color READ getHozSlice5MarkupColor WRITE setHozSlice5MarkupColor)

    /// Used to select the color of the arbitrarty profile line markup.
    ///
    Q_PROPERTY(QColor profileColor READ getProfileMarkupColor WRITE setProfileMarkupColor)

    /// Used to select the color of the area selection markups.
    ///
    Q_PROPERTY(QColor areaColor READ getAreaMarkupColor WRITE setAreaMarkupColor)

    /// Used to select the color of the beam marker.
    ///
    Q_PROPERTY(QColor beamColor READ getBeamMarkupColor WRITE setBeamMarkupColor)

    /// Used to select the color of the target marker.
    ///
    Q_PROPERTY(QColor targetColor READ getTargetMarkupColor WRITE setTargetMarkupColor)

    /// Used to select the color of the timestamp.
    ///
    Q_PROPERTY(QColor timeColor READ getTimeMarkupColor WRITE setTimeMarkupColor)

    /// Used to select the color of the ellipse marker.
    ///
    Q_PROPERTY(QColor ellipseColor READ getEllipseMarkupColor WRITE setEllipseMarkupColor)


    Q_ENUMS(ResizeOptions)
    /// Resize option. Zoom to zoom to the percentage given by the #zoom property, or fit to the window size.
    ///
    Q_PROPERTY(ResizeOptions resizeOption READ getResizeOptionProperty WRITE setResizeOptionProperty)
    /// \enum ResizeOptions
    /// User friendly enumerations for #resizeOption property
    enum ResizeOptions { Zoom   = QEImage::RESIZE_OPTION_ZOOM,  ///< Zoom to selected percentage
                         Fit    = QEImage::RESIZE_OPTION_FIT    ///< Zoom to fit the current window size
                              };
    void setResizeOptionProperty( ResizeOptions resizeOption ){ setResizeOption( (QEImage::resizeOptions)resizeOption ); }  ///< Access function for #resizeOption property - refer to #resizeOption property for details
    ResizeOptions getResizeOptionProperty(){ return (ResizeOptions)getResizeOption(); }                                     ///< Access function for #resizeOption property - refer to #resizeOption property for details

    /// Zoom percentage. Used when #resizeOption is #Zoom
    Q_PROPERTY(int zoom READ getZoom WRITE setZoom)

    /// Stretch X factor. Used when generating canvas  in which fully processed image is presented.
    Q_PROPERTY(double XStretch READ getXStretch WRITE setXStretch)

    /// Stretch Y factor. Used when generating canvas in which fully processed image is presented.
    Q_PROPERTY(double YStretch READ getYStretch WRITE setYStretch)

    Q_ENUMS(RotationOptions)

    /// Image rotation option.
    ///
    Q_PROPERTY(RotationOptions rotation READ getRotationProperty WRITE setRotationProperty)
    /// \enum RotationOptions
    /// User friendly enumerations for #rotation property
    enum RotationOptions { NoRotation    = imageProperties::ROTATION_0,         ///< No image rotation
                           Rotate90Right = imageProperties::ROTATION_90_RIGHT,  ///< Rotate image 90 degrees clockwise
                           Rotate90Left  = imageProperties::ROTATION_90_LEFT,   ///< Rotate image 90 degrees anticlockwise
                           Rotate180     = imageProperties::ROTATION_180        ///< Rotate image 180 degrees
                          };
    void setRotationProperty( RotationOptions rotation ){ setRotation( (imageProperties::rotationOptions)rotation ); }          ///< Access function for #rotation property - refer to #rotation property for details
    RotationOptions getRotationProperty(){ return (RotationOptions)getRotation(); }                                     ///< Access function for #rotation property - refer to #rotation property for details

    /// If true, flip image vertically.
    ///
    Q_PROPERTY(bool verticalFlip READ getVerticalFlip WRITE setVerticalFlip)

    /// If true, flip image horizontally.
    ///
    Q_PROPERTY(bool horizontalFlip READ getHorizontalFlip WRITE setHorizontalFlip)

    /// Sets the initial position of the horizontal scroll bar, if present.
    /// Used to set up an initial view when zoomed in.
    Q_PROPERTY(int initialHosScrollPos READ getInitialHozScrollPos WRITE setInitialHozScrollPos)

    /// Sets the initial position of the vertical scroll bar, if present.
    /// Used to set up an initial view when zoomed in.
    Q_PROPERTY(int initialVertScrollPos READ getInitialVertScrollPos WRITE setInitialVertScrollPos)

    /// If true, the local Image Display Properties controls are displayed.
    Q_PROPERTY(bool enableImageDisplayProperties READ getEnableImageDisplayProperties WRITE setEnableImageDisplayProperties)

    /// If true, the recording controls are displayed.
    Q_PROPERTY(bool enableRecording READ getEnableRecording WRITE setEnableRecording)

    /// If true, auto set local brightness and contrast when any area is selected.
    /// The brightness and contrast is set to use the full range of pixels in the selected area.
    Q_PROPERTY(bool autoBrightnessContrast READ getAutoBrightnessContrast WRITE setAutoBrightnessContrast)

    /// Name of widget for display and identification purpose.
    /// If present is added to the start of dock names provided by a QEImage widget to an application (such as QEGui)
    /// to diferentiate between docks provided by different instances of QEImage.
    /// Note, this name will be a prefix before the title of any external controls in the form '<name> - <title>'.
    /// For example, if this property is set to 'Left' the Image Display Properties control can be identified in
    /// customisation XML like so:  <Title>Left - Image Display Properties</Title>
    Q_PROPERTY(QString name READ getName WRITE setName)
    // Note, the 'name' property above must be before the 'externalControls' property below so it is available when processing #externalControls property.
    // This may not be the best way to code this - perhaps there is a signal on completion of reading all properties where the QEImage widget
    // could then processes #externalControls property regardless of the property order.

    /// If true, image controls and views such as brightness controls and profile plots are hosted by the application as dock windows, toolbars, etc.
    /// Refer to the #ContainerProfile class and the #windowCustomisation class to see how this class asks an application to act as a host.
    Q_PROPERTY(bool externalControls READ getExternalControls WRITE setExternalControls)

    /// If true, a full context menu allowing manipulation of the image is available. If false, a simpler context menu containing common context menu options such as 'copy' is presented.
    ///
    Q_PROPERTY(bool fullContextMenu READ getFullContextMenu WRITE setFullContextMenu)

    /// If true, the information area willl be brief (one row)
    ///
    Q_PROPERTY(bool briefInfoArea READ getBriefInfoArea WRITE setBriefInfoArea)

    /// If true, all markups for which there is data available will be displayed.
    /// If false, markups will only be displayed when a user interacts with the image.
    /// For example, if true and target variables are defined a target position markup will be displayed as soon as target position data is read.
    /// If false, the target position markup will only be displayed when in target selection mode and the user selects a point in the image.
    Q_PROPERTY(bool displayMarkups READ getDisplayMarkups WRITE setDisplayMarkups)

    /// If true, show the widget in full screen
    ///
    Q_PROPERTY(bool fullScreen READ getFullScreen WRITE setFullScreen)

    //=========
    // This group of properties should be kept consistant QE Buttons

    /// Program to run when a request is made to pass on the current image to the first external application.
    /// No attempt to run a program is made if this property is empty.
    /// Example: paint.exe
    Q_PROPERTY(QString program1 READ getProgram1 WRITE setProgram1)

    /// Arguments for program specified in the 'program1' property.
    ///
    Q_PROPERTY(QStringList arguments1 READ getArguments1 WRITE setArguments1)

    /// Startup options for the program specified in the 'program1' property.
    /// Just run the command, run the command within a terminal, or display the output in QE message system.
    ///
    Q_PROPERTY(ProgramStartupOptionNames programStartupOption1 READ getProgramStartupOptionProperty1 WRITE setProgramStartupOptionProperty1)

    /// Program to run when a request is made to pass on the current image to the second external application.
    /// No attempt to run a program is made if this property is empty.
    /// Example: paint.exe
    Q_PROPERTY(QString program2 READ getProgram2 WRITE setProgram2)

    /// Arguments for program specified in the 'program2' property.
    ///
    Q_PROPERTY(QStringList arguments2 READ getArguments2 WRITE setArguments2)

    /// Startup options for the program specified in the 'program2' property.
    /// Just run the command, run the command within a terminal, or display the output in QE message system.
    ///
    Q_PROPERTY(ProgramStartupOptionNames programStartupOption2 READ getProgramStartupOptionProperty2 WRITE setProgramStartupOptionProperty2)

    // Program startup options
    Q_ENUMS(ProgramStartupOptionNames)

    /// Startup options. Just run the command, run the command within a terminal, or display the output in QE message system.
    ///
    enum ProgramStartupOptionNames{
        None      = applicationLauncher::PSO_NONE,       ///< Just run the program
        Terminal  = applicationLauncher::PSO_TERMINAL,   ///< Run the program in a termainal (in Windows a command interpreter will also be started, so the program may be a built-in command like 'dir')
        LogOutput = applicationLauncher::PSO_LOGOUTPUT,  ///< Run the program, and log the output in the QE message system
        StdOutput = applicationLauncher::PSO_STDOUTPUT   ///< Run the program, and send doutput to standard output and standard error
    };

    void setProgramStartupOptionProperty1( ProgramStartupOptionNames programStartupOption ){ setProgramStartupOption1( (applicationLauncher::programStartupOptions)programStartupOption ); }  ///< Access function for #ProgramStartupOptionNames1 property - refer to #ProgramStartupOptionNames1 property for details
    ProgramStartupOptionNames getProgramStartupOptionProperty1(){ return (ProgramStartupOptionNames)getProgramStartupOption1(); }                                                             ///< Access function for #ProgramStartupOptionNames1 property - refer to #ProgramStartupOptionNames1 property for details
    void setProgramStartupOptionProperty2( ProgramStartupOptionNames programStartupOption ){ setProgramStartupOption2( (applicationLauncher::programStartupOptions)programStartupOption ); }  ///< Access function for #ProgramStartupOptionNames2 property - refer to #ProgramStartupOptionNames2 property for details
    ProgramStartupOptionNames getProgramStartupOptionProperty2(){ return (ProgramStartupOptionNames)getProgramStartupOption2(); }                                                             ///< Access function for #ProgramStartupOptionNames2 property - refer to #ProgramStartupOptionNames2 property for details

    //=========

    /// MPEG stream URL. If this is specified, this will be used as the source of the image in preference to variables
    /// (variables defining the image data, width, and height will be ignored)
    Q_PROPERTY(QString URL READ getSubstitutedUrl WRITE setSubstitutedUrl)
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEImage::UserLevels)
Q_DECLARE_METATYPE (QEImage::DisplayAlarmStateOptions)
Q_DECLARE_METATYPE (QEImage::FormatOptions)
Q_DECLARE_METATYPE (QEImage::EllipseVariableDefinitions)
Q_DECLARE_METATYPE (QEImage::TargetOptions)
Q_DECLARE_METATYPE (QEImage::ResizeOptions)
Q_DECLARE_METATYPE (QEImage::RotationOptions)
Q_DECLARE_METATYPE (QEImage::ProgramStartupOptionNames)
#endif

#endif // QE_IMAGE_H
