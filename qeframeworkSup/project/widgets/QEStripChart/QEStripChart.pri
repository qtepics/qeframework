# QEStripChart.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# Copyright (c) 2017-2025 Australian Synchrotron
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

HEADERS += $$PWD/QEStripChart.h
SOURCES += $$PWD/QEStripChart.cpp

HEADERS += $$PWD/QEStripChartAdjustPVDialog.h
SOURCES += $$PWD/QEStripChartAdjustPVDialog.cpp
FORMS   += $$PWD/QEStripChartAdjustPVDialog.ui

HEADERS += $$PWD/QEStripChartContextMenu.h
SOURCES += $$PWD/QEStripChartContextMenu.cpp

HEADERS += $$PWD/QEStripChartDurationDialog.h
SOURCES += $$PWD/QEStripChartDurationDialog.cpp
FORMS   += $$PWD/QEStripChartDurationDialog.ui

HEADERS += $$PWD/QEStripChartItem.h
SOURCES += $$PWD/QEStripChartItem.cpp

HEADERS += $$PWD/QEStripChartNames.h
SOURCES += $$PWD/QEStripChartNames.cpp

HEADERS += $$PWD/QEStripChartRangeDialog.h
SOURCES += $$PWD/QEStripChartRangeDialog.cpp
FORMS   += $$PWD/QEStripChartRangeDialog.ui

HEADERS += $$PWD/QEStripChartState.h
SOURCES += $$PWD/QEStripChartState.cpp

HEADERS += $$PWD/QEStripChartStatistics.h
SOURCES += $$PWD/QEStripChartStatistics.cpp
FORMS   += $$PWD/QEStripChartStatistics.ui

HEADERS += $$PWD/QEStripChartTimeDialog.h
SOURCES += $$PWD/QEStripChartTimeDialog.cpp
FORMS   += $$PWD/QEStripChartTimeDialog.ui

HEADERS += $$PWD/QEStripChartToolBar.h
SOURCES += $$PWD/QEStripChartToolBar.cpp

HEADERS += $$PWD/QEStripChartUtilities.h
SOURCES += $$PWD/QEStripChartUtilities.cpp

OTHER_FILES += $$PWD/archive.png
OTHER_FILES += $$PWD/go_back.png
OTHER_FILES += $$PWD/go_fwd.png
OTHER_FILES += $$PWD/linear_scale.png
OTHER_FILES += $$PWD/log_scale.png
OTHER_FILES += $$PWD/normal_video.png
OTHER_FILES += $$PWD/open_file.png
OTHER_FILES += $$PWD/page_backward.png
OTHER_FILES += $$PWD/page_forward.png
OTHER_FILES += $$PWD/pause.png
OTHER_FILES += $$PWD/play.png
OTHER_FILES += $$PWD/powered_by_asp.png
OTHER_FILES += $$PWD/reverse_video.png
OTHER_FILES += $$PWD/save_file.png
OTHER_FILES += $$PWD/select_date_times.png
OTHER_FILES += $$PWD/select_time.png
OTHER_FILES += $$PWD/write_all.png

RESOURCES   += $$PWD/QEStripChart.qrc

# end
