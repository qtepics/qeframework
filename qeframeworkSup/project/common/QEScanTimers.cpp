/*  QEScanTimers.cpp
 *
 */

#include <QDebug>
#include <QMutex>

#include <QECommon.h>

#include <QEScanTimers.h>

static QMutex *scanTimerMutex = new QMutex ();
static QEScanTimers* theScanTimer  = NULL;


//------------------------------------------------------------------------------
//
QEScanTimers::QEScanTimers (QObject* parent) : QObject (parent)
{
   // Save callers parameters.
   //
   this->count = 0;

   // Create, connect and configure timer object.
   //
   this->timer = new QTimer (this);

   QObject::connect (this->timer, SIGNAL (timeout ()),
                     this,        SLOT   (timeout ()));

   // Medium correspond to 500mS, i.e. 1sec scan period as that is 500mS off + 500mS on.
   //
   this->timer->start (125);
}

//------------------------------------------------------------------------------
//
QEScanTimers::~QEScanTimers ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
void QEScanTimers::timeout ()
{
   bool x;
   this->count = (this->count + 1) & 0xFFFF;

   x = (this->count & 0x01) != 0;
   emit flipFlopVeryFast (x);

   if (!x) {
      x = (this->count & 0x02) != 0;
      emit flipFlopFast     (x);

      if (!x) {
         x = (this->count & 0x04) != 0;
         emit flipFlopMedium   (x);

         if (!x) {
            x = (this->count & 0x08) != 0;
            emit flipFlopSlow     (x);

            if (!x) {
               x = (this->count & 0x10) != 0;
               emit flipFlopVerySlow (x);
            }
         }
      }
   }
}


//------------------------------------------------------------------------------
//
void QEScanTimers::initialise ()
{
   QMutexLocker locker (scanTimerMutex);

   if (!theScanTimer) {
      theScanTimer = new QEScanTimers (NULL);
   }
}


//------------------------------------------------------------------------------
//
bool QEScanTimers::attach (QObject* target,
                           const char* member,
                           const ScanRates scanRate)
{
   bool result = false;

   QEScanTimers::initialise ();      // idempotant

   switch (scanRate) {
      case VerySlow: result = QObject::connect (theScanTimer,  SIGNAL (flipFlopVerySlow (const bool)), target, member);  break;
      case Slow:     result = QObject::connect (theScanTimer,  SIGNAL (flipFlopSlow     (const bool)), target, member);  break;
      case Medium:   result = QObject::connect (theScanTimer,  SIGNAL (flipFlopMedium   (const bool)), target, member);  break;
      case Fast:     result = QObject::connect (theScanTimer,  SIGNAL (flipFlopFast     (const bool)), target, member);  break;
      case VeryFast: result = QObject::connect (theScanTimer,  SIGNAL (flipFlopVeryFast (const bool)), target, member);  break;
      default: break;
   }

   return result;
}


//------------------------------------------------------------------------------
//
void QEScanTimers::detach (QObject* target,
                           const char* member)
{
   QEScanTimers::initialise ();      // idempotant
   theScanTimer->disconnect (target, member);
}

// end
