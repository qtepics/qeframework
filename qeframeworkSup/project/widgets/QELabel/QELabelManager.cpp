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

#include <QELabelManager.h>
#include <QELabel.h>
#include <QtPlugin>

/*
    ???
*/
QELabelManager::QELabelManager( QObject *parent ) : QObject( parent ) {
    initialized = false;
}

/*
    ???
*/
void QELabelManager::initialize( QDesignerFormEditorInterface * ) {
    if( initialized ) {
        return;
    }
    initialized = true;
}

/*
    ???
*/
bool QELabelManager::isInitialized() const {
    return initialized;
}

/*
    Widget factory. Creates a QELabel widget.
*/
QWidget *QELabelManager::createWidget ( QWidget *parent ) {
    return new QELabel(parent);
}

/*
    Name for widget. Used by Qt Designer in widget list.
*/
QString QELabelManager::name() const {
    return "QELabel";
}

/*
    Name of group Qt Designer will add widget to.
*/
QString QELabelManager::group() const {
    return "EPICSQt Monitors";
}

/*
    Icon for widget. Used by Qt Designer in widget list.
*/
QIcon QELabelManager::icon() const {
    return QIcon(":/qe/label/QELabel.png");
}

/*
    Tool tip for widget. Used by Qt Designer in widget list.
*/
QString QELabelManager::toolTip() const {
    return "EPICS Label";
}

/*
    ???
*/
QString QELabelManager::whatsThis() const {
    return "EPICS Label";
}

/*
    ???
*/
bool QELabelManager::isContainer() const {
    return false;
}

/*
    ???
*/
/*QString QELabelManager::domXml() const {
    return "<widget class=\"QELabel\" name=\"qELabel\">\n"
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
QString QELabelManager::includeFile() const {
    return "QELabel.h";
}
