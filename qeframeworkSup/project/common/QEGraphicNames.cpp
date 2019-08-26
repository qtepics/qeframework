/*  QEGraphicNames.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2019 Australian Synchrotron.
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
 *    andrews@ansto.gov.au
 */

#include "QEGraphicNames.h"
#include <QEGraphicMarkup.h>
#include <QDebug>
#include <QECommon.h>

#define DEBUG qDebug () << "QEGraphicNames" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QEGraphicNames::QEGraphicNames () : QObject (NULL) { }

//------------------------------------------------------------------------------
//
QEGraphicNames::~QEGraphicNames () { }

//------------------------------------------------------------------------------
// static
QEGraphicNames::QEGraphicMarkupsSets* QEGraphicNames::createGraphicMarkupsSet (QEGraphic* owner)
{
   QEGraphicMarkupsSets* result;

   // Construct markups and insert into marksup set.
   //
   result = new QEGraphicMarkupsSets ();
   result->insert (Area,             new QEGraphicAreaMarkup (owner));
   result->insert (Line,             new QEGraphicLineMarkup (owner));
   result->insert (Box,              new QEGraphicBoxMarkup (owner));
   result->insert (CrossHair,        new QEGraphicCrosshairsMarkup (owner));

   // There are multiple instances of each type - we need to be explicit.
   //
   result->insert (HorizontalLine_1, new QEGraphicHorizontalMarkup (HorizontalLine_1, owner));
   result->insert (HorizontalLine_2, new QEGraphicHorizontalMarkup (HorizontalLine_2, owner));
   result->insert (HorizontalLine_3, new QEGraphicHorizontalMarkup (HorizontalLine_3, owner));
   result->insert (HorizontalLine_4, new QEGraphicHorizontalMarkup (HorizontalLine_4, owner));
   result->insert (VerticalLine_1,   new QEGraphicVerticalMarkup (VerticalLine_1, owner));
   result->insert (VerticalLine_2,   new QEGraphicVerticalMarkup (VerticalLine_2, owner));
   result->insert (VerticalLine_3,   new QEGraphicVerticalMarkup (VerticalLine_3, owner));
   result->insert (VerticalLine_4,   new QEGraphicVerticalMarkup (VerticalLine_4, owner));

   return result;
}

//------------------------------------------------------------------------------
// static
void QEGraphicNames::cleanGraphicMarkupsSet (QEGraphicMarkupsSets& markupsSet)
{
   const MarkupLists keys = markupsSet.keys ();
   for (int j = 0; j < keys.count (); j++) {
      QEGraphicMarkup* graphicMarkup = markupsSet.value (keys.value (j), NULL);
      if (graphicMarkup) {
         delete graphicMarkup;
      }
   }
   markupsSet.clear ();
}

//------------------------------------------------------------------------------
// static
void QEGraphicNames::saveConfiguration (QEGraphicMarkupsSets& markupsSet,
                                        PMElement& parentElement)
{
   const MarkupLists keys = markupsSet.keys ();

   if (parentElement.isNull ()) return;
   PMElement markupsElement = parentElement.addElement ("markups");
   if (markupsElement.isNull ()) return;

   for (int j = 0; j < keys.count (); j++) {
      QEGraphicMarkup* graphicMarkup = markupsSet.value (keys.value (j), NULL);
      if (graphicMarkup) {
         graphicMarkup->saveConfiguration (markupsElement);
      }
   }
}

//------------------------------------------------------------------------------
// static
void QEGraphicNames::restoreConfiguration (QEGraphicMarkupsSets& markupsSet,
                                           PMElement& parentElement)
{
   const MarkupLists keys = markupsSet.keys ();

   if (parentElement.isNull ()) return;
   PMElement markupsElement = parentElement.getElement ("markups");
   if (markupsElement.isNull ()) return;

   for (int j = 0; j < keys.count (); j++) {
      QEGraphicMarkup* graphicMarkup = markupsSet.value (keys.value (j), NULL);
      if (graphicMarkup) {
         graphicMarkup->restoreConfiguration (markupsElement);
      }
   }
}

//------------------------------------------------------------------------------
//static
QString QEGraphicNames::markupToString (const Markups value)
{
    QEGraphicNames object;
    return QEUtilities::enumToString (object, "Markups", value);
}

//------------------------------------------------------------------------------
//static
QEGraphicNames::Markups QEGraphicNames::stringToMarkup (const QString& image, bool* ok)
{
   QEGraphicNames object;
   int temp;
   temp = QEUtilities::stringToEnum (object, "Markups", image, ok);
   return Markups (temp);
}

// end
