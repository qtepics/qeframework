# QEFrame.pri
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

HEADERS += \
    widgets/QEFrame/QEFrame.h \
    widgets/QEFrame/QEPvFrame.h

SOURCES += \
    widgets/QEFrame/QEFrame.cpp \
    widgets/QEFrame/QEPvFrame.cpp

INCLUDEPATH += \
    widgets/QEFrame

# end
