/*  QELink.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2025 Australian Synchrotron
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

#include "QELink.h"
#include <QDebug>
#include <QString>
#include <QVariant>
#include <QECommon.h>

#define DEBUG  qDebug () << "QELink" << __LINE__ << __FUNCTION__ << " "

//------------------------------------------------------------------------------
// Constructor with no initialisation
//
 QELink::QELink (QWidget * parent):QLabel (parent), QEWidget (this)
{
   // Don't display this widget, by default (will always display in 'Designer'
   this->setRunVisible (false);

   // Set default properties
   this->setAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
   this->setText ("Link");
   this->setIndent (6);

   this->setStyleSheet (QEUtilities::offBackgroundStyle ());

   this->signalFalse = true;
   this->signalTrue = true;
   this->isProcessing = false;
   this->lookupValues.clear();

   this->condition = Equal;
}

//------------------------------------------------------------------------------
// Place holder
QELink::~QELink () { }


//------------------------------------------------------------------------------
//
void QELink::sendLookup(const qlonglong index)
{
   if (this->condition != Lookup) return;  // check
   if ((index < 0) || (index >= this->lookupValues.size())) return;  // out of range

   const QVariant value = QVariant (this->lookupValues.value (index));
   this->emitValue (value);
}

//------------------------------------------------------------------------------
// Common comparison. Macro to evaluate the 'in' signal value.
// Determine if the 'in' signal value matches the condition
// If match and signaling on a match, then send a signal
// If not a match and signaling on no match, then send a signal
//
#define EVAL_CONDITION if (okay) {                              \
   bool match = false;                                          \
   switch (this->condition)                                     \
   {                                                            \
      case Equal:              match = (inVal == val); break;   \
      case NotEqual:           match = (inVal != val); break;   \
      case GreaterThan:        match = (inVal >  val); break;   \
      case GreaterThanOrEqual: match = (inVal >= val); break;   \
      case LessThan:           match = (inVal <  val); break;   \
      case LessThanOrEqual:    match = (inVal <= val); break;   \
      case Lookup: return;                                      \
   }                                                            \
   this->sendValue (match);                                     \
}


//------------------------------------------------------------------------------
// Slot to perform a comparison on a bool
void QELink::in (const bool& inVal)
{
   this->sendLookup (inVal);
   bool okay = true;
   const bool val = this->comparisonValue.toBool ();
   EVAL_CONDITION;
}

//------------------------------------------------------------------------------
// Slot to perform a comparison on an integer (int)
void QELink::in (const int &inVal)
{
   this->sendLookup (inVal);
   bool okay;
   const qlonglong val = this->comparisonValue.toLongLong (&okay);
   EVAL_CONDITION;
}

//------------------------------------------------------------------------------
// Slot to perform a comparison on an integer (long)
void QELink::in (const long& inVal)
{
   this->sendLookup (inVal);
   bool okay;
   const qlonglong val = this->comparisonValue.toLongLong (&okay);
   EVAL_CONDITION;
}

//------------------------------------------------------------------------------
// Slot to perform a comparison on an integer (qLongLong)
void QELink::in (const qlonglong& inVal)
{
   this->sendLookup (inVal);
   bool okay;
   const qlonglong val = this->comparisonValue.toLongLong (&okay);
   EVAL_CONDITION;
}

//------------------------------------------------------------------------------
// Slot to perform a comparison on a floating point number
void QELink::in (const double& inVal)
{
   this->sendLookup (inVal);
   bool okay;
   const double val = this->comparisonValue.toDouble (&okay);
   EVAL_CONDITION;
}

//------------------------------------------------------------------------------
// Slot to perform a comparison on a string
//
void QELink::in (const QString& inVal)
{
   // Note: no (direct) sendLookup (inVal) for a non numeric.

   bool stringIsNum = false;

   // If the string starts with a valid number, compare it as a number

   QStringList inList = QEUtilities::split (inVal);
   if (inList.size () > 0) {
      const double inDouble = inList.value(0).toDouble (&stringIsNum);
      if (stringIsNum) {
         this->in (inDouble);
      }
   }

   // If the string is not a valid number, do a string comparison
   if (!stringIsNum) {
      bool okay = true;
      const QString val = this->comparisonValue.toString ();
      EVAL_CONDITION;
   }
}

#undef EVAL_CONDITION

//------------------------------------------------------------------------------
// Generate appropriate signals following a comparison of an input value
//
void QELink::sendValue (bool match)
{
   // If input comparison matched, emit the appropriate value if required
   if (match) {
      if (this->signalTrue)
         this->emitValue (this->outTrueValue);
   }
   // If input comparison did not match, emit the appropriate value if required
   else {
      if (this->signalFalse)
         this->emitValue (this->outFalseValue);
   }
}

//------------------------------------------------------------------------------
// Emit signals required when input value matches or fails to match
//
void QELink::emitValue (const QVariant& value)
{
   // Avoid infinite signal-slot loops.
   //
   if (!this->isProcessing) {
      this->isProcessing = true;

      bool okay;
      emit this->out (value.toBool ());

      const int ival = value.toInt (&okay);
      if (okay) emit this->out (ival);

      const qlonglong lval = value.toLongLong (&okay);
      if (okay) emit this->out (long (lval));
      if (okay) emit this->out (lval);

      const double dval = value.toDouble (&okay);
      if (okay) emit this->out (dval);

      emit this->out (value.toString ());

      this->isProcessing = false;
   }
}

//------------------------------------------------------------------------------
// Slot to allow signal/slot manipulation of the auto fill background
// attribute of the base label class
//
void QELink::autoFillBackground (const bool& enable)
{
   this->setAutoFillBackground (enable);
}

//==============================================================================
// Property convenience functions
//
// condition
void QELink::setCondition (ConditionNames conditionIn)
{
   this->condition = conditionIn;
}

QELink::ConditionNames QELink::getCondition ()
{
   return this->condition;
}

//------------------------------------------------------------------------------
// comparisonValue Value to compare input signals to
//
void QELink::setComparisonValue (const QString& comparisonValueIn)
{
   this->comparisonValue = QVariant (comparisonValueIn);
}

QString QELink::getComparisonValue () const
{
   return this->comparisonValue.toString ();
}

//------------------------------------------------------------------------------
// signalTrue (Signal if condition is met)
void QELink::setSignalTrue (bool signalTrueIn)
{
   this->signalTrue = signalTrueIn;
}

bool QELink::getSignalTrue () const
{
   return this->signalTrue;
}

//------------------------------------------------------------------------------
// signalFalse (Signal if condition not met)
void QELink::setSignalFalse (bool signalFalseIn)
{
   this->signalFalse = signalFalseIn;
}

bool QELink::getSignalFalse () const
{
   return this->signalFalse;
}

//------------------------------------------------------------------------------
// outTrueValue Value to emit if condition is met
void QELink::setOutTrueValue (const QString& outTrueValueIn)
{
   this->outTrueValue = QVariant (outTrueValueIn);
}

QString QELink::getOutTrueValue () const
{
   return this->outTrueValue.toString ();
}

//------------------------------------------------------------------------------
// outFalseValue Value to emit if condition is not met
void QELink::setOutFalseValue (const QString& outFalseValueIn)
{
   this->outFalseValue = QVariant (outFalseValueIn);
}

QString QELink::getOutFalseValue () const
{
   return this->outFalseValue.toString ();
}

//------------------------------------------------------------------------------
//
void QELink::setLookupValues (const QStringList& lookupValuesIn)
{
   this->lookupValues = lookupValuesIn;
}

QStringList QELink::getLookupValues () const
{
   return this->lookupValues;
}

// end
