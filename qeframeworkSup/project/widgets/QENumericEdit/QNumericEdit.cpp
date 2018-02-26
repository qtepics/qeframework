/*  QNumericEdit.cpp
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
 *  Copyright (c) 2014,2016,2018 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include "QNumericEdit.h"
#include <math.h>
#include <QDebug>

#define DEBUG  qDebug () << "QNumericEdit" << __LINE__ << __FUNCTION__ << "  "


//==============================================================================
// For decimal, this is about 48.9 bits, for the other radix values this is 48 bits exactly.
// Note: order MUST be consistant with enum Radicies specification.
//
#define NUMBER_OF_RADICES  4
const static int maximumNumberDigits [NUMBER_OF_RADICES] = { 15, 12, 16, 48 };


//==============================================================================
// QNumericEdit
//==============================================================================
//
QNumericEdit::QNumericEdit (QWidget * parent) : QWidget (parent)
{
   this->commonConstructor ();
}

//------------------------------------------------------------------------------
//
void QNumericEdit::commonConstructor ()
{
   this->fpr.setRadix (QEFixedPointRadix::Decimal);
   this->fpr.setSeparator (QEFixedPointRadix::None);

   this->lineEdit = new QLineEdit (this);
   this->lineEdit->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);
   this->lineEdit->setAlignment (Qt::AlignRight);
   
   this->lineEdit->installEventFilter (this);

   // The font change event must be applied to the internal widget.
   //
   this->installEventFilter (this);

   this->layout = new QHBoxLayout (this);
   this->layout->setMargin (0);
   this->layout->setSpacing (0);
   this->layout->addWidget (this->lineEdit);

   // Ensure sensible auto values
   //
   this->mPrefix = "";
   this->mSuffix = "";
   this->mLeadingZeros = 3;
   this->mPrecision = 4;
   this->mNotation = Fixed;
   this->minimumMin = this->calcLower ();
   this->maximumMax = this->calcUpper ();
   this->mMinimum = this->minimumMin;
   this->mMaximum = this->maximumMax;

   // force internalSetValue to process.
   //
   this->mValue = 0.1;  // force initial update
   this->internalSetValue (0.0);
   this->cursor = this->cursorFirst;
   this->emitValueChangeInhibited = false;
}

//------------------------------------------------------------------------------
//
QNumericEdit::~QNumericEdit ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
int QNumericEdit::maximumSignificance () const
{
   return maximumNumberDigits [this->fpr.getRadix ()];
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::lineEditKeyPressEvent (QKeyEvent * event)
{
   const int key = event->key ();
   const double dblRadix = double (this->fpr.getRadixValue ());
   const int index = this->getCursor ();

   QChar qc;
   QChar qk;
   bool handledLocally;

   handledLocally = true;
   switch (key) {

      case Qt::Key_Up:
      case Qt::Key_Down:
         qc = this->charAt (index);

         if (this->cursorOverSign ()) {
            // Eg: +1.23  => -1.23
            double fabule = fabs (this->getValue ());
            this->internalSetValue (key == Qt::Key_Up ? +fabule : -fabule);

         } else if (this->cursorOverExpSign ()) {
            // Eg: 1.23E-6  => 1.23E+6
            QString expText = this->lineEdit->text().mid (index, 3);
            bool okay;
            int expValue = expText.toInt (&okay);
            if (okay) {
               if (((expValue < 0) && (key == Qt::Key_Up)) ||
                   ((expValue > 0) && (key == Qt::Key_Down) )) {
                  int p = -2 * expValue;
                  double factor = pow (10.0, p);
                  this->internalSetValue (this->getValue () * factor);
               }
            }

         } else if (this->cursorOverExponent ()) {
            // Eg: 1.23E+07 => 1.23E+08 or 1.23E+07 => 1.23E+17
            int p = (index == (this->cursorLast - 1)) ? 10 : 1;
            p = (key == Qt::Key_Up ? +p : -p);
            double factor = pow (10.0, p);
            this->internalSetValue (this->getValue () * factor);

         } else if (this->isRadixDigit (qc)) {    // Is this a digit charcter?

            int significance = -this->mPrecision;

            if (this->mNotation == Fixed) {
               significance = -this->mPrecision;
               for (int j = index + 1; j <= this->cursorLast; j++) {
                  qc = this->charAt (j);
                  if (this->isRadixDigit (qc)) {
                     significance++;
                  }
               }

            } else if (this->mNotation == Scientific) {
               // Exponent size, exluding the 'e' is 3.
               //
               QString expText = this->lineEdit->text().mid (this->cursorLast - 2, 3);
               bool okay;
               significance = expText.toInt (&okay) - this->mPrecision;
               for (int j = index + 1; j <= this->cursorLast - 3; j++) {
                  qc = this->charAt (j);
                  if (this->isRadixDigit (qc)) {
                     significance++;
                  }
               }
            }

            double delta = pow (dblRadix, significance);
            delta = (key == Qt::Key_Up ? +delta : -delta);
            this->internalSetValue (this->getValue () + delta);
         }
         break;

      case Qt::Key_Left:
         this->setCursor (this->getCursor () - 1);

         // If we have moved onto a filler character, then move again.
         //
         qc = this->charAt (this->getCursor ());
         if (!this->isSignOrDigit (qc)) {
            this->setCursor (this->getCursor () - 1);
         }
         break;


      case Qt::Key_Right:
         this->setCursor (this->getCursor () + 1);

         // If we have moved onto a filler character, then move again.
         //
         qc = this->charAt (this->getCursor ());
         if (!this->isSignOrDigit (qc)) {
            this->setCursor (this->getCursor () + 1);
         }
         break;


      case Qt::Key_Plus:
      case Qt::Key_Minus:
         if (this->cursorOverSign ()) {
            if (key == Qt::Key_Plus) {
               this->internalSetValue (+fabs (this->getValue ()));
            } else {
               this->internalSetValue (-fabs (this->getValue ()));
            }
            this->setCursor (this->getCursor () + 1);

         } else if (this->cursorOverExpSign ()) {
            QString tryThis = this->lineEdit->text ();
            tryThis [index] = QChar (key);

            double newval = this->valueOfImage (tryThis);

            this->internalSetValue (newval);
            this->setCursor (this->getCursor () + 1);
         }
         break;


      case Qt::Key_0:
      case Qt::Key_1:
      case Qt::Key_2:
      case Qt::Key_3:
      case Qt::Key_4:
      case Qt::Key_5:
      case Qt::Key_6:
      case Qt::Key_7:
      case Qt::Key_8:
      case Qt::Key_9:
      // we do not care about shift here, allow 'A' or 'a' etc.
      case Qt::Key_A:
      case Qt::Key_B:
      case Qt::Key_C:
      case Qt::Key_D:
      case Qt::Key_E:
      case Qt::Key_F:
         qk = QChar (key);
         qc = this->charAt (index);

         // Both the new char and the existing char must both be radix digits.
         //
         if (this->isRadixDigit (qk) && this->isRadixDigit (qc)) {
            QString tryThis = this->lineEdit->text ();
            tryThis [index] = QChar (key);

            double newval = this->valueOfImage (tryThis);

            this->internalSetValue (newval);
            this->setCursor (this->getCursor () + 1);

            // If we have moved onto a filler character, then move again.
            //
            qc = this->charAt (this->getCursor ());
            if (!this->isSignOrDigit (qc)) {
               this->setCursor (this->getCursor () + 1);
            }
         }
         break;

      case Qt::Key_Space:
         break;

      case Qt::Key_Backspace:
      case Qt::Key_Delete:
         break;

      case Qt::Key_Return:
      case Qt::Key_Enter:
         emit returnPressed ();
         break;

      case Qt::Key_Tab:
         handledLocally = false;
         break;

      default:
         handledLocally = (key < 256);
         break;
   }

   // All events handled locally.
   //
   return handledLocally;
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::lineEditFocusInEvent (QFocusEvent* event)
{
   bool result = false;   // not handled unless we actually handle this event.

   if (event->gotFocus ()) {
      this->setDigitSelection ();
      result = true;

   } else if (event->lostFocus ()) {
      emit editingFinished ();
      result = false;    // sort of handled, but not totally.

   } else {
      DEBUG << "Unexpected focus event state (not handled)";
   }

   return result;
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::lineEditMouseReleaseEvent (QMouseEvent* /* event*/ )
{
   int posn;

   // Is this just a mouse press-releasse or end of selection text release?
   //
   if (this->lineEdit->hasSelectedText ()) {
      // Go to start of selected text.
      //
      posn = this->lineEdit->selectionStart ();
   } else {
      posn = this->lineEdit->cursorPosition ();
   }

   this->setCursor (posn);
   return true;  //  handled locally
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::lineEditEventFilter (QEvent *event)
{
   const QEvent::Type type = event->type ();

   bool result = false;   // not handled unless we actually handle this event.

   switch (type) {

      case QEvent::MouseButtonRelease:
         result = this->lineEditMouseReleaseEvent (static_cast<QMouseEvent *> (event));
         break;

      case QEvent::KeyPress:
         result = this->lineEditKeyPressEvent (static_cast<QKeyEvent *> (event));
         break;

      case QEvent::FocusIn:
      case QEvent::FocusOut:
         result = this->lineEditFocusInEvent (static_cast<QFocusEvent *> (event));
         break;

      default:
         // Just fall through - not handled.
         result = false;
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QNumericEdit::focusInEvent (QFocusEvent* event)
{
   this->lineEdit->setFocus ();     // pass to enclosed widget
   QWidget::focusInEvent (event);   // pass to parent
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::eventFilter (QObject *obj, QEvent *event)
{
   const QEvent::Type type = event->type ();
   bool result = false;   // not handled unless we actually handle this event.

   if (obj == this->lineEdit) {
      result = this->lineEditEventFilter (event);
      return result;
   }

   switch (type) {
      case QEvent::FontChange:
         if (obj == this) {
            // Font must be mapped to the internal lineEdit
            //
            if (this->lineEdit) {
               this->lineEdit->setFont (this->font ());
            }
         }
         result = true;
         break;

      default:
         result = false;
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setDigitSelection ()
{
   // Only set/update selection if/when the widget has focus.
   //
   if (this->lineEdit->hasFocus ()) {
      int posn = this->getCursor ();
      this->lineEdit->setSelection (posn, 1);
   }
}

//------------------------------------------------------------------------------
//
double QNumericEdit::valueOfImage (const QString& image) const
{
   int length;
   QString intermediate;
   QString sepChar;
   bool okay;
   double result = this->mValue;

   // Excude any prefix and/or suffix.
   //
   length = this->cursorLast - this-> cursorFirst + 1;
   intermediate = image.mid (this->cursorFirst, length);

   switch (this->mNotation) {
      case Fixed:
         result = this->fpr.toValue (intermediate, okay);
         if (!okay) {
            result = this->mValue;
         }
         break;

      case Scientific:
         // Remove thousands separators if needs be.
         //
         switch (this->getSeparator ()) {
            case QEFixedPointRadix::None:       sepChar = "";  break;
            case QEFixedPointRadix::Comma:      sepChar = ",";  break;
            case QEFixedPointRadix::Underscore: sepChar = "_";  break;
            case QEFixedPointRadix::Space:      sepChar = " ";  break;
         }

         if (!sepChar.isEmpty ()) {
            intermediate.remove (sepChar);
         }

         result = intermediate.toDouble (&okay);
         if (!okay) {
            result = this->mValue;
         }
         break;
   }


   return result;
}

//------------------------------------------------------------------------------
//
QString QNumericEdit::getFormattedText (const double value) const
{
   QString result;
   QString signChar;
   QString sepChar;
   int point;

   switch (this->mNotation) {
      case Fixed:
         result = this->fpr.toString (value, this->showSign (),
                                      this->mLeadingZeros, this->mPrecision);
         break;

      case Scientific:
         signChar = (this->showSign () && (value >= 0.0)) ? "+" : "";
         result = QString ("%1%2").arg (signChar).arg (value, 0, 'e', this->mPrecision);

         // Add thousands separators if needs be.
         //
         switch (this->getSeparator ()) {
            case QEFixedPointRadix::None:       sepChar = "";  break;
            case QEFixedPointRadix::Comma:      sepChar = ",";  break;
            case QEFixedPointRadix::Underscore: sepChar = "_";  break;
            case QEFixedPointRadix::Space:      sepChar = " ";  break;
         }
         if (sepChar.isEmpty ()) break;

         // Find decimal point
         //
         point = result.indexOf ('.');
         if (point < 0) break;

         for (int j = 1; j <= 5; j++) { // limit to 5 - safety check
            int q = point + 4*j;
            if (q >= result.length() - 4) break;
            result.insert (q, sepChar);
         }
         break;
   }

   result = result.trimmed ();
   return result;
}

//------------------------------------------------------------------------------
// Compare with cleanText ()
//
QString QNumericEdit::imageOfValue () const
{
   QString image;

   image = this->getFormattedText (this->mValue);
   return this->mPrefix + image + this->mSuffix;
}

//------------------------------------------------------------------------------
//
void QNumericEdit::redisplayText ()
{
   QString image = this->imageOfValue ();

   // Note: this has an intended side effect.
   // TODO: Explain this more !!!
   //
   this->lineEdit->setMaxLength (image.length ());
   this->lineEdit->setText (image);

   this->cursorFirst = this->mPrefix.length ();
   this->cursorLast = image.length () - 1 - this->mSuffix.length ();

   this->setCursor (this->cursor);
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setCursor (const int value)
{
   // Ensure cursor is in range of interest, i.e. excluding prefix/suffix.
   //
   this->cursor = LIMIT (value, this->cursorFirst, this->cursorLast);
   this->setDigitSelection ();
}

//------------------------------------------------------------------------------
//
int QNumericEdit::getCursor () const
{
   return this->cursor;
}

//------------------------------------------------------------------------------
//
QChar QNumericEdit::charAt (const int j) const
{
   return this->lineEdit->text () [j];
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::isRadixDigit (QChar qc) const
{
   return this->fpr.isRadixDigit (qc);
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::isSign (QChar qc) const
{
   char c = qc.toLatin1 ();

   return ((c == '+') || (c == '-'));
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::isSignOrDigit (QChar qc) const
{
   return (this->isSign (qc) || this->isRadixDigit (qc));
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::showSign () const
{
   // Only force '+' if the value can be negative.
   //
   return (this->mMinimum < 0.0);
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::cursorOverSign () const
{
   return (this->showSign () && (this->getCursor () == this->cursorFirst));
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::cursorOverExpSign () const
{
   return ((this->mNotation == Scientific) &&
           (this->getCursor () == this->cursorLast - 2));
}

//------------------------------------------------------------------------------
//
bool QNumericEdit::cursorOverExponent () const
{
   return ((this->mNotation == Scientific) &&
           ((this->getCursor () == this->cursorLast - 1) ||
            (this->getCursor () == this->cursorLast)));
}

//------------------------------------------------------------------------------
// Example: leading zeros = 2, precision = 1, radix = 10, then max
// value is 99.9 =  10**2 - 10**(-1)
//
double QNumericEdit::calcUpper () const
{
   const double dblRadix = double (this->fpr.getRadixValue ());

   double result = 0.0;
   double a, b;

   switch (this->mNotation) {

      case Fixed:
         a = pow (dblRadix, +this->mLeadingZeros);
         b = pow (dblRadix, -this->mPrecision);
         result =  a - b;
         break;

      case Scientific:
         result = +9.999999999999e+99;
         break;

   }
   return result;
}

//------------------------------------------------------------------------------
//
double QNumericEdit::calcLower () const
{
   return -this->calcUpper ();
}

//------------------------------------------------------------------------------
//
void QNumericEdit::applyLimits ()
{
   // Recalculated allowed min/max range.
   //
   this->minimumMin = this->calcLower ();
   this->maximumMax = this->calcUpper ();

   // Restrict min and max as required.
   //
   this->mMinimum = MAX (this->mMinimum, this->minimumMin);
   this->mMaximum = MIN (this->mMaximum, this->maximumMax);

   this->internalSetValue (this->getValue ());   // Set value forces min/max limits.
}

//------------------------------------------------------------------------------
// Property functions.
//------------------------------------------------------------------------------
//
void  QNumericEdit::setPrefix (const QString &prefix)
{
   this->mPrefix = prefix;
   this->redisplayText ();
}

//------------------------------------------------------------------------------
//
QString  QNumericEdit::getPrefix () const
{
   return this->mPrefix;
}


//------------------------------------------------------------------------------
//
void  QNumericEdit::setSuffix (const QString &suffix)
{
   this->mSuffix = suffix;
   this->redisplayText ();
}

//------------------------------------------------------------------------------
//
QString  QNumericEdit::getSuffix () const
{
   return this->mSuffix;
}

//------------------------------------------------------------------------------
//
QString QNumericEdit::getCleanText () const
{
   return this->getFormattedText (this->mValue);
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setLeadingZeros (const int value)
{
   this->mLeadingZeros = LIMIT (value, 0, this->maximumSignificance ());
   if (this->mNotation == Scientific) this->mLeadingZeros = 1;

   // Reduce precision so as not to exceed max significance if required.
   //
   this->mPrecision = MIN (this->mPrecision, this->maximumSignificance () - this->mLeadingZeros);

   this->applyLimits ();
   this->redisplayText ();
}

//------------------------------------------------------------------------------
//
int QNumericEdit::getLeadingZeros () const
{
   return this->mLeadingZeros;
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setPrecision (const int value)
{
   this->mPrecision = LIMIT (value, 0, this->maximumSignificance ());

   // Reduce precision so as not to exceed max significance if required.
   //
   this->mLeadingZeros = MIN (this->mLeadingZeros, this->maximumSignificance () - this->mPrecision);
   if (this->mNotation == Scientific) this->mLeadingZeros = 1;

   this->applyLimits ();
   this->redisplayText ();
}

//------------------------------------------------------------------------------
//
int QNumericEdit::getPrecision () const
{
   return this->mPrecision;
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setNotation (const Notations notationIn )
{
   this->mNotation = notationIn;
   if (this->mNotation == Scientific) {
      // Scientific notation implies decimal notation.
      //
      this->fpr.setRadix (QEFixedPointRadix::Decimal);
      this->mLeadingZeros = 1;
   }
   this->redisplayText ();
}

//------------------------------------------------------------------------------
//
QNumericEdit::Notations QNumericEdit::getNotation () const
{
   return this->mNotation;
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setMinimum (const double value)
{
   this->mMinimum = LIMIT (value, this->minimumMin, this->maximumMax);

   // Ensure consistant
   //
   this->mMaximum = LIMIT (this->mMaximum, this->mMinimum, this->maximumMax);

   this->internalSetValue (this->getValue ());   // Set value forces min/max limits.
   this->redisplayText ();
}

//------------------------------------------------------------------------------
//
double QNumericEdit::getMinimum () const
{
   return this->mMinimum;
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setMaximum (const double value)
{
   this->mMaximum = LIMIT (value, this->minimumMin, this->maximumMax);

   // Ensure consistant
   //
   this->mMinimum = LIMIT (this->mMinimum, this->minimumMin, this->mMaximum);

   this->internalSetValue (this->getValue ());   // Set value forces min/max limits.
   this->redisplayText ();
}

//------------------------------------------------------------------------------
//
double QNumericEdit::getMaximum () const
{
   return this->mMaximum;
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setRadix (const QEFixedPointRadix::Radicies value)
{
   if (value != QEFixedPointRadix::Decimal) {
      // Not decimal - force fixed point notation.
      //
      this->setNotation (Fixed);
   }

   if (this->fpr.getRadix () != value) {
      this->fpr.setRadix (value);

      this->applyLimits ();
      this->redisplayText ();
   }
}

//------------------------------------------------------------------------------
//
QEFixedPointRadix::Radicies QNumericEdit::getRadix () const
{
   return this->fpr.getRadix ();
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setSeparator (const QEFixedPointRadix::Separators value)
{
   if (this->fpr.getSeparator () != value) {
      this->fpr.setSeparator (value);
      this->redisplayText ();
   }
}

//------------------------------------------------------------------------------
//
QEFixedPointRadix::Separators QNumericEdit::getSeparator () const
{
   return this->fpr.getSeparator ();
}

//------------------------------------------------------------------------------
//
void QNumericEdit::internalSetValue (const double value)
{
   double constrainedValue;
   constrainedValue = LIMIT (value, this->mMinimum, this->mMaximum);

   // Exponent limited to two digits.
   //
   if ((constrainedValue > -1.0e-99) && (constrainedValue < +1.0e-99)) {
      constrainedValue = 0.0;
   }

   // If value the same then nothing to do, no signal to emit. This is the
   // behaviour of Qt own combo box, spin edit etc. We try to be consistant.
   //
   if (this->mValue != constrainedValue) {
      this->mValue = constrainedValue;
      // This prevents infinite looping in the case of cyclic connections.
      //
      if (!this->emitValueChangeInhibited) {
         emit valueChanged (this->mValue);
         emit valueChanged (int (this->mValue));   // range check?
      }
      this->redisplayText ();
   }
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setValue (const double value)
{
   // This prevents infinite looping in the case of cyclic connections.
   //
   this->emitValueChangeInhibited = true;
   this->internalSetValue (value);
   this->emitValueChangeInhibited = false;
}

//------------------------------------------------------------------------------
//
double QNumericEdit::getValue () const
{
   return this->mValue;
}

//------------------------------------------------------------------------------
//
void QNumericEdit::setValue (const int value) {
   this->setValue (double (value));
}

// end
