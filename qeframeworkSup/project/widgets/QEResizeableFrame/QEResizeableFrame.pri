# QEResizeableFrame.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEResizeableFrame/QEResizeableFrame.png \

RESOURCES += \
    widgets/QEResizeableFrame/QEResizeableFrame.qrc

HEADERS += \
    widgets/QEResizeableFrame/QEResizeableFrame.h \
    widgets/QEResizeableFrame/QEResizeableFrameManager.h

SOURCES += \
    widgets/QEResizeableFrame/QEResizeableFrame.cpp \
    widgets/QEResizeableFrame/QEResizeableFrameManager.cpp

INCLUDEPATH += \
    widgets/QEResizeableFrame

# end
