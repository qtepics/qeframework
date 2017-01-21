# QELogin.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QELogin/QELogin.png

RESOURCES += \
    widgets/QELogin/QELogin.qrc

HEADERS += \
    widgets/QELogin/QELogin.h \
    widgets/QELogin/QELoginManager.h

SOURCES += \
    widgets/QELogin/QELogin.cpp \
    widgets/QELogin/QELoginManager.cpp

INCLUDEPATH += \
    widgets/QELogin

# end
