/*  brightnessContrast.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2023 Australian Synchrotron
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

#ifndef QE_BRIGHTNESS_CONTRAST_H
#define QE_BRIGHTNESS_CONTRAST_H

#include <QFrame>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QSpinBox>
#include <QDebug>
#include <QIntValidator>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>

#define HISTOGRAM_BINS 256
class imageDisplayProperties;

// Histogram Scroll area class (A QSCrollArea we catch resize events on)
class histogramScroll: public QScrollArea {
   Q_OBJECT
public:
   histogramScroll( QWidget* parent, imageDisplayProperties* idp );
private:
   void resizeEvent( QResizeEvent* event );

   imageDisplayProperties* idp;
};

// Histogram class (A QFrame we catch resize and paint events on)
class histogram: public QFrame {
   Q_OBJECT
public:
   histogram( QWidget* parent, imageDisplayProperties* idp );
private:
   void paintEvent( QPaintEvent* event);
   void resizeEvent( QResizeEvent* event );

   imageDisplayProperties* idp;
};

// Main class to present controls
class imageDisplayProperties : public QFrame {
   Q_OBJECT
public:
   explicit imageDisplayProperties();
   ~imageDisplayProperties();

   void setBrightnessContrast( const unsigned int max, const unsigned int min );
   void setAutoBrightnessContrast( bool autoBrightnessContrast );  // Set 'Auto Brightness' function on or off
   void setContrastReversal( bool contrastReversal );              // Set contrast reversal state on or off
   void setLog( bool log );                                        // Set logarithmic scale on or off
   void setFalseColour( bool falseColour );                        // Set false colour on or off
   void resetBrightnessContrast();                                 // Reset the brightness contrast


   bool getAutoBrightnessContrast();   // Return true if 'Auto Brightness' function is on
   bool getContrastReversal();         // Return true if contrast reversal is on
   bool getLog();                      // Return true if logarithmic scale is to be used
   bool getFalseColour();              // Return true if false colour is to be applied

   int getLowPixel();                  // Return the 'black' pixel value.
   int getHighPixel();                 // Return the 'white' pixel value.

   struct rgbPixel                     // Structure used when setting current image statistics
   {
      unsigned char p[4]; // R/G/B/Alpha
   };
   void setStatistics( unsigned int minPIn,    // Set current image statistics (can be called from any thread)
                       unsigned int maxPIn,
                       unsigned int bitDepth,
                       unsigned int binsIn[HISTOGRAM_BINS],
                       rgbPixel pixelLookup[256] );
   void showStatistics();                      // Must be called from main thread

signals:
   void brightnessContrastAutoImage();     // Issue a request to set the brightness and contrast to match the current image
   void imageDisplayPropertiesChange();    // Notify a change in the image display properties

private slots:

   // Slider motion
   void brightnessSliderValueChanged( int value );
   void minSliderValueChanged( int value );
   void maxSliderValueChanged( int value );
   void gradientSliderValueChanged( int value );

   void histZoomSliderValueChanged( int value );

   // Spinbox changes
   void brightnessSpinBoxChanged( int );
   void gradientSpinBoxChanged( int );
   void minSpinBoxChanged( int );
   void maxSpinBoxChanged( int );

   // Buttons
   void brightnessContrastResetClicked( bool state );
   void brightnessContrastAutoImageClicked();
   void contrastReversalToggled( bool );
   void logToggled( bool );
   void falseColourToggled( bool );
   void advancedToggled( bool );

private:
   // Local brightness and contrast controls and monitors
   QCheckBox* autoBrightnessCheckBox;
   QSlider* brightnessSlider;
   QSlider* contrastSlider;
   QSlider* zeroValueSlider;
   QSlider* fullValueSlider;
   QSlider* gradientSlider;
   QSpinBox* brightnessSpinBox;
   QSpinBox* zeroValueSpinBox;
   QSpinBox* fullValueSpinBox;
   QSpinBox* gradientSpinBox;
   QCheckBox* contrastReversalCheckBox;
   QCheckBox* logCheckBox;
   QCheckBox* falseColourCheckBox;

   histogram* hist;
   QScrollArea* histScroll;
   QSlider* histZoom;

   QLabel* brightnessLabel;
   QLabel* gradientLabel;
   QPushButton* advancedButton;    // advanced toggle button

   // Flags to avoid loops when setting controls
   bool nonInteractive;
   bool inBrightnessSliderCallback;
   bool inGradientSliderCallback;
   bool inZeroValueSliderCallback;
   bool inFullValueSliderCallback;

   bool inBrightnessEditCallback;
   bool inGradientEditCallback;
   bool inZeroValueEditCallback;
   bool inFullValueEditCallback;

   // Functions to update zero and full pixel values based on new values for zero or full values,
   // or derived brightness and contrast valuse, and set controls acordingly.
   void updateBrightness( double val );
   void updateGradient( double val );
   void updateZeroValue( int val );
   void updateFullValue( unsigned int val );
   void updateZeroValueFullValue( unsigned int min, unsigned int max );

   // Update controls to match updated values
   void updateBrightnessInterface();
   void updateGradientInterface();
   void updateZeroValueInterface();
   void updateFullValueInterface();

   // Manage exponential sliders
   double fromExponentialHeadSlider( int value ); // Translate from a composite exponential-linear slider value.
   double fromExponentialTailSlider( int value ); // Translate from a composite linear-exponential slider value.
   int toExponentialHeadSlider( double value );   // Translate to a composite exponential-linear slider value.
   int toExponentialTailSlider( double value );   // Translate to a composite linear-exponential slider value.

   // Manage minimal / full presentation
   void hideShowAll( QObject* obj, bool show );   // Hide or show all the widgets in the dialog.

public:
   // Current brightness/contrast settings
   int zeroValue;     // Pixel value displayed black
   int fullValue;     // Pixel value displayed white
   bool defaultFullValue;

   unsigned int range;  // Pixel range derived from bit depth

   // Current image stats
   unsigned int maxP;  // Highest pixel value in image
   unsigned int minP;  // Lowest pixel value in image
   unsigned int depth; // Bit depth
   unsigned int bins[HISTOGRAM_BINS]; // Histogram bins
   bool statisticsSet; // Statistic have been set ( setStatistics() has been called) and things like range are now available

   rgbPixel* pixelLookup; // Pixel lookup table used to present colour scale in histogram

   QLabel* histXLabel;

   void setHistZoom( int value );                      // Set the histogram zoom percentage
   int getHistZoom();                                  // Return the histogram zoom percentage

   bool statisticsValid() { return statisticsSet; }    // Return if statistics have been set
};

#endif // QE_BRIGHTNESS_CONTRAST_H
