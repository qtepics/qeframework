# common.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#

HEADERS += \
   common/QEActionRequests.h \
   common/QEAxisIterator.h \
   common/QEAxisPainter.h \
   common/QEColourBandList.h \
   common/QECommon.h \
   common/QEDialog.h \
   common/QEDelayedText.h \
   common/QEDisplayRanges.h \
   common/QEExpressionEvaluation.h  \
   common/QEFileMonitor.h  \
   common/QEFixedPointRadix.h \
   common/QEFormMapper.h \
   common/QEFrameworkVersion.h \
   common/QEPvNameSearch.h \
   common/QEPVNameSelectDialog.h \
   common/QEGraphic.h \
   common/QEGraphicMarkup.h \
   common/QEGraphicNames.h \
   common/QEOneToOne.h \
   common/QEPlatform.h \
   common/QEQuickSort.h \
   common/QERecordFieldName.h \
   common/QEScaling.h \
   common/QEScanTimers.h \
   common/PasswordDialog.h \
   common/windowCustomisation.h \
   common/macroSubstitution.h

SOURCES += \
   common/QEActionRequests.cpp \
   common/QEAxisIterator.cpp \
   common/QEAxisPainter.cpp \
   common/QEColourBandList.cpp \
   common/QECommon.cpp \
   common/QEDialog.cpp \
   common/QEDelayedText.cpp \
   common/QEDisplayRanges.cpp \
   common/QEExpressionEvaluation.cpp  \
   common/QEFileMonitor.cpp  \
   common/QEFixedPointRadix.cpp \
   common/QEFormMapper.cpp \
   common/QEFrameworkVersion.cpp \
   common/QEPvNameSearch.cpp \
   common/QEPVNameSelectDialog.cpp \
   common/QEGraphic.cpp \
   common/QEGraphicMarkup.cpp \
   common/QEPlatform.cpp \
   common/QEQuickSort.cpp \
   common/QERecordFieldName.cpp \
   common/QEScaling.cpp \
   common/QEScanTimers.cpp \
   common/PasswordDialog.cpp \
   common/windowCustomisation.cpp \
   common/macroSubstitution.cpp

INCLUDEPATH += \
   common

# Explicity add common, and hence QEFrameworkVersion.h, to the dependacy path
# So that changes to the version/release numbers force relevent recompilations.
#
DEPENDPATH += common

FORMS += \
   common/QEPVNameSelectDialog.ui \
   common/PasswordDialog.ui

#RESOURCES += \
#   common/common.qrc

# end
