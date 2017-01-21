# QENumericEdit.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#

INCLUDEPATH += \
    widgets/QENumericEdit

OTHER_FILES += \
    widgets/QENumericEdit/QNumericEdit.png \
    widgets/QENumericEdit/QENumericEdit.png


RESOURCES += \
    widgets/QENumericEdit/QENumericEdit.qrc

HEADERS += \
    widgets/QENumericEdit/QNumericEdit.h \
    widgets/QENumericEdit/QNumericEditManager.h \
    widgets/QENumericEdit/QENumericEdit.h \
    widgets/QENumericEdit/QENumericEditManager.h

SOURCES += \
    widgets/QENumericEdit/QNumericEdit.cpp \
    widgets/QENumericEdit/QNumericEditManager.cpp \
    widgets/QENumericEdit/QENumericEdit.cpp \
    widgets/QENumericEdit/QENumericEditManager.cpp

# end
