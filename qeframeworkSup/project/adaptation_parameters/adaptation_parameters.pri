# adaptation_parameters.pri
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

AP=adaptation_parameters

INCLUDEPATH += \
   $$AP

HEADERS += \
   $$AP/QEEnvironmentVariables.h \
   $$AP/QEOptions.h  \
   $$AP/QESettings.h  \
   $$AP/QEAdaptationParameters.h

SOURCES += \
   $$AP/QEEnvironmentVariables.cpp \
   $$AP/QEOptions.cpp  \
   $$AP/QESettings.cpp  \
   $$AP/QEAdaptationParameters.cpp

# end
