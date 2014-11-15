# //******************************************************************
# //
# // Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
# //
# //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# //
# // Licensed under the Apache License, Version 2.0 (the "License");
# // you may not use this file except in compliance with the License.
# // You may obtain a copy of the License at
# //
# //      http://www.apache.org/licenses/LICENSE-2.0
# //
# // Unless required by applicable law or agreed to in writing, software
# // distributed under the License is distributed on an "AS IS" BASIS,
# // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# // See the License for the specific language governing permissions and
# // limitations under the License.
# //
# //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
# override with `make BUILD=debug`
# default to release build
BUILD	 := release
PLATFORM := linux
CXX 	 := g++

ROOT_DIR	= ./

# You must create the file "local.properties" on your local machine which contains any local paths, etc
# local.properties should NOT be committed to repo
include $(ROOT_DIR)/../csdk/local.properties
# GTEST_DIR contains the path to Google Test libs and must be defined in local.properties
#  Example:
#  GTEST_DIR := /home/johndoe/utils/gtest-1.7.0

# NOTE:  to run app, make sure that LD_LIBRARY_PATH env variable
#        contains $(GTEST_DIR)/lib/.libs

OUT_DIR	  := $(BUILD)

INC_DIRS  := -I../include/
INC_DIRS  += -I../oc_logger/include
INC_DIRS  += -I../csdk/stack/include
INC_DIRS  += -I../csdk/ocsocket/include
INC_DIRS  += -I../csdk/ocrandom/include
INC_DIRS  += -I../csdk/logger/include
INC_DIRS  += -I../csdk/libcoap
INC_DIRS  += -I$(GTEST_DIR)/include

LIB_OC_LOGGER := ../oc_logger/lib/oc_logger.a

CXX_LIBS  := ../$(BUILD)/obj/liboc.a ../csdk/$(PLATFORM)/$(BUILD)/liboctbstack.a $(LIB_OC_LOGGER)
CXX_LIBS  += $(GTEST_DIR)/lib/.libs/libgtest.a $(GTEST_DIR)/lib/.libs/libgtest_main.a

CC_FLAGS.debug      := -g -O0 -g3 -Wall -ffunction-sections -fdata-sections \
                        -std=c++0x -pedantic $(INC_DIRS) -L$(ROOT_DIR)/$(BUILD) -DTB_LOG
CC_FLAGS.release    := -Os -Wall -fdata-sections -Wl,--gc-sections -Wl,-s \
                        -std=c++0x $(INC_DIRS) -L$(ROOT_DIR)/$(BUILD)

CPPFLAGS	+= $(CC_FLAGS.$(BUILD)) $(CXX_LIBS) -lpthread

all: prep_dirs tests

prep_dirs:
	-mkdir -p $(OUT_DIR)

tests: tests.cpp
	$(CXX) $^ $(CPPFLAGS) -o $(OUT_DIR)/$@

.PHONY: clean

clean:
	-rm -rf release
	-rm -rf debug