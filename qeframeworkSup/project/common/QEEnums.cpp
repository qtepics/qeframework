/*  QEEnums.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2025 Australian Synchrotron
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
 *    andrews@ansto.gov.au
 */

#include "QEEnums.h"
#include <QECommon.h>

//------------------------------------------------------------------------------
//
QE::QE (QObject* parent) : QObject (parent) {}

//------------------------------------------------------------------------------
//
QE::~QE () {}


//------------------------------------------------------------------------------
// Macro to create the actual utility functions.
// Maybe the Number could be auto-magic, however easier just to be explict.
//
#define QE_ENUM_FN_IMPL(typeName, Number)                                      \
                                                                               \
int QE::typeName##Count() { return Number; }                                   \
                                                                               \
QString QE::typeName##Image (const typeName enumValue)                         \
{                                                                              \
   /* This object provides access to the meta objects that allows */           \
   /* access to the enumeration values. */                                     \
   static QE refObject (NULL);                                                 \
   return QEUtilities::enumToString (refObject, #typeName, int (enumValue));   \
}                                                                              \
                                                                               \
QE::typeName QE::typeName##Value (const QString& image, bool& okay)            \
{                                                                              \
   static QE refObject (NULL);                                                 \
   int n = QEUtilities::stringToEnum (refObject, #typeName, image, &okay);     \
   return static_cast<typeName>(n);                                            \
}


//------------------------------------------------------------------------------
// Unfortunatly we have to explicitly define the count/number of items.
// On the plus side, this are very unlikely to change ... much.
//
QE_ENUM_FN_IMPL (MessageFilterOptions, 3)
QE_ENUM_FN_IMPL (UserLevels, 3)
QE_ENUM_FN_IMPL (DisplayAlarmStateOptions, 4)
QE_ENUM_FN_IMPL (Formats, 7)
QE_ENUM_FN_IMPL (Separators, 4)
QE_ENUM_FN_IMPL (Notations, 3)
QE_ENUM_FN_IMPL (ArrayActions, 3)
QE_ENUM_FN_IMPL (Radicies, 4)
QE_ENUM_FN_IMPL (UpdateOptions, 8);
QE_ENUM_FN_IMPL (DisabledRecordPolicy, 3)
QE_ENUM_FN_IMPL (ProgramStartupOptions, 4)
QE_ENUM_FN_IMPL (CreationOptions, 12)
QE_ENUM_FN_IMPL (VideoModes, 2)
QE_ENUM_FN_IMPL (DropOption, 3)
QE_ENUM_FN_IMPL (MouseMoveSignals, 4)
QE_ENUM_FN_IMPL (ImageFormatOptions, 12)
QE_ENUM_FN_IMPL (RotationOptions, 4)
QE_ENUM_FN_IMPL (PVLabelMode, 3)
QE_ENUM_FN_IMPL (LayoutOptions, 4)
QE_ENUM_FN_IMPL (SourceOptions, 2)
QE_ENUM_FN_IMPL (GridOrders, 2)

//------------------------------------------------------------------------------
//
static bool registerMetaTypes()
{
   qRegisterMetaType<QE::MessageFilterOptions> ("QE::MessageFilterOptions");
   qRegisterMetaType<QE::UserLevels> ("QE::UserLevels");
   qRegisterMetaType<QE::DisplayAlarmStateOptions> ("QE::DisplayAlarmStateOptions");
   qRegisterMetaType<QE::Formats> ("QE::Formats");
   qRegisterMetaType<QE::Separators> ("QE::Separators");
   qRegisterMetaType<QE::Notations> ("QE::Notations");
   qRegisterMetaType<QE::ArrayActions> ("QE::ArrayActions");
   qRegisterMetaType<QE::Radicies> ("QE::Radicies");
   qRegisterMetaType<QE::UpdateOptions> ("QE::UpdateOptions");
   qRegisterMetaType<QE::DisabledRecordPolicy> ("QE::DisabledRecordPolicy");
   qRegisterMetaType<QE::ProgramStartupOptions> ("QE::ProgramStartupOptions");
   qRegisterMetaType<QE::CreationOptions> ("QE::CreationOptions");
   qRegisterMetaType<QE::VideoModes> ("QE::VideoModes");
   qRegisterMetaType<QE::DropOption> ("QE::DropOption");
   qRegisterMetaType<QE::MouseMoveSignals> ("QE::MouseMoveSignals");
   qRegisterMetaType<QE::ImageFormatOptions> ("QE::ImageFormatOptions");
   qRegisterMetaType<QE::RotationOptions> ("QE::RotationOptions");
   qRegisterMetaType<QE::PVLabelMode> ("QE::PVLabelMode");
   qRegisterMetaType<QE::LayoutOptions> ("QE::LayoutOptions");
   qRegisterMetaType<QE::SourceOptions> ("QE::SourceOptions");
   qRegisterMetaType<QE::GridOrders> ("QE::GridOrders");
   return true;
}

static const bool elaborate = registerMetaTypes();

// end
