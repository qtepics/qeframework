# File: qepluginApp/Makefile
# DateTime: Mon Apr 21 18:59:51 2025
# Last checked in by: starritt
#
# This make file is the bridge beween the EPICS build system and the Qt project qmake/make paradigm.
# It makes the PLUGIN library.
#

TOP=..

# Pick up configure/RELEASE defitions.
#
include $(TOP)/configure/CONFIG
#
# Note: we do not include the rule $(TOP)/configure/RULES

# Note: this makefile calls up qmake. Depending on which version of Qt you are using, you may
# have to setup qmake in your development PATH to call up the required version of qmake.
# E.g. on Linux with Qt4, create the following symbolic link:
#
#    /usr/local/bin/qmake -> /usr/bin/qmake-qt4
#

# Create environment variable from Make variable as defined in $(TOP)/configure/RELEASE
# Note use of := which avoids "*** Recursive variable `EPICS_BASE' references itself (eventually)" error
#
export EPICS_BASE := $(EPICS_BASE)
export QE_FRAMEWORK := $(QE_FRAMEWORK)

# Commands
#
ifeq ($(OS),Windows_NT)
    define NOOP
       echo ""
    endef

else
    define NOOP
       :
    endef

endif


# Directories of interest.
#
SOURCE_DIR = project

# Must be consistant with project file
#
TARGET_DIR = $(TOP)/lib/$(EPICS_HOST_ARCH)/designer

# Files
#
MAKEFILE = Makefile.$(EPICS_HOST_ARCH)
PROJECT  = qeplugin.pro

ifeq ($(OS),Windows_NT)
   LIBFILE = QEPlugin.dll
else
   LIBFILE = libQEPlugin.so
endif

# Full target path
#
TARGET=$(TARGET_DIR)/$(LIBFILE)


.PHONY: all install clean uninstall  always 

all: $(TARGET)  

install: $(TARGET)


# The Qt widget loader (as used by qegui, designer, etc.) expect plugin libraries
# to be located in a directory called designer, i.e. like:  ...../designer/libQEPlugin.so
# and QT_PLUGIN_PATH excludes the /designer part of the path.
#
# Note: we always run this step
#
$(TARGET) : $(SOURCE_DIR)/$(MAKEFILE)  always
	@echo "=== Building $(LIBFILE) library"      && \
	cd  $(SOURCE_DIR)                            && \
	$(MAKE) -f $(MAKEFILE) -j 3 -w               && \
	echo "=== $(LIBFILE) library build complete"


$(SOURCE_DIR)/$(MAKEFILE) : $(SOURCE_DIR)/$(PROJECT)
	@echo "=== Running qmake - generating $(MAKEFILE)"   && \
	cd  $(SOURCE_DIR)                            && \
	qmake -o $(MAKEFILE) $(PROJECT) -r


# Do a qt clean, then delete all qmake generated Makefiles.
#
clean:
	cd $(SOURCE_DIR) && $(MAKE) -f $(MAKEFILE) clean || $(NOOP)
	cd $(SOURCE_DIR) && $(RM) $(MAKEFILE) .qmake.stash


uninstall:
	$(RM) $(TARGET)

always:

# end
