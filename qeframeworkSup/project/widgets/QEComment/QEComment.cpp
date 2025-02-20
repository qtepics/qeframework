/*  QEComment.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2025 Australian Synchrotron
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

#include "QEComment.h"
#include <QDebug>
#include <QPainter>
#include <QEWidget.h>

#include <QLabel>

#define DEBUG qDebug () << "QEComment" << __LINE__ << __FUNCTION__ << "  "

//==============================================================================
// QEComment
//==============================================================================
//
// Constructor with no initialisation
//
QEComment::QEComment (QWidget *parent) :
   QWidget (parent)
{
   this->commentText.clear();
   this->isRunVisible = false;

   this->setMinimumSize (16, 16);
   this->setMaximumSize (64, 64);

   QFont f = this->font();
   f.setBold (true);
   f.setPointSize (12);
   this->setFont (f);
}

//------------------------------------------------------------------------------
//
QEComment::~QEComment () { }

//------------------------------------------------------------------------------
//
QSize QEComment::sizeHint ()
{
   return QSize (16, 16);
}

//------------------------------------------------------------------------------
//
void QEComment::setComment (const QString& textIn)
{
   this->commentText = textIn;
}

//------------------------------------------------------------------------------
//
QString QEComment::getComment () const
{
   return this->commentText;
}

//------------------------------------------------------------------------------
//
void QEComment::setRunVisible (const bool flagIn)
{
   const bool inDesigner = QEWidget::inDesigner();
   this->setVisible (flagIn | inDesigner);
   this->isRunVisible = flagIn;
   this->update();
}

//------------------------------------------------------------------------------
//
bool QEComment::getRunVisible() const
{
   return this->isRunVisible;
}

//------------------------------------------------------------------------------
//
void QEComment::paintEvent (QPaintEvent* /* event */)
{
   const bool inDesigner = QEWidget::inDesigner();
   if (!this->isRunVisible && !inDesigner) return; // paint nothing

   QPainter painter (this);
   QPen pen;
   QBrush brush;

   pen.setWidth (1);
   pen.setStyle (Qt::SolidLine);
   pen.setColor (QColor (0x000040));
   painter.setPen (pen);

   brush.setStyle (Qt::SolidPattern);
   brush.setColor (QColor (0x0000a0));
   painter.setBrush (brush);
   painter.drawRect (0, 0, this->width () - 1, this->height () - 1);

   pen.setColor (QColor (0xffff00));
   painter.setPen (pen);

   painter.drawText (this->width()/2 - 6, this->height()/2 + 6, "C");
}

// end
