/*  QEPeriodicManager.cpp
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

#include "QEPeriodicManager.h"
#include <QExtensionManager>
#include <QDesignerFormEditorInterface>
#include <QEPeriodic.h>
#include <QEPeriodicTaskMenu.h>
#include <QEDesignerPluginGroupNames.h>

/*
    ???
*/
QEPeriodicManager::QEPeriodicManager( QObject *parent ) : QObject( parent ) {
    initialized = false;
}

/*
    ???
*/
void QEPeriodicManager::initialize( QDesignerFormEditorInterface * formEditor ) {
    if( initialized ) {
        return;
    }

    QExtensionManager *manager = formEditor->extensionManager();
    Q_ASSERT(manager != 0);

    // This is a "smart" widget
    //
    manager->registerExtensions(new QEPeriodicTaskMenuFactory(manager),
                                Q_TYPEID(QDesignerTaskMenuExtension));

    initialized = true;
}

/*
    ???
*/
bool QEPeriodicManager::isInitialized() const {
    return initialized;
}

/*
    Widget factory. Creates a QEPeriodic widget.
*/
QWidget *QEPeriodicManager::createWidget ( QWidget *parent ) {
    return new QEPeriodic(parent);
}

/*
    Name for widget. Used by Qt Designer in widget list.
*/
QString QEPeriodicManager::name() const {
    return "QEPeriodic";
}

/*
    Name of group Qt Designer will add widget to.
*/
QString QEPeriodicManager::group() const {
    return QECONTROL;
}

/*
    Icon for widget. Used by Qt Designer in widget list.
*/
QIcon QEPeriodicManager::icon() const {
    return QIcon(":/qe/plugin/QEPeriodic.png");
}

/*
    Tool tip for widget. Used by Qt Designer in widget list.
*/
QString QEPeriodicManager::toolTip() const {
    return "EPICS Elemental Selection";
}

/*
    ???
*/
QString QEPeriodicManager::whatsThis() const {
    return "EPICS Elemental Selection";
}

/*
    ???
*/
bool QEPeriodicManager::isContainer() const {
    return false;
}


/*
    ???
*/
QString QEPeriodicManager::includeFile() const {
    return "QEPeriodic.h";
}

// end
