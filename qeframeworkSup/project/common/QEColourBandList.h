/*  QEColourBandList.h
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

#ifndef QE_COLOUR_BAND_LIST_H
#define QE_COLOUR_BAND_LIST_H

#include <QColor>
#include <QList>
#include <QEFrameworkLibraryGlobal.h>

namespace qcaobject {
   class QCaObject;   // differed
}

// A colour band element.
// This defines a range of values and an associated colour.
//
struct QEColourBand {
   double lower;
   double upper;
   QColor colour;
};

/// Defines a list of QEColourBands elements.
/// Support class for QAxisPainter
//
// Note: the author would have liked to just inherit from QList<QEColourBands>,
// but this way of specificing a class has issues with the Windows Visual Studio
// Compiler. This class is therefore defined with no paraent class, but with a
// QList<QEColourBand> member, i.e. using compostion rather than inheritance.
// The downside of this is that we must provide list member access functions.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEColourBandList {
public:
   explicit QEColourBandList ();
   ~QEColourBandList ();

   // These functions access the inner container object.
   //
   void clear ();
   void append (const QEColourBand& item);
   int count () const;
   QEColourBand value (int j) const;

   // Conveniance function to set the colour band list based upon alarm levels
   // from within the given QCaObject.
   //
   void setAlarmColours (const double dispLower,
                         const double dispUpper,
                         qcaobject::QCaObject* qca);

private:
   static QEColourBand createColourBand (const double lower, const double upper, const QColor& colour);
   static QEColourBand createColourBand (const double lower, const double upper, const unsigned short severity);

   typedef QList <QEColourBand> QEColourBandQList;

   QEColourBandQList list;
};

#endif  // QE_COLOUR_BAND_LIST_H
