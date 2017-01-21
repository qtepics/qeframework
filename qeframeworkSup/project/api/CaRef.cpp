/*  CaRef.cpp
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
 *  Copyright (c) 2013 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#include <stdlib.h>
#include <CaRef.h>
#include <stdio.h>
#include <QMutex>


#define CAREF_MAGIC 123456789

// This mutex controls access to the carefListHead/carefListTail structure.
static QMutex *carefListMutex = new QMutex();
static CaRef* carefListHead = NULL;
static CaRef* carefListTail = NULL;

unsigned int CaRef::nextSequence = 0;

// This is a more general access lock.
//
static QMutex *accessLockMutex = new QMutex();


// Get exclusive access
void CaRef::accessLock()
{
   accessLockMutex->lock();
}

// Release exclusive access.
void CaRef::accessUnlock()
{
    accessLockMutex->unlock();
}

// Provide a new or reused instance. Call instead of constructor.
CaRef* CaRef::getCaRef( void* ownerIn, bool ownerIsCaObjectIn )
{
    QMutexLocker locker( carefListMutex );

    // If there is any previous CaRef  instances discarded over 5 seconds ago, return the first.
    if( carefListHead )
    {
        CaRef* firstRef = carefListHead;
        if( difftime( time( NULL ), firstRef->idleTime ) > 5.0 )
        {
            // Move the list head to the next (possibly NULL) object
            carefListHead = firstRef->next;

            // If end of queue reached, clear the end of list reference.
            if( firstRef == carefListTail )
            {
                carefListTail = NULL;
            }

            // Re-initialise and return the recycled object
            firstRef->init( ownerIn, ownerIsCaObjectIn );
//            printf("reuse\n");
            return firstRef;
        }
    }

    // There are no old instances to reuse - create a new one
//    printf("create\n");
    return new CaRef( ownerIn, ownerIsCaObjectIn );
}

// Construction.
// Don't use directly. Called by getCaRef() if none available for reuse
CaRef::CaRef( void* ownerIn, bool ownerIsCaObjectIn )
{
    usageCount = 0;
    sequence = nextSequence++;
    init( ownerIn, ownerIsCaObjectIn );
}

// Initialisation. Used for construction and reuse
void CaRef::init( void* ownerIn, bool ownerIsCaObjectIn )
{
    magic = CAREF_MAGIC;
    owner = ownerIn;
    discarded = false;
    channel = NULL;
    next = NULL;
    idleTime = 0;
    ownerIsCaObject = ownerIsCaObjectIn;
    usageCount++;
//    dumpList();
}

// Destrution
// This should never be called. Present just to log an error
CaRef::~CaRef()
{
    magic = 0;
    printf( "CaRef destructor called. This should never occur.");
}

// Mark as discarded and queue for reuse when no further CA callbacks are expected
void CaRef::discard()
{
    // Flag no longer in use
    discarded = true;

    // Note the time discarded
    idleTime = time( NULL );

    // Place the disused item on the discarded queue
    QMutexLocker locker( carefListMutex );

    if( !carefListHead )
    {
        carefListHead = this;
    }

    if( carefListTail )
    {
        carefListTail->next = this;
    }
    carefListTail = this;
//    dumpList();
}

// Return the object referenced, if it is still around.
// Returns NULL if the object is no longer in use.
void* CaRef::getRef( void* channelIn, bool ignoreZeroId )
{
    // Sanity check - was the CA user data really a CaRef pointer
    if( magic != CAREF_MAGIC )
    {
        printf( "CaRef::getRef() called but the CA user data was not really a CaRef pointer. (magic number is bad).  CA user data: %ld\n", (long)this );
        return NULL;
    }

    // If discarded, then a late callback has occured
    if( discarded )
    {
        printf( "Late CA callback. CaRef::getRef() called after associated object has been discarded.  object reference: %ld  variable: %s  expected channel: %ld received channel %ld\n",
                (long)owner, variable.c_str(), (long)channel, (long)channelIn );
        return NULL;
    }

    // If channel in callback is zero, then something is badly wrong (most of the time... zero is OK in 'exception' callbacks)
    if(( ignoreZeroId == false ) && ( channelIn == 0 ))
    {
        printf( "Channel returned in callback is zero. CaRef::getRef() called with zero channel ID.  object reference: %ld  variable: %s  expected channel: %ld\n",
                (long)owner, variable.c_str(), (long)channel );
        return NULL;
    }

    // If a channel has been recorded, but the current channel doesn't match, it is likely due to a late callback
    // calling with a reference to a now re-used CaRef.
    // (note, we have already dealt, or ignored) a channel ID of zero)
    if(( channelIn != 0 ) && channel && (channel != channelIn) )
    {
        printf( "Very late CA callback. CaRef::getRef() called with incorrect channel ID.  object reference: %ld  variable: %s  expected channel: %ld received channel %ld\n",
                (long)owner, variable.c_str(), (long)channel, (long)channelIn );
        return NULL;
    }

    // Return the referenced object
    return owner;
}

// set the variable - for logging only
void CaRef::setPV( std::string variableIn )
{
    variable = variableIn;
}

// set the channel - for checking and logging
void CaRef::setChannelId ( void* channelIn )
{
    channel = channelIn;
    if( channel == 0 )
    {
        printf( "CaRef::setChannelId() cahnnel is zero\n");
    }
}

// Dump the current list - for debugging only
//void CaRef::dumpList()
//{
//    printf( "head: %lu\n", (unsigned long)carefListHead );
//    CaRef* obj = carefListHead;
//    int count = 0;
//    while( obj )
//    {
//        count++;
//// Include the following line if full dump of current list is required
////        printf("   obj: %lu next: %lu PV: %s\n", (unsigned long)obj, (unsigned long)(obj->next), obj->variable.c_str() );
//        obj = obj->next;
//    }
//    printf( "count: %d\n", count );
//    printf( "tail: %lu\n", (unsigned long)carefListTail );
//    fflush(stdout);
//}

// end
