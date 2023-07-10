# QEMenuButton.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework qeplugins.pro project file.
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
# Copyright (c) 2017-2023 Australian Synchrotron
#
# Author:
#   Andrew Starritt
# Contact details:
#   andrew.starritt@synchrotron.org.au
#

INCLUDEPATH += $$PWD

HEADERS += $$PWD/QEMenuButtonArgumentsDialog.h
SOURCES += $$PWD/QEMenuButtonArgumentsDialog.cpp
FORMS   += $$PWD/QEMenuButtonArgumentsDialog.ui

HEADERS += $$PWD/QEMenuButtonManager.h
SOURCES += $$PWD/QEMenuButtonManager.cpp

HEADERS += $$PWD/QEMenuButtonSetupDialog.h
SOURCES += $$PWD/QEMenuButtonSetupDialog.cpp
FORMS   += $$PWD/QEMenuButtonSetupDialog.ui

OTHER_FILES += $$PWD/menu_button_del_disabled.png
OTHER_FILES += $$PWD/menu_button_del_enabled.png
OTHER_FILES += $$PWD/menu_button_down_disabled.png
OTHER_FILES += $$PWD/menu_button_down_enabled.png
OTHER_FILES += $$PWD/menu_button_new_disabled.png
OTHER_FILES += $$PWD/menu_button_new_enabled.png
OTHER_FILES += $$PWD/menu_button_up_disabled.png
OTHER_FILES += $$PWD/menu_button_up_enabled.png

RESOURCES   += $$PWD/QEMenuButton.qrc

# end
