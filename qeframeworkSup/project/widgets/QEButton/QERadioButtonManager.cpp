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

#include <QERadioButtonManager.h>
#include <QERadioButton.h>
#include <QtPlugin>

/*
    ???
*/
QERadioButtonManager::QERadioButtonManager( QObject *parent ) : QObject( parent ) {
    initialized = false;
}

/*
    ???
*/
void QERadioButtonManager::initialize( QDesignerFormEditorInterface * ) {
    if( initialized ) {
        return;
    }
    initialized = true;
}

/*
    ???
*/
bool QERadioButtonManager::isInitialized() const {
    return initialized;
}

/*
    Widget factory. Creates a QERadioButton widget.
*/
QWidget *QERadioButtonManager::createWidget ( QWidget *parent ) {
    return new QERadioButton(parent);
}

/*
    Name for widget. Used by Qt Designer in widget list.
*/
QString QERadioButtonManager::name() const {
    return "QERadioButton";
}

/*
    Name of group Qt Designer will add widget to.
*/
QString QERadioButtonManager::group() const {
    return "EPICSQt Controls";
}

/*
    Icon for widget. Used by Qt Designer in widget list.
*/
QIcon QERadioButtonManager::icon() const {
    return QIcon(":/qe/button/QERadioButton.png");
}

/*
    Tool tip for widget. Used by Qt Designer in widget list.
*/
QString QERadioButtonManager::toolTip() const {
    return "EPICS Radio Button";
}

/*
    ???
*/
QString QERadioButtonManager::whatsThis() const {
    return "EPICS Radio Button";
}

/*
    ???
*/
bool QERadioButtonManager::isContainer() const {
    return false;
}

/*
    ???
*/
/*QString QERadioButtonManager::domXml() const {
    return "<widget class=\"QERadioButton\" name=\"qERadioButton\">\n"
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
QString QERadioButtonManager::includeFile() const {
    return "QERadioButton.h";
}
