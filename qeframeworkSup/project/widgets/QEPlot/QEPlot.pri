# QEPlot.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEPlot.png

RESOURCES += \
    widgets/QEPlot/QEPlot.qrc

HEADERS += \
    widgets/QEPlot/QEPlot.h \
    widgets/QEPlot/QEPlotManager.h

SOURCES += \
    widgets/QEPlot/QEPlot.cpp \
    widgets/QEPlot/QEPlotManager.cpp

INCLUDEPATH += \
    widgets/QEPlot

# end
