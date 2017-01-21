# QESlider.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QESlider.png

RESOURCES += \
    widgets/QESlider/QESlider.qrc

HEADERS += \
    widgets/QESlider/QESlider.h \
    widgets/QESlider/QESliderManager.h

SOURCES += \
    widgets/QESlider/QESlider.cpp \
    widgets/QESlider/QESliderManager.cpp

INCLUDEPATH += \
    widgets/QESlider

# end
