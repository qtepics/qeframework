# QEScratchPad.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEScratchPad.png

RESOURCES += \
    widgets/QEScratchPad/QEScratchPad.qrc

HEADERS += \
    widgets/QEScratchPad/QEScratchPad.h \
    widgets/QEScratchPad/QEScratchPadMenu.h \
    widgets/QEScratchPad/QEScratchPadManager.h

SOURCES += \
    widgets/QEScratchPad/QEScratchPad.cpp \
    widgets/QEScratchPad/QEScratchPadMenu.cpp \
    widgets/QEScratchPad/QEScratchPadManager.cpp

INCLUDEPATH += \
    widgets/QEScratchPad

#FORMS += \
#   widgets/QEScratchPad/QEScratchPadItemDialog.ui


# end
