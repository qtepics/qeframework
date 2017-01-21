# QEComboBox.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEComboBox.png

RESOURCES += \
    widgets/QEComboBox/QEComboBox.qrc

HEADERS += \
    widgets/QEComboBox/QEComboBox.h \
    widgets/QEComboBox/QEComboBoxManager.h

SOURCES += \
    widgets/QEComboBox/QEComboBox.cpp \
    widgets/QEComboBox/QEComboBoxManager.cpp

INCLUDEPATH += \
    widgets/QEComboBox

# end
