# QEFormGrid.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEFormGrid/QEFormGrid.png

RESOURCES += \
    widgets/QEFormGrid/QEFormGrid.qrc

HEADERS += \
    widgets/QEFormGrid/QEFormGrid.h \
    widgets/QEFormGrid/QEFormGridManager.h

SOURCES += \
    widgets/QEFormGrid/QEFormGrid.cpp \
    widgets/QEFormGrid/QEFormGridManager.cpp

INCLUDEPATH += \
    widgets/QEFormGrid

# end
