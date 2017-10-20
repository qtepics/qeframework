# QEPvLoadSave.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# The EPICS QT Framework is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The EPICS QT Framework is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
#
# Copyright (c) 2017 Australian Synchrotron
#
# Author:
#   Andrew Starritt
# Contact details:
#   andrew.starritt@synchrotron.org.au
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
