/*  QEGeneralEditManager.cpp
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
 *  Copyright (c) 2014 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QEGeneralEditManager.h>
#include <QEGeneralEdit.h>
#include <QtPlugin>

//------------------------------------------------------------------------------
//
QEGeneralEditManager::QEGeneralEditManager (QObject * parent):QObject (parent)
{
   initialized = false;
}

//------------------------------------------------------------------------------
//
void QEGeneralEditManager::initialize (QDesignerFormEditorInterface *)
{
   if (initialized) {
      return;
   }
   initialized = true;
}

//------------------------------------------------------------------------------
//
bool QEGeneralEditManager::isInitialized () const
{
   return initialized;
}

//------------------------------------------------------------------------------
// Widget factory. Creates a QEGeneralEdit widget.
//
QWidget *QEGeneralEditManager::createWidget (QWidget * parent)
{
   return new QEGeneralEdit (parent);
}

//------------------------------------------------------------------------------
// Name for widget. Used by Qt Designer in widget list.
//
QString QEGeneralEditManager::name () const
{
   return "QEGeneralEdit";
}

//------------------------------------------------------------------------------
// Name of group Qt Designer will add widget to.
//
QString QEGeneralEditManager::group () const
{
   return "EPICSQt Controls";
}

//------------------------------------------------------------------------------
// Icon for widget. Used by Qt Designer in widget list.
//
QIcon QEGeneralEditManager::icon () const
{
   return QIcon (":/qe/generaledit/QEGeneralEdit.png");
}

//------------------------------------------------------------------------------
// Tool tip for widget. Used by Qt Designer in widget list.
//
QString QEGeneralEditManager::toolTip () const
{
   return "EPICS General PV Edit";
}

//------------------------------------------------------------------------------
//
QString QEGeneralEditManager::whatsThis () const
{
   return "EPICS General PV Edit";
}

//------------------------------------------------------------------------------
//
bool QEGeneralEditManager::isContainer () const
{
   return false;
}

//------------------------------------------------------------------------------
//
/*QString QEGeneralEditManager::domXml() const {
    return "<widget class=\"QEGeneralEdit\" name=\"qERadioGroup\">\n"
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

//------------------------------------------------------------------------------
//
QString QEGeneralEditManager::includeFile () const
{
   return "QEGeneralEdit.h";
}

// end
