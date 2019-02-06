# data.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
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
# Copyright (c) 2017 Australian Synchrotron
#
# Author:
#   Andrew Starritt
# Contact details:
#   andrew.starritt@synchrotron.org.au
#

INCLUDEPATH += \
   data

HEADERS += \
   data/Generic.h \
   data/QCaAlarmInfo.h \
   data/QCaConnectionInfo.h \
   data/QCaDataPoint.h \
   data/QCaDateTime.h \
   data/QCaObject.h \
   data/QCaVariableNamePropertyManager.h \
   data/QEByteArray.h \
   data/QEFloating.h \
   data/QEFloatingArray.h \
   data/QEFloatingFormatting.h \
   data/QEInteger.h \
   data/QEIntegerArray.h \
   data/QEIntegerFormatting.h \
   data/QELocalEnumeration.h \
   data/QEString.h \
   data/QEStringFormatting.h \
   data/qepicspv.h


SOURCES += \
   data/Generic.cpp \
   data/QCaAlarmInfo.cpp \
   data/QCaConnectionInfo.cpp \
   data/QCaDataPoint.cpp \
   data/QCaDateTime.cpp \
   data/QCaObject.cpp \
   data/QCaVariableNamePropertyManager.cpp \
   data/QEByteArray.cpp \
   data/QEFloating.cpp \
   data/QEFloatingArray.cpp \
   data/QEFloatingFormatting.cpp \
   data/QEInteger.cpp \
   data/QEIntegerArray.cpp \
   data/QEIntegerFormatting.cpp \
   data/QELocalEnumeration.cpp \
   data/QEString.cpp \
   data/QEStringFormatting.cpp \
   data/qepicspv.cpp

# end
