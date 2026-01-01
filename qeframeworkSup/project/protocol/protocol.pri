# protocol.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# SPDX-FileCopyrightText: 2017-2026 Australian Synchrotron
# SPDX-License-Identifier: LGPL-3.0-only
#
# Author:     Andrew Starritt
# Maintainer: Andrew Starritt
# Contact:    andrews@ansto.gov.au
#

INCLUDEPATH += $$PWD

HEADERS += $$PWD/QEBaseClient.h
SOURCES += $$PWD/QEBaseClient.cpp

HEADERS += $$PWD/QECaClient.h
SOURCES += $$PWD/QECaClient.cpp

HEADERS += $$PWD/QENTNDArrayConverter.h
SOURCES += $$PWD/QENTNDArrayConverter.cpp

HEADERS += $$PWD/QENTNDArrayData.h
SOURCES += $$PWD/QENTNDArrayData.cpp

HEADERS += $$PWD/QENTTableData.h
SOURCES += $$PWD/QENTTableData.cpp

HEADERS += $$PWD/QENullClient.h
SOURCES += $$PWD/QENullClient.cpp

HEADERS += $$PWD/QEOpaqueData.h
SOURCES += $$PWD/QEOpaqueData.cpp

HEADERS += $$PWD/QEPvNameUri.h
SOURCES += $$PWD/QEPvNameUri.cpp

HEADERS += $$PWD/QEPvaCheck.h

HEADERS += $$PWD/QEPvaClient.h
SOURCES += $$PWD/QEPvaClient.cpp

HEADERS += $$PWD/QEPvaData.h
SOURCES += $$PWD/QEPvaData.cpp

HEADERS += $$PWD/QEVectorVariants.h
SOURCES += $$PWD/QEVectorVariants.cpp

# end
