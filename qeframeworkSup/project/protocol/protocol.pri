# protocol.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# Copyright (C) 2018 Australian Synchrotron
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
#   andrew.starritt@synchrotron.org.au
#

PROTOCOL = protocol

INCLUDEPATH += $$PROTOCOL

HEADERS += $$PROTOCOL/QEPvaCheck.h

HEADERS += $$PROTOCOL/QEPvNameUri.h
SOURCES += $$PROTOCOL/QEPvNameUri.cpp

HEADERS += $$PROTOCOL/QEPvaData.h
SOURCES += $$PROTOCOL/QEPvaData.cpp

HEADERS += $$PROTOCOL/QENTNDArrayData.h
SOURCES += $$PROTOCOL/QENTNDArrayData.cpp

HEADERS += $$PROTOCOL/QENTTableData.h
SOURCES += $$PROTOCOL/QENTTableData.cpp

HEADERS += $$PROTOCOL/QEOpaqueData.h
SOURCES += $$PROTOCOL/QEOpaqueData.cpp

HEADERS += $$PROTOCOL/QEVectorVariants.h
SOURCES += $$PROTOCOL/QEVectorVariants.cpp

HEADERS += $$PROTOCOL/QEBaseClient.h
SOURCES += $$PROTOCOL/QEBaseClient.cpp

HEADERS += $$PROTOCOL/QECaClient.h
SOURCES += $$PROTOCOL/QECaClient.cpp

HEADERS += $$PROTOCOL/QEPvaClient.h
SOURCES += $$PROTOCOL/QEPvaClient.cpp

# end
