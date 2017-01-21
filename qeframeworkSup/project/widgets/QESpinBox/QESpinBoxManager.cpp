/*
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
 *  Copyright (c) 2009, 2010 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#include <QESpinBoxManager.h>
#include <QESpinBox.h>
#include <QtPlugin>

/*
    ???
*/
QESpinBoxManager::QESpinBoxManager( QObject *parent ) : QObject( parent ) {
    initialized = false;
}

/*
    ???
*/
void QESpinBoxManager::initialize( QDesignerFormEditorInterface * ) {
    if( initialized ) {
        return;
    }
    initialized = true;
}

/*
    ???
*/
bool QESpinBoxManager::isInitialized() const {
    return initialized;
}

/*
    Widget factory. Creates a QESpinBox widget.
*/
QWidget *QESpinBoxManager::createWidget ( QWidget *parent ) {
    return new QESpinBox(parent);
}

/*
    Name for widget. Used by Qt Designer in widget list.
*/
QString QESpinBoxManager::name() const {
    return "QESpinBox";
}

/*
    Name of group Qt Designer will add widget to.
*/
QString QESpinBoxManager::group() const {
    return "EPICSQt Controls";
}

/*
    Icon for widget. Used by Qt Designer in widget list.
*/
QIcon QESpinBoxManager::icon() const {
    return QIcon(":/qe/spinbox/QESpinBox.png");
}

/*
    Tool tip for widget. Used by Qt Designer in widget list.
*/
QString QESpinBoxManager::toolTip() const {
    return "EPICS Spin Box";
}

/*
    ???
*/
QString QESpinBoxManager::whatsThis() const {
    return "EPICS Spin Box";
}

/*
    ???
*/
bool QESpinBoxManager::isContainer() const {
    return false;
}

/*
    ???
*/
/*QString QESpinBoxManager::domXml() const {
    return "<widget class=\"QESpinBox\" name=\"qESpinBox\">\n"
           " <property name=\"geometry\">\n"
           "  <rect>\n"
           "   <x>0</x>\n"
           "   <y>0</y>\n"
           "   <width>100</width>\n"
           "   <height>100</height>\n"
           "  </rect>\n"
           " </property>\n"
           " <property name=\"toolTip\" >\n"
           "  <string></string>\n"
           " </property>\n"
           " <property name=\"whatsThis\" >\n"
           "  <string> "
           ".</string>\n"
           " </property>\n"
           "</widget>\n";
}*/

/*
    ???
*/
QString QESpinBoxManager::includeFile() const {
    return "QESpinBox.h";
}
