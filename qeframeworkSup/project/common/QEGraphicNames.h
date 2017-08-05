/*  QEGraphicNames.h
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
 *  Copyright (c) 2013,2014,2016 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_GRAPHIC_NAMES_H
#define QE_GRAPHIC_NAMES_H

#include <QVector>
#include <QEFrameworkLibraryGlobal.h>

// Define grapjic specific names (enumerations). We use a class as opposed to
// a namespace so that QEGraphic and QEGraphicMarkup classes can just inherit
// from this class.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEGraphicNames {
public:

   typedef QVector<double> DoubleVector;

   // Controls the mode of operation of the drawText functions.
   //
   enum TextPositions {
      RealWorldPosition,
      PixelPosition
   };

   // Determines how the associated value is used to find and estimated major value.
   // The estimated value is then rounded to something more appropriate
   //
   enum AxisMajorIntervalModes {
      SelectByValue,   // estimated major interval = (max - min) / value.
      SelectBySize };  // estimated major interval = ((max - min) / (widget size / value)),
                       // i.e. value represents major interval expressed as a pixel size

   // Markup selection enumeration values and associated flags.
   //
   enum Markups {
      None              = 0x0000,
      Area              = 0x0001,
      Line              = 0x0002,
      CrossHair         = 0x0004,
      HorizontalLine_1  = 0x0010,
      HorizontalLine_2  = 0x0020,
      HorizontalLine_3  = 0x0040,
      HorizontalLine_4  = 0x0080,
      VerticalLine_1    = 0x0100,
      VerticalLine_2    = 0x0200,
      VerticalLine_3    = 0x0400,
      VerticalLine_4    = 0x0800,
      Box               = 0x1000
   };

   Q_DECLARE_FLAGS (MarkupFlags, Markups)
   //
   // If number of builtin markups ever exceeds 32 we can change this to a list
   // or hash/map of markups The associated operator declaration is at end of
   // header outside of class.

protected:
   explicit QEGraphicNames () { }
};

Q_DECLARE_OPERATORS_FOR_FLAGS (QEGraphicNames::MarkupFlags)

#endif  // QE_GRAPHIC_NAMES_H
