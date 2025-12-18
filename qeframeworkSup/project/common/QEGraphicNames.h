/*  QEGraphicNames.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
   enum Markups {
      None               = 0x000000,
      Area               = 0x000001,   // Draws rectangle from origin to current mouse position.
      Line               = 0x000002,   // Draws a line from origin to current mouse position.
      CrossHair          = 0x000004,   // Draws crosshairs about to current (mouse) position.
      HorizontalLine_1   = 0x000010,   // Draws horizontal line through current position.
      HorizontalLine_2   = 0x000020,   // Ditto
      HorizontalLine_3   = 0x000040,   // Ditto
      HorizontalLine_4   = 0x000080,   // Ditto
      VerticalLine_1     = 0x000100,   // Draws vertical line through current position.
      VerticalLine_2     = 0x000200,   // Ditto
      VerticalLine_3     = 0x000400,   // Ditto
      VerticalLine_4     = 0x000800,   // Ditto
      Box                = 0x001000,   // Draws a box around the position.
      HorizontalMarker_1 = 0x002000,   // Draws horizontal line - no mouse control.
      HorizontalMarker_2 = 0x004000,   // Ditto
      VerticalMarker_1   = 0x008000,   // Draws vertical line - no mouse control.
      VerticalMarker_2   = 0x010000    // Ditto
   };
   Q_ENUM (Markups)
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
