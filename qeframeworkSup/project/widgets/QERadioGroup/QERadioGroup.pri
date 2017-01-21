# QERadioGroup.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QERadioGroup/QRadioGroup.png \
    widgets/QERadioGroup/QERadioGroup.png

RESOURCES += \
    widgets/QERadioGroup/QERadioGroup.qrc

HEADERS += \
    widgets/QERadioGroup/QRadioGroup.h \
    widgets/QERadioGroup/QRadioGroupManager.h \
    widgets/QERadioGroup/QERadioGroup.h \
    widgets/QERadioGroup/QERadioGroupManager.h

SOURCES += \
    widgets/QERadioGroup/QRadioGroup.cpp \
    widgets/QERadioGroup/QRadioGroupManager.cpp \
    widgets/QERadioGroup/QERadioGroup.cpp \
    widgets/QERadioGroup/QERadioGroupManager.cpp

INCLUDEPATH += \
    widgets/QERadioGroup

# end
