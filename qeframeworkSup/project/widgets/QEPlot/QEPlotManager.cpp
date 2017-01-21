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
 *    Glenn Jackson
 *  Contact details:
 *    glenn.jackson@synchrotron.org.au
 */

// CA Plot Widget Plugin Manager for designer.

#include <QEPlotManager.h>
#include <QEPlot.h>
#include <QtPlugin>

/*
    ???
*/
QEPlotManager::QEPlotManager( QObject *parent ) : QObject( parent ) {
    initialized = false;
}

/*
    ???
*/
void QEPlotManager::initialize( QDesignerFormEditorInterface * ) {
    if( initialized ) {
        return;
    }
    initialized = true;
}

/*
    ???
*/
bool QEPlotManager::isInitialized() const {
    return initialized;
}

/*
    Widget factory. Creates a QEPlot widget.
*/
QWidget *QEPlotManager::createWidget ( QWidget *parent ) {
    return new QEPlot(parent);
}

/*
    Name for widget. Used by Qt Designer in widget list.
*/
QString QEPlotManager::name() const {
    return "QEPlot";
}

/*
    Name of group Qt Designer will add widget to.
*/
QString QEPlotManager::group() const {
    return "EPICSQt Graphics";
}

/*
    Icon for widget. Used by Qt Designer in widget list.
*/
QIcon QEPlotManager::icon() const {
    return QIcon(":/qe/plot/QEPlot.png");
}

/*
    Tool tip for widget. Used by Qt Designer in widget list.
*/
QString QEPlotManager::toolTip() const {
    return "EPICS Strip Chart";
}

/*
    ???
*/
QString QEPlotManager::whatsThis() const {
    return "EPICS Strip Chart";
}

/*
    ???
*/
bool QEPlotManager::isContainer() const {
    return false;
}

/*
    ???
*/
/*QString QEPlotManager::domXml() const {
    return "<widget class=\"QEPlot\" name=\"qEPlot\">\n"
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
QString QEPlotManager::includeFile() const {
    return "QEPlot.h";
}
