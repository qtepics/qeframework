/*  managePixmaps.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2011-2025 Australian Synchrotron
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

#include "managePixmaps.h"
#include <QDebug>
#include <QECommon.h>

#define DEBUG qDebug () << "managePixmaps" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// constructor
QEManagePixmaps::QEManagePixmaps()
{
   // Initialy set all pixmaps to a blank pixmap
   //
   for (int i = 0; i < NUM_PIXMAPS_MANAGED; i++) {
      static QPixmap blank;
      this->pixmaps.append (blank);
   }
}

//------------------------------------------------------------------------------
// place holder destructor
QEManagePixmaps::~QEManagePixmaps() { }

//------------------------------------------------------------------------------
// place holder function intended to be overiden by sub class if required.
//
void QEManagePixmaps::pixmapUpdated (const int) { }

//------------------------------------------------------------------------------
//
void QEManagePixmaps::setDataPixmap (const QPixmap& pixmap, const int index)
{
   // Sanity check
   if ((index < 0) || (index >= this->pixmaps.count())) return;

   // Save the pixmap
   this->pixmaps[index] = pixmap;

   // Call virtual pixmapUpdated function to allow sub-classes to take any
   // required action if/wghen a pix map changes.
   //
   this->pixmapUpdated (index);
}

//------------------------------------------------------------------------------
//
QPixmap QEManagePixmaps::getDataPixmap (const int index) const
{
   static QPixmap blank;
   return this->pixmaps.value (index, blank);
}

//------------------------------------------------------------------------------
//
QPixmap QEManagePixmaps::getDataPixmap( const QString& text ) const
{
   const QStringList list = QEUtilities::split (text.simplified());

   // Attempt to interpret the text as a floating point number
   bool ok = false;
   double dValue = 0.0;

   if (list.count() >= 1)
      dValue = list[0].toDouble (&ok);

   // Convert any resultant floating point number to a pixmap index.
   //
   int iValue = int (dValue);

   // If the text was interpreted as a floating point number, select and return the pixmap.
   if (ok) {
      return getDataPixmap( iValue );
   } else {
      // The text could not be interpreted as a floating point number, return a blank pixmap.
      //
      static QPixmap blank;
      return blank;
   }
}

// end
