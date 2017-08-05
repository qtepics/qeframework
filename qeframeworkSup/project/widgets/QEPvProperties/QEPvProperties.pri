# QEPvProperties.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#

OTHER_FILES += \
    widgets/QEPvProperties/record_field_list.txt

RESOURCES += \
    widgets/QEPvProperties/QEPvProperties.qrc

HEADERS += \
    widgets/QEPvProperties/QEPvProperties.h \
    widgets/QEPvProperties/QEPvPropertiesUtilities.h

SOURCES += \
    widgets/QEPvProperties/QEPvProperties.cpp \
    widgets/QEPvProperties/QEPvPropertiesUtilities.cpp

INCLUDEPATH += \
    widgets/QEPvProperties

# end
