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

#include <QELoginManager.h>
#include <QELogin.h>



QELoginManager::QELoginManager(QObject *pParent):QObject(pParent)
{

    initialized = false;

}



void QELoginManager::initialize(QDesignerFormEditorInterface *)
{

    if (initialized == false)
    {
        initialized = true;
    }

}



bool QELoginManager::isInitialized() const
{

    return initialized;

}



QWidget *QELoginManager::createWidget(QWidget *pParent)
{

    return new QELogin(pParent);
}



QString QELoginManager::name() const
{

    return "QELogin";

}



QString QELoginManager::group() const
{

    return "EPICSQt Application Support Widgets";

}



QIcon QELoginManager::icon() const
{

    return QIcon(":/qe/login/QELogin.png");

}



QString QELoginManager::toolTip() const
{

    return "EPICS Login";

}



QString QELoginManager::whatsThis() const
{

    return "EPICS Login";

}



bool QELoginManager::isContainer() const
{

    return false;
}



QString QELoginManager::includeFile() const
{

    return "QELoginManager.h";

}




/*QString QELoginManager::domXml() const {
    return "<widget class=\"QELogin\" name=\"qELogin\">\n"
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

