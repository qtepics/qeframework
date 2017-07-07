/*  QEFixedPointRadix.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *   Australian Synchrotron.
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
 *  Copyright (c) 2014 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_FIXED_POINT_RADIX_H
#define QE_FIXED_POINT_RADIX_H

#include <QObject>
#include <QString>
#include <QEFrameworkLibraryGlobal.h>

/// The QString class provides general value to/from radix string representation,
/// but only for integer types. This class provdies similar functionality for
/// the double type to/from a fixed point radix representation.
/// The class restricts the allowed radix values to 2, 8, 10 and 16.
/// The class allows the for the insertion of 'thousands' separators, typically
/// comma, but space and under score are also allowed.
//
// Refactored out of QENumericEdit.
// This class is QObject mainly to allow Radicies/Separators to become part of the
// meta object system.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEFixedPointRadix : public QObject {
   Q_OBJECT
public:

   /// Defines allowed radix values.
   enum Radicies { Decimal = 0, Hexadecimal, Octal, Binary };
   Q_ENUMS (Radicies)

   /// Defines the digit 'thousands' separator character.
   enum Separators { None = 0, Comma, Underscore, Space };
   Q_ENUMS (Separators)

   explicit QEFixedPointRadix (QObject* parent = 0);  // default is Decimal, None
   explicit QEFixedPointRadix (const Radicies radix, const Separators separator, QObject* parent = 0);
   ~QEFixedPointRadix();

   void setRadix (const Radicies radix);
   Radicies getRadix () const;

   void setSeparator (const Separators separator);
   Separators getSeparator () const;

   int getRadixValue () const;   // i.e. 10, 16, 8 or 2.

   // Returns true if qc is a vaid digit for the current radix setting.
   //
   bool isRadixDigit (const QChar qc) const;

   // Provides double to QString.
   //
   QString toString (const double value,
                     const bool sign,
                     const int zeros,
                     const int precision) const;

   // Provides QString to double.
   // Returned value only meaningful if okay set true.
   //
   double toValue (const QString& image,
                   bool& okay) const;

private:
   Radicies radix;
   Separators separator;
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEFixedPointRadix::Radicies)
Q_DECLARE_METATYPE (QEFixedPointRadix::Separators)
#endif

#endif // QE_FIXED_POINT_RADIX_H
