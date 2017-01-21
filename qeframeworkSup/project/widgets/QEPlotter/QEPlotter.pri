# QEPlotter.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEPlotter/QEPlotter.png

RESOURCES += \
    widgets/QEPlotter/QEPlotter.qrc

HEADERS += \
    widgets/QEPlotter/QEPlotter.h \
    widgets/QEPlotter/QEPlotterNames.h \
    widgets/QEPlotter/QEPlotterItemDialog.h \
    widgets/QEPlotter/QEPlotterMenu.h \
    widgets/QEPlotter/QEPlotterState.h \
    widgets/QEPlotter/QEPlotterToolBar.h \
    widgets/QEPlotter/QEPlotterManager.h

SOURCES += \
    widgets/QEPlotter/QEPlotter.cpp \
    widgets/QEPlotter/QEPlotterItemDialog.cpp \
    widgets/QEPlotter/QEPlotterMenu.cpp \
    widgets/QEPlotter/QEPlotterState.cpp \
    widgets/QEPlotter/QEPlotterToolBar.cpp \
    widgets/QEPlotter/QEPlotterManager.cpp

INCLUDEPATH += \
    widgets/QEPlotter

FORMS += \
    widgets/QEPlotter/QEPlotterItemDialog.ui

# end
