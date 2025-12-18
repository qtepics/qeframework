/*  QNumericEdit.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef Q_NUMERIC_EDIT_H
#define Q_NUMERIC_EDIT_H

#include <QChar>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QString>
#include <QWidget>

#include <QECommon.h>
#include <QEEnums.h>
#include <QEFixedPointRadix.h>
#include <QEFrameworkLibraryGlobal.h>

/// This class provides a non EPICS aware numeric edit widget, somewhat
/// akin to QDoubleSpin. However QNumericEdit provides a number of advantages
/// over QDoubleSpin:
/// a/ the former is restricted to a single spin value, where as QNumericEdit
///    allows any digit to be selected and used as the spin value;
/// b/ the former is restricted to decimal representation where as QNumericEdit
///    allows for hexadecimal, octal and binary represntations;
/// c/ this QNumericEdit allow allows a 'thousands' seperator character to be
///    specified; and
/// d/ by setting the precision to 0, it effectively becems akin to a QSpinBox.
///
// Note: QENumericEdit will be modified to sub-class or side-class this widget
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QNumericEdit : public QWidget {
   Q_OBJECT
public:
   /// This property holds whether the numeric edit draws itself with a frame.
   /// If enabled (the default) the numeric edit draws itself inside a frame,
   /// otherwise the line edit draws itself without any frame.
   ///
   Q_PROPERTY (bool    frame           READ hasFrame       WRITE setFrame)

   /// This property holds any fixed text (default is "") displayed after the numeric value.
   ///
   Q_PROPERTY (QString suffix          READ getSuffix      WRITE setSuffix)

   /// This property holds any fixed text (default is "") displayed before the numeric value.
   ///
   Q_PROPERTY (QString prefix          READ getPrefix      WRITE setPrefix)

   /// This property holds the alignment of the numeric edit.
   /// Both horizontal and vertical alignment is allowed here, Qt::AlignJustify will map to Qt::AlignLeft.
   /// By default, this property contains a combination of Qt::AlignRight and Qt::AlignVCenter.
   ///
   Q_PROPERTY (Qt::Alignment alignment READ alignment      WRITE setAlignment)

   /// This property holds the displayed text. Not a property available to designer.
   /// It exludes any prefix/suffix.
   //
   Q_PROPERTY (QString cleanText       READ getCleanText)

   // Note: the order of declaration affects the order in which these are applied
   //       which is important given the nature of how radix, leading zeros,
   //       precision, min and max are related.
   /// Notation used for formatting/editing. Default is fixed.
   /// The widget does not allow Automatic
   ///
   Q_PROPERTY (QE::Notations notation        READ getNotation       WRITE setNotation)

   /// Specify radix, default is Decimal.
   ///
   Q_PROPERTY (QE::Radicies radix  READ getRadix WRITE setRadix)

   /// Specify digit 'thousands' separator character, default is NoSeparator.
   ///
   Q_PROPERTY (QE::Separators separator  READ getSeparator      WRITE setSeparator)

   /// Speficies the number of leading zeros.
   /// Strictly speaking, this should be an unsigned int, but designer int properties editor much 'nicer'.
   ///
   Q_PROPERTY (int leadingZeros          READ getLeadingZeros   WRITE setLeadingZeros)

   /// Precision used for the display and editing of numbers. The default is 4.
   /// Strictly speaking, this should be an unsigned int, but designer int properties editor much 'nicer'.
   ///
   Q_PROPERTY (int  precision            READ getPrecision      WRITE setPrecision)

   /// When true, there is always a sign charactere "+" or "-".
   /// When false (default), no "+" sign when range of valued non-negative.
   ///
   Q_PROPERTY (bool forceSign        READ getForceSign      WRITE setForceSign)

   /// When set true (default is false) the numeric edit widget will wrap around
   /// from the max/min to min/max value.
   ///
   Q_PROPERTY (bool wrapValue        READ getWrapValue      WRITE setWrapValue)

   /// Specify the mimimum allowed value.
   ///
   Q_PROPERTY (double minimum        READ getMinimum        WRITE setMinimum)

   /// Specify the maximum allowed value.
   ///
   Q_PROPERTY (double maximum        READ getMaximum        WRITE setMaximum)

   /// Specify the value after min/max
   ///
   Q_PROPERTY (double value          READ getValue          WRITE setValue)

public:
   /// Construction
   QNumericEdit (QWidget *parent = 0);

   /// Destruction
   virtual ~QNumericEdit ();

   // Property set and get functions.
   //
public slots:
   // The is no signal (callback) when widget value is set programatically.
   //
   void setValue (const double value);
   void setValue (const int value);    // overloaded form
public:
   double getValue () const;

   void setPrefix (const QString &prefix);
   QString getPrefix () const;

   void setSuffix (const QString &suffix);
   QString getSuffix () const;

   QString getCleanText () const;

   void setLeadingZeros (const int value);
   int getLeadingZeros () const;

   void setPrecision (const int value);
   int getPrecision () const;

   void setForceSign (const bool value);
   bool getForceSign () const;

   void setWrapValue (const bool value);
   bool getWrapValue () const;

   void setNotation (const QE::Notations notation);
   QE::Notations getNotation () const;

   void setMinimum (const double value);
   double getMinimum () const;

   void setMaximum (const double value);
   double getMaximum () const;

   void setRadix (const QE::Radicies value);
   QE::Radicies getRadix () const;

   void setSeparator (const QE::Separators value);
   QE::Separators getSeparator () const;

   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (lineEdit, bool,          hasFrame,   setFrame)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (lineEdit, Qt::Alignment, alignment,  setAlignment)

signals:
   void valueChanged (const double value);
   void valueChanged (const int value);   // overloaded form
   void returnPressed ();
   void editingFinished ();

protected:
   void focusInEvent (QFocusEvent* event);
   bool eventFilter (QObject *obj, QEvent *event);

   QEFixedPointRadix fpr;   // holds radix and separator

private:
   QLineEdit* lineEdit;     // internal widget
   QHBoxLayout* layout;     // holds the QLineEdit - any layout type will do

   // Property values
   //
   QString mPrefix;
   QString mSuffix;
   double mMinimum;
   double mMaximum;
   int mLeadingZeros;
   int mPrecision;
   bool mForceSign;
   bool mWrapValue;
   QE::Notations mNotation;
   double mValue;         // the actual value

   // Other values
   //
   double minimumMin;
   double maximumMax;
   int cursorFirst;       // First allowed number character within line edit
   int cursorLast;        // Last allowed number character within line edit
   int cursorPosition;    // Current cursor position
   bool emitValueChangeInhibited;

   void commonConstructor ();
   void internalSetValue (const double value);  // used from within the widget.

   bool lineEditKeyPressEvent (QKeyEvent *event);
   bool lineEditFocusInEvent (QFocusEvent *event);
   bool lineEditMouseReleaseEvent (QMouseEvent *event);
   bool lineEditEventFilter (QEvent *event);

   // Maximum number of allowed digits - e.g. for decimal this is 15.
   //
   int maximumSignificance () const;

   QString imageOfValue () const;  // Generate image of the current value - include prefix and suffix.

   bool showSign () const;
   bool cursorOverSign () const;      // Cursor is over the leading sign char
   bool cursorOverExpSign () const;   // Cursor is over the exponent sign char
   bool cursorOverExponent () const;  // Cursor is over the exponent nunber

   void redisplayText ();             // Calls embedded lineEdit's setText using string from imageOfValue.
   void setDigitSelection ();         // High-lights selected digit

   void setCursorPosition (const int value);
   int getCursorPosition () const;

   QChar charAt (const int j) const;      // Conveniance function to extract jth character of line edit text.
   bool isRadixDigit (QChar qc) const;    // Is the character a valid digit for the selected radix
   bool isSign (QChar qc) const;          // Is the character on of '+' or '-'
   bool isSignOrDigit (QChar qc) const;   // Is the character either of the above.

   double calcUpper () const;             // Max maximum value (based on leading zeros and precsion)
   double calcLower () const;             // Min minimum value (based on leading zeros and precsion)
   void applyLimits ();                   // Ensure minMin <= min < value < max < maxMax

   // Extract value of given image. If image does not produce a valid value, then
   // the function returns the widget's current value.
   //
   double valueOfImage (const QString& image) const;

   // Format the given value using define radix, separators, leading zeros and precsion.
   //
   QString getFormattedText (const double value) const;

   friend class QENumericEdit;
};

#endif  // Q_NUMERIC_EDIT_H
