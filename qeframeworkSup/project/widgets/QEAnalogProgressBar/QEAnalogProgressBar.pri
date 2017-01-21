# QEAnalogProgressBar.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEAnalogProgressBar/QEAnalogProgressBar.png

RESOURCES += \
    widgets/QEAnalogProgressBar/QEAnalogProgressBar.qrc

HEADERS += \
    widgets/QEAnalogProgressBar/QEAnalogProgressBar.h \
    widgets/QEAnalogProgressBar/QEAnalogProgressBarManager.h

SOURCES += \
    widgets/QEAnalogProgressBar/QEAnalogProgressBar.cpp \
    widgets/QEAnalogProgressBar/QEAnalogProgressBarManager.cpp

INCLUDEPATH += \
    widgets/QEAnalogProgressBar

# end
