# $File: //ASP/tec/gui/qeframework/trunk/qeframeworkSup/project/framework.pro $
# $Revision: #7 $
# $DateTime: 2017/07/08 14:39:51 $
# Last checked in by: $Author: starritt $
#
# Copyright (c) 2009,2010,2016 Australian Synchrotron
#
# This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
# The EPICS QT Framework is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# The EPICS QT Framework is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# You should have received a copy of the GNU Lesser General Public License
# along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
#
# Author: Anthony Owen
# Contact details: anthony.owen@gmail.com
#

#===========================================================
# To alanyse code performance using the GNU gprof profiling tool:
# - Include the following two lines
# - Clean all
# - Run qmake
# - Rebuild all
# - Run the program
# - analyse the results with the command: gprof <your-program-name>
#QMAKE_CXXFLAGS_DEBUG += -pg
#QMAKE_LFLAGS_DEBUG += -pg

# To look at optimised performance (with or without the profileing above, include the following:
#QMAKE_CXXFLAGS_DEBUG += -O2

#===========================================================
# Include the following gdbmacros line for debugging only
#SOURCES += <YOUR-QTSDK-DIRECTORY>/share/qtcreator/gdbmacros/gdbmacros.cpp

#===========================================================
# Since Qt 4.7.4, enable-auto-import is required to avoid a crash on windows when the qwt dll is loaded
# Depending on compiler the '-W1' may or may not be required.
# If an error relating to nable-auto-import occurs (missing leading -e) then use -W1 option
#win32:QMAKE_LFLAGS += -enable-auto-import
win32QMAKE_LFLAGS += -Wl,-enable-auto-import

#==========================================================
# _MINGW=TRUE is now automatically defined when needed
# See line 86 (approx).

#===========================================================
# Project configuration
# Points to the target directoy in which lib/EPICS_HOST_ARCH/QEFramework
# will be created. This follows the regular EPICS Makefile paradigm.
#
TOP=../..

message ("QT_MAJOR_VERSION = " $$QT_MAJOR_VERSION )

# Qt 4 configuration
equals( QT_MAJOR_VERSION, 4 ) {
    CONFIG += plugin uitools designer qwt
    QT += core gui xml network
}

# Qt 5 configuration
equals( QT_MAJOR_VERSION, 5 ) {
    CONFIG += plugin qwt
    QT += core gui xml network uitools designer
}

# Check EPICS dependancies
#
_EPICS_BASE = $$(EPICS_BASE)
isEmpty( _EPICS_BASE ) {
    error( "EPICS_BASE must be defined. Ensure EPICS is installed and EPICS_BASE environment variable is defined, typically in your configure/RELEASE file." )
}

_EPICS_HOST_ARCH = $$(EPICS_HOST_ARCH)
isEmpty( _EPICS_HOST_ARCH ) {
    error( "EPICS_HOST_ARCH must be defined. Ensure EPICS is installed and EPICS_HOST_ARCH environment variable is defined." )
}

# Define _MINGW if using a MinGW compiler
#
equals( _EPICS_HOST_ARCH, "win32-x86-mingw" ) {
    message("MINGW compiler in use, defining _MINGW=TRUE")
    DEFINES += _MINGW=TRUE
}
equals( _EPICS_HOST_ARCH, "windows-x64-mingw" ) {
    message("MINGW compiler in use, defining _MINGW=TRUE")
    DEFINES += _MINGW=TRUE
}

TEMPLATE = lib
DEFINES += QE_FRAMEWORK_LIBRARY

# Install the generated plugin library and include files in QE_TARGET_DIR if defined.
_QE_TARGET_DIR = $$(QE_TARGET_DIR)
isEmpty( _QE_TARGET_DIR ) {
    INSTALL_DIR = $$TOP
    message( "QE_TARGET_DIR is not defined. The QE framework library/include files will be installed into the <top> directory." )
} else {
    INSTALL_DIR = $$(QE_TARGET_DIR)
    warning( "QE_TARGET_DIR is defined. The QE plugin library/include files will be installed directly into" $$INSTALL_DIR )
    unix:warning( "Applications may need to load the framework library directly. To ensure this can happen one option is to set up LD_LIBRARY_PATH")
    unix:warning( " ... to include the directory" $$INSTALL_DIR/lib/$$(EPICS_HOST_ARCH) ". LD_LIBRARY_PATH is currently" $(LD_LIBRARY_PATH) )
}

# The library ends up here.
#
DESTDIR = $$INSTALL_DIR/lib/$$(EPICS_HOST_ARCH)

TARGET = QEFramework

# Place all intermediate generated files in architecture specific locations
#
MOC_DIR        = O.$$(EPICS_HOST_ARCH)/moc
OBJECTS_DIR    = O.$$(EPICS_HOST_ARCH)/obj
UI_DIR         = O.$$(EPICS_HOST_ARCH)/ui
RCC_DIR        = O.$$(EPICS_HOST_ARCH)/rcc
MAKEFILE       = Makefile.$$(EPICS_HOST_ARCH)

# We don't get this include path for free - need to be explicit.
#
INCLUDEPATH += O.$$(EPICS_HOST_ARCH)/ui


DEFINES += QWT_DLL=TRUE
win32:DEFINES += EPICS_CALL_DLL

#===========================================================
# Include MPEG streaming into QEImage widget
# If mpeg streaming is required, define environment variable QE_FFMPEG (on windows, this must point to the FFMPEG directory)

_QE_FFMPEG = $$(QE_FFMPEG)
isEmpty( _QE_FFMPEG ) {
    message( "MPEG viewer will NOT be included in QEImage. If you want MPEG support install FFmpeg and define environment variable" )
    message( ".... QE_FFMPEG. This can be defined as anything on linux, but must point to the FFmpeg directory on windows." )
} else {
    message( "MPEG viewer will be included in QEImage. FFmpeg libraries will be expected. On windows the environment variable" )
    message( ".... QE_FFMPEG must point to the FFmpeg directory. Remove environment variable QE_FFMPEG if you don't want this." )
    DEFINES += QE_USE_MPEG
}

#===========================================================
# Project files
#
# Widget and other sub projects
# Included .pri (project include) files for each widget
#
include (adaptation_parameters/adaptation_parameters.pri)
include (common/common.pri)
include (api/api.pri)
include (data/data.pri)
include (archive/archive.pri)
include (widgets/QEWidget/QEWidget.pri)
include (widgets/QEAbstractWidget/QEAbstractWidget.pri)
include (widgets/QEAnalogIndicator/QEAnalogIndicator.pri)
include (widgets/QEAnalogProgressBar/QEAnalogProgressBar.pri)
include (widgets/QEAnalogSlider/QEAnalogSlider.pri)
include (widgets/QEBitStatus/QEBitStatus.pri)
include (widgets/QEButton/QEButton.pri)
include (widgets/QEComboBox/QEComboBox.pri)
include (widgets/QEConfiguredLayout/QEConfiguredLayout.pri)
include (widgets/QEFileBrowser/QEFileBrowser.pri)
include (widgets/QEFileImage/QEFileImage.pri)
include (widgets/QEForm/QEForm.pri)
include (widgets/QEFormGrid/QEFormGrid.pri)
include (widgets/QEFrame/QEFrame.pri)
include (widgets/QEGeneralEdit/QEGeneralEdit.pri)
include (widgets/QEGroupBox/QEGroupBox.pri)
include (widgets/QEHistogram/QEHistogram.pri)
include (widgets/QEImage/QEImage.pri)
include (widgets/QELabel/QELabel.pri)
include (widgets/QELineEdit/QELineEdit.pri)
include (widgets/QELink/QELink.pri)
include (widgets/QELog/QELog.pri)
include (widgets/QELogin/QELogin.pri)
include (widgets/QEMenuButton/QEMenuButton.pri)
include (widgets/QENumericEdit/QENumericEdit.pri)
include (widgets/QEPeriodic/QEPeriodic.pri)
include (widgets/QEPlot/QEPlot.pri)
include (widgets/QEPlotter/QEPlotter.pri)
include (widgets/QEPvLoadSave/QEPvLoadSave.pri)
include (widgets/QEPvProperties/QEPvProperties.pri)
include (widgets/QERadioGroup/QERadioGroup.pri)
include (widgets/QERecipe/QERecipe.pri)
include (widgets/QEResizeableFrame/QEResizeableFrame.pri)
include (widgets/QEScratchPad/QEScratchPad.pri)
include (widgets/QEScript/QEScript.pri)
include (widgets/QEShape/QEShape.pri)
include (widgets/QESimpleShape/QESimpleShape.pri)
include (widgets/QESlider/QESlider.pri)
include (widgets/QESpinBox/QESpinBox.pri)
include (widgets/QEStripChart/QEStripChart.pri)
include (widgets/QESubstitutedLabel/QESubstitutedLabel.pri)
include (widgets/QETable/QETable.pri)

#===========================================================
# Install include files
# THIS MUST BE AFTER SUB-PROJECT INCLUDES
#
QEINCLUDEPATH = $$INSTALL_DIR/include
QEINCLUDE.files = $$HEADERS
QEINCLUDE.path = $$QEINCLUDEPATH
INSTALLS += QEINCLUDE

message( "Note: By default qtcreator does not have a 'make install' build step. When using qtcreator, modify project " )
message( "....: to add an install build step which is required to install header files to $$INSTALL_DIR/include" )

#===========================================================
# Set up EPICS

INCLUDEPATH += $$(EPICS_BASE)/include

unix:INCLUDEPATH += $$(EPICS_BASE)/include/os/Linux
unix:INCLUDEPATH += $$(EPICS_BASE)/include/compiler/gcc

win32:INCLUDEPATH += $$(EPICS_BASE)/include/os/WIN32
win32:INCLUDEPATH += $$(EPICS_BASE)/include/compiler/msvc

LIBS += -L$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH) \
    -lca \
    -lCom

# Set runtime path for shared libraries
#
unix: QMAKE_LFLAGS += -Wl,-rpath,$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)

#===========================================================
# QWT
#
# Note, the following checks for QWT are repeated in QEGuiApp.pro - keep in sync
#
# Check QWT is accessable. Check there is a chance QMAKEFEATURES includes a path to
# the qwt features directory, or that QWT_INCLUDE_PATH is defined.
# Note, qwt install may set up QMAKEFEATURES to point to the product features file, rather than
# the directory. Not sure if this is wrong, but changing it to the directory works (C:\Qwt-6.0.1\features\qwt.prf  to  C:\Qwt-6.0.1\features)
#
_QWT_INCLUDE_PATH = $$(QWT_INCLUDE_PATH)
isEmpty( _QWT_INCLUDE_PATH ) {
    _QMAKEFEATURES = $$(QMAKEFEATURES)
    _QWT_FEATURE = $$find( _QMAKEFEATURES, [Q|q][W|w][T|t] )
    isEmpty( _QWT_FEATURE ) {
        error( "Qwt does not appear to be available. It is required when building the QE framework. I've checked if 'qwt' is in QMAKEFEATURES or if QWT_INCLUDE_PATH is defined" )
    }
}

# The following QWT include path and library path are only required if
# qwt was not installed fully, with qwt available as a Qt 'feature'.
# When installed as a Qt 'feature' all that is needed is CONFIG += qwt (above)
INCLUDEPATH += $$(QWT_INCLUDE_PATH)
#win32:LIBS += -LC:/qwt-6.0.1/lib
win32:LIBS += -LC:/qwt-6.1.3/lib
#win32:LIBS += -LC:/qwt-6.1.1/lib

# Depending on build, the qwt library below may need to be -lqwt or -lqwt6
# The 'scope' labels Debug and Release need to have first letter capitalised for it to work in win32.
#
win32 {
    Debug {
        message( "Using qwtd (not qwt) for this debug build" )
        LIBS += -lqwtd
    }
    Release {
        message( "Using qwt (not qwtd) for this release build" )
        LIBS += -lqwt
    }
}

unix {
    _QWT_ROOT = $$(QWT_ROOT)
    isEmpty( _QWT_ROOT ) {
        message( "QWT_ROOT is not defined, so using default location of QWT library" )
        LIBS += -lqwt
    } else {
        message( "Using QWT_ROOT environment variable to locate QWT library" )
        LIBS += -L$$(QWT_ROOT)/lib -lqwt
    }
}

# ffmpeg stuff
#
isEmpty( _QE_FFMPEG ) {
} else {
    unix:INCLUDEPATH += /usr/include/ffmpeg
    unix:LIBS += -L/usr/local/lib/

    win32:INCLUDEPATH += $$_QE_FFMPEG/include
    win32:LIBS += -L$$_QE_FFMPEG/lib

    LIBS += -lavutil -lavcodec -lavformat
    DEFINES += __STDC_CONSTANT_MACROS
}

#
# end
