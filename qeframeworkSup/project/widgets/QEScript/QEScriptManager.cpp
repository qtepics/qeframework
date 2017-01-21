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
 *  Copyright (c) 2012 Australian Synchrotron
 *
 *  Author:
 *    Ricardo Fernandes
 *  Contact details:
 *    ricardo.fernandes@synchrotron.org.au
 */

#include <QEScriptManager.h>
#include <QEScript.h>



QEScriptManager::QEScriptManager(QObject *pParent):QObject(pParent)
{

    initialized = false;

}



void QEScriptManager::initialize(QDesignerFormEditorInterface *)
{

    if (initialized == false)
    {
        initialized = true;
    }

}



bool QEScriptManager::isInitialized() const
{

    return initialized;

}



QWidget *QEScriptManager::createWidget(QWidget *pParent)
{

    return new QEScript(pParent);
}



QString QEScriptManager::name() const
{

    return "QEScript";

}



QString QEScriptManager::group() const
{

    return "EPICSQt Application Support Widgets";

}



QIcon QEScriptManager::icon() const
{

    return QIcon(":/qe/script/QEScript.png");

}



QString QEScriptManager::toolTip() const
{

    return "EPICS Script";

}



QString QEScriptManager::whatsThis() const
{

    return "EPICS Script";

}



bool QEScriptManager::isContainer() const
{

    return false;
}



QString QEScriptManager::includeFile() const
{

    return "QEScriptManager.h";

}




/*QString QEScriptManager::domXml() const {
    return "<widget class=\"QEScript\" name=\"qEScript\">\n"
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

