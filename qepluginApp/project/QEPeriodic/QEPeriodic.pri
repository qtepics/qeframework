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

INCLUDEPATH += QEPeriodic

HEADERS     += QEPeriodic/QEPeriodicManager.h
SOURCES     += QEPeriodic/QEPeriodicManager.cpp

HEADERS     += QEPeriodic/QEPeriodicTaskMenu.h
SOURCES     += QEPeriodic/QEPeriodicTaskMenu.cpp

HEADERS     += QEPeriodic/PeriodicSetupDialog.h
SOURCES     += QEPeriodic/PeriodicSetupDialog.cpp
FORMS       += QEPeriodic/PeriodicSetupDialog.ui

HEADERS     += QEPeriodic/PeriodicElementSetupForm.h
SOURCES     += QEPeriodic/PeriodicElementSetupForm.cpp
FORMS       += QEPeriodic/PeriodicElementSetupForm.ui

# end
