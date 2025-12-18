# QEPlotter.pri
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

HEADERS += \
    widgets/QEPlotter/QEPlotter.h \
    widgets/QEPlotter/QEPlotterNames.h \
    widgets/QEPlotter/QEPlotterItemDialog.h \
    widgets/QEPlotter/QEPlotterMenu.h \
    widgets/QEPlotter/QEPlotterState.h \
    widgets/QEPlotter/QEPlotterToolBar.h

SOURCES += \
    widgets/QEPlotter/QEPlotter.cpp \
    widgets/QEPlotter/QEPlotterItemDialog.cpp \
    widgets/QEPlotter/QEPlotterMenu.cpp \
    widgets/QEPlotter/QEPlotterState.cpp \
    widgets/QEPlotter/QEPlotterToolBar.cpp

INCLUDEPATH += \
    widgets/QEPlotter

FORMS += \
    widgets/QEPlotter/QEPlotterItemDialog.ui

# end
