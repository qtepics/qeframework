# QEFileImage.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEFileImage.png

RESOURCES += \
    widgets/QEFileImage/QEFileImage.qrc

HEADERS += \
    widgets/QEFileImage/QEFileImage.h \
    widgets/QEFileImage/QEFileImageManager.h

SOURCES += \
    widgets/QEFileImage/QEFileImage.cpp \
    widgets/QEFileImage/QEFileImageManager.cpp

INCLUDEPATH += \
    widgets/QEFileImage

# end
