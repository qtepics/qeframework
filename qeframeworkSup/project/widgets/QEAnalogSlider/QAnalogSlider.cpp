/*  QAnalogSlider.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2015-2022 Australian Synchrotron
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include "QAnalogSlider.h"
#include <stdio.h>
#include <QDebug>
#include <QPushButton>
#include <QEPlatform.h>
#include <QECommon.h>

#define DEBUG qDebug () << "QAnalogSlider" << __LINE__ << __FUNCTION__ << "  "

// Keep these consistent with Axis Painter.
//
#define MIN_VALUE         (-1.0E+24)
#define MAX_VALUE         (+1.0E+24)
#define MIN_INTERVAL      (+1.0E-6)

#define SAVE_REVERT_MARKER         0


//-----------------------------------------------------------------------------
// Constructor with no initialisation
//
QAnalogSlider::QAnalogSlider (QWidget* parent) : QFrame (parent)
{
   this->commonSetup ();
}

//---------------------------------------------------------------------------------
//
QSize QAnalogSlider::sizeHint () const
{
   return QSize (300, 72);
}

//-----------------------------------------------------------------------------
// Setup common to all constructors
//
void QAnalogSlider::commonSetup ()
{
   const int sliderMargin = 15;

   this->setMinimumSize (120, 72);

   // Set up default properties
   //
   this->mFontColour  = QColor (0,     0,   0);   // black
   this->mValue = 0.0;
   this->mMinimum = 0.0;
   this->mMaximum = 10.0;
   this->mPrecision = 2;
   this->mIsActive = true;
   this->mShowSaveRevert = false;
   this->mShowApply = false;
   this->savedValue = 0.0;


   // Setup internal widgets.
   //
   this->layout = new QVBoxLayout (this);   // VBox
   this->layout->setMargin (2);
   this->layout->setSpacing (0);

   this->sliderLayout = new QHBoxLayout ();
   this->sliderLayout->setContentsMargins (sliderMargin, 0, sliderMargin, 0);  // left, top, right, bottom
   this->sliderLayout->setSpacing (0);

   this->intSlilder = new QSlider (this);
   this->intSlilder->setOrientation (Qt::Horizontal);
   this->intSlilder->setMinimum (0);
   this->intSlilder->setMaximum (1000);   // consistant with defaults (prec = 2 , range 0 to 10)
   this->intSlilder->setPageStep (1);

   this->sliderLayout->addWidget (this->intSlilder);

   this->axisPainter = new QEAxisPainter (this);

   // Reduce to 75% - this is static - do we need to be dynamic?
   //
   QFont apf = this->axisPainter->font();
   apf.setPointSize ((3*apf.pointSize())/4);
   this->axisPainter->setFont (apf);

   this->axisPainter->setOrientation (QEAxisPainter::Left_To_Right);
   this->axisPainter->setMinimum (this->mMinimum);
   this->axisPainter->setMaximum (this->mMaximum);
   this->axisPainter->setMinorInterval (0.2);
   this->axisPainter->setMajorMinorRatio (5);   // => MajorInterval = 1.0
   this->axisPainter->setIndent (sliderMargin + 5);

   this->labelFrame = new QFrame (this);

   this->labelLayout = new QHBoxLayout (this->labelFrame);
   this->labelLayout->setContentsMargins (4, 0, 4, 0);  // left, top, right, bottom
   this->labelLayout->setSpacing (3);

   this->leftImage = new QLabel (this->labelFrame);
   this->leftImage->setFixedHeight (17);
   this->leftImage->setAlignment (Qt::AlignLeft);
   this->leftImage->setIndent (6);

   this->centreImage = new QLabel (this->labelFrame);
   this->centreImage->setFixedHeight (17);
   this->centreImage->setAlignment (Qt::AlignHCenter);
   this->centreImage->setIndent (6);

   this->rightImage = new QLabel (this->labelFrame);
   this->rightImage->setFixedHeight (17);
   this->rightImage->setAlignment (Qt::AlignRight);
   this->rightImage->setIndent (6);

   const int buttonSize = 19;
   this->saveButton = new QPushButton ("S", this->labelFrame);
   this->saveButton->setFixedSize (buttonSize, buttonSize);
   this->saveButton->setStyleSheet (QEUtilities::colourToStyle (QColor (0x00C000)));
   this->saveButton->setFocusPolicy (Qt::NoFocus);
   this->saveButton->setToolTip (" save current value ");

   this->revertButton = new QPushButton ("R", this->labelFrame);
   this->revertButton->setFixedSize (buttonSize, buttonSize);
   this->revertButton->setStyleSheet (QEUtilities::colourToStyle (QColor (0x40A0FF)));
   this->revertButton->setFocusPolicy (Qt::NoFocus);
   this->revertButton->setToolTip (" revert to saved value ");

   this->applyButton = new QPushButton ("A", this->labelFrame);
   this->applyButton->setFixedSize (buttonSize, buttonSize);
   this->applyButton->setStyleSheet (QEUtilities::colourToStyle (QColor (0x80E880)));
   this->applyButton->setFocusPolicy (Qt::NoFocus);
   this->applyButton->setToolTip (" apply value ");

   this->labelLayout->addWidget (this->saveButton);
   this->labelLayout->addWidget (this->revertButton);
   this->labelLayout->addWidget (this->leftImage);
   this->labelLayout->addWidget (this->centreImage);
   this->labelLayout->addWidget (this->rightImage);
   this->labelLayout->addWidget (this->applyButton);

   this->layout->addLayout (this->sliderLayout);
   this->layout->addWidget (this->axisPainter);
   this->layout->addWidget (this->labelFrame);

   this->setShowSaveRevert (false);
   this->setShowApply (false);

   QObject::connect (this->intSlilder, SIGNAL (valueChanged          (int)),
                     this,             SLOT   (sliderPositionChanged (const int)));

   QObject::connect (this->saveButton, SIGNAL (clicked           (bool)),
                     this,             SLOT   (saveButtonClicked (bool)));

   QObject::connect (this->revertButton, SIGNAL (clicked             (bool)),
                     this,               SLOT   (revertButtonClicked (bool)));

   QObject::connect (this->applyButton, SIGNAL (clicked            (bool)),
                     this,              SLOT   (applyButtonClicked (bool)));


   // Ensure widgets consistant with current settings
   //
   this->saveButton->setVisible (this->mShowSaveRevert);
   this->revertButton->setVisible (this->mShowSaveRevert);
   this->applyButton->setVisible (this->mShowApply);

   this->axisPainter->setMarkerValue  (SAVE_REVERT_MARKER, this->mShowSaveRevert);
   this->axisPainter->setMarkerColour (SAVE_REVERT_MARKER, QColor (55, 155, 255));

   this->slotValueChangeInhibited = true;
   this->mValue = 1.0;  // force internalSetValue function to apply change
   this->internalSetValue (0.0);
   this->slotValueChangeInhibited = false;
   this->emitValueChangeInhibited = false;
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setTextImage ()
{
   QString format;
   QString x;

   // The arg (...,'f') almost works but has no '+' option.
   //
   format.sprintf ("%%+0.%df", this->mPrecision);
   x.sprintf(format.toStdString().c_str(), this->mValue);

   this->rightImage->setText (x);
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::internalSetValue (const double value)
{
   const double low = this->getMinimum ();
   const double high  = this->getMaximum ();
   const double workingValue = LIMIT (value, low, high);

   if (this->mValue != workingValue) {
      this->mValue = workingValue;
      this->setSliderValue ();
      this->setTextImage ();

      // This prevents infinite looping in the case of cyclic connections.
      //
      if (!this->emitValueChangeInhibited) {
         emit this->valueChanged (this->mValue);
         emit this->valueChanged (int (this->mValue));   // range check?
      }
   }
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setIsActive (const bool value)
{
   this->mIsActive = value;

   // We enable/disable all internal widgets, the QAnalogSlider itself remains active.
   //
   this->intSlilder->setEnabled (this->mIsActive);
   this->axisPainter->setEnabled (this->mIsActive);
   this->leftImage->setEnabled (this->mIsActive);
   this->centreImage->setEnabled (this->mIsActive);
   this->rightImage->setEnabled (this->mIsActive);
}

//------------------------------------------------------------------------------
//
bool QAnalogSlider::getIsActive ()
{
   return this->mIsActive;
}

//------------------------------------------------------------------------------
//
int QAnalogSlider::getPrecision () const
{
   return this->mPrecision;
}

//------------------------------------------------------------------------------
//
double QAnalogSlider::getMinimum () const
{
   return this->mMinimum;
}

//------------------------------------------------------------------------------
//
double QAnalogSlider::getMaximum () const
{
   return this->mMaximum;
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setColourBandList (const QEColourBandList& bandList)
{
   this->axisPainter->setColourBandList (bandList);
}

//------------------------------------------------------------------------------
//
QEColourBandList QAnalogSlider::getColourBandList () const
{
   return this->axisPainter->getColourBandList ();
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setValue (const double value)
{
   // This prevents infinite looping in the case of cyclic connections.
   //
   this->emitValueChangeInhibited = true;
   this->internalSetValue (value);
   this->emitValueChangeInhibited = false;
}

//------------------------------------------------------------------------------
//
double QAnalogSlider::getValue () const
{
   return this->mValue;
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setValue (const int value)
{
   this->setValue (double (value));
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setDesignPrecision (const int precision)
{
   this->mPrecision = LIMIT (precision, 0, 12);
   this->updateAxisAndSlider ();
   this->setTextImage ();
}

//------------------------------------------------------------------------------
//
int QAnalogSlider::getDesignPrecision () const
{
   return this->mPrecision;
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setDesignMinimum (const double minimum)
{
   // Ensure in range
   //
   this->mMinimum = LIMIT (minimum, MIN_VALUE, MAX_VALUE);

   // Ensure consistant.
   //
   this->mMaximum = MAX (this->mMaximum, this->mMinimum + MIN_INTERVAL);

   this->setValue (this->getValue());
   this->updateAxisAndSlider ();
}

//------------------------------------------------------------------------------
//
double QAnalogSlider::getDesignMinimum () const
{
   return this->mMinimum;
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setDesignMaximum (const double maximum)
{
   // Ensure in range
   //
   this->mMaximum = LIMIT (maximum, MIN_VALUE, MAX_VALUE);

   // Ensure consistant.
   //
   this->mMinimum = MIN (this->mMinimum, this->mMaximum - MIN_INTERVAL);

   this->setValue (this->getValue());
   this->updateAxisAndSlider ();
}

//------------------------------------------------------------------------------
//
double QAnalogSlider::getDesignMaximum () const
{
   return this->mMaximum;
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setMinorInterval (const double minorInterval)
{
   const double currentMajorInterval = this->getMajorInterval ();
   this->axisPainter->setMinorInterval (minorInterval);
   this->setMajorInterval (currentMajorInterval);
}

//------------------------------------------------------------------------------
//
double QAnalogSlider::getMinorInterval () const
{
   return this->axisPainter->getMinorInterval ();
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setMajorInterval (const double majorInterval)
{
   int temp;

   temp = int ((majorInterval / this->axisPainter->getMinorInterval()) + 0.4999);
   this->axisPainter->setMajorMinorRatio (temp);
}

//------------------------------------------------------------------------------
//
double QAnalogSlider::getMajorInterval () const
{
   return this->axisPainter->getMajorMinorRatio () *
         this->axisPainter->getMinorInterval ();
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setLeftText (const QString& leftText)
{
   this->leftImage->setText (leftText);
}

//------------------------------------------------------------------------------
//
QString QAnalogSlider::getLeftText () const
{
   return this->leftImage->text ();
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setCentreText (const QString& centreText)
{
   this->centreImage->setText (centreText);
}

//------------------------------------------------------------------------------
//
QString QAnalogSlider::getCentreText () const
{
   return this->centreImage->text ();
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setRightText (const QString& rightText)
{
   this->rightImage->setText (rightText);
}

//------------------------------------------------------------------------------
//
QString QAnalogSlider::getRightText () const
{
   return this->rightImage->text ();
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setShowSaveRevert (const bool show)
{
   if (this->mShowSaveRevert != show) {
      this->mShowSaveRevert = show;
      this->saveButton->setVisible (show);
      this->revertButton->setVisible (show);
      this->axisPainter->setMarkerVisible (SAVE_REVERT_MARKER, show);

      if (show) {
         // When turn on we re-save the current value as the saved value.
         //
         this->saveButtonClicked (false);
      } else {
         this->leftImage->clear ();
      }
   }
}

//------------------------------------------------------------------------------
//
bool QAnalogSlider::getShowSaveRevert () const
{
   return this->mShowSaveRevert;
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setShowApply (const bool show)
{
   this->mShowApply = show;
   this->applyButton->setVisible (show);
}

//------------------------------------------------------------------------------
//
bool QAnalogSlider::getShowApply () const
{
   return this->mShowApply;
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setFontColour (const QColor fontColourIn)
{
   this->mFontColour = fontColourIn;
   this->axisPainter->setPenColour (this->mFontColour);
}

//------------------------------------------------------------------------------
//
QColor QAnalogSlider::getFontColour () const
{
   return this->mFontColour;
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::updateAxisAndSlider ()
{
   const double min = this->getMinimum ();
   const double max = this->getMaximum ();
   const double span = max - min;

   this->axisPainter->setMinimum (min);
   this->axisPainter->setMaximum (max);

   int significance = this->getPrecision();
   double tick = EXP10 (-significance);

   while ((span / tick) >= 100000.0) {
      significance--;
      tick = EXP10 (-significance);
   }

   int a = int (min / tick);
   int b = int (max / tick);

   this->slotValueChangeInhibited = true;
   this->intSlilder->setMinimum (a);
   this->intSlilder->setMaximum (b);
   this->intSlilder->setMinimum (a);
   this->slotValueChangeInhibited = false;

   // Must reset slider value/position.
   //
   this->setSliderValue ();
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setSliderValue ()
{
   int posn = this->convertToInt (this->mValue);
   if (this->intSlilder->value () != posn) {
      // We are setting the slider position - esure we disgard signal/slot updates.
      this->slotValueChangeInhibited = true;
      this->intSlilder->setValue (posn);
      this->slotValueChangeInhibited = false;
   }
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::sliderPositionChanged (const int posn)
{
   if (!this->slotValueChangeInhibited) {
      this->internalSetValue (convertToFloat (posn));
   }
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::setSavedValue (const double savedValueIn)
{
   this->savedValue = savedValueIn;

   QString format;
   QString x;

   format.sprintf ("%%+0.%df", this->mPrecision);
   x.sprintf (format.toStdString().c_str(), this->savedValue);
   this->leftImage->setText (x);

   this->axisPainter->setMarkerValue (SAVE_REVERT_MARKER, this->savedValue);
}

//------------------------------------------------------------------------------
//
double QAnalogSlider::getSavedValue () const
{
   return this->savedValue;
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::saveButtonClicked (bool)
{
   this->setSavedValue (this->getValue ());
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::revertButtonClicked (bool)
{
   this->internalSetValue (this->savedValue);
}

//------------------------------------------------------------------------------
//
void QAnalogSlider::applyButtonClicked (bool)
{
   emit this->appliedValue (this->mValue);
   emit this->appliedValue (int (this->mValue));   // range check?
}

//------------------------------------------------------------------------------
//
int QAnalogSlider::convertToInt (const double x)
{
   double x0 = this->getMinimum ();
   double x1 = this->getMaximum ();

   double y0 = (double) this->intSlilder->minimum ();
   double y1 = (double) this->intSlilder->maximum ();

   double m = (y1 - y0) / (x1 - x0);
   double c = y0 - m*x0;

   return int (m*x + c);
}

//------------------------------------------------------------------------------
//
double QAnalogSlider::convertToFloat (const int ix)
{
   double x0 = (double) this->intSlilder->minimum ();
   double x1 = (double) this->intSlilder->maximum ();

   double y0 = this->getMinimum ();
   double y1 = this->getMaximum ();

   double m = (y1 - y0) / (x1 - x0);
   double c = y0 - m*x0;

   return m*ix + c;
}

// end
