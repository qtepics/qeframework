# QELink.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QELink.png

RESOURCES += \
    widgets/QELink/QELink.qrc

HEADERS += \
    widgets/QELink/QELink.h \
    widgets/QELink/QELinkManager.h

SOURCES += \
    widgets/QELink/QELink.cpp \
    widgets/QELink/QELinkManager.cpp

INCLUDEPATH += \
    widgets/QELink

# end
