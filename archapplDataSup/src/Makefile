# $File: //ASP/tec/gui/qeframework/trunk/archapplDataSup/src/Makefile $
# $Revision: #2 $
# $DateTime: 2018/07/24 10:38:38 $
# Last checked in by: $Author: pozara $
#
# Makefile to build a library that parses Google Protocol Buffers data recieved from Archiver Appliance
#

TOP=../..

include $(TOP)/configure/CONFIG

CXX = g++
CXXFLAGS = -Wall -g -fPIC -I$(EPICS_BASE)/include

LD = g++
LDFLAGS = -shared

OBJS = archapplProto.pb.o archapplData.o
STATIC_LIB = libarchapplData.a

ifeq ($(OS),Windows_NT)
   SHARED_LIB = archapplData.dll
   CXXFLAGS += -DEPICS_BUILD_DLL
   LINKER_OPTIONS = -Wl,--out-implib,$(STATIC_LIB)
else
   SHARED_LIB = libarchapplData.so
   LINKER_OPTIONS =
endif

LIBS = $(SHARED_LIB) $(STATIC_LIB)


install: check-env $(LIBS) archapplData.h
	$(INSTALL_LIBRARY) -d -m 555 $(SHARED_LIB) $(INSTALL_HOST_LIB)
	$(INSTALL_LIBRARY) -d -m 444 $(STATIC_LIB) $(INSTALL_HOST_LIB)
	$(INSTALL) -d -m 444 archapplData.h $(INSTALL_INCLUDE)

check-env:
ifdef PROTOBUF_LIB_DIR
   LDFLAGS += -L$(PROTOBUF_LIB_DIR)
else
   $(warning PROTOBUF_LIB_DIR is undefined which could result in protobuf library not being found)
endif

# Link protobuf library and object files into libraries
#
$(SHARED_LIB): $(OBJS)
	$(LD) $(LDFLAGS) -o $(SHARED_LIB) $(OBJS) -lprotobuf $(LINKER_OPTIONS)

# Archive protobuf library and object files to static library
#
# NOTE: If we're building this on Windows this doesn't get executed as
#       the library has already been created in the previous target
#
$(STATIC_LIB): $(OBJS)
	ar rv $(STATIC_LIB) $(OBJS)

	
# Compile protobuf object file
#
archapplProto.pb.o: archapplProto.pb.h
	$(CXX) $(CXXFLAGS) -c archapplProto.pb.cc

# Compile archiver data object file
#
archapplData.o: archapplData.h archapplProto.pb.h

# In order to get protobuf source and header file we have
# to run protoc command with .proto file as an input
#
archapplProto.pb.h: archapplProto.proto
	protoc --cpp_out=. archapplProto.proto
	
# Clean up
#
clean:
	$(RM) $(OBJS) *.pb.* $(LIBS) O.*

# end