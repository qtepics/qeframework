# QESubstitutedLabel.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QESubstitutedLabel.png

RESOURCES += \
    widgets/QESubstitutedLabel/QESubstitutedLabel.qrc

HEADERS += \
    widgets/QESubstitutedLabel/QESubstitutedLabel.h \
    widgets/QESubstitutedLabel/QESubstitutedLabelManager.h

SOURCES += \
    widgets/QESubstitutedLabel/QESubstitutedLabel.cpp \
    widgets/QESubstitutedLabel/QESubstitutedLabelManager.cpp

INCLUDEPATH += \
    widgets/QESubstitutedLabel

# end
