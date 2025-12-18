# QELabel.pri
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
    widgets/QELabel/QELabel.h \
    widgets/QELabel/QEDescriptionLabel.h

SOURCES += \
    widgets/QELabel/QELabel.cpp \
    widgets/QELabel/QEDescriptionLabel.cpp

INCLUDEPATH += \
    widgets/QELabel

# end
