# QESpinBox.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QESpinBox.png

RESOURCES += \
    widgets/QESpinBox/QESpinBox.qrc

HEADERS += \
    widgets/QESpinBox/QESpinBox.h \
    widgets/QESpinBox/QESpinBoxManager.h

SOURCES += \
    widgets/QESpinBox/QESpinBox.cpp \
    widgets/QESpinBox/QESpinBoxManager.cpp

INCLUDEPATH += \
    widgets/QESpinBox

# end
