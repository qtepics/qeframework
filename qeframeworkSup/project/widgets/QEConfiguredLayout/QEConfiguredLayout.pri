# QEConfiguredLayout.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEConfiguredLayout/QEConfiguredLayout.png

RESOURCES += \
    widgets/QEConfiguredLayout/QEConfiguredLayout.qrc

HEADERS += \
    widgets/QEConfiguredLayout/QEConfiguredLayout.h \
    widgets/QEConfiguredLayout/QEConfiguredLayoutManager.h

SOURCES += \
    widgets/QEConfiguredLayout/QEConfiguredLayout.cpp \
    widgets/QEConfiguredLayout/QEConfiguredLayoutManager.cpp

INCLUDEPATH += \
    widgets/QEConfiguredLayout

# end
