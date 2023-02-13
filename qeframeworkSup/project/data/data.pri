# data.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# Copyright (c) 2017-2023 Australian Synchrotron
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

INCLUDEPATH += $$PWD

HEADERS += $$PWD/QCaAlarmInfo.h
SOURCES += $$PWD/QCaAlarmInfo.cpp

HEADERS += $$PWD/QCaConnectionInfo.h
SOURCES += $$PWD/QCaConnectionInfo.cpp

HEADERS += $$PWD/QCaDataPoint.h
SOURCES += $$PWD/QCaDataPoint.cpp

HEADERS += $$PWD/QCaDateTime.h
SOURCES += $$PWD/QCaDateTime.cpp

HEADERS += $$PWD/QCaObject.h
SOURCES += $$PWD/QCaObject.cpp

HEADERS += $$PWD/QCaVariableNamePropertyManager.h
SOURCES += $$PWD/QCaVariableNamePropertyManager.cpp

HEADERS += $$PWD/QEByteArray.h
SOURCES += $$PWD/QEByteArray.cpp

HEADERS += $$PWD/QEFloating.h
SOURCES += $$PWD/QEFloating.cpp

HEADERS += $$PWD/QEFloatingArray.h
SOURCES += $$PWD/QEFloatingArray.cpp

HEADERS += $$PWD/QEFloatingFormatting.h
SOURCES += $$PWD/QEFloatingFormatting.cpp

HEADERS += $$PWD/QEInteger.h
SOURCES += $$PWD/QEInteger.cpp

HEADERS += $$PWD/QEIntegerArray.h
SOURCES += $$PWD/QEIntegerArray.cpp

HEADERS += $$PWD/QEIntegerFormatting.h
SOURCES += $$PWD/QEIntegerFormatting.cpp

HEADERS += $$PWD/QELocalEnumeration.h
SOURCES += $$PWD/QELocalEnumeration.cpp

HEADERS += $$PWD/QEString.h
SOURCES += $$PWD/QEString.cpp

HEADERS += $$PWD/QEStringFormatting.h
SOURCES += $$PWD/QEStringFormatting.cpp

HEADERS += $$PWD/qepicspv.h
SOURCES += $$PWD/qepicspv.cpp

# end
