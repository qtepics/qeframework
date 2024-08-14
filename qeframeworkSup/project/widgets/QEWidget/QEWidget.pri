# QEWidget.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# Copyright (c) 2017-2024 Australian Synchrotron
#
# The EPICS QT Framework is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The EPICS QT Framework is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
#
# Author:
#   Andrew Starritt
# Contact details:
#   andrews@ansto.gov.au
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
