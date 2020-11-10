# QEAbstract2DData.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# Copyright (c) 2020 Australian Synchrotron
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

HEADERS += widgets/QE2DDataVisualisation/QEAbstract2DData.h
SOURCES += widgets/QE2DDataVisualisation/QEAbstract2DData.cpp

HEADERS += widgets/QE2DDataVisualisation/QESpectrogram.h
SOURCES += widgets/QE2DDataVisualisation/QESpectrogram.cpp

HEADERS += widgets/QE2DDataVisualisation/QESurface.h
SOURCES += widgets/QE2DDataVisualisation/QESurface.cpp

HEADERS += widgets/QE2DDataVisualisation/QEWaterfall.h
SOURCES += widgets/QE2DDataVisualisation/QEWaterfall.cpp

INCLUDEPATH += widgets/QE2DDataVisualisation

# end
