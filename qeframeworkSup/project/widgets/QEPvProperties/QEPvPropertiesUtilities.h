/*  QEPvPropertiesUtilities.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/* The QEPvPropertiesUtilities class provides utilities for the QEPvProperties widget.
 */

#ifndef QE_PV_PROPERTIES_UTILITIES_H
#define QE_PV_PROPERTIES_UTILITIES_H

#include <QList>
#include <QString>
#include <QStringList>


//------------------------------------------------------------------------------
// This class provides a named (by record type: ai, bo, calc etc) list of
// record field names.
//
class QERecordSpec {
public:
   explicit QERecordSpec (const QString& recordType);
   ~QERecordSpec ();

   void append (const QString item);

   int size () const;

   QString getRecordType () const;

   // If index is out of bounds, this function returns "".
   //
   QString getFieldName (const int index) const;

   // Indicates if long string mode is applicable.
   // If index is out of bounds, this function returns false.
   //
   bool fieldMayUseCharArray (const int index) const;

   // Indicates if the field is an INLINK, OUTLINK or FWDLINK field, or is
   // otherwise used to hold a PV name (e.g. sscan record PV name fields).
   // If index is out of bounds, this function returns false.
   //
   bool fieldIsLinkField (const int index) const;

   // Returns field description.
   // If index is out of bounds, this function returns "".
   //
   QString getDescription (const int index) const;

private:
   QString getFieldText (const int index) const;
   QStringList list;
   QString recordType;
};


//------------------------------------------------------------------------------
// This class provides a list of RecordSpec specs, with additional find functions.
//
class QERecordSpecList : public QList< QERecordSpec *> {
public:
   QERecordSpecList ();
   QERecordSpec* find (const QString& recordType) const;
   void appendOrReplace (QERecordSpec* recordSpec);

   bool processRecordSpecFile (const QString& filename);

private:
   int findSlot (const QString& recordType) const;
};

# endif  // QE_PV_PROPERTIES_UTILITIES_H
