/*  QEFloatingFormatting.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2023 Australian Synchrotron
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

// Provides floating (double) values for QEFloating data.

#ifndef QE_FLOATING_FORMATTING_H
#define QE_FLOATING_FORMATTING_H

#include <QString>
#include <QVariant>

#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEFloatingFormatting {
public:
   // Construction
   explicit QEFloatingFormatting();
   ~QEFloatingFormatting();

   //===============================================
   // Main functions of this class:
   //   - Format a double based on a value
   //   - Translate a double and generate a value
   //===============================================
   /// This is used to convert the QVariant value received from a QCaObject,
   /// which is still based on the data variable type, to a double.
   double formatFloating( const QVariant &value ) const;
   double formatFloating( const QVariant &value, int arrayIndex ) const;

   /// This is used to convert the QVariant value received from a QCaObject,
   /// which is still based on the data variable type, to a double array.
   /// Typically used where the input QVariant value is an array of data values,
   /// but will work for any QVariant type.
   QVector<double> formatFloatingArray( const QVariant &value ) const;

   QVariant formatValue( const double &floatingValue) const;
   QVariant formatValue( const QVector<double> &floatingValue) const;

private:
   // Utility function to convert variant to double.
   double varToDouble( const QVariant& item ) const;

   // Error reporting
   double formatFailure( QString message ) const;
};

#endif // QE_FLOATING_FORMATTING_H
