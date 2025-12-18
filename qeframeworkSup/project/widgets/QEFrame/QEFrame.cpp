/*  QEFrame.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include <QEFrame.h>
#include <QPainter>

//------------------------------------------------------------------------------
//
QEFrame::QEFrame (QWidget * parent):QFrame (parent), QEWidget (this), QEManagePixmaps ()
{
   // These are the settings for a drag-and-dropped QFrame within designer, but
   // not set in the constructor, so we don't automatically inherit these settings.
   //
   this->setFrameShape (QFrame::StyledPanel);
   this->setFrameShadow (QFrame::Raised);

   // This is not an EPICS aware widget.
   //
   this->setVariableAsToolTip (false);
   this->setAllowDrop (false);
   this->setNumVariables (0);

   this->selectedPixmapIndex = 0;    // first pixmap selected.
}

//------------------------------------------------------------------------------
//
QEFrame::~QEFrame ()
{
}

//------------------------------------------------------------------------------
//
QSize QEFrame::sizeHint () const
{
   return QSize (120, 80);
}

//------------------------------------------------------------------------------
// Manage background pixmap if required
void QEFrame::paintEvent (QPaintEvent * event)
{
   // Do base class paint
   QFrame::paintEvent (event);

   QPixmap pixmap = this->getDataPixmap (this->selectedPixmapIndex);

   // Nothing more to do if no pixmap
   if (pixmap.isNull ()) {
      return;
   }

   // Determine the area to draw into
   // If scaling, draw into the entire frame
   // If not scaling, draw into a rectangle the same size as the pixmap
   QRect area;
   if (this->scaledContents) {
      area.setSize (size ());
   } else {
      area.setLeft ((size ().width () - pixmap.size ().width ()) / 2);
      area.setTop ((size ().height () - pixmap.size ().height ()) / 2);
      area.setSize (pixmap.size ());
   }

   // Draw the pixmap
   QPainter p (this);
   p.setRenderHints (QPainter::Antialiasing);
   p.drawPixmap (area, pixmap);
}


//------------------------------------------------------------------------------
// The base class has had the index-th pixmap update.
//
void QEFrame::pixmapUpdated (const int index)
{
   // Is the updated oixmap the selected pixmap...
   //
   if (index == this->selectedPixmapIndex) {
      this->update ();
   }
}

//------------------------------------------------------------------------------
// Set pixmap index 0-7 (or -1 for no pixmap.
//
void QEFrame::setSelectPixmap (const int indexIn)
{
   if (this->selectedPixmapIndex != indexIn) {
      this->selectedPixmapIndex = indexIn;
      this->update ();
   }
}

//------------------------------------------------------------------------------
//
int QEFrame::getSelectedPixmap () const
{
   return this->selectedPixmapIndex;
}


//------------------------------------------------------------------------------
// Set the flag used to indicate the background is to be scaled to fit the frame.
// Similar operation to scaledContents property for a QLabel
void QEFrame::setScaledContents (bool scaledContentsIn)
{
   if (this->scaledContents != scaledContentsIn) {
      this->scaledContents = scaledContentsIn;
      this->update ();
   }
}

//------------------------------------------------------------------------------
// Get the flag used to indicate the background is to be scaled to fit the frame.
// Similar operation to scaledContents property for a QLabel.
bool QEFrame::getScaledContents () const
{
   return this->scaledContents;
}

// end
