/*  QEScanTimers.h
 *
 */

#ifndef QE_SCAN_TIMERS_H
#define QE_SCAN_TIMERS_H

#include <QObject>
#include <QTimer>

#include <QEPluginLibrary_global.h>

class QEPLUGINLIBRARYSHARED_EXPORT QEScanTimers : public QObject {
Q_OBJECT
public:
   enum ScanRates {
      VerySlow,     // 0.25 Hz
      Slow,         // 0.5 Hz
      Medium,       // 1 Hz
      Fast,         // 2 Hz
      VeryFast      // 4 Hz
   };

   Q_ENUMS (ScanRates)

   // Connext (attach) ir disconnect (detach) form specific scan timer.
   // Example: attach (this, SLOT (scanSlot (const bool)));
   //
   static bool attach (QObject* target, const  char* member, const ScanRates scanRate);
   static void detach (QObject* target, const  char* member);

private:
   // Actual instances of these objects are set up statically and ONLY created from
   // within this class.
   //
   explicit QEScanTimers (QObject* parent = 0);
   ~QEScanTimers ();

   static void initialise ();

   QTimer* timer;

   unsigned int count;

private slots:
   void timeout ();

signals:
   void flipFlopVerySlow (const bool);
   void flipFlopSlow     (const bool);
   void flipFlopMedium   (const bool);
   void flipFlopFast     (const bool);
   void flipFlopVeryFast (const bool);
};


#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEScanTimers::ScanRates)
#endif

#endif  // QE_SCAN_TIMERS_H
