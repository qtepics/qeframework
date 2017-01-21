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

#include <QESliderManager.h>
#include <QESlider.h>
#include <QtPlugin>

/*
    ???
*/
QESliderManager::QESliderManager( QObject *parent ) : QObject( parent ) {
    initialized = false;
}

/*
    ???
*/
void QESliderManager::initialize( QDesignerFormEditorInterface * ) {
    if( initialized ) {
        return;
    }
    initialized = true;
}

/*
    ???
*/
bool QESliderManager::isInitialized() const {
    return initialized;
}

/*
    Widget factory. Creates a QESlider widget.
*/
QWidget *QESliderManager::createWidget ( QWidget *parent ) {
    return new QESlider(parent);
}

/*
    Name for widget. Used by Qt Designer in widget list.
*/
QString QESliderManager::name() const {
    return "QESlider";
}

/*
    Name of group Qt Designer will add widget to.
*/
QString QESliderManager::group() const {
    return "EPICSQt Controls";
}

/*
    Icon for widget. Used by Qt Designer in widget list.
*/
QIcon QESliderManager::icon() const {
    return QIcon(":/qe/slider/QESlider.png");
}

/*
    Tool tip for widget. Used by Qt Designer in widget list.
*/
QString QESliderManager::toolTip() const {
    return "EPICS Slider";
}

/*
    ???
*/
QString QESliderManager::whatsThis() const {
    return "EPICS Slider";
}

/*
    ???
*/
bool QESliderManager::isContainer() const {
    return false;
}

/*
    ???
*/
/*QString QESliderManager::domXml() const {
    return "<widget class=\"QESlider\" name=\"qESlider\">\n"
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
QString QESliderManager::includeFile() const {
    return "QESlider.h";
}
