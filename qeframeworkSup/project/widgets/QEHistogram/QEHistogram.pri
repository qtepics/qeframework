# QEHistogram.pri
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

HEADERS += $$PWD/QEHistogram.h
SOURCES += $$PWD/QEHistogram.cpp

HEADERS += $$PWD/QEScalarHistogram.h
SOURCES += $$PWD/QEScalarHistogram.cpp

HEADERS += $$PWD/QEWaveformHistogram.h
SOURCES += $$PWD/QEWaveformHistogram.cpp

# end
