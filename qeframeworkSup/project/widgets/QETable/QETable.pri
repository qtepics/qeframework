# QETable.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QETable/QETable.png

RESOURCES += \
    widgets/QETable/QETable.qrc

HEADERS += \
    widgets/QETable/QETable.h \
    widgets/QETable/QETableManager.h

SOURCES += \
    widgets/QETable/QETable.cpp \
    widgets/QETable/QETableManager.cpp

INCLUDEPATH += \
    widgets/QETable

# end
