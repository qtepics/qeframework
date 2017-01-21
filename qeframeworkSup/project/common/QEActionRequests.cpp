/*  QEActionRequests.cpp
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
 *
 */

#include "QEActionRequests.h"

//---------------------------------------------------------------------------
//
QEActionRequests::QEActionRequests ()
{
   this->kind = KindNone;
   option = OptionNewWindow;
   initialise = false;
   originator = NULL;
}

//---------------------------------------------------------------------------
//
QEActionRequests::QEActionRequests( const QString& actionIn,
                                    const QString& pvName)
{
   kind = KindAction;
   action = actionIn;
   arguments << pvName;
   option = OptionNewWindow;
   initialise = false;
   originator = NULL;
   formHandle = QEFormMapper::nullHandle();
}

//---------------------------------------------------------------------------
//
QEActionRequests::QEActionRequests( const QString& actionIn,
                                    const QString& widgetNameIn,
                                    const QStringList& argumentsIn,
                                    const bool& initialiseIn,
                                    QAction* originatorIn )
{
   kind = KindWidgetAction;
   action = actionIn;
   widgetName = widgetNameIn;
   arguments = argumentsIn;
   option = OptionNewWindow; // not required, but keep things neat
   initialise = initialiseIn;
   originator = originatorIn;
   formHandle = QEFormMapper::nullHandle();
}

//---------------------------------------------------------------------------
//
QEActionRequests::QEActionRequests (const QString & filename,
                                    const QString & customisationIn,
                                    const Options optionIn,
                                    const QEFormMapper::FormHandles& formHandleIn)
{
   this->kind = KindOpenFile;
   this->arguments << filename;
   this->option = optionIn;
   initialise = false;
   originator = NULL;
   customisation = customisationIn;
   formHandle = formHandleIn;
}

//---------------------------------------------------------------------------
//
QEActionRequests::QEActionRequests( const QList<windowCreationListItem> windowsIn )
{
   kind = KindOpenFiles;
   option = OptionNewWindow; // not required, but keep things neat
   initialise = false;
   originator = NULL;
   formHandle = QEFormMapper::nullHandle();

   for( int i = 0; i < windowsIn.count(); i++ )
   {
       windows.append( windowsIn.at(i) );
   }
}

//---------------------------------------------------------------------------
//
QEActionRequests::QEActionRequests( const componentHostListItem& componentIn )
{
   kind = KindHostComponents;
   option = OptionFloatingDockWindow; // not required, but keep things neat
   initialise = false;
   originator = NULL;
   formHandle = QEFormMapper::nullHandle();
   components.append( componentIn );
}

//---------------------------------------------------------------------------
//
QEActionRequests::QEActionRequests( const QList<componentHostListItem>& componentsIn )
{
   kind = KindHostComponents;
   option = OptionFloatingDockWindow; // not required, but keep things neat
   initialise = false;
   originator = NULL;
   formHandle = QEFormMapper::nullHandle();

   for( int i = 0; i < componentsIn.count(); i++ )
   {
       components.append( componentsIn.at(i) );
   }
}

//---------------------------------------------------------------------------
// Set and get functions.
//
void QEActionRequests::setKind (const Kinds kindIn)
{
   this->kind = kindIn;
}

QEActionRequests::Kinds QEActionRequests::getKind () const
{
   return this->kind;
}

//---------------------------------------------------------------------------
//
void QEActionRequests::setArguments (const QStringList & argumentsIn)
{
   this->arguments = argumentsIn;
}

QStringList QEActionRequests::getArguments () const
{
   return this->arguments;
}

//---------------------------------------------------------------------------//
//
void QEActionRequests::setOption (const Options optionIn)
{
   this->option = optionIn;
}

QEActionRequests::Options QEActionRequests::getOption () const
{
   return this->option;
}

//---------------------------------------------------------------------------//
//
void QEActionRequests::setAction(const QString actionIn )
{
   action = actionIn;
}

QString QEActionRequests::getAction() const
{
   return action;
}

//---------------------------------------------------------------------------//
//
void QEActionRequests::setWidgetName(const QString widgetNameIn )
{
   widgetName = widgetNameIn;
}

QString QEActionRequests::getWidgetName() const
{
   return widgetName;
}

//---------------------------------------------------------------------------
//
void QEActionRequests::setInitialise (const bool initialiseIn )
{
   initialise = initialiseIn;
}

bool QEActionRequests::getInitialise() const
{
   return initialise;
}

//---------------------------------------------------------------------------
//
void QEActionRequests::setOriginator( QAction* originatorIn )
{
   originator = originatorIn;
}

QAction* QEActionRequests::getOriginator() const
{
   return originator;
}

//---------------------------------------------------------------------------
//
void QEActionRequests::setFormHandle (const QEFormMapper::FormHandles& formHandleIn)
{
   formHandle = formHandleIn;
}

QEFormMapper::FormHandles QEActionRequests::getFormHandle () const
{
   return formHandle;
}

//---------------------------------------------------------------------------//
//
QString QEActionRequests::getCustomisation() const
{
   return customisation;
}

//---------------------------------------------------------------------------//
// Return true if creation option creates a dock
bool QEActionRequests::isDockCreationOption( const Options createOption )
{
    // Use a switch so compiler can complain if all cases are not considered
    bool result = false;
    switch( createOption )
    {
        case OptionOpen:
        case OptionNewTab:
        case OptionNewWindow:
            result = false;
            break;

        case OptionTopDockWindow:
        case OptionBottomDockWindow:
        case OptionLeftDockWindow:
        case OptionRightDockWindow:
        case OptionTopDockWindowTabbed:
        case OptionBottomDockWindowTabbed:
        case OptionLeftDockWindowTabbed:
        case OptionRightDockWindowTabbed:
        case OptionFloatingDockWindow:
            result = true;
            break;
    }
    return result;
}

//---------------------------------------------------------------------------//
// Creation option creates a tabbed dock
bool QEActionRequests::isTabbedDockCreationOption( const Options createOption )
{
    // Use a switch so compiler can complain if all cases are not considered
    bool result = false;
    switch( createOption )
    {
        case OptionOpen:
        case OptionNewTab:
        case OptionNewWindow:
        case OptionTopDockWindow:
        case OptionBottomDockWindow:
        case OptionLeftDockWindow:
        case OptionRightDockWindow:
        case OptionFloatingDockWindow:
            result = false;
            break;

        case OptionTopDockWindowTabbed:
        case OptionBottomDockWindowTabbed:
        case OptionLeftDockWindowTabbed:
        case OptionRightDockWindowTabbed:
            result = true;
            break;
    }
    return result;
}

//---------------------------------------------------------------------------//
//
QList<windowCreationListItem> QEActionRequests::getWindows() const
{
    return windows;
}

//---------------------------------------------------------------------------//
//
QList<componentHostListItem> QEActionRequests::getComponents() const
{
    return components;
}

// end
