/*  QAnalogSlider.h
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

#ifndef QE_QANALOG_SLIDER_H
#define QE_QANALOG_SLIDER_H

#include <QFrame>
#include <QPushButton>
#include <QList>
#include <QSize>
#include <QLabel>
#include <QSlider>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QECommon.h>
#include <QEColourBandList.h>
#include <QEAxisPainter.h>
#include <QEFrameworkLibraryGlobal.h>

/// QAnalogSlider is a non EPICS aware slider that provides an analog equivilent
/// of the QSlider.  It is deemed analog as it can be se by/emits floating point
/// (double) values as opposed to interger values.  It is also decorated with a
/// scale and text showing the current value; it also provides a local save and
/// restore capability.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QAnalogSlider : public QFrame
{
   Q_OBJECT
public:
   /// Speficies the current value.
   Q_PROPERTY (double value          READ getValue            WRITE setValue)

   /// Precision used for the display and editing of numbers. The default is 2.
   /// Strictly speaking, this should be an unsigned int, but the designer int
   /// properties editor much 'nicer'.
   Q_PROPERTY (int  precision        READ getDesignPrecision  WRITE setDesignPrecision)

   /// Speficies the mimimum allowed value.
   Q_PROPERTY (double minimum        READ getDesignMinimum    WRITE setDesignMinimum)

   /// Speficies the maximum allowed value.
   Q_PROPERTY (double maximum        READ getDesignMaximum    WRITE setDesignMaximum)

   /// Minor scale interval.
   /// Only applies for linear scale (not log scale)
   Q_PROPERTY (double minorInterval  READ getMinorInterval  WRITE setMinorInterval)

   /// Minor scale interval.
   /// Only applies for linear scale (not log scale)
   Q_PROPERTY (double majorInterval  READ getMajorInterval  WRITE setMajorInterval)

   /// Controls when valueChanged signal is emitted.
   /// If tracking is enabled (the default), the slider emits the valueChanged () signal while the slider is being dragged.
   /// If tracking is disabled, the slider emits the valueChanged () signal only when the user releases the slider.
   Q_PROPERTY (bool tracking READ hasTracking WRITE setTracking)

   /// Controls the left, centre and right texts.
   Q_PROPERTY (QString leftText      READ getLeftText       WRITE setLeftText)
   Q_PROPERTY (QString centreText    READ getCentreText     WRITE setCentreText)
   Q_PROPERTY (QString rightText     READ getRightText      WRITE setRightText)

   /// Enables/disables the save-revert capability.
   Q_PROPERTY (bool showSaveRevert   READ getShowSaveRevert   WRITE setShowSaveRevert)

   /// Enables/disables the apply value capability.
   Q_PROPERTY (bool showApply        READ getShowApply        WRITE setShowApply)

   /// Font colour
   ///
   Q_PROPERTY (QColor fontColour     READ getFontColour       WRITE setFontColour)

   /// Alternative to isEnabled. Default is true.
   Q_PROPERTY (bool   isActive       READ getIsActive         WRITE setIsActive)

public:
   /// Create with default title.
   ///
   explicit QAnalogSlider (QWidget* parent = 0);

   /// Destruction
   virtual ~QAnalogSlider() { }

   QSize sizeHint () const;

   void setColourBandList (const QEColourBandList& bandList);
   QEColourBandList getColourBandList () const;

   // Property setters and getters
   //
public slots:
   void setValue (const double value);
   void setValue (const int value);                     // overloaded form

public:
   double getValue () const;

   void setDesignPrecision (const int precision);       ///< Access function for #precision - refer to #precision property for details
   int getDesignPrecision () const;                     ///< Access function for #precision - refer to #precision property for details

   void setDesignMinimum (const double minimum);        ///< Access function for #minimum - refer to #minimum property for details
   double getDesignMinimum () const;                    ///< Access function for #minimum - refer to #minimum property for details

   void setDesignMaximum (const double maximum);        ///< Access function for #maximum - refer to #maximum property for details
   double getDesignMaximum () const;                    ///< Access function for #maximum - refer to #maximum property for details

   void setMinorInterval (const double minorInterval);  ///< Access function for #minorInterval - refer to #minorInterval property for details
   double getMinorInterval () const;                    ///< Access function for #minorInterval - refer to #minorInterval property for details

   void setMajorInterval (const double majorInterval);  ///< Access function for #majorInterval - refer to #majorInterval property for details
   double getMajorInterval () const;                    ///< Access function for #majorInterval - refer to #majorInterval property for details

   void setLeftText (const QString& leftText);          ///< Access function for #leftText - refer to #leftText property for details
   QString getLeftText () const;                        ///< Access function for #leftText - refer to #leftText property for details

   void setCentreText (const QString& centreText);      ///< Access function for #centreText - refer to #centreText property for details
   QString getCentreText () const;                      ///< Access function for #centreText - refer to #centreText property for details

   void setRightText (const QString& rightText);        ///< Access function for #rightText - refer to #rightText property for details
   QString getRightText () const;                       ///< Access function for #rightText - refer to #rightText property for details

   void setShowSaveRevert (const bool show);
   bool getShowSaveRevert () const;

   void setShowApply (const bool show);
   bool getShowApply () const;

   void setFontColour (const QColor fontColour);        ///< Access function for #fontColour - refer to #fontColour property for details
   QColor getFontColour () const;                       ///< Access function for #fontColour - refer to #fontColour property for details

   // A nod to the EPICS aware class derived from this class.
   //
   void setIsActive (const bool value);
   bool getIsActive ();


   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (intSlilder, bool, hasTracking, setTracking)

   // Access saved value.
   //
   void setSavedValue (const double savedValue);
   double getSavedValue () const;

signals:
   void valueChanged (const double value);              // Send when value changes.
   void valueChanged (const int value);                 // overloaded form

   void appliedValue (const double value);              // Send when internal apply button clicked
   void appliedValue (const int value);                 // overloaded form

protected:
   // internal widget access
   //
   QEAxisPainter* getAxisPainter () { return this->axisPainter; }

   // Allows sub-class to override designer (property) parameters.
   // Default, i.e. non overriden, functions just returns designer values.
   //
   // While this widget has no knowledge of QEAnalogSlider per se, these virtual
   // functions are included specifically to allow an QEAnalogSlider widget to auto
   // scale without the need to change design time property values. This allows
   // toggling between auto scale on/off while still maintaining property values.
   //
   virtual int getPrecision () const;
   virtual double getMinimum () const;
   virtual double getMaximum () const;

   void updateAxisAndSlider ();

protected slots:
    virtual void applyButtonClicked (bool);  /// default action is to emit appliedValue

private:
   void commonSetup ();
   void setTextImage ();
   void internalSetValue (const double value);
   void setSliderValue ();

   // Converts between slider integer positions and associated real values.
   //
   int convertToInt (const double x);
   double convertToFloat (const int j);

   // intervals are stored in the AxisPainter. We do not duplicate
   // them here. However, the widget value is stored here (as opposed to in the
   // QSlider) to both maintain precision and avoid conversion difficulties when
   // the mapping between posn and value changes.
   //
   double mValue;
   double mMinimum;
   double mMaximum;
   int mPrecision;
   bool mIsActive;
   bool mShowSaveRevert;
   bool mShowApply;
   QColor mFontColour;

   double savedValue;

   // internal widgets
   //
   QVBoxLayout* layout;
   QBoxLayout* sliderLayout;   // we need a gap each siode of the slider.
   QSlider* intSlilder;
   QFrame* labelFrame;
   QBoxLayout* labelLayout;    // and save, revert and apply buttons
   QEAxisPainter* axisPainter;
   QLabel* leftImage;
   QLabel* centreImage;
   QLabel* rightImage;
   QPushButton* saveButton;
   QPushButton* revertButton;
   QPushButton* applyButton;

   bool emitValueChangeInhibited;
   bool slotValueChangeInhibited;

private slots:
   // Driven by internal QSlider.
   //
    void sliderPositionChanged (const int posn);
    void saveButtonClicked (bool);
    void revertButtonClicked (bool);
};

#endif // QE_QANALOG_SLIDER_H
