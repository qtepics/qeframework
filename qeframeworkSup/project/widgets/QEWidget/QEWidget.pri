# QEWidgets.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework plugins.pro project file.
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
# Copyright (c) 2017 Australian Synchrotron
#
# Author:
#   Andrew Starritt
# Contact details:
#   andrew.starritt@synchrotron.org.au

INCLUDEPATH += \
    widgets/QEWidget

HEADERS += \
    widgets/QEWidget/VariableManager.h \
    widgets/QEWidget/VariableNameManager.h \
    widgets/QEWidget/UserMessage.h \
    widgets/QEWidget/contextMenu.h \
    widgets/QEWidget/QEWidget.h \
    widgets/QEWidget/QEWidgetProperties.h \
    widgets/QEWidget/managePixmaps.h \
    widgets/QEWidget/QEDragDrop.h \
    widgets/QEWidget/styleManager.h \
    widgets/QEWidget/standardProperties.h \
    widgets/QEWidget/QESingleVariableMethods.h \
    widgets/QEWidget/QEStringFormattingMethods.h \
    widgets/QEWidget/QEToolTip.h \
    widgets/QEWidget/ContainerProfile.h \
    widgets/QEWidget/persistanceManager.h \
    widgets/QEWidget/applicationLauncher.h \
    widgets/QEWidget/QEEmitter.h \
    widgets/QEWidget/QEGlobalStyle.h \
    widgets/QEWidget/QEDesignerPluginCommon.h \
    widgets/QEWidget/QEFrameworkLibraryGlobal.h

SOURCES += \
    widgets/QEWidget/VariableManager.cpp \
    widgets/QEWidget/VariableNameManager.cpp \
    widgets/QEWidget/UserMessage.cpp \
    widgets/QEWidget/contextMenu.cpp \
    widgets/QEWidget/QEWidget.cpp \
    widgets/QEWidget/QEWidgetProperties.cpp \
    widgets/QEWidget/managePixmaps.cpp \
    widgets/QEWidget/QEDragDrop.cpp \
    widgets/QEWidget/styleManager.cpp \
    widgets/QEWidget/standardProperties.cpp \
    widgets/QEWidget/QESingleVariableMethods.cpp \
    widgets/QEWidget/QEStringFormattingMethods.cpp \
    widgets/QEWidget/QEToolTip.cpp \
    widgets/QEWidget/ContainerProfile.cpp \
    widgets/QEWidget/persistanceManager.cpp \
    widgets/QEWidget/applicationLauncher.cpp \
    widgets/QEWidget/QEEmitter.cpp \
    widgets/QEWidget/QEGlobalStyle.cpp

# end
