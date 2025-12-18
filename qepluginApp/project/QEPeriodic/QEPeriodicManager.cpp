/*  QEPeriodicManager.cpp
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
