# QE2DDataVisualisation.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# SPDX-FileCopyrightText: 2020-2025 Australian Synchrotron
# SPDX-License-Identifier: LGPL-3.0-only
#
# Author:     Andrew Starritt
# Maintainer: Andrew Starritt
# Contact:    andrews@ansto.gov.au
#

INCLUDEPATH += $$PWD

HEADERS += $$PWD/QEAbstract2DData.h
SOURCES += $$PWD/QEAbstract2DData.cpp

HEADERS += $$PWD/QESpectrogram.h
SOURCES += $$PWD/QESpectrogram.cpp

HEADERS += $$PWD/QESurface.h
SOURCES += $$PWD/QESurface.cpp

HEADERS += $$PWD/QEWaterfall.h
SOURCES += $$PWD/QEWaterfall.cpp

# end
