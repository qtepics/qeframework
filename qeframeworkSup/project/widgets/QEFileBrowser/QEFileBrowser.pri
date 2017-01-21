# QEFileBrowser
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEFileBrowser/QEFileBrowser.png

RESOURCES += \
    widgets/QEFileBrowser/QEFileBrowser.qrc

HEADERS += \
    widgets/QEFileBrowser/QEFileBrowser.h \
    widgets/QEFileBrowser/QEFileBrowserManager.h

SOURCES += \
    widgets/QEFileBrowser/QEFileBrowser.cpp \
    widgets/QEFileBrowser/QEFileBrowserManager.cpp

INCLUDEPATH += \
    widgets/QEFileBrowser

# end
