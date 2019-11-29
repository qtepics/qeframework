/*  QEDateTime.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
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
 *  Copyright (c) 2019 Australian Synchrotron *
 *  Author:
 *    Zai Wang
 *  Contact details:
 *    @ansto.gov.au
 */

/*!
  This class is a non EPICS aware label widget based on the Qt label widget and Qt DateTime class.
  It is only used for displaying data and time of the day
 */

#include <QEDateTime.h>
#include <QDebug>
#include <QTimer>
#include <QECommon.h>

#define DEBUG  qDebug () << "QEDateTime" << __LINE__ << __FUNCTION__ << "  "

/*
    Constructor with no initialisation
*/
QEDateTime::QEDateTime( QWidget *parent ) : QLabel( parent )
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(kick()));
    // initializations
    dateTimeFormat = "yyyy-MM-dd hh:mm:ss";
    timeZone = Qt::LocalTime;
    showZone = false;
    setMinimumSize(180,20);
    setAlignment(Qt::AlignHCenter);
    // initial date time
    kick();
    timer->start(1000);
}

void QEDateTime::kick(){

    if (timeZone == Qt::LocalTime){
        timeNow = QDateTime::currentDateTime().toLocalTime ();
    }
    else{
        timeNow = QDateTime::currentDateTime().toUTC();
    }
    QString timeString = timeNow.toString(dateTimeFormat);
    if (showZone){
        QString zoneName = QEUtilities::getTimeZoneTLA  (timeNow);
        timeString.append(" " + zoneName);
//        DEBUG << timeString;
    }
    setText(timeString);
}

// end
