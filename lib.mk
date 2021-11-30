################################################################################
# 
# COCADA - COcada COllection of Algorithms and DAta structures
# http://gitlab.com/paguso/cocada
#
# (c) 2016- Paulo G. S. Fonseca 
#
################################################################################


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
DOC_DIR      := $(ROOT_DIR)/doc

INSTALL_DIR  := /usr/local/lib/
INCLUDE_DIR  := /usr/local/include/$(LIB_NAME)


###############################################################################
# Source and object names and paths  
###############################################################################

DEP_SRC_BASE_DIRS 	:= $(addprefix "../lib", $(addsuffix "/src", $(DEP_LIBS)))
DEP_SRC_PATHS   	:= $(foreach sdir, $(DEP_SRC_BASE_DIRS), $(shell find $(sdir) -name '*.$(SRC_EXT)'))
DEP_SRCS   			:= $(notdir $(DEP_SRC_PATHS))
DEP_SRC_DIRS    	:= $(sort $(dir $(DEP_SRC_PATHS)))
DEP_HEAD_PATHS   	:= $(foreach sdir, $(DEP_SRC_BASE_DIRS), $(shell find $(sdir) -name '*.$(HEAD_EXT)'))
DEP_HEADS   		:= $(notdir $(DEP_HEAD_PATHS))
DEP_HEAD_DIRS		:= $(sort $(dir $(DEP_HEAD_PATHS)))
SRC_PATHS			:= $(shell find $(SRC_DIR) -name '*.$(SRC_EXT)')
#THRDPTY_PATHS  	:= $(shell find $(THRDPTY_DIR) -name '*.[$(SRC_EXT),$(HEAD_EXT)]')
SRCS           		:= $(notdir $(SRC_PATHS))
SRC_DIRS       		:= $(sort $(dir $(SRC_PATHS)))
OBJS           		:= $(patsubst %.$(SRC_EXT), %.$(OBJ_EXT), $(SRCS))
HEAD_PATHS			:= $(shell find $(SRC_DIR) -name '*.$(HEAD_EXT)')
HEAD_DIRS			:= $(sort $(dir $(HEAD_PATHS)))
HEADS				:= $(notdir $(HEAD_PATHS))
TEST_SRC_PATHS 		:= $(shell find $(TEST_DIR) -name '*.$(SRC_EXT)')
TEST_HEAD_PATHS		:= $(shell find $(TEST_DIR) -name '*.$(HEAD_EXT)')
TEST_HEAD_DIRS		:= $(sort $(dir $(TEST_SRC_PATHS)))
TEST_SRCS			:= $(notdir $(TEST_SRC_PATHS)) 

ALL_HEAD_DIRS  := $(DEP_HEAD_DIRS) $(HEAD_DIRS) $(TEST_HEAD_DIRS)

VPATH          =  $(DEP_SRC_DIRS) $(SRC_DIRS) $(TEST_DIR)

.PHONY: clean doc debugclean debugbuild debugrebuild build staticlib install-static confirm_uninstall_static uninstall-static


###############################################################################
# Debug build
###############################################################################

DBG_DIR := $(BUILD_DIR)/debug

DBG_EXE := $(LIB_NAME)

# -DDEBUG_LVL=0  No checks performed
# -DDEBUG_LVL=1  Only errors are captured 
# -DDEBUG_LVL=2  Errors and warnings are captured
# -DDEBUG_LVL=3  Errors, warnings, and debug messages are captured
DBG_CFLAGS := -Wall -g3 $(addprefix -I, $(ALL_HEAD_DIRS)) \
-DDEBUG_LVL=3 -DMEM_DEBUG -DXCHAR_BYTESIZE=4

DBG_OBJS :=  $(patsubst %.$(SRC_EXT), $(DBG_DIR)/%.$(OBJ_EXT) , $(DEP_SRCS))\
$(patsubst %.$(SRC_EXT), $(DBG_DIR)/%.$(OBJ_EXT) , $(SRCS))\
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
CFLAGS  := -Wall -g -O3 $(addprefix -I, $(ALL_HEAD_DIRS)) \
-DDEBUG_LVL=1

OBJ_DIR := $(BUILD_DIR)/release

OBJS := $(patsubst %.$(SRC_EXT), $(OBJ_DIR)/%.$(OBJ_EXT) , $(SRCS))

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
STATIC_LIB_DIR := $(LIB_DIR)/static
SHARED_LIB_DIR := $(LIB_DIR)/shared

$(STATIC_LIB_DIR):
	$(MKDIR) $(STATIC_LIB_DIR)
	$(MKDIR) $(STATIC_LIB_DIR)/include


$(STLIB):
	ar rcs $(STATIC_LIB_DIR)/$(STLIB) $(OBJ_DIR)/*.$(OBJ_EXT)
	cp $(HEAD_PATHS) $(STATIC_LIB_DIR)/include
ifdef DEP_LIBS
	cp $(DEP_HEAD_PATHS) $(STATIC_LIB_DIR)/include
endif


DEP_STLIBS = $(patsubst %, $(LIB_PREFIX)%$(STLIB_SUFFIX), $(DEP_LIBS))


$(LIB_PREFIX)%$(STLIB_SUFFIX): ../lib%/Makefile
	$(MAKE) -C $(subst /Makefile,, $<) clean
	$(MAKE) -C $(subst /Makefile,, $<) staticlib 
	cp $(subst /Makefile,/build/lib/static/include/*, $<) $(STATIC_LIB_DIR)/include
	cp $(subst /Makefile,/build/lib/static/*.a, $<) $(STATIC_LIB_DIR)


staticlib: $(STATIC_LIB_DIR) $(DEP_STLIBS) rebuild $(STLIB)
	@echo "Static library file output to $(STATIC_LIB_DIR)/$(STLIB)"


install-staticlib: staticlib
	@echo Installing headers to $(INCLUDE_DIR)
	$(MKDIR) $(INCLUDE_DIR)
	cp $(STATIC_LIB_DIR)/include/* $(INCLUDE_DIR)
	@echo Installing static library to $(INSTALL_DIR)
	$(MKDIR) $(INSTALL_DIR)
	cp $(STATIC_LIB_DIR)/$(STLIB) $(INSTALL_DIR)
	@echo Done

confirm_uninstall_staticlib:
	@echo Removing headers dir $(INCLUDE_DIR)
	@echo Removing static library $(STLIB) from $(INSTALL_DIR)
	@echo -n "Are you sure? [y/N] " && read ans && [ $${ans:-N} = y ]

uninstall-staticlib: confirm_uninstall_staticlib
	$(RMDIR) $(INCLUDE_DIR)
	$(RMDIR) $(INSTALL_DIR)/$(STLIB)
	@echo Done




DEP_SHARED_LIBS := $(addsuffix .so, $(DEP_LIBS))

%.so: ../lib%/Makefile
	@echo "($(LIB_PREFIX)$(LIB_NAME)) Building required shared lib $@..."
	$(MAKE) -C $(subst /Makefile,, $<) clean
	$(MAKE) -C $(subst /Makefile,, $<) sharedlib 
	cp $(subst /Makefile,/build/lib/shared/include/*, $<) $(SHARED_LIB_DIR)/include
	cp $(subst /Makefile,/build/lib/shared/lib$@.*, $<) $(SHARED_LIB_DIR)
	@echo "($(LIB_PREFIX)$(LIB_NAME)) Done building required shared lib $@."


DEP_SHARED_LIBS_SONAMES := $(patsubst %,$(SHARED_LIB_DIR)/lib%.so,$(DEP_LIBS))

$(SHARED_LIB_DIR)/lib%.so:
	ln -rs $(wildcard $@.*) $@


PHONY: clean-sharedlib make-sharedlib

clean-sharedlib:
	$(RMDIR) $(SHARED_LIB_DIR)
	$(MKDIR) $(SHARED_LIB_DIR)
	$(MKDIR) $(SHARED_LIB_DIR)/include

make-sharedlib:
	$(CC) -shared -Wl,-soname,$(LIB_SONAME) -o $(SHARED_LIB_DIR)/$(LIB_REALNAME) $(OBJ_DIR)/*.o -lc -L$(SHARED_LIB_DIR) $(addprefix -l,$(DEP_LIBS))
	cp $(HEAD_PATHS) $(SHARED_LIB_DIR)/include
ifdef DEP_LIBS
	cp $(DEP_HEAD_PATHS) $(SHARED_LIB_DIR)/include
endif

sharedlib: CFLAGS += -fPIC
sharedlib: clean-sharedlib $(DEP_SHARED_LIBS) $(DEP_SHARED_LIBS_SONAMES) rebuild make-sharedlib
	@echo "Shared library file output to $(SHARED_LIB_DIR)/$(LIB_REALNAME)"
	

install-sharedlib: sharedlib
	@echo Installing headers to $(INCLUDE_DIR)
	$(MKDIR) $(INCLUDE_DIR)
	cp $(SHARED_LIB_DIR)/include/* $(INCLUDE_DIR)
	@echo Installing shared library to $(INSTALL_DIR)
	$(MKDIR) $(INSTALL_DIR)
	cp $(SHARED_LIB_DIR)/$(LIB_REALNAME) $(INSTALL_DIR)
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
# Clean up
###############################################################################

clean:
	$(RMDIR) $(BUILD_DIR)	

