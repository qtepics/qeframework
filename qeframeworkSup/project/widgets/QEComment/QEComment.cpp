/*  QEComment.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2025-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
