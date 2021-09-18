################################################################################
# 
# COCADA Collection of Algorithms and DAta structures
# http://gitlab.com/paguso/cocada
#
# (c) 2016- Paulo G. S. Fonseca 
#
################################################################################

# compiler commands and default options
CC 	   := gcc

#shell commands
RMDIR := rm -rf
MKDIR := mkdir -p

# extensions and directories
SRC_EXT      := c
HEAD_EXT     := h
OBJ_EXT      := o

COCADA_ROOT			:= ../libcocada
COCADA_SRC_DIR		:= $(COCADA_ROOT)/src
COCADA_SRC_PATHS	:= $(shell find $(COCADA_SRC_DIR)/core -name '*.$(SRC_EXT)') 
COCADA_SRC_PATHS	+= $(shell find $(COCADA_SRC_DIR)/util -name '*.$(SRC_EXT)') 
COCADA_SRC_PATHS	+= $(shell find $(COCADA_SRC_DIR)/io -name '*.$(SRC_EXT)') 
COCADA_SRC_PATHS	+= $(shell find $(COCADA_SRC_DIR)/container -name '*.$(SRC_EXT)') 
COCADA_SRC_PATHS	+= $(shell find $(COCADA_SRC_DIR)/thrdpty -name '*.$(SRC_EXT)') 
COCADA_SRC_PATHS	+= $(shell find $(COCADA_SRC_DIR)/app -name '*.$(SRC_EXT)') 
COCADA_SRCS			:= $(notdir $(COCADA_SRC_PATHS))
COCADA_SRC_DIRS		:= $(sort $(dir $(COCADA_SRC_PATHS)))

CPM_ROOT			:= .
CPM_SRC_DIR 		:= $(CPM_ROOT)/src
CPM_SRC_PATHS      	:= $(shell find $(CPM_SRC_DIR) -name '*.$(SRC_EXT)') 
CPM_SRCS			:= $(notdir $(CPM_SRC_PATHS))
CPM_SRC_DIRS			:= $(sort $(dir $(CPM_SRC_PATHS)))

SRCS 				:= $(COCADA_SRCS) $(CPM_SRCS)
OBJS				:= $(patsubst %.$(SRC_EXT), %.$(OBJ_EXT), $(SRCS))
HEAD_PATHS			:= $(shell find $(COCADA_SRC_DIRS) $(CPM_SRC_DIRS) -name '*.$(HEAD_EXT)')
HEAD_DIRS			:= $(sort $(dir $(HEAD_PATHS)))
HEADS				:= $(notdir $(HEAD_PATHS))

# Where to look for sources
VPATH := $(COCADA_SRC_DIRS) $(CPM_SRC_DIRS) 

BUILD_DIR		:= $(CPM_ROOT)/build
DBG_BUILD_DIR	:= $(CPM_ROOT)/build/debug
REL_BUILD_DIR	:= $(CPM_ROOT)/build/release

CPM_EXE := cpm

.PHONY: clean install  

# -DDEBUG_LVL=0  No checks performed
# -DDEBUG_LVL=1  Only errors are captured 
# -DDEBUG_LVL=2  Errors and warnings are captured
# -DDEBUG_LVL=3  Errors, warnings, and debug messages are captured
CFLAGS := -Wall -g3 $(patsubst %, -I %,  $(HEAD_DIRS)) 
DBG_CFLAGS := -DDEBUG_LVL=3 -DMEM_DEBUG -DXCHAR_BYTESIZE=4
REL_CFLAGS := -DDEBUG_LVL=1 -DXCHAR_BYTESIZE=4 -O2


DBG_OBJS := $(OBJS:%=$(DBG_BUILD_DIR)/%)

$(DBG_BUILD_DIR)/%.$(OBJ_EXT): %.$(SRC_EXT)
	@echo "Running target $@"
	@echo "First prerequisite is $<"
	@echo "Compiling $< to $(DBG_BUILD_DIR)/$(notdir $@)..."
	$(CC) $(CFLAGS) $(DBG_CFLAGS) -c $< -o $(DBG_BUILD_DIR)/$(notdir $@)

$(DBG_BUILD_DIR):
	$(MKDIR) $(DBG_BUILD_DIR)


REL_OBJS := $(OBJS:%=$(REL_BUILD_DIR)/%)

$(REL_BUILD_DIR)/%.$(OBJ_EXT): %.$(SRC_EXT)
	@echo "Running target $@"
	@echo "First prerequisite is $<"
	@echo "Compiling $< to $(REL_BUILD_DIR)/$(notdir $@)..."
	$(CC) $(CFLAGS) $(REL_CFLAGS) -c $< -o $(REL_BUILD_DIR)/$(notdir $@)

$(REL_BUILD_DIR):
	$(MKDIR) $(REL_BUILD_DIR)


clean:
	$(RMDIR) $(BUILD_DIR)

debug: $(DBG_BUILD_DIR) $(DBG_OBJS)
	$(CC) $(CFLAGS) $(DBG_CFLAGS) $(DBG_BUILD_DIR)/*.$(OBJ_EXT) -lm -o $(DBG_BUILD_DIR)/$(CPM_EXE) 

release: $(REL_BUILD_DIR) $(REL_OBJS)
	$(CC) $(CFLAGS) $(REL_CFLAGS) $(REL_BUILD_DIR)/*.$(OBJ_EXT) -lm -o $(REL_BUILD_DIR)/$(CPM_EXE) 

rebuild: clean debug

install: rebuild
	@echo "Installing CPM"


print:
	@echo "SRCS" $(SRCS)
	@echo "HEADS" $(HEADS)
	@echo "OBJS" $(OBJS)
	@echo "DBG_OBJS" $(DBG_OBJS)

###############################################################################
# Code formatting 
###############################################################################
HEAD_LICENSE_CMD := bash ./headlicense.sh
FMT_CMD := astyle
FMT_OPTIONS := --style=kr --indent=tab --recursive -n

fmt:
	$(HEAD_LICENSE_CMD) $(filter-out $(THRDPTY_PATHS), $(SRC_PATHS)) $(filter-out $(THRDPTY_PATHS), $(HEAD_PATHS)) $(TEST_SRC_PATHS) $(TEST_HEAD_PATHS)
	$(FMT_CMD) $(FMT_OPTIONS) $(SRC_DIR)/*.$(SRC_EXT),*.$(HEAD_EXT) $(TEST_DIR)/*.$(SRC_EXT),*.$(HEAD_EXT)
	