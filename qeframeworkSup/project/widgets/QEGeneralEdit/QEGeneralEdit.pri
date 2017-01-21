# QEGeneralEdit.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEGeneralEdit/QEGeneralEdit.png

RESOURCES += \
    widgets/QEGeneralEdit/QEGeneralEdit.qrc

HEADERS += \
    widgets/QEGeneralEdit/QEGeneralEdit.h \
    widgets/QEGeneralEdit/QEGeneralEditManager.h

SOURCES += \
    widgets/QEGeneralEdit/QEGeneralEdit.cpp \
    widgets/QEGeneralEdit/QEGeneralEditManager.cpp

INCLUDEPATH += \
    widgets/QEGeneralEdit

# end
