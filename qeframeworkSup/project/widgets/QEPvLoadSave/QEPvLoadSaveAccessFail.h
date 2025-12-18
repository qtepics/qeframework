/*  QEPvLoadSaveAccessFail.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2017-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_PV_LOAD_SAVE_ACCESS_FAIL_H
#define QE_PV_LOAD_SAVE_ACCESS_FAIL_H

#include <QString>
#include <QStringList>
#include <QWidget>
#include <QEFrameworkLibraryGlobal.h>

namespace Ui {
   class QEPvLoadSaveAccessFail;
}

class QEPvLoadSave;   // differed.

/// This widget displays a list of PVs that failed to extracted, applied or read from the archive,
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPvLoadSaveAccessFail : public QWidget
{
   Q_OBJECT
public:
   // constructor.
   explicit QEPvLoadSaveAccessFail (QEPvLoadSave* owner,
                                    QWidget* parent = 0);
   ~QEPvLoadSaveAccessFail ();

   void clear ();
   void addPVName (const QString& pvName);
   void addPVNames (const QStringList& pvNameList);

protected:

private:
   QEPvLoadSave* owner;
   Ui::QEPvLoadSaveAccessFail* ui;


private slots:
};

#endif  // QE_PV_LOAD_SAVE_ACCESS_FAIL_H
