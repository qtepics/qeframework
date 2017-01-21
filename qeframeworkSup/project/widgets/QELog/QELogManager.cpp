/*  QELogManager.cpp
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
 *  Copyright (c) 2012 Australian Synchrotron
 *
 *  Author:
 *    Ricardo Fernandes
 *  Contact details:
 *    ricardo.fernandes@synchrotron.org.au
 */

#include <QELogManager.h>
#include <QELog.h>



QELogManager::QELogManager(QObject *pParent):QObject(pParent)
{

    initialized = false;

}



void QELogManager::initialize(QDesignerFormEditorInterface *)
{

    if (initialized == false)
    {
        initialized = true;
    }

}



bool QELogManager::isInitialized() const
{

    return initialized;

}



QWidget *QELogManager::createWidget(QWidget *pParent)
{

    return new QELog(pParent);
}



QString QELogManager::name() const
{

    return "QELog";

}



QString QELogManager::group() const
{

    return "EPICSQt Application Support Widgets";

}



QIcon QELogManager::icon() const
{

    return QIcon(":/qe/log/QELog.png");

}



QString QELogManager::toolTip() const
{

    return "Message Log";

}



QString QELogManager::whatsThis() const
{

    return "Message Log";

}



bool QELogManager::isContainer() const
{

    return false;
}



QString QELogManager::includeFile() const
{

    return "QELog.h";

}




/*QString QELogManager::domXml() const {
    return "<widget class=\"QELog\" name=\"qELog\">\n"
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

// end
