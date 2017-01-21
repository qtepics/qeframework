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
 * QCaObjects recieve data from a CaObject within the contexct of the foreign CA thread.
 * The QCaObject posts this data to itself as an event. When it recieves the event it
 * will be processing the object within the context of a Qt thread.
 *
 * This class defines the event used to carry the data and QCaObject reference.
 */

#include <CaConnection.h>
#include <CaObject.h>
#include <QCaEventUpdate.h>
#include <QCaObject.h>


// The Qt event type for update events. This is allocated by Qt once at application startup.
#if QT_VERSION >= 0x040400
    QEvent::Type QCaEventUpdate::EVENT_UPDATE_TYPE = (QEvent::Type)QEvent::registerEventType();
#else
    QEvent::Type QCaEventUpdate::EVENT_UPDATE_TYPE = QEvent::User;
#endif

/*
   Create an event holding a CA update
*/
QCaEventUpdate::QCaEventUpdate( qcaobject::QCaObject *emitterObjectIn, // The object that emited the event
                                long newReason,                        // Callback reason. Actually of type caobject::callback_reasons
                                void* newDataPtr                       // CA data. This is actually of type carecord::CaRecord*
                               ) : QEvent( EVENT_UPDATE_TYPE ) {

    // Flag this event should be used. An event may no longer be required. For example, if
    // the originating QCaObject is deleted while the event is still in the event queue.
    // This flag can be cleared while the event is in the event queue.
    acceptThisEvent = true;

    // Set up the event mechanism
    emitterObject = emitterObjectIn;

    // Populate the update
    reason = newReason;
    dataPtr = newDataPtr;
}

/*
   Delete an event holding a CA update
*/
QCaEventUpdate::~QCaEventUpdate()
{
    // Ensure no QCaObject still holds this event in its 'pending' list
    qcaobject::QCaObject::deletingEventStatic( this );
}
