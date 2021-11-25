# QEArchive.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# Copyright (c) 2017-2021 Australian Synchrotron
#
# The EPICS QT Framework is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The EPICS QT Framework is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
#
# Author:
#   Andrew Starritt
# Contact details:
#   andrews@ansto.gov.au
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
