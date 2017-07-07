# QEPvLoadSave.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#

OTHER_FILES += \
    widgets/QEPvLoadSave/archive_time.png \
    widgets/QEPvLoadSave/copy_all.png \
    widgets/QEPvLoadSave/copy_subset.png \
    widgets/QEPvLoadSave/read_all.png \
    widgets/QEPvLoadSave/read_subset.png \
    widgets/QEPvLoadSave/write_all.png \
    widgets/QEPvLoadSave/write_subset.png \
    widgets/QEPvLoadSave/ypoc_all.png \
    widgets/QEPvLoadSave/ypoc_subset.png

RESOURCES += \
    widgets/QEPvLoadSave/QEPvLoadSave.qrc

HEADERS += \
    widgets/QEPvLoadSave/QEPvLoadSave.h \
    widgets/QEPvLoadSave/QEPvLoadSaveCommon.h \
    widgets/QEPvLoadSave/QEPvLoadSaveAccessFail.h \
    widgets/QEPvLoadSave/QEPvLoadSaveCompare.h \
    widgets/QEPvLoadSave/QEPvLoadSaveItem.h \
    widgets/QEPvLoadSave/QEPvLoadSaveModel.h \
    widgets/QEPvLoadSave/QEPvLoadSaveUtilities.h \
    widgets/QEPvLoadSave/QEPvLoadSaveGroupNameDialog.h \
    widgets/QEPvLoadSave/QEPvLoadSaveTimeDialog.h \
    widgets/QEPvLoadSave/QEPvLoadSaveValueEditDialog.h


SOURCES += \
    widgets/QEPvLoadSave/QEPvLoadSave.cpp \
    widgets/QEPvLoadSave/QEPvLoadSaveCommon.cpp \
    widgets/QEPvLoadSave/QEPvLoadSaveAccessFail.cpp \
    widgets/QEPvLoadSave/QEPvLoadSaveCompare.cpp \
    widgets/QEPvLoadSave/QEPvLoadSaveItem.cpp \
    widgets/QEPvLoadSave/QEPvLoadSaveModel.cpp \
    widgets/QEPvLoadSave/QEPvLoadSaveUtilities.cpp \
    widgets/QEPvLoadSave/QEPvLoadSaveGroupNameDialog.cpp \
    widgets/QEPvLoadSave/QEPvLoadSaveTimeDialog.cpp \
    widgets/QEPvLoadSave/QEPvLoadSaveValueEditDialog.cpp

INCLUDEPATH += \
    widgets/QEPvLoadSave

FORMS += \
    widgets/QEPvLoadSave/QEPvLoadSaveAccessFail.ui \
    widgets/QEPvLoadSave/QEPvLoadSaveCompare.ui \
    widgets/QEPvLoadSave/QEPvLoadSaveGroupNameDialog.ui \
    widgets/QEPvLoadSave/QEPvLoadSaveTimeDialog.ui \
    widgets/QEPvLoadSave/QEPvLoadSaveValueEditDialog.ui

# end
