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
 *  Copyright (c) 2011 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QEPvLoadSaveManager.h>
#include <QEPvLoadSave.h>
#include <QtPlugin>

/*
    ???
*/
QEPvLoadSaveManager::QEPvLoadSaveManager (QObject * parent):QObject (parent)
{
   initialized = false;
}

/*
    ???
*/
void QEPvLoadSaveManager::initialize (QDesignerFormEditorInterface *)
{
   if (initialized) {
      return;
   }
   initialized = true;
}

/*
    ???
*/
bool QEPvLoadSaveManager::isInitialized () const
{
   return initialized;
}

/*
    Widget factory. Creates a QEPvLoadSave widget.
*/
QWidget *QEPvLoadSaveManager::createWidget (QWidget * parent)
{
   return new QEPvLoadSave (parent);
}

/*
    Name for widget. Used by Qt Designer in widget list.
*/
QString QEPvLoadSaveManager::name () const
{
   return "QEPvLoadSave";
}

/*
    Name of group Qt Designer will add widget to.
*/
QString QEPvLoadSaveManager::group () const
{
   return "EPICSQt Application Support Widgets";
}

/*
    Icon for widget. Used by Qt Designer in widget list.
*/
QIcon QEPvLoadSaveManager::icon () const
{
   return QIcon (":/qe/pvloadsave/QEPvLoadSave.png");
}

/*
    Tool tip for widget. Used by Qt Designer in widget list.
*/
QString QEPvLoadSaveManager::toolTip () const
{
   return "EPICS Alarm Handler";
}

/*
    ???
*/
QString QEPvLoadSaveManager::whatsThis () const
{
   return "EPICS Alarm Handler";
}

/*
    ???
*/
bool QEPvLoadSaveManager::isContainer () const
{
   return false;
}

/*
    ???
*/
/*QString QEPvLoadSaveManager::domXml() const {
    return "<widget class=\"QEPvLoadSave\" name=\"qEPvLoadSave\">\n"
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
QString QEPvLoadSaveManager::includeFile () const
{
   return "QEPvLoadSave.h";
}

// end
