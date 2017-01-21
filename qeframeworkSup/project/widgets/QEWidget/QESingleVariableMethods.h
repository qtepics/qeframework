/*  QESingleVariableMethods.h
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

#ifndef QE_SINGLE_VARIABLE_METHODS_H
#define QE_SINGLE_VARIABLE_METHODS_H

#include <QString>
#include <QEPluginLibrary_global.h>
#include <QCaVariableNamePropertyManager.h>
#include <QCaObject.h>

class QEWidget;   // differed

// This class intended to be used as:
//
// class QEPLUGINLIBRARYSHARED_EXPORT YourWidget :
//     public QBaseWidget,
//     public QEWidget,
//     public QESingleVariableMethods { .... };
//
// And supports the standard set of Single Variable Properties, viz:
//
//   QString variable
//   QString variableSubstitutions
//   int arrayIndex
//
// Use of this class by inheritance does not preclude a QE widget have more than one variable.
// Also a second, or third, variable may be manged by adding additional instance(s) of this
// class by inclusion as well as by inheritance.
//
// Examples: QEAnalogSlider and QESimpleShape
//
class QEPLUGINLIBRARYSHARED_EXPORT QESingleVariableMethods {
public:
   // Typically for a single variable widget, we would expect the PV to use
   // variable index slot 0. We encourage, but do not enforce this.
   //
   explicit QESingleVariableMethods (QEWidget* owner, unsigned int variableIndex = 0);
   virtual ~QESingleVariableMethods ();

   /// Property access function for #variable property.
   /// This has special behaviour to work well within designer.
   ///
   void setVariableNameProperty (const QString& variableName);

   /// Property access function for #variable property.
   /// This has special behaviour to work well within designer.
   ///
   QString getVariableNameProperty () const;

   /// Property access function for #variableSubstitutions property.
   /// This has special behaviour to work well within designer.
   ///
   void setVariableNameSubstitutionsProperty (const QString& substitutions);

   /// Property access function for #variableSubstitutions property.
   /// This has special behaviour to work well within designer.
   ///
   QString getVariableNameSubstitutionsProperty () const;

   /// Property access function for #arrayIndex property. Array element to access if
   /// variable is an array variable. Defaults to 0, i.e. first element.
   /// arrayIndex value is restricted to be >= 0
   ///
   /// If the assocated qcaobject::QCaObject exists then calls its setArrayIndex function
   /// and then requests that the object resend last data.
   ///
   void setArrayIndex (const int arrayIndex);

   /// Property access function for #arrayIndex property. Array element to access if
   /// variable is an array variable. Defaults to 0, i.e. first element.
   ///
   int getArrayIndex () const;

   /// Connects internal variable name property manager's newVariableNameProperty signal
   /// to the specified slot.
   ///
   /// Example:
   ///    this->connectNewVariableNameProperty
   ///            (SLOT (useNewVariableNameProperty (QString, QString, unsigned int));
   ///
   void connectNewVariableNameProperty (const char* useNameSlot);

   // Conveniance function
   //
   // createQcaItem may call this function, which does:
   //    qca->setArrayIndex (this->getArrayIndex ());
   //
   // The QCaObjects are destroyed and re-created as the name/substitution values change
   // so the array index must be re-applied each time the QCaObjects is created.
   //
   void setQCaArrayIndex (qcaobject::QCaObject* qca);

private:
   QEWidget* owner;
   int arrayIndex;                        // defaults to 0, restricted to >= 0
   QCaVariableNamePropertyManager vnpm;
};

#endif     // QE_SINGLE_VARIABLE_METHODS_H
