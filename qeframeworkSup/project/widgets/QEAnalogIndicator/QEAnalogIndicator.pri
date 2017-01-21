# QEAnalogIndicator.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEAnalogIndicator/QEAnalogIndicator.png

RESOURCES += \
    widgets/QEAnalogIndicator/QEAnalogIndicator.qrc

HEADERS += \
    widgets/QEAnalogIndicator/QEAnalogIndicator.h \
    widgets/QEAnalogIndicator/QEAnalogIndicatorManager.h

SOURCES += \
    widgets/QEAnalogIndicator/QEAnalogIndicator.cpp \
    widgets/QEAnalogIndicator/QEAnalogIndicatorManager.cpp

INCLUDEPATH += \
    widgets/QEAnalogIndicator

# end
