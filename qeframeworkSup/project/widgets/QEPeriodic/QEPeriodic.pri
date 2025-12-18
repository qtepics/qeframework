# QEPeriodic.pri
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
    widgets/QEPeriodic/QEPeriodic.h \
    widgets/QEPeriodic/PeriodicDialog.h

SOURCES += \
    widgets/QEPeriodic/QEPeriodic.cpp \
    widgets/QEPeriodic/PeriodicDialog.cpp

INCLUDEPATH += \
    widgets/QEPeriodic

FORMS += \
    widgets/QEPeriodic/PeriodicDialog.ui

# end
