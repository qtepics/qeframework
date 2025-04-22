# File: Makefile
# DateTime: Mon Apr 21 18:59:50 2025
# Last checked in by: starritt
#
# Makefile at top of application tree for QE Framework

TOP = .
include $(TOP)/configure/CONFIG

# Directories to build, any order
DIRS += configure
ifeq ($(QE_ARCHAPPL_SUPPORT),YES)
	DIRS += archapplDataSup
endif
DIRS += qeframeworkSup
DIRS += $(wildcard *App)
DIRS += $(wildcard *Top)
DIRS += $(wildcard iocBoot)

# The build order is controlled by these dependency rules:

# All dirs except configure depend on configure
$(foreach dir, $(filter-out configure, $(DIRS)), \
    $(eval $(dir)_DEPEND_DIRS += configure))

# Any *App dirs depend on all *Sup dirs
$(foreach dir, $(filter %App, $(DIRS)), \
    $(eval $(dir)_DEPEND_DIRS += $(filter %Sup, $(DIRS))))

# Any *Top dirs depend on all *Sup and *App dirs
$(foreach dir, $(filter %Top, $(DIRS)), \
    $(eval $(dir)_DEPEND_DIRS += $(filter %Sup %App, $(DIRS))))

# If we're using archiver appliance qeframeworkSup depends on archapplDataSup
#
ifeq ($(QE_ARCHAPPL_SUPPORT),YES)
	qeframeworkSup_DEPEND_DIRS += archapplDataSup
endif

# iocBoot depends on all *App dirs
iocBoot_DEPEND_DIRS += $(filter %App,$(DIRS))

# Add any additional dependency rules here:

include $(TOP)/configure/RULES_TOP

# end
