# $File: //ASP/tec/gui/qeframework/trunk/qepluginApp/project/qeplugin.pro $
# $Revision: #6 $
# $DateTime: 2022/03/09 21:09:41 $
# Last checked in by: $Author: starritt $
#
# Copyright (c) 2017-2022 Australian Synchrotron
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
# Author: Andrew Starritt
# Contact details: andrew.starritt@synchrotron.org.au
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
# Depending on compiler the '-W1' may or may not be required. If an error relating to nable-auto-import occurs (missing leading -e) then use -W1 option
#win32:QMAKE_LFLAGS += -enable-auto-import
win32QMAKE_LFLAGS += -Wl,-enable-auto-import

#==========================================================
#Define this if you are using MinGW compiler - needed ?
#DEFINES += _MINGW=TRUE

#===========================================================
# Project configuration

# Points to the target directoy in which lib/EPICS_HOST_ARCH/QEPlugin
# will be created. This follows the regular EPICS Makefile paradigm.
#
TOP=../..

message ("QT_VERSION = "$$QT_MAJOR_VERSION"."$$QT_MINOR_VERSION"."$$QT_PATCH_VERSION )

# How much of this do we need ?
#
# Qt 4 configuration
#
equals( QT_MAJOR_VERSION, 4 ) {
    CONFIG += plugin uitools designer qwt
    QT += core gui xml network
    warning( "**** QT4 is getting old. Active QT4 support will cease June 2021. ****" )
}

# Qt 5 configuration
#
equals( QT_MAJOR_VERSION, 5 ) {
    CONFIG += plugin qwt
    QT += core gui xml network uitools designer
}

# Qt 6 configuration
#
equals( QT_MAJOR_VERSION, 6 ) {
    error( "**** The plugin project file has not been configured for Qt6 (yet) ****" )
}

# Check QE_FRAMEWORK dependancies
#
_QE_FRAMEWORK = $$(QE_FRAMEWORK)
isEmpty( _QE_FRAMEWORK ) {
    error( "QE_FRAMEWORK must be defined. Ensure QE_FRAMEWORK environment variable is defined." )
}

_EPICS_HOST_ARCH = $$(EPICS_HOST_ARCH)
isEmpty( _EPICS_HOST_ARCH ) {
    error( "EPICS_HOST_ARCH must be defined. Ensure EPICS is installed and EPICS_HOST_ARCH environment variable is defined." )
}

TEMPLATE = lib
DEFINES += QEPLUGIN_LIBRARY

# Install the generated plugin library and include files in QE_TARGET_DIR if defined.
_QE_TARGET_DIR = $$(QE_TARGET_DIR)
isEmpty( _QE_TARGET_DIR ) {
    INSTALL_DIR = $$TOP
    message( "QE_TARGET_DIR is not defined. The QE plugin library file will be installed into the <top> directory." )
} else {
    INSTALL_DIR = $$(QE_TARGET_DIR)
    warning( "QE_TARGET_DIR is defined. The QE plugin library file will be installed directly into" $$INSTALL_DIR )
    warning( "Applications may need to load the framework library as a Qt plugin. To ensure this can happen one option is to set up QT_PLUGIN_PATH to include a directory containing a 'designer' directory with a link to the plugin library. QT_PLUGIN_PATH is currently" $(QT_PLUGIN_PATH) )
}

# The plugin ends up here.
#
DESTDIR = $$INSTALL_DIR/lib/$$(EPICS_HOST_ARCH)/designer

TARGET = QEPlugin

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


#===========================================================
# Project files
#
# Widget and other sub projects
# Included .pri (project include) files for each widget
#

# Widgets with designer dialogs
#
include (QEMenuButton/QEMenuButton.pri)
include (QEPeriodic/QEPeriodic.pri)

# Regular widgets
#
include (qeplugin/qeplugin.pri)

INCLUDEPATH += $$(QE_FRAMEWORK)/include
LIBS += -L$$(QE_FRAMEWORK)/lib/$$(EPICS_HOST_ARCH) -lQEFramework
unix: QMAKE_LFLAGS += -Wl,-rpath,$$(QE_FRAMEWORK)/lib/$$(EPICS_HOST_ARCH)

#
# end
