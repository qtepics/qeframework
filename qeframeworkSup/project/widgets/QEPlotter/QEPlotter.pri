# QEPlotter.pri
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
