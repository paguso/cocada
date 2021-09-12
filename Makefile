################################################################################
# 
# COCADA - COcada COllection of Algorithms and DAta structures
# http://gitlab.com/paguso/cocada
#
# (c) 2016- Paulo G. S. Fonseca 
#
################################################################################


# library identification and version
LIB_NAME     := cocada
LIB_MAJOR    := 0
LIB_MINOR    := 1
LIB_RELEASE  := 1

# library file names
LIB_PREFIX   := lib
LIB_SUFFIX   := .so
LIB_SONAME   := $(LIB_PREFIX)$(LIB_NAME)$(LIB_SUFFIX).$(LIB_MAJOR)
LIB_REALNAME := $(LIB_PREFIX)$(LIB_NAME)$(LIB_SUFFIX).$(LIB_MAJOR).$(LIB_MINOR).$(LIB_RELEASE)
STLIB_SUFFIX := .a
STLIB        := $(LIB_PREFIX)$(LIB_NAME)$(STLIB_SUFFIX)

# compiler commands and default options
CC 	   := gcc

#shell commands
RMDIR := rm -rf
MKDIR := mkdir -p

# extensions and directories
SRC_EXT      := c
HEAD_EXT     := h
OBJ_EXT      := o

ROOT_DIR     := .
SRC_DIR      := $(ROOT_DIR)/src
THRDPTY_DIR  := $(SRC_DIR)/thrdpty
TEST_DIR     := $(ROOT_DIR)/test
BUILD_DIR    := $(ROOT_DIR)/build
CPM_DIR      := $(ROOT_DIR)/cpm
DOC_DIR      := $(ROOT_DIR)/doc

INSTALL_DIR  := /usr/local/lib/$(LIB_NAME)
INCLUDE_DIR  := /usr/local/include/$(LIB_NAME)

DOXYGEN_CMD := doxygen
DOXYGEN_CONFIG_FILE := $(ROOT_DIR)/cocada.doxy


HEAD_LICENSE_CMD := bash ./headlicense.sh
FMT_CMD := astyle
FMT_OPTIONS := --style=kr --indent=tab --recursive -n


###############################################################################
# Source and object names and paths  
###############################################################################

SRC_PATHS      := $(shell find $(SRC_DIR) -name '*.$(SRC_EXT)')
THRDPTY_PATHS  := $(shell find $(THRDPTY_DIR) -name '*.[$(SRC_EXT),$(HEAD_EXT)]')
SRCS           := $(notdir $(SRC_PATHS))
SRC_DIRS       := $(sort $(dir $(SRC_PATHS)))
OBJS           := $(patsubst %.$(SRC_EXT), %.$(OBJ_EXT), $(SRCS))
HEAD_PATHS     := $(shell find $(SRC_DIR) -name '*.$(HEAD_EXT)')
HEAD_DIRS      := $(sort $(dir $(HEAD_PATHS)))
HEADS          := $(notdir $(HEAD_PATHS))
TEST_SRC_PATHS := $(shell find $(TEST_DIR) -name '*.$(SRC_EXT)')
TEST_HEAD_PATHS := $(shell find $(TEST_DIR) -name '*.$(HEAD_EXT)')
TEST_HEAD_DIRS := $(sort $(dir $(TEST_SRC_PATHS)))
TEST_SRCS      := $(notdir $(TEST_SRC_PATHS)) 

VPATH          =  $(SRC_DIRS) $(TEST_DIR) $(CPM_DIR)

.PHONY: clean doc debugclean debugbuild debugrebuild build staticlib install-static confirm_uninstall_static uninstall-static

###############################################################################
# Debug build
###############################################################################

DBG_EXE := cocada

# -DDEBUG_LVL=0  No checks performed
# -DDEBUG_LVL=1  Only errors are captured 
# -DDEBUG_LVL=2  Errors and warnings are captured
# -DDEBUG_LVL=3  Errors, warnings, and debug messages are captured
DBG_CFLAGS := -Wall -g3 $(patsubst %, -I %,  $(HEAD_DIRS) $(TEST_HEAD_DIRS)) \
-DDEBUG_LVL=3 -DMEM_DEBUG -DXCHAR_BYTESIZE=4

DBG_DIR := $(BUILD_DIR)/debug

DBG_OBJS := $(patsubst %.$(SRC_EXT), $(DBG_DIR)/%.$(OBJ_EXT) , $(SRCS))\
$(patsubst %.$(SRC_EXT), $(DBG_DIR)/%.$(OBJ_EXT) , $(TEST_SRCS))

$(DBG_DIR)/%.$(OBJ_EXT): %.$(SRC_EXT)
	@echo "Compiling $< to $(DBG_DIR)/$(notdir $@)..."
	@echo "$(CC) $(DBG_CFLAGS) -c $< -o $(DBG_DIR)/$(notdir $@)"
	$(CC) $(DBG_CFLAGS) -c $< -o $(DBG_DIR)/$(notdir $@)

$(DBG_DIR):
	$(MKDIR) $(DBG_DIR)

debugclean:
	$(RMDIR) $(DBG_DIR)

debug: $(DBG_DIR) $(DBG_OBJS)
	$(CC) $(DBG_CFLAGS) $(DBG_DIR)/*.$(OBJ_EXT) -lm -o $(DBG_DIR)/$(DBG_EXE) 

debugrebuild: debugclean debug



###############################################################################
# Library build and installation
###############################################################################

# -DDEBUG_LVL=0  No checks performed
# -DDEBUG_LVL=1  Only errors are captured 
# -DDEBUG_LVL=2  Errors and warnings are captured
# -DDEBUG_LVL=3  Errors, warnings, and debug messages are captured
CFLAGS  := -Wall -g -O3 $(patsubst %, -I %,  $(HEAD_DIRS) $(TEST_HEAD_DIRS)) -DDEBUG_LVL=1

OBJ_DIR := $(BUILD_DIR)/release

OBJS := $(patsubst %.$(SRC_EXT), $(OBJ_DIR)/%.$(OBJ_EXT) , $(SRCS))\

$(OBJ_DIR)/%.$(OBJ_EXT): %.$(SRC_EXT)
	@echo "Compiling $< to $(OBJ_DIR)/$(notdir $@)..."
	@echo "$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$(notdir $@)"
	$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$(notdir $@)

$(OBJ_DIR):
	$(MKDIR) $(OBJ_DIR)

buildclean: 
	$(RMDIR) $(OBJ_DIR)

build: $(OBJ_DIR) $(OBJS)  

rebuild: buildclean build

LIB_DIR := $(BUILD_DIR)/lib

$(LIB_DIR):
	$(MKDIR) $(LIB_DIR)

$(STLIB):
	ar rcs $(LIB_DIR)/$(STLIB) $(OBJ_DIR)/*.$(OBJ_EXT)

staticlib: build $(LIB_DIR) $(STLIB)
	@echo "Static library file output to $(LIB_DIR)/$(STLIB)"

install-static: staticlib
	@echo Installing headers to $(INCLUDE_DIR)
	$(MKDIR) $(INCLUDE_DIR)
	cp $(HEAD_PATHS) $(INCLUDE_DIR)
	@echo Installing static library to $(INSTALL_DIR)
	$(MKDIR) $(INSTALL_DIR)
	cp $(LIB_DIR)/$(STLIB) $(INSTALL_DIR)
	@echo Done

confirm_uninstall_static:
	@echo Removing headers dir $(INCLUDE_DIR)
	@echo Removing static library dir $(INSTALL_DIR)
	@echo -n "Are you sure? [y/N] " && read ans && [ $${ans:-N} = y ]

uninstall-static: confirm_uninstall_static
	$(RMDIR) $(INCLUDE_DIR)
	$(RMDIR) $(INSTALL_DIR)
	@echo Done


$(LIB_REALNAME):
	$(CC) -shared -Wl,-soname,$(LIB_SONAME) -o $(LIB_DIR)/$(LIB_REALNAME) $(OBJ_DIR)/*.o -lc


sharedlib: CFLAGS += -fPIC
sharedlib: rebuild $(LIB_DIR) $(LIB_REALNAME)
	@echo "Shared library file output to $(LIB_DIR)/$(LIB_REALNAME)"
	

install-shared: sharedlib
	@echo Installing headers to $(INCLUDE_DIR)
	$(MKDIR) $(INCLUDE_DIR)
	cp $(SRC_DIR)/*.h $(INCLUDE_DIR)
	@echo Installing shared library to $(INSTALL_DIR)
	$(MKDIR) $(INSTALL_DIR)
	cp $(LIB_DIR)/$(LIB_REALNAME) $(INSTALL_DIR)
	ldconfig -n $(INSTALL_DIR)
	@echo Done


###############################################################################
# Integrated tests
###############################################################################

TEST_OBJ := $(patsubst %,$(TEST_OBJ_DIR)/%,$(_TEST_OBJ))


$(TEST_OBJ_DIR):
	$(MKDIR) $(TEST_OBJ_DIR)
	

$(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $< 


$(TEST_BIN_DIR):
	$(MKDIR) $(TEST_BIN_DIR)


$(TEST_EXE): 
	#$(CC) $(CFLAGS) -static $(TEST_SRC_DIR)/*.c -L $(LIB_DIR) -l$(LIB_NAME) -lm -o $(TEST_BIN_DIR)/$(TEST_EXE) 
	$(CC) $(CFLAGS) -static $(TEST_OBJ_DIR)/*.o -L $(LIB_DIR) -l$(LIB_NAME) -lm -o $(TEST_BIN_DIR)/$(TEST_EXE) 


testbuild: staticlib $(TEST_OBJ_DIR) $(TEST_BIN_DIR) $(TEST_OBJ) $(TEST_EXE)


testrun:
	$(TEST_CMD)
	

test: testbuild testrun


###############################################################################
# CPM
###############################################################################

CPM_OBJ_DIR := $(BUILD_DIR)/cpm
CPM_EXE := cpm

# -DDEBUG_LVL=0  No checks performed
# -DDEBUG_LVL=1  Only errors are captured 
# -DDEBUG_LVL=2  Errors and warnings are captured
# -DDEBUG_LVL=3  Errors, warnings, and debug messages are captured
CPM_CFLAGS := -Wall -g3 $(patsubst %, -I %,  $(HEAD_DIRS) $(TEST_HEAD_DIRS)) \
-DDEBUG_LVL=3 -DMEM_DEBUG -DXCHAR_BYTESIZE=4

CPM_SRC_PATHS	:= $(shell find $(SRC_DIR)/core $(SRC_DIR)/util $(SRC_DIR)/io $(SRC_DIR)/thrdpty $(SRC_DIR)/container $(SRC_DIR)/app $(ROOT_DIR)/cpm -name '*.$(SRC_EXT)')
CPM_SRCS	:= $(notdir $(CPM_SRC_PATHS))
CPM_OBJS	:= $(patsubst %.$(SRC_EXT), $(CPM_OBJ_DIR)/%.$(OBJ_EXT) , $(CPM_SRCS))


$(CPM_OBJ_DIR)/%.$(OBJ_EXT): %.$(SRC_EXT)
	@echo "Running target $@"
	@echo "First prerequisite is $<"
	@echo "Compiling $< to $(CPM_OBJ_DIR)/$(notdir $@)..."
	$(CC) $(CPM_CFLAGS) -c $< -o $(CPM_OBJ_DIR)/$(notdir $@)

$(CPM_OBJ_DIR):
	$(MKDIR) $(CPM_OBJ_DIR)

cpmclean:
	$(RMDIR) $(CPM_OBJ_DIR)

cpmbuild: $(CPM_OBJ_DIR) $(CPM_OBJS)
	$(CC) $(CPM_DBG_CFLAGS) $(CPM_OBJ_DIR)/*.$(OBJ_EXT) -lm -o $(CPM_OBJ_DIR)/$(CPM_EXE) 

cpmrebuild: cpmclean cpmbuild


###############################################################################
# Documentation
###############################################################################

doc:
	$(DOXYGEN_CMD) $(DOXYGEN_CONFIG_FILE)



###############################################################################
# Clean up
###############################################################################

clean:
	$(RMDIR) $(BUILD_DIR)	
	$(RMDIR) $(DOC_DIR)	


###############################################################################
# Code formatting 
###############################################################################

fmt:
	$(HEAD_LICENSE_CMD) $(filter-out $(THRDPTY_PATHS), $(SRC_PATHS)) $(filter-out $(THRDPTY_PATHS), $(HEAD_PATHS)) $(TEST_SRC_PATHS) $(TEST_HEAD_PATHS)
	$(FMT_CMD) $(FMT_OPTIONS) $(SRC_DIR)/*.$(SRC_EXT),*.$(HEAD_EXT) $(TEST_DIR)/*.$(SRC_EXT),*.$(HEAD_EXT)
	