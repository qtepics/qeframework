# QEShape.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEShape.png

RESOURCES += \
    widgets/QEShape/QEShape.qrc

HEADERS += \
    widgets/QEShape/QEShape.h \
    widgets/QEShape/QEShapeManager.h

SOURCES += \
    widgets/QEShape/QEShape.cpp \
    widgets/QEShape/QEShapeManager.cpp

INCLUDEPATH += \
    widgets/QEShape

# end
