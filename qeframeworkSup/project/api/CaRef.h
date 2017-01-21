/*  CaRef.h
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
 *  Copyright (c) 2012 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

// CA callbacks have occured after the objects that were using them were deleted.
// The user data in the callbacks was a pointer to a framework class (either a
// CaObject or CaConnection class reference). If the object had been deleted, the
// pointer in the CA callback was invalid, causing crashes.
// This may be a bug in this framework, or in CA.
// The workaround is to add a CaRef object to each of the classes that must be
// referenced in a CA callback. A pointer to the CaRef object is used as the CA callback data.
// When a CaObject or CaConnection object is deleted, it does not delete its CaRef obejct - it just
// marks it as 'discarded'.
// If a CA callback occurs after the CaObject or CaConnection object has been deleted, the CaRef
// object still exists. If it is 'discarded', the callback is aborted before refering to any
// deleted objects.
// The CaRef objects are never deleted. Rather, they are added to a list and reused after some time
// of inactivity.
//
// This unit also holds and manages the access lock used by both CaObject and CaConnection classess.

#ifndef CAOBJECTREF_H_
#define CAOBJECTREF_H_

#include <string>
#include <time.h>

class CaRef
{
public:
    static CaRef* getCaRef( void* ownerIn, bool ownerIsCaObjectIn );// Provide a new or reused instance. Call instead of constructor.
    CaRef( void* ownerIn, bool ownerIsCaObjectIn );                 // Constructor. Don't use directly. Called by getCaRef() if none available for reuse
    void init( void* ownerIn, bool ownerIsCaObjectIn );             // Common initialisation. Used for construction and reuse

    ~CaRef();                               // Destructor should never be called. Call discard() instead.
    void discard();                         // Mark this as 'discarded' and queue for reuse. Owner no longer exists.

    void setPV( std::string variableIn );   // Set variable name. Used for logging only
    void setChannelId ( void* channelIn );  // Set channel name. used for logging and also for validating callback
    void* getRef( void* channelIn, bool ignoreZeroId = false );        // Get the owner (validating the channel if known) else NULL

    static void accessLock ();              //
    static void accessUnlock ();            //


//    void dumpList();                        // Diagnostics only

    std::string getVariable(){ return variable; } // Get variable name when checking CA callbacks are OK

private:
    int magic;                              // Magic number used as a sanitly check that this object is very likley to be a CaRef
    void* owner;                            // Owner object - either a CaObject or a CaConnection
    bool discarded;                         // True if owner is no longer valid
    void* channel;                          // CA channel id
    std::string variable;                   // CA PV name
    CaRef* next;                            // List link
    time_t idleTime;                        // Time discarded
    bool ownerIsCaObject;                   // True if owner is a CaObject class (otherwise a CaConnection class) - diagnostic only
    unsigned int usageCount;                // Note the number of times this class instance has been recycled (equals 1 for first use)
    unsigned int sequence;                  // Instance creation order
    static unsigned int nextSequence;       // Next value for 'sequence'
};

#endif  // CAOBJECTREF_H_
