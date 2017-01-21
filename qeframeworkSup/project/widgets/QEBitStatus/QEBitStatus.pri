# QEBitStatus.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEBitStatus/QBitStatus.png \
    widgets/QEBitStatus/QEBitStatus.png

RESOURCES += \
    widgets/QEBitStatus/QEBitStatus.qrc

HEADERS += \
    widgets/QEBitStatus/QBitStatus.h \
    widgets/QEBitStatus/QBitStatusManager.h \
    widgets/QEBitStatus/QEBitStatus.h \
    widgets/QEBitStatus/QEBitStatusManager.h

SOURCES += \
    widgets/QEBitStatus/QBitStatus.cpp \
    widgets/QEBitStatus/QBitStatusManager.cpp \
    widgets/QEBitStatus/QEBitStatus.cpp \
    widgets/QEBitStatus/QEBitStatusManager.cpp

INCLUDEPATH += \
    widgets/QEBitStatus

# end
