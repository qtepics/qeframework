# QEFrame.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEFrame/QEFrame.png \
    widgets/QEFrame/QEPvFrame.png

RESOURCES += \
    widgets/QEFrame/QEFrame.qrc

HEADERS += \
    widgets/QEFrame/QEFrame.h \
    widgets/QEFrame/QEFrameManager.h \
    widgets/QEFrame/QEPvFrame.h \
    widgets/QEFrame/QEPvFrameManager.h

SOURCES += \
    widgets/QEFrame/QEFrame.cpp \
    widgets/QEFrame/QEFrameManager.cpp \
    widgets/QEFrame/QEPvFrame.cpp \
    widgets/QEFrame/QEPvFrameManager.cpp

INCLUDEPATH += \
    widgets/QEFrame

# end
