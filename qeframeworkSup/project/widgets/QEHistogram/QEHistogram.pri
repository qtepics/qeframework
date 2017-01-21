# QEHistogram.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
OTHER_FILES += \
    widgets/QEHistogram/QEHistogram.png \
    widgets/QEHistogram/QEScalarHistogram.png \
    widgets/QEHistogram/QEWaveformHistogram.png

RESOURCES += \
    widgets/QEHistogram/QEHistogram.qrc

HEADERS += \
    widgets/QEHistogram/QEHistogram.h \
    widgets/QEHistogram/QEHistogramManager.h \
    widgets/QEHistogram/QEScalarHistogram.h \
    widgets/QEHistogram/QEScalarHistogramManager.h \
    widgets/QEHistogram/QEWaveformHistogram.h \
    widgets/QEHistogram/QEWaveformHistogramManager.h

SOURCES += \
    widgets/QEHistogram/QEHistogram.cpp \
    widgets/QEHistogram/QEHistogramManager.cpp \
    widgets/QEHistogram/QEScalarHistogram.cpp \
    widgets/QEHistogram/QEScalarHistogramManager.cpp \
    widgets/QEHistogram/QEWaveformHistogram.cpp \
    widgets/QEHistogram/QEWaveformHistogramManager.cpp

INCLUDEPATH += \
    widgets/QEHistogram

# end
