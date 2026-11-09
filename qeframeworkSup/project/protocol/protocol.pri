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

HEADERS += $$PWD/QEEnvClient.h
SOURCES += $$PWD/QEEnvClient.cpp

HEADERS += $$PWD/QELitClient.h
SOURCES += $$PWD/QELitClient.cpp

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

HEADERS += $$PWD/QEPvaClient.h

equals(_PVACCESS_SUPPORT, "YES") {
  equals(_PVXS_SUPPORT, "YES") {
    HEADERS += $$PWD/QEPvaClient_PVXS.h
    SOURCES += $$PWD/QEPvaClient_PVXS.cpp
  } else {
    HEADERS += $$PWD/QEPvaClient_PVA.h
    SOURCES += $$PWD/QEPvaClient_PVA.cpp
  }
} else {
  HEADERS += $$PWD/QEPvaClient_STUB.h
  SOURCES += $$PWD/QEPvaClient_STUB.cpp
}

# do similar for QEPvaData.h
#
HEADERS += $$PWD/QEPvaData.h

equals(_PVACCESS_SUPPORT, "YES") {
  equals(_PVXS_SUPPORT, "YES") {
    HEADERS += $$PWD/QEPvaData_PVXS.h
    SOURCES += $$PWD/QEPvaData_PVXS.cpp
  } else {
    # rename
    HEADERS += $$PWD/QEPvaData_PVA.h
    SOURCES += $$PWD/QEPvaData_PVA.cpp
  }
} else {
  # Do we need this ??
  HEADERS += $$PWD/QEPvaData_STUB.h
  SOURCES += $$PWD/QEPvaData_STUB.cpp
}

# end
