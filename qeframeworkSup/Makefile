# $File: //ASP/tec/gui/qeframework/trunk/qeframeworkSup/Makefile $
# $Revision: #13 $
# $DateTime: 2024/02/09 15:18:38 $
# Last checked in by: $Author: starritt $
#
# This make file is the bridge beween the EPICS build system and the Qt project qmake/make paradigm.
# It makes the FRAMEWORK library.
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
export ACAI := $(ACAI)

# Are we running EPICS 7.0 or later. If yes - then define support required environment variable.
# This needs to be defined for qmake make phase. It is also checked during the make phase.
#
ifdef BASE_7_0
export QE_PVACCESS_SUPPORT := YES
else
export QE_PVACCESS_SUPPORT := NO
endif

# If ADSUPPORT is defined, then re-export for qmake processing of project file.
#
ifdef ADSUPPORT
export ADSUPPORT := $(ADSUPPORT)
endif

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
TARGET_DIR = $(TOP)/lib/$(EPICS_HOST_ARCH)

# Files
#
MAKEFILE = Makefile.$(EPICS_HOST_ARCH)
PROJECT  = framework.pro

ifeq ($(OS),Windows_NT)
   LIBFILE = QEFramework.dll
else
   LIBFILE = libQEFramework.so
endif

# Full target path
#
TARGET=$(TARGET_DIR)/$(LIBFILE)


.PHONY: all install clean uninstall  always  headers 

all: $(TARGET)  headers

install: $(TARGET)  headers


# Note: we always run this step
#
$(TARGET) : $(SOURCE_DIR)/$(MAKEFILE)  always
	@echo "=== Building $(LIBFILE) library"      && \
	cd  $(SOURCE_DIR)                            && \
	$(MAKE) -f $(MAKEFILE) -j 3 -w               && \
	echo "=== $(LIBFILE) library build complete"


headers : $(TARGET) 
	@echo "=== Installing library headers"       && \
	cd  $(SOURCE_DIR)                            && \
	$(MAKE) -f $(MAKEFILE) install               && \
	echo "=== Header installation complete"


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
