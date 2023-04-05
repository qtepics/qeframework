/*  QECalcout.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2018-2022 Australian Synchrotron
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

#include "QECalcout.h"
#include <QDebug>
#include <QECommon.h>
#include <postfix.h>   // out of EPICS

#define DEBUG  qDebug () << "QECalcout" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QECalcout::QECalcout (QWidget* parent) :
   QLabel (parent),
   QEWidget (this)
{
   // Set default properties
   //
   for (int a = 0; a < ARRAY_LENGTH (this->args); a++) {
      this->args [a] = 0.0;
   }

   // Don't display this widget, by default (will always display in 'designer')
   // This is the only QE feature we are using which allows normally hiddden
   // widgets to be available in designer.
   //
   this->setRunVisible (false);

   this->setAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
   this->setText ("Calcout");
   this->setIndent (6);
   this->setStyleSheet (QEUtilities::offBackgroundStyle());

   this->dopt = Use_CALC;
   this->oopt = Every_Time;
   this->egu  = "";
   this->prec = 3;
   this->format = 'f';
   this->lastCalcValue = 0.0;

   // We use the set functions to initialise the exprerssions
   // They do all the smart work.
   // Note "" is not a valid expression.
   //
   this->setCalc ("0");
   this->setOcal ("0");

   // Lastly clear the is processing flag.
   //
   this->isProcessing = false;
}

//------------------------------------------------------------------------------
// Destructor - place holder
//
QECalcout::~QECalcout () { }

//------------------------------------------------------------------------------
// Emit signals in various formats.
// This is, except for the string signal, a direct crib from QELink.
// This makes for consistency.
//
void QECalcout::emitValue (const QVariant& value)
{
   emit this->out (value.toBool ());
   emit this->out (value.toInt ());
   emit this->out (long (value.toLongLong ()));
   emit this->out (value.toLongLong ());
   emit this->out (value.toDouble ());

   const char f = this->format.toLatin1 ();
   QString image = QString::number (value.toDouble (), f, this->prec);
   if (!this->egu.isEmpty ()) {
      image.append (" ").append (this->egu);
   }

   emit this->out (image);
}

//------------------------------------------------------------------------------
// By using an inner function, the inner function may return at any time
// and the outer function can clear the is-proccesing flag.
//
void QECalcout::processInner ()
{
   // Perform expression validation.
   //
   if (!this->calcExpressionIsValid) return;
   if ((this->dopt == Use_OCAL) && !this->ocalExpressionIsValid) return;

   // Perform evaluation and evaluation validation.
   //
   double calcValue = 0.0;
   double ocalValue = 0.0;
   long status;

   status = calcPerform (this->args, &calcValue, this->calcPostFix);
   if (status != 0) return;

   if (this->dopt == Use_OCAL) {
       status = calcPerform (this->args, &ocalValue, this->ocalPostFix);
       if (status != 0) return;
   }

   // All good to go.
   // Set own text - incase run time visible.
   //
   const char f = this->format.toLatin1 ();
   QString image = QString::number (calcValue, f, this->prec);
   if (!this->egu.isEmpty ()) {
      image.append (" ").append (this->egu);
   }
   this->setText (image);

   bool outputNow = false;

   switch (this->oopt) {
      case Every_Time:
         outputNow = true;
         break;

      case On_Change:
         // Note: we have no mdel equivilent.
         //
         outputNow = (calcValue != this->lastCalcValue);
         break;

      case When_Zero:
         outputNow = (calcValue == 0.0);
         break;

      case When_Non_zero:
         outputNow = (calcValue != 0.0);
         break;

      case Transition_To_Zero:
         outputNow = (calcValue == 0.0) && (this->lastCalcValue != 0.0);
         break;

      case Transition_To_Non_zero:
         outputNow = (calcValue != 0.0) && (this->lastCalcValue == 0.0);
         break;

      case Never:
         outputNow = false;
         break;
   }

   // Save the current value for the next time we process.
   //
   this->lastCalcValue = calcValue;

   if (!outputNow) return;

   if (this->dopt == Use_CALC) {
      QVariant value (calcValue);   // convert to variant
      this->emitValue (value);      // and emit

   } else if (this->dopt == Use_OCAL) {
      QVariant value (ocalValue);   // convert to variant
      this->emitValue (value);      // and emit
   }
}

//------------------------------------------------------------------------------
//
void QECalcout::process ()
{
   // Avoid infinite signal-slot loops.
   //
   if (!this->isProcessing) {
      this->isProcessing = true;
      this->processInner ();
      this->isProcessing = false;
   }
}

//------------------------------------------------------------------------------
// Keep in step with setOcal (below)
//
void QECalcout::setCalc (const QString& calcIn)
{
   this->calc = calcIn;

   if (this->calc.length() > MAX_POSTFIX_SIZE) {
      this->calcExpressionIsValid = false;
      this->calcExpressionStatus = tr("Expression too long");
      return;
   }

   // Convert to a POD string.
   //
   const char* pinfix;
   pinfix = this->calc.toStdString().c_str();

   short error = 0;
   long status = postfix (pinfix, this->calcPostFix, &error);

   this->calcExpressionIsValid = (status == 0);

   if (this->calcExpressionIsValid) {
      this->calcExpressionStatus = tr("okay");
   } else {
      this->calcExpressionStatus = QString (calcErrorStr (error));
   }
}

//------------------------------------------------------------------------------
//
QString QECalcout::getCalc () const
{
   return this->calc;
}

//------------------------------------------------------------------------------
// We need a setter function in order for the calc property to work.
//
void QECalcout::setCalcStatus (const QString&) { }

//------------------------------------------------------------------------------
//
QString QECalcout::getCalcStatus () const
{
   return this->calcExpressionStatus;
}

//------------------------------------------------------------------------------
// Keep in step with setCalc (above)
//
void QECalcout::setOcal (const QString& ocalIn)
{
   this->ocal = ocalIn;

   if (this->ocal.length() > MAX_POSTFIX_SIZE) {
      this->ocalExpressionIsValid = false;
      this->ocalExpressionStatus = tr("Expression too long");
      return;
   }

   // Convert to a POD string.
   //
   const char* pinfix;
   pinfix = this->ocal.toStdString().c_str();

   short error = 0;
   long status = postfix (pinfix, this->ocalPostFix, &error);

   this->ocalExpressionIsValid = (status == 0);

   if (this->ocalExpressionIsValid) {
      this->ocalExpressionStatus = tr("okay");
   } else {
      this->ocalExpressionStatus = QString (calcErrorStr (error));
   }
}

//------------------------------------------------------------------------------
//
QString QECalcout::getOcal () const
{
   return this->ocal;
}

//------------------------------------------------------------------------------
// We need a setter function in order for the ocal property to work.
//
void QECalcout::setOcalStatus (const QString&) { }

//------------------------------------------------------------------------------
//
QString QECalcout::getOcalStatus () const
{
   return this->ocalExpressionStatus;
}

//------------------------------------------------------------------------------
//
void QECalcout::setDopt (const DataOptions doptIn)
{
   this->dopt = doptIn;
}

//------------------------------------------------------------------------------
//
QECalcout::DataOptions QECalcout::getDopt () const
{
   return this->dopt;
}

//------------------------------------------------------------------------------
//
void QECalcout::setOopt (const OutputOptions ooptIn)
{
   this->oopt = ooptIn;
}

//------------------------------------------------------------------------------
//
QECalcout::OutputOptions QECalcout::getOopt () const
{
   return this->oopt;
}

//------------------------------------------------------------------------------
//
void QECalcout::setEgu (const QString& eguIn)
{
   this->egu = eguIn;
}

//------------------------------------------------------------------------------
//
QString QECalcout::getEgu () const
{
   return this->egu;
}

//------------------------------------------------------------------------------
//
void QECalcout::setPrec (const int precIn)
{
   this->prec = LIMIT (precIn, 0, 15);
}

//------------------------------------------------------------------------------
//
int QECalcout::getPrec () const
{
   return this->prec;
}

//------------------------------------------------------------------------------
//
void QECalcout::setFormat (const QChar formatIn)
{
   this->format = formatIn;

   // Ensure the format char is sensible.
   //
   if (this->format != 'e' && this->format != 'E' &&
       this->format != 'g' && this->format != 'G' &&
       this->format != 'f') {
      this->format = 'f';
   }
}

//------------------------------------------------------------------------------
//
QChar QECalcout::getFormat () const
{
   return this->format;
}

//------------------------------------------------------------------------------
// Macro implementation of 12 sets of input slots and value functions.
//
#define INPX_GETX(name, offset)                  \
void QECalcout::inp##name (const double v)       \
{                                                \
   this->args [offset] = v;                      \
   this->process ();                             \
}                                                \
                                                 \
void QECalcout::inp##name (const int v)          \
{                                                \
   this->args [offset] = double (v);             \
   this->process ();                             \
}                                                \
                                                 \
void QECalcout::inp##name (const bool v)         \
{                                                \
   this->args [offset] = double (v);             \
   this->process ();                             \
}                                                \
                                                 \
double QECalcout::get##name () const             \
{                                                \
   return this->args [offset];                   \
}

INPX_GETX (a, 0);
INPX_GETX (b, 1);
INPX_GETX (c, 2);
INPX_GETX (d, 3);
INPX_GETX (e, 4);
INPX_GETX (f, 5);
INPX_GETX (g, 6);
INPX_GETX (h, 7);
INPX_GETX (i, 8);
INPX_GETX (j, 9);
INPX_GETX (k, 10);
INPX_GETX (l, 11);

#undef INPX_GETX

// end
