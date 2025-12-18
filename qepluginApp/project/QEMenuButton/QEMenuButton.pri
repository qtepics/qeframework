# QEMenuButton.pri
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
OTHER_FILES += $$PWD/value_reset.png

RESOURCES   += $$PWD/QEMenuButton.qrc

# end
