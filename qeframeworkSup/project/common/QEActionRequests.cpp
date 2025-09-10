/*  QEActionRequests.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2025 Australian Synchrotron
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
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#include "QEActionRequests.h"

//==============================================================================
// QEActionRequests
//==============================================================================
//
QEActionRequests::QEActionRequests ()
{
   this->kind = KindNone;
   this->option = QE::NewWindow;
   this->initialise = false;
   this->originator = NULL;
}

//------------------------------------------------------------------------------
//
QEActionRequests::QEActionRequests( const QString& actionIn,
                                    const QString& pvName)
{
   this->kind = KindAction;
   this->action = actionIn;
   this->arguments << pvName;
   this->option = QE::NewWindow;
   this->initialise = false;
   this->originator = NULL;
   this->formHandle = QEFormMapper::nullHandle();
}

//------------------------------------------------------------------------------
//
QEActionRequests::QEActionRequests( const QString& actionIn,
                                    const QString& widgetNameIn,
                                    const QStringList& argumentsIn,
                                    const bool& initialiseIn,
                                    QAction* originatorIn )
{
   this->kind = KindWidgetAction;
   this->action = actionIn;
   this->widgetName = widgetNameIn;
   this->arguments = argumentsIn;
   this->option = QE::NewWindow; // not required, but keep things neat
   this->initialise = initialiseIn;
   this->originator = originatorIn;
   this->formHandle = QEFormMapper::nullHandle();
}

//------------------------------------------------------------------------------
//
QEActionRequests::QEActionRequests (const QString & filename,
                                    const QString & customisationIn,
                                    const QE::CreationOptions optionIn,
                                    const QEFormMapper::FormHandles& formHandleIn)
{
   this->kind = KindOpenFile;
   this->arguments << filename;
   this->option = optionIn;
   this->initialise = false;
   this->originator = NULL;
   this->customisation = customisationIn;
   this->formHandle = formHandleIn;
}

//------------------------------------------------------------------------------
//
QEActionRequests::QEActionRequests( const QList<windowCreationListItem> windowsIn )
{
   this->kind = KindOpenFiles;
   this->option = QE::NewWindow; // not required, but keep things neat
   this->initialise = false;
   this->originator = NULL;
   this->formHandle = QEFormMapper::nullHandle();

   for( int i = 0; i < windowsIn.count(); i++ )
   {
      this->windows.append( windowsIn.at(i) );
   }
}

//------------------------------------------------------------------------------
//
QEActionRequests::QEActionRequests( const componentHostListItem& componentIn )
{
   this->kind = KindHostComponents;
   this->option = QE::DockFloating; // not required, but keep things neat
   this->initialise = false;
   this->originator = NULL;
   this->formHandle = QEFormMapper::nullHandle();
   this->components.append (componentIn);
}

//------------------------------------------------------------------------------
//
QEActionRequests::QEActionRequests( const QList<componentHostListItem>& componentsIn )
{
   this->kind = KindHostComponents;
   this->option = QE::DockFloating; // not required, but keep things neat
   this->initialise = false;
   this->originator = NULL;
   this->formHandle = QEFormMapper::nullHandle();

   for( int i = 0; i < componentsIn.count(); i++ )
   {
      this->components.append( componentsIn.at(i) );
   }
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
//
void QEActionRequests::setArguments (const QStringList & argumentsIn)
{
   this->arguments = argumentsIn;
}

QStringList QEActionRequests::getArguments () const
{
   return this->arguments;
}

//------------------------------------------------------------------------------
//
void QEActionRequests::setOption (const QE::CreationOptions optionIn)
{
   this->option = optionIn;
}

QE::CreationOptions QEActionRequests::getOption () const
{
   return this->option;
}

//------------------------------------------------------------------------------
//
void QEActionRequests::setAction(const QString actionIn )
{
   this->action = actionIn;
}

QString QEActionRequests::getAction() const
{
   return this->action;
}

//------------------------------------------------------------------------------
//
void QEActionRequests::setWidgetName(const QString widgetNameIn )
{
   this->widgetName = widgetNameIn;
}

QString QEActionRequests::getWidgetName() const
{
   return this->widgetName;
}

//------------------------------------------------------------------------------
//
void QEActionRequests::setInitialise (const bool initialiseIn )
{
   this->initialise = initialiseIn;
}

bool QEActionRequests::getInitialise() const
{
   return this->initialise;
}

//------------------------------------------------------------------------------
//
void QEActionRequests::setOriginator( QAction* originatorIn )
{
   this->originator = originatorIn;
}

QAction* QEActionRequests::getOriginator() const
{
   return this->originator;
}

//------------------------------------------------------------------------------
//
void QEActionRequests::setFormHandle (const QEFormMapper::FormHandles& formHandleIn)
{
   this->formHandle = formHandleIn;
}

QEFormMapper::FormHandles QEActionRequests::getFormHandle () const
{
   return this->formHandle;
}

//------------------------------------------------------------------------------
//
QString QEActionRequests::getCustomisation() const
{
   return this->customisation;
}

//------------------------------------------------------------------------------
// Return true if creation option creates a dock.
//
bool QEActionRequests::isDockCreationOption( const QE::CreationOptions createOption )
{
   // Use a switch so compiler can complain if all cases are not considered
   bool result = false;
   switch( createOption )
   {
      case QE::Open:
      case QE::NewTab:
      case QE::NewWindow:
         result = false;
         break;

      case QE::DockTop:
      case QE::DockBottom:
      case QE::DockLeft:
      case QE::DockRight:
      case QE::DockTopTabbed:
      case QE::DockBottomTabbed:
      case QE::DockLeftTabbed:
      case QE::DockRightTabbed:
      case QE::DockFloating:
         result = true;
         break;
   }
   return result;
}

//------------------------------------------------------------------------------
// Creation option creates a tabbed dock.
//
bool QEActionRequests::isTabbedDockCreationOption( const QE::CreationOptions createOption )
{
   // Use a switch so compiler can complain if all cases are not considered
   bool result = false;
   switch( createOption )
   {
      case QE::Open:
      case QE::NewTab:
      case QE::NewWindow:
      case QE::DockTop:
      case QE::DockBottom:
      case QE::DockLeft:
      case QE::DockRight:
      case QE::DockFloating:
         result = false;
         break;

      case QE::DockTopTabbed:
      case QE::DockBottomTabbed:
      case QE::DockLeftTabbed:
      case QE::DockRightTabbed:
         result = true;
         break;
   }
   return result;
}

//------------------------------------------------------------------------------
//
QList<windowCreationListItem> QEActionRequests::getWindows() const
{
   return this->windows;
}

//------------------------------------------------------------------------------
//
QList<componentHostListItem> QEActionRequests::getComponents() const
{
   return this->components;
}

//------------------------------------------------------------------------------
//
QDebug operator<<(QDebug dbg, const QEActionRequests& ar)
{
   const QEActionRequests::Kinds kind = ar.getKind();
   switch (kind) {
      case QEActionRequests::KindNone:            // no action (default, not valid in any request)
         dbg << "Kind: None";
         break;

      case QEActionRequests::KindOpenFile:        // by file name, e.g. "detector_control.ui"
         dbg << "Kind: Open File" << ar.getArguments();
         break;

      case QEActionRequests::KindOpenFiles:       // by file names, tahe a list of windowCreationListItems
         dbg << "Kind: Open Files" << ar.getWindows();
         break;

      case QEActionRequests::KindAction:          // inbuilt application action, e.g "PV Properties..."
         dbg << "Kind: Action" << ar.getAction() << ar.getArguments();
         break;

      case QEActionRequests::KindWidgetAction:    // inbuilt QE widget action
         dbg << "Kind: Widget Action" << ar.getAction() << ar.getWidgetName() << ar.getArguments();
         break;

      case QEActionRequests::KindHostComponents:  // componentHostListItem or list thereof.
         dbg << "Kind: Host Components" << ar.getComponents();
         break;

      default:
         dbg << "Kind: Error " << int(kind);
         break;
   }

   return dbg.maybeSpace ();
}

//==============================================================================
// windowCreationListItem
//==============================================================================
//
windowCreationListItem::windowCreationListItem()
{
   this->hidden = false;
   this->creationOption = QE::NewWindow;
   this->formHandle = QEFormMapper::nullHandle ();
}

//------------------------------------------------------------------------------
//
windowCreationListItem::windowCreationListItem (windowCreationListItem* item)
{
   this->uiFile = item->uiFile;
   this->macroSubstitutions = item->macroSubstitutions;
   this->customisationName = item->customisationName;
   this->formHandle = item->formHandle;
   this->creationOption = item->creationOption;
   this->hidden = item->hidden;
   this->title = item->title;
}

//------------------------------------------------------------------------------
//
windowCreationListItem::~windowCreationListItem() {}


//==============================================================================
// componentHostListItem
//==============================================================================
//
componentHostListItem::componentHostListItem()
{
   this->widget = NULL;
   this->hidden = false;
   this->creationOption = QE::DockFloating;
}

//------------------------------------------------------------------------------
//
componentHostListItem::componentHostListItem (QWidget* widgetIn,
                                              QE::CreationOptions creationOptionIn,
                                              bool hiddenIn,
                                              QString titleIn)
{
   this->widget = widgetIn;
   this->creationOption = creationOptionIn;
   this->hidden = hiddenIn;
   this->title = titleIn;
}

//------------------------------------------------------------------------------
//
componentHostListItem::componentHostListItem (componentHostListItem* item)
{
   this->widget = item->widget;
   this->creationOption = item->creationOption;
   this->hidden = item->hidden;
   this->title = item->title;
}

//------------------------------------------------------------------------------
//
componentHostListItem::~componentHostListItem() {}

// end
