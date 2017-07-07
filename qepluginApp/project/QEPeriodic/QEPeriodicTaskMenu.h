/*  QEPeriodicTaskMenu.h
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
 *  Copyright (c) 2011 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
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
