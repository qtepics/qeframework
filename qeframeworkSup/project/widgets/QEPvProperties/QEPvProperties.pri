# QEPvProperties.pri
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

OTHER_FILES += \
    widgets/QEPvProperties/record_field_list.txt

RESOURCES += \
    widgets/QEPvProperties/QEPvProperties.qrc

HEADERS += \
    widgets/QEPvProperties/QEPvProperties.h \
    widgets/QEPvProperties/QEPvPropertiesUtilities.h

SOURCES += \
    widgets/QEPvProperties/QEPvProperties.cpp \
    widgets/QEPvProperties/QEPvPropertiesUtilities.cpp

INCLUDEPATH += \
    widgets/QEPvProperties

# end
