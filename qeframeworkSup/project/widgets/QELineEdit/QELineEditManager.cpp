/*  QELineEditManager.cpp
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
 *  Copyright (c) 2009, 2010 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#include <QELineEditManager.h>
#include <QELineEdit.h>
#include <QtPlugin>

/*
    ???
*/
QELineEditManager::QELineEditManager( QObject *parent ) : QObject( parent ) {
    initialized = false;
}

/*
    ???
*/
void QELineEditManager::initialize( QDesignerFormEditorInterface * ) {
    if( initialized ) {
        return;
    }
    initialized = true;
}

/*
    ???
*/
bool QELineEditManager::isInitialized() const {
    return initialized;
}

/*
    Widget factory. Creates a QELineEditPlugin widget.
*/
QWidget *QELineEditManager::createWidget ( QWidget *parent ) {
    return new QELineEdit( parent );
}

/*
    Name for widget. Used by Qt Designer in widget list.
*/
QString QELineEditManager::name() const {
    return "QELineEdit";
}

/*
    Name of group Qt Designer will add widget to.
*/
QString QELineEditManager::group() const {
    return "EPICSQt Controls";
}

/*
    Icon for widget. Used by Qt Designer in widget list.
*/
QIcon QELineEditManager::icon() const {
    return QIcon(":/qe/lineedit/QELineEdit.png");
}

/*
    Tool tip for widget. Used by Qt Designer in widget list.
*/
QString QELineEditManager::toolTip() const {
    return "EPICS Line Edit";
}

/*
    ???
*/
QString QELineEditManager::whatsThis() const {
    return "EPICS Line Edit";
}

/*
    ???
*/
bool QELineEditManager::isContainer() const {
    return false;
}

/*
    ???
*/
/*QString QELineEditManager::domXml() const {
    return "<widget class=\"QELineEdit\" name=\"qELineEdit\">\n"
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
QString QELineEditManager::includeFile() const {
    return "QELineEdit.h";
}

// end
