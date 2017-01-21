/*
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
 *  Copyright (c) 2009, 2010 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

// CA event filter for QT.

#ifndef QCAEVENTFILTER_H
#define QCAEVENTFILTER_H

#include <QtDebug>
#include <QObject>
#include <QMutex>

/*
    This class is simply a reference to an object that has installed an event filter (an instance of class QCaEventFilter)
    This class exists so that a list of references to QCaEventFilter instances can be maintained.
*/
class QCaInstalledFiltersListItem {
  public:
    QCaInstalledFiltersListItem( QObject* eventObjectIn ) { eventObject = eventObjectIn; referenceCount = 1; }
    // QObject that an event filter has been added to
    QObject* eventObject;
    // Count of how many QCaObjects expect the event filter to be on the QObject
    long referenceCount;
};

/*
    This class allows instances of QCaObject to recieve events posted to them by foriegn CA threads.
    Each event recipient creates an instance of this class. This class, however, manages only a single
    event filter for all instances of itself.
*/
class QCaEventFilter : public QObject {
    Q_OBJECT
  public:
    // Add a fitler to an object
    void addFilter( QObject* objectIn );
    // Remove an event filter from an object
    void deleteFilter( QObject* objectIn );
    // Event filter added to a QObject
    bool eventFilter( QObject *watched, QEvent *e );

  private:
    // Used to protect access to installedFilters
    QMutex installedFiltersLock;
    // List of QObjects the event filter is added to
    QList<QCaInstalledFiltersListItem> installedFilters;
};

#endif // QCAEVENTFILTER_H
