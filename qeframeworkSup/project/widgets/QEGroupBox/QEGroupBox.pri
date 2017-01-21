# QEGroupBox.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEGroupBox/QEGroupBox.png

RESOURCES += \
    widgets/QEGroupBox/QEGroupBox.qrc

HEADERS += \
    widgets/QEGroupBox/QEGroupBox.h \
    widgets/QEGroupBox/QEGroupBoxManager.h

SOURCES += \
    widgets/QEGroupBox/QEGroupBox.cpp \
    widgets/QEGroupBox/QEGroupBoxManager.cpp

INCLUDEPATH += \
    widgets/QEGroupBox

# end
