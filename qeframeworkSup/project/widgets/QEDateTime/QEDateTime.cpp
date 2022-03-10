/*  QEDateTime.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2019-2022 Australian Synchrotron
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
 *    Zai Wang
 *  Contact details:
 *    @ansto.gov.au
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
#include <QTimer>
#include <QECommon.h>
#include <QEPlatform.h>

#define DEBUG  qDebug () << "QEDateTime" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QEDateTime::QEDateTime(QWidget *parent) : QLabel(parent)
{
   QTimer *timer = new QTimer(this);
   connect(timer, SIGNAL(timeout()), this, SLOT(kick()));

   // initializations
   this->dateTimeFormat = "yyyy-MM-dd hh:mm:ss";
   this->timeZone = Qt::LocalTime;
   this->showZone = false;
   this->setMinimumSize(180, 20);
   this->setAlignment(Qt::AlignHCenter);

   timer->start(1000);  // at 1 sec interval.
   QTimer::singleShot(1, this, SLOT (kick())); // initial date time set

   // Allow and setup local context menu.
   //
   this->setContextMenuPolicy (Qt::CustomContextMenu);
   QObject::connect (this, SIGNAL (customContextMenuRequested (const QPoint&)),
                     this, SLOT   (customContextMenuRequested (const QPoint&)));

   // Build the context menu - note unlike QEWidget paradigm, we do this only once.
   //
   this->contextMenu = new QMenu (this);
   QAction* action = new QAction("Copy time text ", this->contextMenu);
   this->contextMenu->addAction (action);

   QObject::connect (this->contextMenu, SIGNAL (triggered (QAction*)),
                     this, SLOT (contextMenuTriggered(QAction*)));

   // Setup event filtering
   //
   this->installEventFilter(this);
}

//------------------------------------------------------------------------------
//
QEDateTime::~QEDateTime( ) { }  // place holder

//------------------------------------------------------------------------------
//
QString QEDateTime::getDateTimeFormat() const
{
   return this->dateTimeFormat;
}

//------------------------------------------------------------------------------
//
void QEDateTime::setDateTimeFormat(const QString format)
{
   this->dateTimeFormat = format;
}

//------------------------------------------------------------------------------
//
Qt::TimeSpec QEDateTime::getTimeZone() const
{
   return this->timeZone;
}

//------------------------------------------------------------------------------
//
void QEDateTime::setTimeZone(const Qt::TimeSpec zone)
{
   // We disallow OffsetFromUTC and TimeZone
   //
   this->timeZone = zone == Qt::LocalTime ? Qt::LocalTime : Qt::UTC;
}

//------------------------------------------------------------------------------
//
bool QEDateTime::getShowZone() const
{
   return this->showZone;
}

//------------------------------------------------------------------------------
//
void QEDateTime::setShowZone(const bool zone)
{
   this->showZone = zone;
}

//------------------------------------------------------------------------------
//
bool QEDateTime::eventFilter (QObject* watched, QEvent *event)
{
   const QEvent::Type type = event->type ();
   QMouseEvent* mouseEvent = NULL;

   switch (type) {

      case QEvent::MouseButtonPress:
         mouseEvent = static_cast<QMouseEvent *> (event);
         if ((watched == this) && (mouseEvent->button() == MIDDLE_BUTTON)) {
            QClipboard* cb = QApplication::clipboard();
            cb->setText(this->text());
            return true;  // we have handled this mouse button event
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
   QClipboard* cb = QApplication::clipboard();
   cb->setText(this->text());
}

//------------------------------------------------------------------------------
// slot
void QEDateTime::kick()
{
   QDateTime timeNow;

   if (this->timeZone == Qt::LocalTime){
      timeNow = QDateTime::currentDateTime().toLocalTime();
   } else {
      timeNow = QDateTime::currentDateTime().toUTC();
   }

   QString timeString = timeNow.toString(this->dateTimeFormat);
   if (this->showZone){
      QString zoneName = QEUtilities::getTimeZoneTLA(timeNow);
      timeString.append(" " + zoneName);
   }

   this->setText(timeString);
}

// end
