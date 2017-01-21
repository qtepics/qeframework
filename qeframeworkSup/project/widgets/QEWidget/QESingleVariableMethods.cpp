/*  QESingleVariableMethods.cpp
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
 *  Copyright (c) 2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>
#include <QObject>
#include <QECommon.h>
#include <QEWidget.h>

#include "QESingleVariableMethods.h"

#define DEBUG qDebug () << "QESingleVariableMethods" << __LINE__ << __FUNCTION__ << "  "

//-----------------------------------------------------------------------------
//
QESingleVariableMethods::QESingleVariableMethods
   (QEWidget* ownerIn, unsigned int variableIndex) :
   owner (ownerIn)
{
   this->arrayIndex = 0;
   this->vnpm.setVariableIndex (variableIndex);
}

//-----------------------------------------------------------------------------
//
QESingleVariableMethods::~QESingleVariableMethods ()
{
}

//-----------------------------------------------------------------------------
//
void QESingleVariableMethods::setVariableNameProperty (const QString& variableName)
{
   this->vnpm.setVariableNameProperty (variableName);
}

//-----------------------------------------------------------------------------
//
QString QESingleVariableMethods::getVariableNameProperty () const
{
   return this->vnpm.getVariableNameProperty ();
}

//-----------------------------------------------------------------------------
//
void QESingleVariableMethods::setVariableNameSubstitutionsProperty (const QString& substitutions)
{
   this->vnpm.setSubstitutionsProperty (substitutions);
}

//------------------------------------------------------------------------------
//
QString QESingleVariableMethods::getVariableNameSubstitutionsProperty () const
{
   return this->vnpm.getSubstitutionsProperty ();
}

//------------------------------------------------------------------------------
//
void QESingleVariableMethods::setArrayIndex (const int arrayIndexIn)
{
   this->arrayIndex = MAX (0, arrayIndexIn);   // must be non-negative

   unsigned int pvIndex = this->vnpm.getVariableIndex();
   qcaobject::QCaObject* qca = this->owner->getQcaItem (pvIndex);
   if (qca) {
      // Apply to qca object and force update
      // Note: we can't just store array Index in the qca object as it may not
      // exist when array index defined.
      //
      qca->setArrayIndex (this->arrayIndex);
      qca->resendLastData ();
   }
}

//------------------------------------------------------------------------------
//
int QESingleVariableMethods::getArrayIndex () const
{
    return this->arrayIndex;
}

//------------------------------------------------------------------------------
//
void QESingleVariableMethods::connectNewVariableNameProperty (const char* useNameSlot)
{
   static const char* newNameSignal =
         SIGNAL (newVariableNameProperty (QString, QString, unsigned int));

   QObject::connect (&this->vnpm, newNameSignal,
                     this->owner->getQWidget (), useNameSlot);
}

//------------------------------------------------------------------------------
//
void QESingleVariableMethods::setQCaArrayIndex (qcaobject::QCaObject* qca)
{
   if (qca) {   // sainity check
      if (qca->getVariableIndex () == this->vnpm.getVariableIndex()) {
         qca->setArrayIndex (this->arrayIndex);
      } else {
         DEBUG << "variable index mismatch qca:" << qca->getVariableIndex ()
               << "  property name:" <<  this->vnpm.getVariableNameProperty ();
      }
   }
}

// end
