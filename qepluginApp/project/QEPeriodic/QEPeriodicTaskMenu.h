/*  QEPeriodicTaskMenu.h
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

#ifndef QE_PERIODIC_TASK_MENU_H
#define QE_PERIODIC_TASK_MENU_H

#include <QDesignerTaskMenuExtension>
#include <QExtensionFactory>
#include <QEPluginLibrary_global.h>

QT_BEGIN_NAMESPACE
class QAction;
class QExtensionManager;
QT_END_NAMESPACE
class QEPeriodic;

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEPeriodicTaskMenu :
      public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)

public:
    QEPeriodicTaskMenu(QEPeriodic *periodic, QObject *parent);

    QAction *preferredEditAction() const;
    QList<QAction *> taskActions() const;

private slots:
    void editUserInfo();

private:
    QAction *editUserInfoAction;
    QEPeriodic *periodic;
};

//------------------------------------------------------------------------------
//
class QEPeriodicTaskMenuFactory : public QExtensionFactory
{
    Q_OBJECT

public:
    QEPeriodicTaskMenuFactory(QExtensionManager *parent = 0);

protected:
    QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif  // QE_PERIODIC_TASK_MENU_H
