/*  QEFixedPointRadix.h
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

#ifndef QE_FIXED_POINT_RADIX_H
#define QE_FIXED_POINT_RADIX_H

#include <QObject>
#include <QString>
#include <QEEnums.h>
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
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEFixedPointRadix : public QObject
{
   Q_OBJECT
public:
   explicit QEFixedPointRadix (QObject* parent = 0);  // default is Decimal, NoSeparator
   explicit QEFixedPointRadix (const QE::Radicies radix,
                               const QE::Separators separator,
                               QObject* parent = 0);
   ~QEFixedPointRadix();

   void setRadix (const QE::Radicies radix);
   QE::Radicies getRadix () const;

   void setSeparator (const QE::Separators separator);
   QE::Separators getSeparator () const;

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
   QE::Radicies radix;
   QE::Separators separator;
};

#endif // QE_FIXED_POINT_RADIX_H
