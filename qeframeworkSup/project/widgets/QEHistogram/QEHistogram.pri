# QEHistogram.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#

HEADERS += \
    widgets/QEHistogram/QEHistogram.h \
    widgets/QEHistogram/QEScalarHistogram.h \
    widgets/QEHistogram/QEWaveformHistogram.h

SOURCES += \
    widgets/QEHistogram/QEHistogram.cpp \
    widgets/QEHistogram/QEScalarHistogram.cpp \
    widgets/QEHistogram/QEWaveformHistogram.cpp

INCLUDEPATH += \
    widgets/QEHistogram

# end
