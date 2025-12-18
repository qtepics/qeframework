# QEWidget.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# SPDX-FileCopyrightText: 2017-2025 Australian Synchrotron
# SPDX-License-Identifier: LGPL-3.0-only
#
# Author:     Andrew Starritt
# Maintainer: Andrew Starritt
# Contact:    andrews@ansto.gov.au
#

INCLUDEPATH += $$PWD

HEADERS += $$PWD/ContainerProfile.h
SOURCES += $$PWD/ContainerProfile.cpp

HEADERS += $$PWD/QEDesignerPluginCommon.h

HEADERS += $$PWD/QEDragDrop.h
SOURCES += $$PWD/QEDragDrop.cpp

HEADERS += $$PWD/QEEmitter.h
SOURCES += $$PWD/QEEmitter.cpp

HEADERS += $$PWD/QEFrameworkLibraryGlobal.h

HEADERS += $$PWD/QEGlobalStyle.h
SOURCES += $$PWD/QEGlobalStyle.cpp

HEADERS += $$PWD/QESingleVariableMethods.h
SOURCES += $$PWD/QESingleVariableMethods.cpp

HEADERS += $$PWD/QEStringFormattingMethods.h
SOURCES += $$PWD/QEStringFormattingMethods.cpp

HEADERS += $$PWD/QEToolTip.h
SOURCES += $$PWD/QEToolTip.cpp

HEADERS += $$PWD/QEWidget.h
SOURCES += $$PWD/QEWidget.cpp

HEADERS += $$PWD/UserMessage.h
SOURCES += $$PWD/UserMessage.cpp

HEADERS += $$PWD/VariableManager.h
SOURCES += $$PWD/VariableManager.cpp

HEADERS += $$PWD/VariableNameManager.h
SOURCES += $$PWD/VariableNameManager.cpp

HEADERS += $$PWD/applicationLauncher.h
SOURCES += $$PWD/applicationLauncher.cpp

HEADERS += $$PWD/contextMenu.h
SOURCES += $$PWD/contextMenu.cpp

HEADERS += $$PWD/managePixmaps.h
SOURCES += $$PWD/managePixmaps.cpp

HEADERS += $$PWD/persistanceManager.h
SOURCES += $$PWD/persistanceManager.cpp

HEADERS += $$PWD/standardProperties.h
SOURCES += $$PWD/standardProperties.cpp

HEADERS += $$PWD/styleManager.h
SOURCES += $$PWD/styleManager.cpp

# end
