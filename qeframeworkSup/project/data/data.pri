# data.pri
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
