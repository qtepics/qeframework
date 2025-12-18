# archive.pri
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

QT += xml network

INCLUDEPATH += $$PWD

HEADERS += $$PWD/QEArchapplInterface.h
SOURCES += $$PWD/QEArchapplInterface.cpp

HEADERS += $$PWD/QEArchiveAccess.h
SOURCES += $$PWD/QEArchiveAccess.cpp

HEADERS += $$PWD/QEArchiveInterface.h
SOURCES += $$PWD/QEArchiveInterface.cpp

HEADERS += $$PWD/QEArchiveInterfaceManager.h
SOURCES += $$PWD/QEArchiveInterfaceManager.cpp

HEADERS += $$PWD/QEArchiveManager.h
SOURCES += $$PWD/QEArchiveManager.cpp

HEADERS += $$PWD/QEArchiveNameSearch.h
SOURCES += $$PWD/QEArchiveNameSearch.cpp

HEADERS += $$PWD/QEArchiveStatus.h
SOURCES += $$PWD/QEArchiveStatus.cpp

HEADERS += $$PWD/QEChannelArchiveInterface.h
SOURCES += $$PWD/QEChannelArchiveInterface.cpp

HEADERS += $$PWD/maiaFault.h
SOURCES += $$PWD/maiaFault.cpp

HEADERS += $$PWD/maiaObject.h
SOURCES += $$PWD/maiaObject.cpp

HEADERS += $$PWD/maiaXmlRpcClient.h
SOURCES += $$PWD/maiaXmlRpcClient.cpp

OTHER_FILES += $$PWD/drag_icon.png

RESOURCES += $$PWD/archive.qrc

# end
