/*  QEPlotterManager.cpp
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
 *  Copyright (c) 2013 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

// CA Plotter Widget Plugin Manager for designer.

#include <QEPlotterManager.h>
#include <QEPlotter.h>
#include <QtPlugin>

//---------------------------------------------------------------------------------
//
QEPlotterManager::QEPlotterManager( QObject *parent ) : QObject( parent ) {
   initialized = false;
}

//---------------------------------------------------------------------------------
//
void QEPlotterManager::initialize( QDesignerFormEditorInterface * ) {
   if (initialized) {
      return;
   }
   initialized = true;
}

//---------------------------------------------------------------------------------
//
bool QEPlotterManager::isInitialized() const {
   return initialized;
}

//---------------------------------------------------------------------------------
// Widget factory. Creates a QEPlotter widget.
//
QWidget *QEPlotterManager::createWidget ( QWidget *parent ) {
   return new QEPlotter(parent);
}

//---------------------------------------------------------------------------------
// Name for widget. Used by Qt Designer in widget list.
//
QString QEPlotterManager::name() const {
   return "QEPlotter";
}

//---------------------------------------------------------------------------------
// Name of group Qt Designer will add widget to.
//
QString QEPlotterManager::group() const {
   return "EPICSQt Graphics";
}

//---------------------------------------------------------------------------------
// Icon for widget. Used by Qt Designer in widget list.
//
QIcon QEPlotterManager::icon() const {
   return QIcon(":/qe/plotter/QEPlotter.png");
}

//---------------------------------------------------------------------------------
// Tool tip for widget. Used by Qt Designer in widget list.
//
QString QEPlotterManager::toolTip() const {
   return "EPICS Waveform Plotter";
}

//---------------------------------------------------------------------------------
//
QString QEPlotterManager::whatsThis() const {
   return "EPICS Waveform Plotter";
}

//---------------------------------------------------------------------------------
//
bool QEPlotterManager::isContainer() const {
   return false;
}

//---------------------------------------------------------------------------------
//
/*QString QEPlotterManager::domXml() const {
    return "<widget class=\"QEPlotter\" name=\"qEPlotter\">\n"
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

//---------------------------------------------------------------------------------
//
QString QEPlotterManager::includeFile() const {
   return "QEPlotter.h";
}

// end
