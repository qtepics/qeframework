/*  brightnessContrast.cpp
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
  This class manages the QEImage image display properties controls such as brightness and contrast controls.
 */

#include "brightnessContrast.h"
#include <QPainter>
#include <math.h>

#define SCALE_HEIGHT 20

// Gradient scale is the tangent of the gradient.
//
// Gradient range is from almost horizontal (0) to almost
// vertical (pi/2) with normal at pi/4.
// Horizontal gradient can never be reached. Theoretical limit is where all
// pixel values translate to one value. Practical limit is imposed
// where all pixel values translate to 10% of displayed range.
// Vertical gradient can never be reached (min and max pixels are always kept at
// least one apart) so depending on the bit depth, maximum is near pi/2.
//
// User gradient range is from 0 to 1000.
// Angular range is from atan(1/10) = 0.099668652 rad to pi/2 rad.
// Angular scale is pi/2-atan(1/10) = 1.471127674
// Gradient scale factor = user gradient range / angular scale
//                       = 1000/(pi/2-atan(1/10))
//                       = 679.7506549
//
#define GRADIENT_USER_SCALE_FACTOR 679.7506549
#define GRADIENT_BASE 0.099668652

//------------------------------------------------------------------------------
// Construction.
imageDisplayProperties::imageDisplayProperties()
{
   statisticsSet = false;

   nonInteractive = false;

   inBrightnessSliderCallback = false;
   inGradientSliderCallback = false;
   inZeroValueSliderCallback = false;
   inFullValueSliderCallback = false;

   inBrightnessEditCallback = false;
   inGradientEditCallback = false;
   inZeroValueEditCallback = false;
   inFullValueEditCallback = false;

   zeroValue = 0;
   fullValue = 255;
   range = 255;

   // Note the full value is only a default. It will be set when the first set of statistics arrive to the real full range,
   defaultFullValue = true;

   range = 255;

   // Initialise image stats
   for( int i = 0; i < HISTOGRAM_BINS; i++ )
   {
      bins[i] = 0;
   }
   maxP = 0;
   minP = UINT_MAX;

   setFrameStyle( QFrame::StyledPanel|QFrame::Raised );

   QGridLayout* imageDisplayPropertiesMainLayout = new QGridLayout();
   imageDisplayPropertiesMainLayout->setSpacing( 10 );
   setLayout( imageDisplayPropertiesMainLayout );

   // layouts
   QHBoxLayout* imageDisplayPropertiesSub1Layout = new QHBoxLayout();
   QGridLayout* imageDisplayPropertiesSub2Layout = new QGridLayout();
   QHBoxLayout* imageDisplayPropertiesSub3Layout = new QHBoxLayout();
   QHBoxLayout* imageDisplayPropertiesSub4Layout = new QHBoxLayout();
   QHBoxLayout* imageDisplayPropertiesSub5Layout = new QHBoxLayout();

   brightnessLabel = new QLabel( "Brightness:", this );
   gradientLabel = new QLabel( "Gradient:\n(Contrast)", this );
   QLabel* minLabel = new QLabel( "Minimum:", this );
   QLabel* maxLabel = new QLabel( "Maximum:", this );

   advancedButton = new QPushButton( "+", this );
   advancedButton->setToolTip( "Switch between minimal and full image display properties");
   advancedButton->setMaximumWidth(20);
   advancedButton->setCheckable(true);
   advancedButton->setContentsMargins( 0, 0, 0, 0);

   autoBrightnessCheckBox = new QCheckBox( "Auto", this );
   autoBrightnessCheckBox->setToolTip( "Set brightness and contrast to use the full dynamic range of an area when an area is selected");

   QPushButton* autoImageButton = new QPushButton( "Auto all", this );
   autoImageButton->setToolTip( "Set brightness and contrast to use the full dynamic range for the entire image");
   QObject::connect( autoImageButton, SIGNAL( clicked ( bool ) ), this,  SLOT  ( brightnessContrastAutoImageClicked()) );

   QPushButton* resetButton = new QPushButton( "Reset", this );
   resetButton->setToolTip( "Reset brightness and contrast");
   QObject::connect( resetButton, SIGNAL( clicked ( bool ) ), this,  SLOT  ( brightnessContrastResetClicked( bool )) );

   brightnessSlider = new QSlider( Qt::Horizontal, this );
   brightnessSlider->setToolTip( "Set brightness.");
   brightnessSlider->setMinimum( 0 );
   brightnessSlider->setMaximum( 100 );
   brightnessSlider->setMinimumWidth(200);
   QObject::connect( brightnessSlider, SIGNAL( valueChanged ( int ) ), this,  SLOT  ( brightnessSliderValueChanged( int )) );

   gradientSlider = new QSlider( Qt::Horizontal, this );
   gradientSlider->setToolTip( "Set contrast (gradient).");
   gradientSlider->setMinimum( 0 );
   gradientSlider->setMaximum( 1000 );
   QObject::connect( gradientSlider, SIGNAL( valueChanged ( int ) ), this,  SLOT  ( gradientSliderValueChanged( int )) );

   zeroValueSlider = new QSlider( Qt::Horizontal, this );
   zeroValueSlider->setToolTip( "Pixel value at low end of brightness / colour scale");
   zeroValueSlider->setMinimum( 0 );
   zeroValueSlider->setMaximum( 1000 );
   zeroValueSlider->setValue( toExponentialHeadSlider( 0 ) );
   QObject::connect( zeroValueSlider, SIGNAL( valueChanged ( int ) ), this,  SLOT  ( minSliderValueChanged( int )) );

   fullValueSlider = new QSlider( Qt::Horizontal, this );
   zeroValueSlider->setToolTip( "Pixel value at high end of brightness / colour scale");
   fullValueSlider->setMinimum( 0 );
   fullValueSlider->setMaximum( 1000 );
   fullValueSlider->setValue( toExponentialTailSlider( 255 ) );
   QObject::connect( fullValueSlider, SIGNAL( valueChanged ( int ) ), this,  SLOT  ( maxSliderValueChanged( int )) );

   hist = new histogram( this, this );

   histScroll = new histogramScroll( this, this );
   histScroll->setMinimumWidth( 256 );
   histScroll->setMinimumHeight(200 );
   histScroll->setWidget( hist );

   histZoom = new QSlider( Qt::Vertical, this );
   histZoom->setMinimum( 100 );
   histZoom->setMaximum( 1000 );
   histZoom->setValue( 100 );
   histZoom->setToolTip( "Zoom histogram");
   QObject::connect( histZoom, SIGNAL( valueChanged ( int ) ), this,  SLOT  ( histZoomSliderValueChanged( int )) );

   histXLabel = new QLabel( hist );
   histXLabel->setAlignment( Qt::AlignRight );

   brightnessSpinBox = new QSpinBox( this );
   brightnessSpinBox->setToolTip( "Brightness percentage (0 to 100)");
   brightnessSpinBox->setMinimum( 0 );
   brightnessSpinBox->setMaximum( 100 );
   brightnessSpinBox->setValue( brightnessSlider->value() );
   QObject::connect( brightnessSpinBox, SIGNAL( valueChanged ( int ) ), this,  SLOT  ( brightnessSpinBoxChanged( int )) );

   brightnessSpinBox->setMinimumWidth( 60 ); // Set width for all

   gradientSpinBox = new QSpinBox( this );
   gradientSpinBox->setToolTip( "Gradient (0 to 1000)");
   gradientSpinBox->setMinimum( 0 );
   gradientSpinBox->setMaximum( 1000 );
   gradientSpinBox->setValue( gradientSlider->value() );
   QObject::connect( gradientSpinBox, SIGNAL( valueChanged ( int ) ), this,  SLOT  ( gradientSpinBoxChanged( int )) );

   zeroValueSpinBox = new QSpinBox( this );
   zeroValueSpinBox->setToolTip( "Pixel value at low end of brightness / colour scale");
   zeroValueSpinBox->setMinimum( -10000 );
   zeroValueSpinBox->setMaximum( 254 );
   zeroValueSpinBox->setValue( fromExponentialHeadSlider( zeroValueSlider->value() ) );
   QObject::connect( zeroValueSpinBox, SIGNAL( valueChanged ( int ) ), this,  SLOT  ( minSpinBoxChanged( int )) );

   fullValueSpinBox = new QSpinBox( this );
   fullValueSpinBox->setToolTip( "Pixel value at high end of brightness / colour scale");
   fullValueSpinBox->setMinimum( 1 );
   fullValueSpinBox->setMaximum( 10000 );
   fullValueSpinBox->setValue( fromExponentialTailSlider( fullValueSlider->value() ) );

   QObject::connect( fullValueSpinBox, SIGNAL( valueChanged ( int ) ), this,  SLOT  ( maxSpinBoxChanged( int )) );

   contrastReversalCheckBox = new QCheckBox( "Contrast Reversal", this );
   contrastReversalCheckBox->setToolTip( "Reverse light for dark");
   QObject::connect( contrastReversalCheckBox, SIGNAL( toggled( bool ) ), this,  SLOT  ( contrastReversalToggled( bool )) );

   logCheckBox = new QCheckBox( "Log scale", this );
   logCheckBox->setToolTip( "Logarithmic brightness scale");
   QObject::connect( logCheckBox, SIGNAL( toggled( bool ) ), this,  SLOT  ( logToggled( bool )) );

   falseColourCheckBox = new QCheckBox( "False Colour", this );
   falseColourCheckBox->setToolTip( "Interpret intensitiy scale as a range of colours");
   QObject::connect( falseColourCheckBox, SIGNAL( toggled( bool ) ), this,  SLOT  ( falseColourToggled( bool )) );

   imageDisplayPropertiesSub1Layout->addWidget( autoBrightnessCheckBox, 0, Qt::AlignLeft );
   imageDisplayPropertiesSub1Layout->addWidget( autoImageButton,        0, Qt::AlignLeft );
   imageDisplayPropertiesSub1Layout->addWidget( resetButton,            1, Qt::AlignLeft );

   imageDisplayPropertiesSub2Layout->addWidget( brightnessLabel,   0, 0 );
   imageDisplayPropertiesSub2Layout->addWidget( brightnessSlider,  0, 1 );
   imageDisplayPropertiesSub2Layout->addWidget( brightnessSpinBox, 0, 2 );

   imageDisplayPropertiesSub2Layout->addWidget( gradientLabel,     1, 0 );
   imageDisplayPropertiesSub2Layout->addWidget( gradientSlider,    1, 1 );
   imageDisplayPropertiesSub2Layout->addWidget( gradientSpinBox,   1, 2 );

   imageDisplayPropertiesSub4Layout->setContentsMargins( 0, 0, 0, 0 );
   imageDisplayPropertiesSub4Layout->addWidget( zeroValueSlider,     100 );
   imageDisplayPropertiesSub4Layout->addWidget( new QWidget( this ), 20 );

   imageDisplayPropertiesSub2Layout->addWidget( minLabel,          2, 0 );
   imageDisplayPropertiesSub2Layout->addLayout( imageDisplayPropertiesSub4Layout,   2, 1 );
   imageDisplayPropertiesSub2Layout->addWidget( zeroValueSpinBox,  2, 2 );

   imageDisplayPropertiesSub5Layout->setContentsMargins( 0, 0, 0, 0 );
   imageDisplayPropertiesSub5Layout->addWidget( new QWidget( this ), 20 );
   imageDisplayPropertiesSub5Layout->addWidget( fullValueSlider,     100 );

   imageDisplayPropertiesSub2Layout->addWidget( maxLabel,          3, 0 );
   imageDisplayPropertiesSub2Layout->addLayout( imageDisplayPropertiesSub5Layout, 3, 1 );
   imageDisplayPropertiesSub2Layout->addWidget( fullValueSpinBox,  3, 2 );

   imageDisplayPropertiesSub2Layout->setColumnStretch( 1, 1 );  // Sliders to take all spare room

   imageDisplayPropertiesSub3Layout->addWidget( contrastReversalCheckBox, 0, Qt::AlignLeft );
   imageDisplayPropertiesSub3Layout->addWidget( falseColourCheckBox,      0, Qt::AlignLeft );
   imageDisplayPropertiesSub3Layout->addWidget( logCheckBox,              1, Qt::AlignLeft );

   imageDisplayPropertiesMainLayout->addLayout( imageDisplayPropertiesSub1Layout, 0, 0 );
   imageDisplayPropertiesMainLayout->addLayout( imageDisplayPropertiesSub2Layout, 1, 0 );
   imageDisplayPropertiesMainLayout->addLayout( imageDisplayPropertiesSub3Layout, 2, 0 );

   imageDisplayPropertiesMainLayout->addWidget( histZoom,       0, 1, 3, 1 );
   imageDisplayPropertiesMainLayout->addWidget( histScroll,     0, 2, 3, 1 );
   imageDisplayPropertiesMainLayout->addWidget( advancedButton, 0, 3, 3, 1, Qt::AlignBottom );
   imageDisplayPropertiesMainLayout->setColumnStretch( 1, 2 );  // Histogram to take all spare room

   // Update brightness and contrast to match zero and full values
   updateBrightnessInterface();
   updateGradientInterface();

   // Apply the layouts
   adjustSize();

   // Prepare to use signals from the advanced / minimal toggle button
   QObject::connect( advancedButton, SIGNAL( clicked ( bool ) ), this,  SLOT  ( advancedToggled( bool )) );
   advancedToggled(false);
}

//------------------------------------------------------------------------------
// Destruction
imageDisplayProperties::~imageDisplayProperties()
{
}

//------------------------------------------------------------------------------
// Return the 'black' pixel value.
// All pixel values will be translated to be black below this value and increasing in brightness above.
int imageDisplayProperties::getLowPixel()
{
   return zeroValue;
}

//------------------------------------------------------------------------------
// Return the 'white' pixel value.
// All pixel values will be translated to be white above this value and decreasing in brightness below.
int imageDisplayProperties::getHighPixel()
{
   return fullValue;
}

//------------------------------------------------------------------------------
// Return true if the 'auto brightness and contrast' check box is checked
bool imageDisplayProperties::getAutoBrightnessContrast()
{
   return autoBrightnessCheckBox->isChecked();
}

//------------------------------------------------------------------------------
// Return true if the 'contrast reversal' check box is checked
bool imageDisplayProperties::getContrastReversal()
{
   return contrastReversalCheckBox->isChecked();
}

//------------------------------------------------------------------------------
// Return true if the 'log' check box is checked
bool imageDisplayProperties::getLog()
{
   return logCheckBox->isChecked();
}

//------------------------------------------------------------------------------
// Return true if the 'false colour' check box is checked
bool imageDisplayProperties::getFalseColour()
{
   return falseColourCheckBox->isChecked();
}

//------------------------------------------------------------------------------
// Reset the brightness and contrast to normal
void imageDisplayProperties::resetBrightnessContrast()
{
   zeroValue = 0;
   fullValue = range;

   updateBrightnessInterface();
   updateGradientInterface();
   updateZeroValueInterface();
   updateFullValueInterface();

   emit imageDisplayPropertiesChange();
}

//------------------------------------------------------------------------------
// Invoke reset the brightness and contrast to normal
void imageDisplayProperties::brightnessContrastResetClicked( bool )
{
   this->resetBrightnessContrast();
}

//------------------------------------------------------------------------------
// Auto brightness and contrast check box has been checked or unchecked
void imageDisplayProperties::brightnessContrastAutoImageClicked()
{
   emit brightnessContrastAutoImage();
}

//------------------------------------------------------------------------------
// Contrast reversal check box has been checked or unchecked
void imageDisplayProperties::contrastReversalToggled( bool )
{
   emit imageDisplayPropertiesChange();
}

//------------------------------------------------------------------------------
// Log brightness check box has been checked or unchecked
void imageDisplayProperties::logToggled( bool )
{
   emit imageDisplayPropertiesChange();
}

//------------------------------------------------------------------------------
// False colour check box has been checked or unchecked
void imageDisplayProperties::falseColourToggled( bool )
{
   emit imageDisplayPropertiesChange();
}

//=============================================


// Set brightness and contrast controls based on values for black and white
void imageDisplayProperties::setBrightnessContrast( const unsigned int max, const unsigned int min )
{
   updateZeroValueFullValue( min, max );
   emit imageDisplayPropertiesChange();
}

//------------------------------------------------------------------------------
// Set the state of the 'Auto brightness and contrast' check box
void imageDisplayProperties::setAutoBrightnessContrast( bool autoBrightnessContrast )
{
   autoBrightnessCheckBox->setChecked( autoBrightnessContrast );
}

//------------------------------------------------------------------------------
// Set the state of the 'Contrast reversal' check box
void imageDisplayProperties::setContrastReversal( bool contrastReversal )
{
   contrastReversalCheckBox->setChecked( contrastReversal );
}

//------------------------------------------------------------------------------
// Set the state of the 'Log' check box
void imageDisplayProperties::setLog( bool log )
{
   logCheckBox->setChecked( log );
}

//------------------------------------------------------------------------------
// Set the state of the 'False colour' check box
void imageDisplayProperties::setFalseColour( bool falseColour )
{
   falseColourCheckBox->setChecked( falseColour );
}

//==========================================================

//------------------------------------------------------------------------------
// The local brightness slider has been moved
void imageDisplayProperties::brightnessSliderValueChanged( int localBrightnessIn )
{
   if( nonInteractive )
   {
      return;
   }

   inBrightnessSliderCallback = true;
   updateBrightness( (double)(localBrightnessIn)/100.0 );
   inBrightnessSliderCallback = false;

   emit imageDisplayPropertiesChange();
}

//------------------------------------------------------------------------------
// The local brightness spin box has changed
void imageDisplayProperties::brightnessSpinBoxChanged( int value )
{
   if( nonInteractive )
   {
      return;
   }

   inBrightnessEditCallback = true;
   updateBrightness( value/100.0 );
   inBrightnessEditCallback = false;

}

//------------------------------------------------------------------------------
// The gradient slider has been moved
void imageDisplayProperties::gradientSliderValueChanged( int value )
{
   if( nonInteractive )
   {
      return;
   }

   inGradientSliderCallback = true;
   updateGradient( (double)(value)/GRADIENT_USER_SCALE_FACTOR+GRADIENT_BASE );
   inGradientSliderCallback = false;

   emit imageDisplayPropertiesChange();
}

//------------------------------------------------------------------------------
// The gradient spin box has changed
void imageDisplayProperties::gradientSpinBoxChanged( int value )
{
   if( nonInteractive )
   {
      return;
   }

   inGradientEditCallback = true;
   updateGradient( (double)(value)/GRADIENT_USER_SCALE_FACTOR+GRADIENT_BASE );
   inGradientEditCallback = false;

   emit imageDisplayPropertiesChange();
}

//------------------------------------------------------------------------------
// The minimum slider has been moved
void imageDisplayProperties::minSliderValueChanged( int value )
{
   if( nonInteractive )
   {
      return;
   }

   inZeroValueSliderCallback = true;
   updateZeroValue( fromExponentialHeadSlider( value ) * range / 256.0 );
   inZeroValueSliderCallback = false;

   emit imageDisplayPropertiesChange();
}

//------------------------------------------------------------------------------
// The minimum spin box has changed
void imageDisplayProperties::minSpinBoxChanged( int value )
{
   if( nonInteractive )
   {
      return;
   }

   inZeroValueEditCallback = true;
   updateZeroValue( value );
   inZeroValueEditCallback = false;

   emit imageDisplayPropertiesChange();
}

//------------------------------------------------------------------------------
// The maximum slider has been moved
void imageDisplayProperties::maxSliderValueChanged( int value )
{
   if( nonInteractive )
   {
      return;
   }

   inFullValueSliderCallback = true;
   updateFullValue( fromExponentialTailSlider( value ) * range / 256.0 );
   inFullValueSliderCallback = false;

   emit imageDisplayPropertiesChange();
}

//------------------------------------------------------------------------------
// The maximum spin box has changed
void imageDisplayProperties::maxSpinBoxChanged( int value )
{
   if( nonInteractive )
   {
      return;
   }

   inFullValueEditCallback = true;
   updateFullValue( value );
   inFullValueEditCallback = false;


   emit imageDisplayPropertiesChange();
}

//=========================================================

//------------------------------------------------------------------------------
// Update all other values based on a brightness change.
void imageDisplayProperties::updateBrightness( double val )
{
   // Brightness ranges from 0.0 (0%) to 1.0 (100%)
   // Validate brightness
   if( val < 0.0 )
   {
      val = 0.0;
   }
   else if( val > 1.0 )
   {
      val = 1.0;
   }

   // Update brightness contrast values according to new brightness
   // Note, this never alters the span, so gradient never changes
   double span = fullValue - zeroValue;
   fullValue = (range+span)*val;
   zeroValue = fullValue-span;

   // Update interface
   updateZeroValueInterface();
   updateFullValueInterface();
   updateBrightnessInterface();
   updateGradientInterface();

   hist->update();
}

//------------------------------------------------------------------------------
// Update all other values based on a gradient change.
void imageDisplayProperties::updateGradient( double angularVal )
{
   // Gradient is range / span
   // Maximum gradient is limited to a zeroValue at most one less than fullValue.
   // Minimum gradient is limited to a practical 1/10.
   double val = tan( angularVal );
   if( val < 0.1 )
   {
      val = 0.1;
   } else if( val > range )
   {
      val = range;
   }

   double mid = (double)(fullValue+zeroValue)/2;
   double span = (double)range/(double)val;

   double low =  mid-(span/2);

   zeroValue = floor( low + 0.5 );        // Note, round() not in windows math.h. Using floor+0.5 instead
   fullValue = floor( low + span + 0.5 ); // Note, round() not in windows math.h. Using floor+0.5 instead

   updateZeroValueInterface();
   updateFullValueInterface();
   updateBrightnessInterface();
   updateGradientInterface();

   hist->update();
}

//------------------------------------------------------------------------------
// Update all other values based on a Zero value change.
void imageDisplayProperties::updateZeroValue( int val )
{
   if( val >= (int)range )
   {
      val = range-1;
   }

   zeroValue = val;
   if( zeroValue >= fullValue )
   {
      fullValue = zeroValue+1;
   }

   updateZeroValueInterface();
   updateFullValueInterface();
   updateBrightnessInterface();
   updateGradientInterface();

   hist->update();
}

//------------------------------------------------------------------------------
// Update all other values based on a Full value change.
void imageDisplayProperties::updateFullValue( unsigned int val )
{
   if( val < 1 )
   {
      val = 1;
   }

   fullValue = val;
   if( fullValue <= zeroValue )
   {
      zeroValue = fullValue-1;
   }

   updateZeroValueInterface();
   updateFullValueInterface();
   updateBrightnessInterface();
   updateGradientInterface();

   hist->update();
}

//------------------------------------------------------------------------------
// Update all other values based on a Zero and Full value change.
void imageDisplayProperties::updateZeroValueFullValue( unsigned int min, unsigned int max )
{
   if( min >= range )
   {
      min = range-1;
   }

   zeroValue = min;

   if( max > range )
   {
      max = range;
   }

   fullValue = max;
   defaultFullValue = false;

   if( zeroValue >= fullValue )
   {
      fullValue = zeroValue+1;
   }

   updateZeroValueInterface();
   updateFullValueInterface();
   updateBrightnessInterface();
   updateGradientInterface();

   hist->update();
}

//=========================================================

//------------------------------------------------------------------------------
// Update the brightness controls to reflect current values
void imageDisplayProperties::updateBrightnessInterface()
{
   // Calculate brightness (derived)
   unsigned int span = fullValue-zeroValue;
   unsigned int brightnessScale = range+span;
   double brightness = (double)fullValue/(double)brightnessScale;

   // Update interface
   nonInteractive = true;

   if( !inBrightnessEditCallback )
   {
      brightnessSpinBox->setValue( (int)(brightness*100) );
   }

   if( !inBrightnessSliderCallback )
   {
      brightnessSlider->setValue( brightness*100 );
   }

   nonInteractive = false;
}

//------------------------------------------------------------------------------
// Update the gradient controls to reflect current values
void imageDisplayProperties::updateGradientInterface()
{
   // Calculate gradient (derived)
   double gradient = (atan((double)range/(double)(fullValue-zeroValue))-GRADIENT_BASE)*GRADIENT_USER_SCALE_FACTOR;

   // Update interface
   nonInteractive = true;

   if( !inGradientEditCallback )
   {
      gradientSpinBox->setValue( gradient );
   }

   if( !inGradientSliderCallback )
   {
      gradientSlider->setValue( gradient );
   }

   nonInteractive = false;
}

//------------------------------------------------------------------------------
// Update the Zero value controls to reflect current values
void imageDisplayProperties::updateZeroValueInterface()
{
   // Update interface
   nonInteractive = true;

   if( !inZeroValueEditCallback )
   {
      zeroValueSpinBox->setValue( zeroValue );
   }

   if( !inZeroValueSliderCallback )
   {
      zeroValueSlider->setValue( toExponentialHeadSlider( (double)zeroValue/(double)range*256.0 ) );
   }

   nonInteractive = false;
}

//------------------------------------------------------------------------------
// Update the Full value controls to reflect current values
void imageDisplayProperties::updateFullValueInterface()
{
   // Update interface
   nonInteractive = true;

   if( !inFullValueEditCallback )
   {
      fullValueSpinBox->setValue( fullValue );
   }

   if( !inFullValueSliderCallback )
   {
      fullValueSlider->setValue( toExponentialTailSlider( (double)fullValue/(double)range*256.0 ) );
   }

   nonInteractive = false;
}

//=========================================================

// Set current image statistics.
// This can be called from the image processing thread
void imageDisplayProperties::setStatistics( unsigned int minPIn,                // Minimum pixel value
                                            unsigned int maxPIn,                // Maximum pixel value
                                            unsigned int bitDepth,              // Bit depth
                                            unsigned int binsIn[HISTOGRAM_BINS],// Histogram bins
                                            rgbPixel pixelLookupIn[256] )       // Color translation lookup
{
   // Update image statistics
   minP = minPIn;
   maxP = maxPIn;
   depth = bitDepth;
   for( int i = 0; i < HISTOGRAM_BINS; i++ )
   {
      bins[i] = binsIn[i];
   }
   pixelLookup = pixelLookupIn;
}

//------------------------------------------------------------------------------
// Show the current image statistics.
// This can not be called from the image processing thread.
// It must be called from the main thread after setStatistics()
// is called from the image processing thread.
void imageDisplayProperties::showStatistics()
{
   // Recalculate dependand variables
   range = ((unsigned long)(1)<<depth)-1;

   // Apply changes
   zeroValueSpinBox->setMinimum( -(int)range*10 );
   zeroValueSpinBox->setMaximum( range-1 );
   fullValueSpinBox->setMinimum( 0 );
   fullValueSpinBox->setMaximum( range*10 );

   if( defaultFullValue )
   {
      defaultFullValue = false;

      fullValue = range;
      updateFullValueInterface();
   }

   histXLabel->setText( QString( "%1" ).arg( range ) );

   hist->update();

   // Flag statistics can now be used (before this things like depth were just default values)
   statisticsSet = true;
}

//=========================================================

// The histogram zoom slider has been moved
void imageDisplayProperties::histZoomSliderValueChanged( int value )
{
   setHistZoom( value );
}

//------------------------------------------------------------------------------
// The histogram zoom slider has been moved
void imageDisplayProperties::setHistZoom( int value )
{
   // Determine the width and height that will just fit without scroll bars
   double fitWidth  = (double)(histScroll->width()  - histScroll->contentsMargins().left()*2 );//  - histScroll->frameWidth()*2);
   double fitHeight = (double)(histScroll->height() - histScroll->contentsMargins().top()*2 );//  - histScroll->frameWidth()*2);

   // Set the new zoomed size
   QRect currentGeom = hist->geometry();
   hist->setGeometry( currentGeom.x(), currentGeom.y(), (double)(value)/100*fitWidth, (double)(value)/100*fitHeight );
}

//=========================================================

// Get the current histogram zoom percentage
int imageDisplayProperties::getHistZoom()
{
   return histZoom->value();
}

//------------------------------------------------------------------------------
// Construct the histogram
histogramScroll::histogramScroll( QWidget* parent, imageDisplayProperties* idpIn )
   : QScrollArea( parent )
{
   idp = idpIn;
}

//------------------------------------------------------------------------------
// Histogram resize event
void histogramScroll::resizeEvent( QResizeEvent* )
{
   idp->setHistZoom( idp->getHistZoom() );
}


//------------------------------------------------------------------------------
// Construct the histogram
histogram::histogram( QWidget* parent, imageDisplayProperties* idpIn )
   : QFrame( parent )
{
   setFrameStyle( QFrame::Panel );
   idp = idpIn;
}

//------------------------------------------------------------------------------
// Histogram resize event
void histogram::resizeEvent( QResizeEvent* )
{
   // Keep the X asis label in the bottom right of the histogram
   idp->histXLabel->setGeometry( width()-idp->histXLabel->width()-2,
                                 height()-idp->histXLabel->height()-10, //SCALE_HEIGHT,
                                 idp->histXLabel->width(),
                                 idp->histXLabel->height());
}

//------------------------------------------------------------------------------
// Histogram repaint event
void histogram::paintEvent(QPaintEvent* )
{
   // Do nothing if no image info yet
   if( idp->bins == NULL )
   {
      return;
   }

   // Determine range (ignore counts in first and last buckets as it is common for huge counts in one or both ends)
   unsigned int binRange = 0;
   for( int i = 1; i < HISTOGRAM_BINS-1; i++ )
   {
      if( idp->bins[i] > binRange )
      {
         binRange = idp->bins[i];
      }
   }

   // Do nothing if no data present
   if( binRange == 0 )
   {
      return;
   }

   // Determine bin stuff
   unsigned int bitsPerBin;
   int minBin;
   int maxBin;

   if( idp->depth<=8)
   {
      bitsPerBin = 1;
      minBin = idp->zeroValue;
      maxBin = idp->fullValue;
   }
   else
   {
      // Determine bins for maximum and minimum values
      bitsPerBin = idp->depth-8;

      minBin = idp->zeroValue/(1<<bitsPerBin); // OK for negative values
      if( minBin > 254 )
      {
         minBin = 254;
      }

      maxBin = idp->fullValue>>bitsPerBin;
      if( maxBin <= minBin )
      {
         maxBin = minBin+1;
      }
   }

   QPainter p( this );

   // Draw the histogram with scale...

   // Determine overall size
   int h = height()-1-SCALE_HEIGHT;
   double w = width();

   // Initialise rectangle used for both histogram and scale
   QRectF barRect;

   barRect.setBottom( h+1 );

   barRect.setLeft( 0 );
   barRect.setWidth( w/HISTOGRAM_BINS );

   // Draw histogram bins
   for( unsigned int i = 0; i < HISTOGRAM_BINS; i++ )
   {
      // Draw histogram bar
      barRect.setTop( h - (double)(idp->bins[i])*h/binRange );
      p.fillRect( barRect, Qt::red );

      // Move on to the next bar
      barRect.moveLeft( barRect.right() );
   }

   // Draw scale bar
   int minX = idp->zeroValue*w/idp->range;
   int maxX = idp->fullValue*w/idp->range;
   imageDisplayProperties::rgbPixel* col;

   int scaleTop = h+3;
   int scaleHeight = SCALE_HEIGHT-4;

   // Draw the first colour in the lookup table for the entire area to the left of the minimum value
   QRect scaleRect = QRect( 0, scaleTop, minX, scaleHeight );
   col = &(idp->pixelLookup[0] );
   p.fillRect( scaleRect, QColor( col->p[2], col->p[1], col->p[0] ) );

   // Draw the last colour in the lookup table for the entire area to the right of the maximum value
   scaleRect = QRect( maxX, scaleTop, w-maxX, scaleHeight );
   col = &(idp->pixelLookup[255] );
   p.fillRect( scaleRect, QColor( col->p[2], col->p[1], col->p[0] ) );

   // Display all colors in the lookup table
   QRectF colourRect;

   colourRect.setTop( scaleTop );
   colourRect.setBottom( scaleTop + scaleHeight );

   colourRect.setLeft( minX );
   colourRect.setWidth( (double)(maxX-minX)/255 );

   for( unsigned int i = 0; i < 256; i++ )
   {
      // Draw the color rectangle
      imageDisplayProperties::rgbPixel* col = &(idp->pixelLookup[i] );
      p.fillRect( colourRect, QColor( col->p[2], col->p[1], col->p[0] ) );

      // Move on to the next colour
      colourRect.moveLeft( colourRect.right() );
   }

   // Prepare to draw the bounds and gradient

   QPen pen( Qt::blue );
   p.setPen( pen );

   // Draw max and min
   pen.setStyle( Qt::DashLine );
   double minScaled = (double)(minBin)*w/HISTOGRAM_BINS;
   double maxScaled = (double)(maxBin)*w/HISTOGRAM_BINS;
   p.drawLine( minScaled,0,minScaled,h);
   p.drawLine( maxScaled,0,maxScaled,h);

   // Draw gradient
   pen.setStyle( Qt::SolidLine );
   p.setPen( pen );

   p.drawLine( minScaled,h,maxScaled,0);
}

//------------------------------------------------------------------------------
// Translate from a composite exponential-linear slider value.
// Translate a slider value converting the slider range to a composite of an
// exponential range for the first 20% (the head) followed by a linear range for
// the remaining 80%.
// This allows the slider to display zeroValue with a wide linear range for
// the full pixel range, and a narrow exponential range for an extended region
// below the pixel range.
// The translated output value is for a pixel range of 256 and should be scaled
// according to the bit depth.
//
//        zeroValue
//           ^
//           |
//        400-
//           |
//           |
//           |                                                 x
//        200-                                         x
//           |                                 x
//           |                         x
//           |                 x
//    -------+---------x---------|---------|---------|---------|----> Slider value
//           0      x 200       400       600       800       1000
//           |     x
//           |
//       -200-    x
//           |
//           |
//           |
//       -400-
//           |
//           |
//           |
//       -600-  x
//           |
//           |
//           |
//       -800-
//           |
//           |
//           |
//      -1000-
//           |
//           |
//           |
//      -1200-
//           |x
//           |
//           |
//      -1400-
//           |
//
double imageDisplayProperties::fromExponentialHeadSlider( int value )
{
   if( value > 200.0 )
   {
      return (256.0/800.0)*((double)(value)-200.0);
   }
   else
   {
      return -(pow( 10, (0.01*(0-(double)(value))+3.145)) - 13.9639 );
   }
}

//------------------------------------------------------------------------------
// Translate from a composite linear-exponential slider value.
// Translate a slider value converting the slider range to a composite of a
// linear range for the first 80% followed by an exponential range for the last
// 20% (the tail)
// This allows the slider to display fullValue with a wide linear range for
// the full pixel range, and a narrow exponential range for an extended region
// above the pixel range.
// The translated output value is for a pixel range of 256 and should be scaled
// according to the bit depth.
//
//        fullValue
//           ^
//           |
//       1400-                                              x
//           |
//           |
//           |
//       1200-
//           |
//           |
//           |
//       1000-
//           |
//           |
//           |
//        800-                                             x
//           |
//           |
//           |
//        600-
//           |
//           |
//           |
//        400-                                           x
//           |
//           |                                          x
//           |                                       x
//        200-                               x
//           |                       x
//           |               x
//           |       x
//    -------x---------|---------|---------|---------|---------|----> Slider value
//           0        200       400       600       800       1000
//           |
//
double imageDisplayProperties::fromExponentialTailSlider( int value )
{
   if( value < 800.0 )
   {
      return (double)(value)*(256.0/800.0);
   }
   else
   {
      return pow( 10, 0.01*(double)(value)-6.83 ) + 241.207;
   }
}

//------------------------------------------------------------------------------
// Translate to a composite exponential-linear slider value.
// Translate a pixel value to a slider value where the slider range is a composite of an
// exponential range for the first 20% (the head) followed by a linear range for
// the remaining 80%.
// This is the inverse function to fromExponentialHeadSlider(). See that function for full details.
int imageDisplayProperties::toExponentialHeadSlider( double value )
{
   if( value > 0.0 )
   {
      // Inverse of: (256.0/800.0)*((double)(value)-200.0);
      return value/(256.0/800.0)+200.0;
   }
   else
   {
      // Inverse of -(pow( 10, (0.01*(0-(double)(value))+3.145)) - 13.9639 );
      return -(log10(13.9639-value)-3.145)/0.01;
   }
}

//------------------------------------------------------------------------------
// Translate to a composite linear-exponential slider value.
// Translate a pixel value to a slider value where the slider range is a composite of a
// linear range for the first 80% followed by an exponential range for the last
// 20% (the tail).
// This is the inverse function to fromExponentialTailSlider(). See that function for full details.
int imageDisplayProperties::toExponentialTailSlider( double value )
{
   if( value < 256.0 )
   {
      // Inverse of: (double)(value)*(256.0/800.0);
      return value * (800.0/256.0);
   }
   else
   {
      // Inverse of: pow( 10, 0.01*(double)(value)-6.83 ) + 241.207;
      return (log10(value-241.207)+6.83)/0.01;
   }
}

//------------------------------------------------------------------------------
// Slot for advanced button toggled
void imageDisplayProperties::advancedToggled( bool toggled )
{
   // If checked, show all controls
   if (toggled)
   {
      // Show all controls
      hideShowAll( this, true );
      advancedButton->show();

      // Ensure the widget will resize correctly
      setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

      // Change the text to reflect the current state
      advancedButton->setText("-");
   }

   // If unchecked, show minimal controls
   else
   {
      // Show minimal controls (brightness and contrast sliders only)
      hideShowAll( this, false );

      brightnessLabel->show();
      gradientLabel->show();
      brightnessSlider->show();
      gradientSlider->show();
      advancedButton->show();

      // Ensure the widget will resize correctly when presented in a dock
      setMaximumSize(360, 100);

      // Change the text to reflect the current state
      advancedButton->setText("+");
   }
}

//------------------------------------------------------------------------------
// Hide or show all the widgets in the dialog.
// Used when switching between minimal or full presentation modes.
// After blindly hiding all widget, the minimal set should then be shown.
//
// This function can be changed to be recursive if the hierarchy of this widget changes
// such that the widgets in minimal mode are not top level children.
void imageDisplayProperties::hideShowAll( QObject* obj, bool show )
{
   // Show or hide all children of the supplied widget
   QObjectList objList = obj->children();
   for( int i = 0; i < objList.size(); i++ )
   {
      QWidget* w;
      QObject* child = objList[i];
      if( child->isWidgetType() )
      {
         w = (QWidget*)child;
         w->setVisible( show );
      }
      // UNCOMMENT the line below if the hierarchy of this widget changes
      // such that the widgets in minimal mode are not top level children.
      //        hideShowAll( child, show );
   }
}

// end
