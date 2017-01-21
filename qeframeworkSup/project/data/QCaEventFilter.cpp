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

/* Description:
 *
 * Manage an event filter to catch events posted from within the
 * QCaObject by the CA callback thread (using the QCaObject::signalCallback().
 *
 * This is part of the mechanism for translating a foreign-thread callback
 * from a CaObject to a Qt signal from a QCaObject.
 *
 * A QObject is supplied by the creator of a QCaObject object. An event filter
 * is added to this object to catch events posted by the QCaObject object in the
 * context of a foreign thread.
 * See the description of QCaObject (in QCaObject.cpp) for details of the entire
 * data update process and where the event filters managed by this class fit
 * in to the process.
 *
 * Only a single static instance of this class is required.
 * Multiple QCaObjects can be created specifying the same event handling object,
 * but the event filter should only be added once. A list is maintained of
 * what event objects a filter has been added to and maintains a reference
 * count of how many QCaObjects are relying on it processing events for that object.
 *
 * Typically the number of event objects used by an application to process QCaObject events
 * will be small. Examples are:
 *  - A single object processing all data updates
 *  - One object (on one thread) for general updates and another for special updates
 *    requiring extensive processing
 *  - An object (on it's own thread) for each CPU to maximise the machine utilisation.
 *
 * Note, if an event has been posted by a QCaObject and the QCaObject is deleted before
 * the event is processed, the event will still be processed if the event filter is
 * still in place - and the filter will still be there if any other QCaObjects are using
 * the same QObject to process events. In this case the event will reference a QCaObject
 * which no longer exists. This case is not managed here. Refer to the QCaObject destructor to see how
 * this potentially dangerous situation is dealt with.
 *
 * This class is thread safe
 */

#include <QCaEventFilter.h>
#include <QCaEventUpdate.h>
#include <QCaObject.h>

/*
    Add the event filter to an object.
*/
void QCaEventFilter::addFilter( QObject *eventObject ) {

    // Protect access to the list of objects
    QMutexLocker locker( &installedFiltersLock );

    // Check if the filter is already present. If so, just increase the reference count
    int i;
    for( i = 0; i < installedFilters.size(); i++ ) {
        QCaInstalledFiltersListItem* item = &(installedFilters[i]);
        if( item->eventObject == eventObject ) {
            item->referenceCount++;
            break;
        }
    }

    // If the filter was not present (known because the search reached the end of the list) then
    // add the object to the list of objects using the filter and add the filter to the object.
    if( i >= installedFilters.size() ) {
        // Add the object to the list
        QCaInstalledFiltersListItem item( eventObject );
        installedFilters.append( item );

        // Add the filter
        eventObject->installEventFilter( this );
    }
}

/*
    Remove the event filter from an object.
*/
void QCaEventFilter::deleteFilter( QObject *eventObject ) {

    // Protect access to the list of objects
    QMutexLocker locker( &installedFiltersLock );

    // Search for the QObject the filter is to be removed from.
    bool itemFound = false;
    int i;
    for( i = 0; i < installedFilters.size(); i++ ) {

        // Get the next item and check for a match
        QCaInstalledFiltersListItem* item = &(installedFilters[i]);
        if( item->eventObject == eventObject ) {
            // Item matched. Reduce the count of QCaObjects relying on
            // this filter and remove the filter if there are no more
            // QCaObject requiring the filter
            item->referenceCount--;

            if( item->referenceCount == 0 ) {
                eventObject->removeEventFilter( this );
                installedFilters.removeAt( i );
            }
            itemFound = true;
            break;
        }
    }

    // If the item containing the object was not found, log an error
    if( !itemFound ) {
        // ??? log an error better than this
        qDebug() << "QCaEventFilter::deleteFilter() Error locating object";
    }
}

/*
    Filter events for a QObject, processing events posted by a QCaObject.
    This is an overloaded function of QObject
    As the originating QCaObject is referenced in the event, events can be processed
    back within the originating QCaObject using the QCaObject::processEvent() method.
*/
bool QCaEventFilter::eventFilter( QObject *watched, QEvent *e ) {
    if( e->type() == QCaEventUpdate::EVENT_UPDATE_TYPE ) {
        // The event is our update event.
        // Pass it back to the originating QCaObject for processing if still required
        QCaEventUpdate *dataUpdateEvent = static_cast<QCaEventUpdate*>( e );
        qcaobject::QCaObject::processEventStatic( dataUpdateEvent );

        // The event has been dealt with. Indicate it requires no further processing (return true)
        return true;
    }
    return QObject::eventFilter( watched, e );
}
