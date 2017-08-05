/*  QEDesignerPluginCommon.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2014,2016,2107 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_DESIGNER_PLUGIN_COMMON_H
#define QE_DESIGNER_PLUGIN_COMMON_H

#include <QtGlobal>

// Provide Qt version independent plugin-related includes.
//
#if (QT_VERSION >= 0x050500)
#include <QtUiPlugin/QDesignerCustomWidgetInterface>
#else
#include <QDesignerCustomWidgetInterface>
#endif

#include <QIcon>
#include <QObject>
#include <QString>
#include <QWidget>

// Macros to do all the designer plugin plumbing for straight forward widgets.
// If the plugin defines design time dialog (as per QEPeriodic) or provides any
// other extra-ordinary functionality then this macro is not suitable.
//
// Note: where as these macros are only used within QEPlugin library (as opposed to
// in the QEFramework library), this file has not been relocated as other 3rd party
// plugins still expect these defitions to be placed in the include directory.

// This macro declares the plugin in class members. Note: there are moc issues
// with trying to declare whole class using a macro.
//
// Macro formal parameter:
// widgetName    - class type name (no quotes). The manager class name must be the
//                 class name with Manager appended.
//
// Example: QE_DECLARE_PLUGIN_MANAGER (QESimpleShape)
//
#define QE_DECLARE_PLUGIN_MANAGER(widgetName)                   \
public:                                                         \
   widgetName##Manager (QObject* parent = 0);                   \
   bool isContainer () const;                                   \
   bool isInitialized () const;                                 \
   QIcon icon () const;                                         \
   QString group () const;                                      \
   QString includeFile () const;                                \
   QString name () const;                                       \
   QString toolTip () const;                                    \
   QString whatsThis () const;                                  \
   QWidget* createWidget (QWidget* parent);                     \
   void initialize (QDesignerFormEditorInterface* core);        \
private:                                                        \
   bool initialized;



// This macro provides the complete class function implementations.
//
// Macro formal parameters:
// widgetName    - class type name (no quotes). The manager class name must be the
//                 class name with Manager appended, and the include file name must
//                 be the class name with ".h" appended, e.g.:
//                 QENumericEdit, QENumericEditManager, "QENumericEdit.h"
// groupName     - string - typically "EPICS Widgets"
// iconFilepath  - string - typically a resoure file ":/qe/plugin/classname.png"
// containerFlag - true/false - typically false except for QEFrame, QEGroupBox and
//                 other container widgets.
//
// Example: QE_IMPLEMENT_PLUGIN_MANAGER
//             (QESimpleShape, "EPICSQt Monitors", ":/qe/plugin/QESimpleShape.png", false)
//
//
#define QE_IMPLEMENT_PLUGIN_MANAGER(widgetName, groupName, iconFilepath, containerFlag)  \
                                                                      \
widgetName##Manager::widgetName##Manager (QObject* parent) :          \
   QObject (parent)                                                   \
{                                                                     \
   initialized = false;                                               \
}                                                                     \
                                                                      \
void widgetName##Manager::initialize (QDesignerFormEditorInterface*)  \
{                                                                     \
   if (initialized) {                                                 \
      return;                                                         \
   }                                                                  \
   initialized = true;                                                \
}                                                                     \
                                                                      \
bool widgetName##Manager::isInitialized () const                      \
{                                                                     \
   return initialized;                                                \
}                                                                     \
                                                                      \
QWidget* widgetName##Manager::createWidget (QWidget* parent)          \
{                                                                     \
   return new widgetName (parent);                                    \
}                                                                     \
                                                                      \
QString widgetName##Manager::name() const                             \
{                                                                     \
   return #widgetName;                                                \
}                                                                     \
                                                                      \
QString widgetName##Manager::group() const                            \
{                                                                     \
   return groupName;                                                  \
}                                                                     \
                                                                      \
QIcon widgetName##Manager::icon() const                               \
{                                                                     \
   return QIcon (iconFilepath);                                       \
}                                                                     \
                                                                      \
QString widgetName##Manager::toolTip() const                          \
{                                                                     \
   return #widgetName;                                                \
}                                                                     \
                                                                      \
QString widgetName##Manager::whatsThis() const                        \
{                                                                     \
   return #widgetName;                                                \
}                                                                     \
                                                                      \
bool widgetName##Manager::isContainer() const                         \
{                                                                     \
   return containerFlag;                                              \
}                                                                     \
                                                                      \
QString widgetName##Manager::includeFile() const                      \
{                                                                     \
   return #widgetName".h";                                            \
}

#endif  // QE_DESIGNER_PLUGIN_COMMON_H
