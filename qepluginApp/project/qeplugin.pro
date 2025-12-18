# qeplugin.pro
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron.
#
# SPDX-FileCopyrightText: 2017-2025 Australian Synchrotron
# SPDX-License-Identifier: LGPL-3.0-only
#
# Author:     Andrew Starritt
# Maintainer: Andrew Starritt
# Contact:    andrews@ansto.gov.au
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
    error( "**** QT4 is old. Active QT4 support has ceased. ****" )
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
    CONFIG += plugin qwt
    QT += core gui xml network uitools designer
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
include (QEComment/QEComment.pri)
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
