/*  QEColourBandList.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2015 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
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
