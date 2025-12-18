/*  QEPeriodicManager.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2011-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_PERIODIC_MANAGER_H
#define QE_PERIODIC_MANAGER_H

#include <QtGlobal>

// The QT_VERSION check is in QEDesignerPluginCommon.h, but does not
// work with moc unless restated here.  Possibly relevant:
// https://stackoverflow.com/questions/4119688/macro-expansion-in-moc
// Provide Qt version independent plugin-related includes.
//
#if (QT_VERSION >= 0x050500)
#include <QtUiPlugin/QDesignerCustomWidgetInterface>
#else
#include <QDesignerCustomWidgetInterface>
#endif

#include <QEDesignerPluginCommon.h>
#include <QEPluginLibrary_global.h>

class QEPLUGINLIBRARYSHARED_EXPORT QEPeriodicManager
      : public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEPeriodic)
};

#endif // QE_PERIODIC_MANAGER_H
