/*  QEOneToOne.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_ONE_TO_ONE
#define QE_ONE_TO_ONE

#include <QHash>
#include <QEFrameworkLibraryGlobal.h>

/// This template provides a one-to-one mapping from one type, the Domain type,
/// to a another, possiblely the same, type, the CoDomain type. Despite the
/// terminology used here (i.e. domain/codomain), the association between the two
/// types is symetrical, therefore QEOneToOne <T1, T2> provides the effectively
/// the same functionality as QEOneToOne <T2, T1>
///
/// Most functions are available in both forward (domain to co-domain) and in the
/// inverse (co-domain to domain) modes of operation. These are named as <func>F
/// and <func>I.  We cannot use on overloading as user may require one to one
/// mapping between the same type, e.g. an int to int association.
///
/// The undelying mappings are provided by a QHash. This may change.
///
// Note: While it is possible to extend this class, thus:
// class MyOneToOne : public QEOneToOne <int, int> { .... };
// this sort of construct has caused issue on some platforms.
//
template <class D, class C>
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEOneToOne {
private:
   QHash <D,C> forward;
   QHash <C,D> inverse;

public:
   explicit QEOneToOne () {
      this->clear ();
   }

   /// Clear all mapping relationships.
   ///
   void clear () {
      this->forward.clear ();
      this->inverse.clear ();
   }

   /// Insert a key-key mapping into the association. Returns true if successfull.
   /// Returns false on fail. The insertF function fails if the association
   /// currently contains the domain key dkey and/or the codomain key ckey.
   ///
   bool insertF (const D& dkey, const C& ckey) {
      if (!(this->forward.contains (dkey) || this->inverse.contains (ckey))) {
         this->forward.insert (dkey, ckey);
         this->inverse.insert (ckey, dkey);
         return true;
      } else {
         return false;
      }
   }
   bool insertI (const C& ckey, const D& dkey) {
      // Just leverage off the forward function.
      //
      return this->insertF (dkey, ckey);
   }


   /// Indicates if assoication contains specified key.
   ///
   bool containsF (const D& key) const {
      return this->forward.contains (key);
   }
   bool containsI (const C& key) const {
      return this->inverse.contains (key);
   }

   /// Extracts the b/a value associated with a/b key. If the association does
   /// not contain a/b key, then returns A ()/ B()  as default.
   ///
   C valueF (const D& key) const {
      return this->forward.value (key);
   }
   D valueI (const C& key) const {
      return this->inverse.value (key);
   }

   /// Extracts the value associated with key. If the association does
   /// not contain the key, then returns specified default value.
   ///
   C valueF (const D& key, const C& defaultValue) const {
      return this->forward.value (key, defaultValue);
   }

   D valueI (const C& key, const D& defaultValue) const {
      return this-> inverse.value (key, defaultValue);
   }

   /// Removes the association containing specified key.
   ///
   bool removeF (const D& dkey) {
      const C& ckey = this->valueF (dkey);
      bool b1 = this->forward.remove (dkey);
      bool b2 = this->inverse.remove (ckey);
      return b1 && b2;
   }
   bool removeI (const C& ckey) {
      const D& dkey = this->valueI (ckey);
      bool b1 = this->forward.remove (dkey);
      bool b2 = this->inverse.remove (ckey);
      return b1 && b2;
   }
};

#endif  // QE_ONE_TO_ONE
