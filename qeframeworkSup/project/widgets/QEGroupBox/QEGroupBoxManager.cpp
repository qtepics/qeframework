/*  QEGroupBoxManager.cpp
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QEGroupBoxManager.h>
#include <QEGroupBox.h>
#include <QtPlugin>

//------------------------------------------------------------------------------
QEGroupBoxManager::QEGroupBoxManager (QObject * parent):QObject (parent)
{
   initialized = false;
}

//------------------------------------------------------------------------------
void QEGroupBoxManager::initialize (QDesignerFormEditorInterface *)
{
   if (initialized) {
      return;
   }
   initialized = true;
}

//------------------------------------------------------------------------------
bool QEGroupBoxManager::isInitialized () const
{
   return initialized;
}

//------------------------------------------------------------------------------
QWidget *QEGroupBoxManager::createWidget (QWidget * parent)
{
   return new QEGroupBox (parent);
}

//------------------------------------------------------------------------------
QString QEGroupBoxManager::name () const
{
   return "QEGroupBox";
}

//------------------------------------------------------------------------------
QString QEGroupBoxManager::group () const
{
   return "EPICSQt Application Support Widgets";
}

//------------------------------------------------------------------------------
QIcon QEGroupBoxManager::icon () const
{
   return QIcon (":/qe/groupbox/QEGroupBox.png");
}

//------------------------------------------------------------------------------
QString QEGroupBoxManager::toolTip () const
{
   return "QE framework groupbox";
}

//------------------------------------------------------------------------------
QString QEGroupBoxManager::whatsThis () const
{
   return "QE framework groupbox";
}

//------------------------------------------------------------------------------
bool QEGroupBoxManager::isContainer () const
{
   return true;
}

//------------------------------------------------------------------------------
QString QEGroupBoxManager::includeFile () const
{
   return "QEGroupBox.h";
}

// end
