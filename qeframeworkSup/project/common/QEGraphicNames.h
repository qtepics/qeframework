/*  QEGraphicNames.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2019 Australian Synchrotron.
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

#ifndef QE_GRAPHIC_NAMES_H
#define QE_GRAPHIC_NAMES_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QVector>
#include <QString>

#include <persistanceManager.h>
#include <QEFrameworkLibraryGlobal.h>

class QEGraphic;        // differed declaration
class QEGraphicMarkup;  // differed declaration

// We use a QObject class as opposed to a namespace as this allows us leverage
// off the meta object compiler output, specifically allows us to use the
// enumToString and stringToEnum functions in QEUtilities.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEGraphicNames : public QObject
{
   Q_OBJECT
public:
   typedef QVector<double> DoubleVector;

   // Controls the mode of operation of the drawText functions.
   //
   enum TextPositions {
      RealWorldPosition,
      PixelPosition
   };

   // Determines how the associated value is used to find and estimated major value.
   // The estimated value is then rounded to something more appropriate if needs be.
   //
   enum AxisMajorIntervalModes {
      SelectByValue,   // estimated major interval = (max - min) / value.
      SelectBySize,    // estimated major interval = ((max - min) / (widget size / value)),
                       // i.e. value represents major interval expressed as a pixel size
      UserInterval     // Use exact value give by the user.
   };

   // Markup selection enumeration values and associated flags.
   //
   Q_ENUMS (Markups)

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

   // This function returns the image, as a QString, of a enumeration value.
   // An invalid enumeration value returns a null string.
   //
   static QString markupToString (const Markups value);

   // This function returns the enumeration value given an enumeration image.
   // The image must be exact match including case. The only tolerance allowed
   // for is that the image is trimmed.
   // An invalid image cause this function to return -1.
   // The caller may also specify and check ok.
   //
   static Markups stringToMarkup (const QString& image, bool* ok = 0);

   // Provide a mapping from Markups enum to actual mark up object.
   // We use a map (as opposed to a hash) because the iteration order is
   // predictable and consistant.
   //
   typedef QMap <Markups, QEGraphicMarkup*> QEGraphicMarkupsSets;
   typedef QList<Markups> MarkupLists;

   // Create a set of available markups.
   //
   static QEGraphicMarkupsSets* createGraphicMarkupsSet (QEGraphic* owner);

   // Cleans the markupsSet, but does not delete the object itself..
   //
   static void cleanGraphicMarkupsSet (QEGraphicMarkupsSets& markupsSet);

   // Save/restore a markup configuration
   //
   static void saveConfiguration (QEGraphicMarkupsSets& markupsSet, PMElement& parentElement);
   static void restoreConfiguration (QEGraphicMarkupsSets& markupsSet, PMElement& parentElement);

protected:
   explicit QEGraphicNames ();
   virtual ~QEGraphicNames ();
};

Q_DECLARE_OPERATORS_FOR_FLAGS (QEGraphicNames::MarkupFlags)

#endif  // QE_GRAPHIC_NAMES_H
