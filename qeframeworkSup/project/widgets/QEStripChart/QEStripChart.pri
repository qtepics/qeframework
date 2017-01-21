# QEStripChart.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#

OTHER_FILES += \
    widgets/QEStripChart/QEStripChart.png \
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
    widgets/QEStripChart/select_time.png

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
    widgets/QEStripChart/QEStripChartManager.h \
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
    widgets/QEStripChart/QEStripChartManager.cpp \
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
