# QEFormGrid.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# SPDX-FileCopyrightText: 2019-2025 Australian Synchrotron
# SPDX-License-Identifier: LGPL-3.0-only
#
# Author:     Andrew Starritt
# Maintainer: Andrew Starritt
# Contact:    andrews@ansto.gov.au
#

HEADERS += \
    widgets/QEFormGrid/QEDynamicFormGrid.h \
    widgets/QEFormGrid/QEFormGrid.h

SOURCES += \
    widgets/QEFormGrid/QEDynamicFormGrid.cpp \
    widgets/QEFormGrid/QEFormGrid.cpp

INCLUDEPATH += \
    widgets/QEFormGrid

FORMS += \
    widgets/QEFormGrid/QEDynamicFormGrid.ui

# end
