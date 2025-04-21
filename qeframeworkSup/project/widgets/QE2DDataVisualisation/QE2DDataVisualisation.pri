# QE2DDataVisualisation.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# Copyright (c) 2020-2024 Australian Synchrotron
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

HEADERS += $$PWD/QEAbstract2DData.h
SOURCES += $$PWD/QEAbstract2DData.cpp

HEADERS += $$PWD/QESpectrogram.h
SOURCES += $$PWD/QESpectrogram.cpp

HEADERS += $$PWD/QESurface.h
SOURCES += $$PWD/QESurface.cpp

HEADERS += $$PWD/QEWaterfall.h
SOURCES += $$PWD/QEWaterfall.cpp

# end
