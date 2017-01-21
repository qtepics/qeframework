# QEMenuButton.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEMenuButton/QEMenuButton.png


RESOURCES += \
    widgets/QEMenuButton/QEMenuButton.qrc


HEADERS += \
    widgets/QEMenuButton/QEMenuButton.h \
    widgets/QEMenuButton/QEMenuButtonData.h  \
    widgets/QEMenuButton/QEMenuButtonItem.h \
    widgets/QEMenuButton/QEMenuButtonManager.h \
    widgets/QEMenuButton/QEMenuButtonModel.h \
    widgets/QEMenuButton/QEMenuButtonSetupDialog.h


SOURCES += \
    widgets/QEMenuButton/QEMenuButton.cpp \
    widgets/QEMenuButton/QEMenuButtonData.cpp \
    widgets/QEMenuButton/QEMenuButtonItem.cpp \
    widgets/QEMenuButton/QEMenuButtonManager.cpp \
    widgets/QEMenuButton/QEMenuButtonModel.cpp \
    widgets/QEMenuButton/QEMenuButtonSetupDialog.cpp


FORMS += \
    widgets/QEMenuButton/QEMenuButtonSetupDialog.ui


INCLUDEPATH += \
    widgets/QEMenuButton

# end
