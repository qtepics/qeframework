# QEForm.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEForm/QEForm.png

RESOURCES += \
    widgets/QEForm/QEForm.qrc

HEADERS += \
    widgets/QEForm/QEForm.h \
    widgets/QEForm/QEFormManager.h

SOURCES += \
    widgets/QEForm/QEForm.cpp \
    widgets/QEForm/QEFormManager.cpp

INCLUDEPATH += \
    widgets/QEForm

# end
