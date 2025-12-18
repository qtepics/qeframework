# QEImage.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# SPDX-FileCopyrightText: 2017-2025 Australian Synchrotron
# SPDX-License-Identifier: LGPL-3.0-only
#
# Author:     Andrew Starritt
# Maintainer: Andrew Starritt
# Contact:    andrews@ansto.gov.au
#

OTHER_FILES += \
    widgets/QEImage/cameraROI.png \
    widgets/QEImage/cameraROIreset.png \
    widgets/QEImage/flipRotate.png \
    widgets/QEImage/pause.png \
    widgets/QEImage/play.png \
    widgets/QEImage/save.png \
    widgets/QEImage/select.png \
    widgets/QEImage/zoom.png \
    widgets/QEImage/target.png \
    widgets/QEImage/circleCursor.png \
    widgets/QEImage/targetCursor.png \
    widgets/QEImage/vLineCursor.png \
    widgets/QEImage/hLineCursor.png \
    widgets/QEImage/lineCursor.png \
    widgets/QEImage/regionCursor.png \
    widgets/QEImage/markupDisplay.png

RESOURCES += \
    widgets/QEImage/QEImage.qrc

HEADERS += \
    widgets/QEImage/imageContextMenu.h \
    widgets/QEImage/flipRotateMenu.h \
    widgets/QEImage/imageMarkup.h \
    widgets/QEImage/profilePlot.h \
    widgets/QEImage/QEImage.h \
    widgets/QEImage/selectMenu.h \
    widgets/QEImage/videowidget.h \
    widgets/QEImage/zoomMenu.h \
    widgets/QEImage/imageInfo.h \
    widgets/QEImage/markupItem.h \
    widgets/QEImage/markupBeam.h \
    widgets/QEImage/markupHLine.h \
    widgets/QEImage/markupVLine.h \
    widgets/QEImage/markupLine.h \
    widgets/QEImage/markupRegion.h \
    widgets/QEImage/markupTarget.h \
    widgets/QEImage/markupText.h \
    widgets/QEImage/markupEllipse.h \
    widgets/QEImage/brightnessContrast.h \
    widgets/QEImage/QEImageOptionsDialog.h \
    widgets/QEImage/QEImageMarkupThickness.h \
    widgets/QEImage/fullScreenWindow.h \
    widgets/QEImage/imageDataFormats.h \
    widgets/QEImage/markupDisplayMenu.h \
    widgets/QEImage/recording.h \
    widgets/QEImage/screenSelectDialog.h \
    widgets/QEImage/colourConversion.h \
    widgets/QEImage/imageProcessor.h \
    widgets/QEImage/imageProperties.h \
    widgets/QEImage/imageMarkupLegendSetText.h \
    widgets/QEImage/mpeg.h


SOURCES += \
    widgets/QEImage/QEImage.cpp \
    widgets/QEImage/imageContextMenu.cpp \
    widgets/QEImage/imageMarkup.cpp \
    widgets/QEImage/videowidget.cpp \
    widgets/QEImage/flipRotateMenu.cpp \
    widgets/QEImage/profilePlot.cpp \
    widgets/QEImage/selectMenu.cpp \
    widgets/QEImage/zoomMenu.cpp \
    widgets/QEImage/imageInfo.cpp \
    widgets/QEImage/markupItem.cpp \
    widgets/QEImage/markupBeam.cpp \
    widgets/QEImage/markupHLine.cpp \
    widgets/QEImage/markupVLine.cpp \
    widgets/QEImage/markupLine.cpp \
    widgets/QEImage/markupRegion.cpp \
    widgets/QEImage/markupTarget.cpp \
    widgets/QEImage/markupText.cpp \
    widgets/QEImage/markupEllipse.cpp \
    widgets/QEImage/brightnessContrast.cpp \
    widgets/QEImage/QEImageOptionsDialog.cpp \
    widgets/QEImage/QEImageMarkupThickness.cpp \
    widgets/QEImage/fullScreenWindow.cpp \
    widgets/QEImage/imageDataFormats.cpp \
    widgets/QEImage/markupDisplayMenu.cpp \
    widgets/QEImage/recording.cpp \
    widgets/QEImage/screenSelectDialog.cpp \
    widgets/QEImage/imageProcessor.cpp \
    widgets/QEImage/imageProperties.cpp \
    widgets/QEImage/imageMarkupLegendSetText.cpp  \
    widgets/QEImage/mpeg.cpp

INCLUDEPATH += \
    widgets/QEImage

FORMS += \
    widgets/QEImage/QEImageOptionsDialog.ui \
    widgets/QEImage/QEImageMarkupThickness.ui \
    widgets/QEImage/recording.ui \
    widgets/QEImage/screenSelectDialog.ui \
    widgets/QEImage/imageMarkupLegendSetText.ui

# end
