/*  QEDateTime.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2019-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Zai Wang
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/*!
  This class is a non EPICS aware label widget based on the Qt label widget and
  Qt DateTime class. It is only used for displaying data and time of the day.
 */

#include "QEDateTime.h"
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QMouseEvent>
#include <QECommon.h>
#include <QEPlatform.h>

#define DEBUG  qDebug () << "QEDateTime" << __LINE__ << __FUNCTION__ << "  "

// We use a shared timer for all QEDateTime widget instances.
// This means they will all update in sync.
//
QTimer *QEDateTime::tickTimer = NULL;

//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QEDateTime::QEDateTime (QWidget* parent) : QLabel (parent)
{
   // Create common QEDateTime timer if needs be.
   //
   if (QEDateTime::tickTimer == NULL) {
      QEDateTime::tickTimer = new QTimer (NULL);
      QEDateTime::tickTimer->start (1000);      // Go at 1Hz
   }

   // Initialisations
   //
   this->dateTimeFormat = "yyyy-MM-dd hh:mm:ss";
   this->timeZone = Qt::LocalTime;
   this->showZone = false;
   this->setAlignment (Qt::AlignHCenter);

   // Create connection from timer to this object.
   //
   this->connect (QEDateTime::tickTimer, SIGNAL (timeout ()),
                  this, SLOT (kick ()));

   QTimer::singleShot (1, this, SLOT (kick ()));        // initial date time set

   // Allow and setup local context menu.
   //
   this->setContextMenuPolicy (Qt::CustomContextMenu);
   QObject::connect (this, SIGNAL (customContextMenuRequested (const QPoint&)),
                     this, SLOT   (customContextMenuRequested (const QPoint&)));

   // Build the context menu - note unlike QEWidget paradigm, we do this only once.
   //
   this->contextMenu = new QMenu (this);
   QAction* action = new QAction ("Copy time text ", this->contextMenu);
   this->contextMenu->addAction (action);

   QObject::connect (this->contextMenu, SIGNAL (triggered (QAction*)),
                     this, SLOT (contextMenuTriggered (QAction*)));

   // Setup event filtering
   //
   this->installEventFilter (this);
}

//------------------------------------------------------------------------------
//
QEDateTime::~QEDateTime () {}  // place holder

//------------------------------------------------------------------------------
//
QSize QEDateTime::sizeHint () const
{
   return QSize (180, 17);
}

//------------------------------------------------------------------------------
//
QString QEDateTime::getDateTimeFormat () const
{
   return this->dateTimeFormat;
}

//------------------------------------------------------------------------------
//
void QEDateTime::setDateTimeFormat (const QString format)
{
   this->dateTimeFormat = format;
}

//------------------------------------------------------------------------------
//
Qt::TimeSpec QEDateTime::getTimeZone () const
{
   return this->timeZone;
}

//------------------------------------------------------------------------------
//
void QEDateTime::setTimeZone (const Qt::TimeSpec zone)
{
   // We disallow OffsetFromUTC and TimeZone
   //
   this->timeZone = (zone == Qt::LocalTime) ? Qt::LocalTime : Qt::UTC;
}

//------------------------------------------------------------------------------
//
bool QEDateTime::getShowZone () const
{
   return this->showZone;
}

//------------------------------------------------------------------------------
//
void QEDateTime::setShowZone (const bool zone)
{
   this->showZone = zone;
}

//------------------------------------------------------------------------------
//
bool QEDateTime::eventFilter (QObject* watched, QEvent* event)
{
   const QEvent::Type type = event->type ();
   QMouseEvent *mouseEvent = NULL;

   switch (type) {

      case QEvent::MouseButtonPress:
         mouseEvent = static_cast < QMouseEvent*>(event);
         if ((watched == this) && (mouseEvent->button () == Qt::MiddleButton)) {
            QClipboard *cb = QApplication::clipboard ();
            cb->setText (this->text ());
            return true;        // we have handled this mouse button event
         }
         break;

      default:
         break;
   }

   return false;
}

//------------------------------------------------------------------------------
// slot
void QEDateTime::customContextMenuRequested (const QPoint& pos)
{
   const QPoint golbalPos = this->mapToGlobal (pos);
   this->contextMenu->exec (golbalPos);
}

//------------------------------------------------------------------------------
// slot
void QEDateTime::contextMenuTriggered (QAction*)
{
   // Is only one action - no need to decode the required action.
   //
   QClipboard *cb = QApplication::clipboard ();
   cb->setText (this->text ());
}

//------------------------------------------------------------------------------
// slot
void QEDateTime::kick ()
{
   QDateTime timeNow;

   if (this->timeZone == Qt::LocalTime) {
      timeNow = QDateTime::currentDateTime ().toLocalTime ();
   } else {
      timeNow = QDateTime::currentDateTime ().toUTC ();
   }

   QString timeString = timeNow.toString (this->dateTimeFormat);
   if (this->showZone) {
      QString zoneName = QEUtilities::getTimeZoneTLA (timeNow);
      timeString.append (" " + zoneName);
   }

   this->setText (timeString);
}

// end
