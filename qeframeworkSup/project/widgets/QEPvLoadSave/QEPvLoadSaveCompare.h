/*  QEPvLoadSaveCompare.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2015-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_PV_LOAD_SAVE_COMPARE_H
#define QE_PV_LOAD_SAVE_COMPARE_H

#include <QColor>
#include <QList>
#include <QWidget>
#include <QEFrameworkLibraryGlobal.h>
#include <UserMessage.h>

namespace Ui {
   class QEPvLoadSaveCompare;
}

class QEPvLoadSave;   // differed.

/// This widget displays graphically the difference between two PV data sets
/// from the left/right hand side of a QEPvLoadSave.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPvLoadSaveCompare : public QWidget, public UserMessage
{
   Q_OBJECT
public:
   // constructor.
   explicit QEPvLoadSaveCompare (QEPvLoadSave* owner,
                                 const int side,
                                 QWidget* parent = 0);
   ~QEPvLoadSaveCompare ();

   // Uses the owner's QEPvLoadSave model data to create histogram value pair data.
   //
   void processModelData ();

protected:

private:
   struct ValuePairs {
      QString name;
      double a;
      double b;
      double dB;  // pre-calculated from a and b.
   };

   typedef QList<ValuePairs> ValuePairLists;

   ValuePairLists* valuePairList;

   const QEPvLoadSave* owner;
   const int side;
   Ui::QEPvLoadSaveCompare* ui;

   double scale;

   double dB (const double x) const;               // Raw dB (deciBell) function
   double safe_dB (const double x,
                   const double y) const;          // Safe dB ratio of  y/x.
   QColor calculateColour (const double q) const;  // Determine bar colour
   void updateHistogram ();                        // Configures internal histogram widget.
   void updateReadout (const int index);

private slots:
   void valueChanged (const double value);
   void mouseIndexChangedSlot (const int index);
};

#endif  // QE_PV_LOAD_SAVE_COMPARE_H
