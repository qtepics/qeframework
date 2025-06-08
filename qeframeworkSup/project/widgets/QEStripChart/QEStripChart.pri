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
#   andrew.starritt@synchrotron.org.au
#

OTHER_FILES += \
    widgets/QEStripChart/archive.png \
    widgets/QEStripChart/go_back.png \
    widgets/QEStripChart/go_fwd.png \
    widgets/QEStripChart/linear_scale.png \
    widgets/QEStripChart/log_scale.png \
    widgets/QEStripChart/normal_video.png \
    widgets/QEStripChart/open_file.png \
    widgets/QEStripChart/page_backward.png \
    widgets/QEStripChart/page_forward.png \
    widgets/QEStripChart/pause.png \
    widgets/QEStripChart/play.png \
    widgets/QEStripChart/powered_by_asp.png \
    widgets/QEStripChart/reverse_video.png  \
    widgets/QEStripChart/save_file.png \
    widgets/QEStripChart/select_date_times.png \
    widgets/QEStripChart/select_time.png \
    widgets/QEStripChart/write_all.png

RESOURCES += \
    widgets/QEStripChart/QEStripChart.qrc

HEADERS += \
    widgets/QEStripChart/QEStripChart.h \
    widgets/QEStripChart/QEStripChartState.h \
    widgets/QEStripChart/QEStripChartAdjustPVDialog.h \
    widgets/QEStripChart/QEStripChartContextMenu.h \
    widgets/QEStripChart/QEStripChartDurationDialog.h \
    widgets/QEStripChart/QEStripChartItem.h \
    widgets/QEStripChart/QEStripChartNames.h \
    widgets/QEStripChart/QEStripChartRangeDialog.h \
    widgets/QEStripChart/QEStripChartTimeDialog.h \
    widgets/QEStripChart/QEStripChartToolBar.h \
    widgets/QEStripChart/QEStripChartUtilities.h \
    widgets/QEStripChart/QEStripChartStatistics.h

SOURCES += \
    widgets/QEStripChart/QEStripChart.cpp \
    widgets/QEStripChart/QEStripChartState.cpp \
    widgets/QEStripChart/QEStripChartAdjustPVDialog.cpp \
    widgets/QEStripChart/QEStripChartContextMenu.cpp \
    widgets/QEStripChart/QEStripChartDurationDialog.cpp \
    widgets/QEStripChart/QEStripChartItem.cpp \
    widgets/QEStripChart/QEStripChartNames.cpp \
    widgets/QEStripChart/QEStripChartRangeDialog.cpp \
    widgets/QEStripChart/QEStripChartTimeDialog.cpp \
    widgets/QEStripChart/QEStripChartToolBar.cpp \
    widgets/QEStripChart/QEStripChartUtilities.cpp \
    widgets/QEStripChart/QEStripChartStatistics.cpp

INCLUDEPATH += \
    widgets/QEStripChart

FORMS += \
    widgets/QEStripChart/QEStripChartAdjustPVDialog.ui \
    widgets/QEStripChart/QEStripChartDurationDialog.ui \
    widgets/QEStripChart/QEStripChartRangeDialog.ui \
    widgets/QEStripChart/QEStripChartTimeDialog.ui \
    widgets/QEStripChart/QEStripChartStatistics.ui

# end
