/*  QEArchiveNameSearchManager.cpp
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

#include <QtPlugin>

#include "QEArchiveNameSearch.h"
#include "QEArchiveNameSearchManager.h"

//------------------------------------------------------------------------------
//
QEArchiveNameSearchManager::QEArchiveNameSearchManager (QObject * parent):QObject (parent)
{
   initialized = false;
}

//------------------------------------------------------------------------------
//
void QEArchiveNameSearchManager::initialize (QDesignerFormEditorInterface *)
{
   if (initialized) {
      return;
   }
   initialized = true;
}

//------------------------------------------------------------------------------
//
bool QEArchiveNameSearchManager::isInitialized () const
{
   return initialized;
}

//------------------------------------------------------------------------------
//
QWidget *QEArchiveNameSearchManager::createWidget (QWidget * parent)
{
   return new QEArchiveNameSearch (parent);
}

//------------------------------------------------------------------------------
//
QString QEArchiveNameSearchManager::name () const
{
   return "QEArchiveNameSearch";
}

//------------------------------------------------------------------------------
//
QString QEArchiveNameSearchManager::group () const
{
   return "EPICSQt Infrastructure Widgets";
}

//------------------------------------------------------------------------------
//
QIcon QEArchiveNameSearchManager::icon () const
{
   return QIcon (":/qe/archive/name_search.png");
}

//------------------------------------------------------------------------------
//
QString QEArchiveNameSearchManager::toolTip () const
{
   return "Channel Archiver Name Search";
}

//------------------------------------------------------------------------------
//
QString QEArchiveNameSearchManager::whatsThis () const
{
   return "Channel Archiver Name Search";
}

//------------------------------------------------------------------------------
//
bool QEArchiveNameSearchManager::isContainer () const
{
   return false;
}

//------------------------------------------------------------------------------
//
QString QEArchiveNameSearchManager::includeFile () const
{
   return "QEArchiveNameSearch.h";
}

// end
