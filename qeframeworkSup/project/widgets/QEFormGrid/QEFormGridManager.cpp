/*  QEFormGridManager.cpp
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
 *  Copyright (c) 2011 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QEFormGridManager.h>
#include <QEFormGrid.h>
#include <QtPlugin>

//---------------------------------------------------------------------------------
//
QEFormGridManager::QEFormGridManager (QObject * parent):QObject (parent)
{
   initialized = false;
}

//---------------------------------------------------------------------------------
//
void QEFormGridManager::initialize (QDesignerFormEditorInterface *)
{
   if (initialized) {
      return;
   }
   initialized = true;
}

//---------------------------------------------------------------------------------
//
bool QEFormGridManager::isInitialized () const
{
   return initialized;
}

//---------------------------------------------------------------------------------
// Widget factory. Creates a QEFormGrid widget.
//
QWidget *QEFormGridManager::createWidget (QWidget * parent)
{
   return new QEFormGrid (parent);
}

//---------------------------------------------------------------------------------
// Name for widget. Used by Qt Designer in widget list.

QString QEFormGridManager::name () const
{
   return "QEFormGrid";
}

//---------------------------------------------------------------------------------
// Name of group Qt Designer will add widget to.

QString QEFormGridManager::group () const
{
   return "EPICSQt Application Support Widgets";
}

//---------------------------------------------------------------------------------
// Icon for widget. Used by Qt Designer in widget list.
//
QIcon QEFormGridManager::icon () const
{
   return QIcon (":/qe/formgrid/QEFormGrid.png");
}

//---------------------------------------------------------------------------------
// Tool tip for widget. Used by Qt Designer in widget list.
//
QString QEFormGridManager::toolTip () const
{
   return "EPICS QE Form Grid";
}

//---------------------------------------------------------------------------------
//
QString QEFormGridManager::whatsThis () const
{
   return "EPICS Radio Group";
}

//---------------------------------------------------------------------------------
//
bool QEFormGridManager::isContainer () const
{
   return false;
}

//---------------------------------------------------------------------------------
//
/*QString QEFormGridManager::domXml() const {
    return "<widget class=\"QEFormGrid\" name=\"qEFormGrid\">\n"
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
}
*/

//---------------------------------------------------------------------------------
//
QString QEFormGridManager::includeFile () const
{
   return "QEFormGrid.h";
}

// end
