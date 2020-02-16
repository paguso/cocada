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

# extendsions and directories
SRC_EXT 	 := c
HEAD_EXT 	 := h
OBJ_EXT      := o

ROOT_DIR     := .
SRC_DIR      := $(ROOT_DIR)/src
TEST_DIR     := $(ROOT_DIR)/test
BUILD_DIR    := $(ROOT_DIR)/build
DOC_DIR      := $(ROOT_DIR)/doc

INSTALL_DIR  := /usr/local/lib/$(LIB_NAME)
INCLUDE_DIR  := /usr/local/include/$(LIB_NAME)

DOXYGEN_CMD := doxygen
DOXYGEN_CONFIG_FILE := $(ROOT_DIR)/cocada.doxy


###############################################################################
# Source and object names and paths  
###############################################################################

SRC_PATHS      := $(shell find $(SRC_DIR) -name '*.$(SRC_EXT)')
SRCS           := $(notdir $(SRC_PATHS))
SRC_DIRS       := $(sort $(dir $(SRC_PATHS)))
OBJS           := $(patsubst %.$(SRC_EXT), %.$(OBJ_EXT), $(SRCS))
HEAD_PATHS     := $(shell find $(SRC_DIR) -name '*.$(HEAD_EXT)')
HEAD_DIRS      := $(sort $(dir $(HEAD_PATHS)))
TEST_SRC_PATHS := $(shell find $(TEST_DIR) -name '*.$(SRC_EXT)')
TEST_HEAD_DIRS := $(sort $(dir $(TEST_SRC_PATHS)))
TEST_SRCS      := $(notdir $(TEST_SRC_PATHS)) 

VPATH          =  $(SRC_DIRS) $(TEST_DIR)

.PHONY: clean doc debugclean debugbuild debugrebuild build staticlib install-static

###############################################################################
# Debug build
###############################################################################

DBG_EXE := cocada
DBG_CFLAGS := -Wall -g3 $(patsubst %, -I %,  $(HEAD_DIRS) $(TEST_HEAD_DIRS))

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

debugbuild: $(DBG_DIR) $(DBG_OBJS)
	$(CC) $(DBG_cFLAGS) $(DBG_DIR)/*.$(OBJ_EXT) -lm -o $(DBG_DIR)/$(DBG_EXE) 

debugrebuild: debugclean debugbuild



###############################################################################
# Library build and installation
###############################################################################

CFLAGS  := -Wall -g -O3 $(patsubst %, -I %,  $(HEAD_DIRS) $(TEST_HEAD_DIRS))

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