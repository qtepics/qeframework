# QEPvLoadSave.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# Copyright (c) 2017-2021 Australian Synchrotron
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
# Author:
#   Andrew Starritt
# Contact details:
#   andrew.starritt@synchrotron.org.au
#

QEPVLOADSAVE = widgets/QEPvLoadSave

INCLUDEPATH += $$QEPVLOADSAVE

HEADERS += $$QEPVLOADSAVE/QEPVLoadSaveNameSelectDialog.h
SOURCES += $$QEPVLOADSAVE/QEPVLoadSaveNameSelectDialog.cpp
FORMS   += $$QEPVLOADSAVE/QEPVLoadSaveNameSelectDialog.ui

HEADERS += $$QEPVLOADSAVE/QEPvLoadSave.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSave.cpp

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveAccessFail.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveAccessFail.cpp
FORMS   += $$QEPVLOADSAVE/QEPvLoadSaveAccessFail.ui

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveCommon.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveCommon.cpp

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveCompare.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveCompare.cpp
FORMS   += $$QEPVLOADSAVE/QEPvLoadSaveCompare.ui

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveGroupNameDialog.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveGroupNameDialog.cpp
FORMS   += $$QEPVLOADSAVE/QEPvLoadSaveGroupNameDialog.ui

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveItem.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveItem.cpp

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveModel.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveModel.cpp

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveTimeDialog.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveTimeDialog.cpp
FORMS   += $$QEPVLOADSAVE/QEPvLoadSaveTimeDialog.ui

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveUtilities.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveUtilities.cpp

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveValueEditDialog.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveValueEditDialog.cpp
FORMS   += $$QEPVLOADSAVE/QEPvLoadSaveValueEditDialog.ui

OTHER_FILES += $$QEPVLOADSAVE/archive_time.png
OTHER_FILES += $$QEPVLOADSAVE/copy_all.png
OTHER_FILES += $$QEPVLOADSAVE/copy_subset.png
OTHER_FILES += $$QEPVLOADSAVE/read_all.png
OTHER_FILES += $$QEPVLOADSAVE/read_subset.png
OTHER_FILES += $$QEPVLOADSAVE/write_all.png
OTHER_FILES += $$QEPVLOADSAVE/write_subset.png
OTHER_FILES += $$QEPVLOADSAVE/ypoc_all.png
OTHER_FILES += $$QEPVLOADSAVE/ypoc_subset.png

RESOURCES += $$QEPVLOADSAVE/QEPvLoadSave.qrc

# end
