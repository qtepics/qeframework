/*  QAnalogSlider.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2015-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
   Q_PROPERTY (int  precision        READ getPrecision  WRITE setPrecision)

   /// Speficies the mimimum allowed value.
   Q_PROPERTY (double minimum        READ getMinimum    WRITE setMinimum)

   /// Speficies the maximum allowed value.
   Q_PROPERTY (double maximum        READ getMaximum    WRITE setMaximum)

   /// Minor scale interval.
   /// Only applies for linear scale (not log scale)
   Q_PROPERTY (double minorInterval  READ getMinorInterval  WRITE setMinorInterval)

   /// Minor scale interval.
   /// Only applies for linear scale (not log scale)
   Q_PROPERTY (double majorInterval  READ getMajorInterval  WRITE setMajorInterval)

   /// Inverted apperance.
   /// Default (false) is left ot right.
   Q_PROPERTY (bool invertedAppearance READ getInvertedAppearance WRITE setInvertedAppearance)

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

   void setPrecision (const int precision);       ///< Access function for #precision - refer to #precision property for details
   int getPrecision () const;                     ///< Access function for #precision - refer to #precision property for details

   void setMinimum (const double minimum);        ///< Access function for #minimum - refer to #minimum property for details
   double getMinimum () const;                    ///< Access function for #minimum - refer to #minimum property for details

   void setMaximum (const double maximum);        ///< Access function for #maximum - refer to #maximum property for details
   double getMaximum () const;                    ///< Access function for #maximum - refer to #maximum property for details

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
   bool getIsActive () const;

   void setInvertedAppearance (const bool invertedAppearance);     ///< Access function for #invertedAppearance
   bool getInvertedAppearance () const;                            ///< Access function for #invertedAppearance

   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (intSlider, bool, hasTracking, setTracking)

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
   int convertToInt (const double x) const;
   double convertToFloat (const int j) const;

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
   bool mInvertedAppearance;
   bool mShowSaveRevert;
   bool mShowApply;
   QColor mFontColour;

   double savedValue;

   // internal widgets
   //
   QVBoxLayout* layout;
   QBoxLayout* sliderLayout;   // we need a gap each siode of the slider.
   QSlider* intSlider;
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
