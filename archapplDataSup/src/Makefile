# $File: //ASP/tec/gui/qeframework/trunk/archapplDataSup/src/Makefile $
# $Revision: #4 $
# $DateTime: 2020/02/05 18:13:24 $
# Last checked in by: $Author: starritt $
#
# Makefile to build a library that parses Google Protocol Buffers data recieved from Archiver Appliance
#

TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#========================================

#========================================
# build a support library.

LIBRARY_IOC += archapplData

# This control variable can be modified in $(TOP)/configuration/CONFIG_SITE
# If set to YES, we're building with C++ 11 support
#
ifeq ($(ARCHAPPL_USE_CPP11), YES)
   USR_CPPFLAGS += -std=c++11
endif

# This control variable can be modified in $(TOP)/configuration/CONFIG_SITE
# or defined as an environment variable.
# If the protocol buf header files are not in the standard system
# location, then this can be specified using PROTOBUF_INCLUDE_PATH
#
ifdef PROTOBUF_INCLUDE_PATH
   USR_CPPFLAGS += -I$(PROTOBUF_INCLUDE_PATH)
else
   $(warning PROTOBUF_INCLUDE_PATH is undefined which could result in protobuf headers not being found)
endif

# This control variable can be modified in $(TOP)/configuration/CONFIG_SITE
# or defined as an environment variable.
# Required libraries location
#
ifdef PROTOBUF_LIB_DIR
   SHRLIB_SEARCH_DIRS += $(PROTOBUF_LIB_DIR)
else
   $(warning PROTOBUF_LIB_DIR is undefined which could result in protobuf library not being found)
endif

# specify header files to be visible to library users.
#
INC += archapplData.h

# specify all source files to be compiled and added to the library.
#
archapplData_SRCS += archapplProto.pb.cc
archapplData_SRCS += archapplData.cpp

archapplData_LIBS += protobuf

#========================================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

# Set dependency. Part of generating archapplProto.pb.cc is generating
# archapplProto.pb.h which is used by archapplData.cpp
#
archapplData.cpp : archapplProto.pb.cc

# Recall this runs in the O.<epics host arch> sub-directory/
#
archapplProto.pb.cc : ../archapplProto.proto
	protoc --proto_path=.. --cpp_out=.  ../archapplProto.proto && \
	$(CP) archapplProto.pb.h ../

# end