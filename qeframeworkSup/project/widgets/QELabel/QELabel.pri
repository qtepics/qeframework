# QELabel.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QELabel.png

RESOURCES += \
    widgets/QELabel/QELabel.qrc

HEADERS += \
    widgets/QELabel/QELabel.h \
    widgets/QELabel/QELabelManager.h

SOURCES += \
    widgets/QELabel/QELabel.cpp \
    widgets/QELabel/QELabelManager.cpp

INCLUDEPATH += \
    widgets/QELabel

# end
