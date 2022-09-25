# COCADA - COCADA Collection of Algorithms and DAta Structures
#
# Copyright (C) 2016  Paulo G S Fonseca
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
#
 

#
# Global variable definitions
#

# Input dirs and files

src_dir = ./src
test_src_dir = ./test

lib_src_paths = $(shell find $(src_dir) -name '*.c')
lib_src_dirs = $(sort $(dir $(lib_src_paths)))
lib_srcs = $(notdir $(lib_src_paths))
lib_hdr_paths = $(shell find $(src_dir) -name '*.h')
lib_hdr_dirs = $(sort $(dir $(lib_hdr_paths)))
lib_hdrs = $(notdir $(lib_hdr_paths))

test_src_paths = $(shell find $(test_src_dir) -name '*.c')
test_src_dirs = $(sort $(dir $(test_src_paths)))
test_srcs = $(notdir $(test_src_paths))
test_hdr_paths = $(shell find $(test_src_dir) -name '*.h')
test_hdr_dirs = $(sort $(dir $(test_hdr_paths)))
test_hdrs = $(notdir $(test_hdr_paths))

VPATH += $(lib_src_dirs)
VPATH += $(test_src_dirs)

# Build output dirs

build_dir = ./build
debug_build_dir = $(build_dir)/debug
test_build_dir = $(build_dir)/test
static_build_dir = $(build_dir)/static
shared_build_dir = $(build_dir)/shared


# Library installation dirs

include_dir = /usr/local/include/
lib_include_dir=$(include_dir)/$(lib_name)
install_dir = /usr/local/lib/


# Make directory targets

$(build_dir) $(debug_build_dir) $(test_build_dir) $(static_build_dir) $(shared_build_dir)\
$(include_dir) $(lib_include_dir) $(install_dir):
	mkdir -p $@ 


# 
# Common compilation options and targets
#

INCLUDE_CFLAGS = $(addprefix -I, $(lib_hdr_dirs) $(test_hdr_dirs) $(all_deps_hdr_dirs)) 



#
# Recursively collects library dependencies 
#

deps_file = $(build_dir)/lib.deps 

export dep_chain += $(lib_name)

chain_deps_files = $(patsubst %,../lib%/$(deps_file), $(dep_chain))

$(deps_file): | $(build_dir)
	$(shell echo -n > $(deps_file))


.PHONY: reset_deps grab_deps deps

reset_deps: $(addprefix reset_deps_, $(lib_deps)) $(deps_file)
#$(file > $(deps_file))
	$(shell echo -n > $(deps_file))

reset_deps_%:
	cd ../$(@:reset_deps_%=lib%) && $(MAKE) reset_deps

grab_deps: $(addprefix grab_deps_, $(lib_deps))
#	@echo "grab_deps for $(lib_name) chain is $(dep_chain)"
#	@echo "(foreach f, $(chain_deps_files), (shell echo -n "$(lib_name) " >> (f)))"
	$(foreach f, $(chain_deps_files), $(shell echo -n "$(lib_name) " >> $(f)))

grab_deps_%:
	cd ../$(@:grab_deps_%=lib%) && $(MAKE) grab_deps
	
deps: reset_deps grab_deps ;
#	@echo "INCLUDES $(all_deps_hdr_dirs)"

all_deps = $(sort $(shell cat $(deps_file)))
all_deps_hdr_paths = $(foreach lib, $(all_deps), $(shell find ../lib$(lib)/$(src_dir) -name '*.h'))
all_deps_hdr_dirs = $(sort $(dir $(all_deps_hdr_paths)))
strict_deps = $(filter-out $(lib_name), $(all_deps))
#req_hdrs = $(notdir $(req_hdr_paths))


#
# Source code cloning
#

clone_dest = ~/cocada
clone_dir = $(strip $(clone_dest))

clone_tree =

clone_lib_deps = $(addprefix clone_lib_,$(lib_deps))

clone_lib_%:
	$(MAKE) -C ../$(@:clone_lib_%=lib%) clone 

$(clone_dir):
	mkdir -p $@

clone_tree_dirs =

ifdef clone_tree

define clone_tree_dir_template
clone_tree_dirs += $$(patsubst $(strip $(src_dir))%, $(clone_dir)/$(lib_name)%, $(1)) 

$$(patsubst $(strip $(src_dir))%, $(clone_dir)/$(lib_name)%, $(1)):
	mkdir -p $$@

endef 

$(foreach dir, $(lib_src_dirs), $(eval $(call clone_tree_dir_template, $(dir))))
endif

.PHONY: clone

define copy_template
$$(shell cp $(1) $$(patsubst $(strip $(src_dir))%, $(clone_dir)/$(lib_name)%, $(1))) 
endef

clone: $(clone_lib_deps) | $(clone_dir) $(clone_tree_dirs)
ifdef clone_tree
	$(foreach f,$(lib_hdr_paths),$(eval $(call copy_template, $(f))))
	$(foreach f,$(lib_src_paths),$(eval $(call copy_template, $(f))))
else
	cp $(lib_hdr_paths) $(clone_dir)
	cp $(lib_src_paths) $(clone_dir)
endif


#
# Template for dependency libraries targets
# Many targets require that the same action be performed on 
# dependency libraries before. The makefile structure for these
# targets is repetitive and can be generalised in the following
# template. 
# In order to create a target that triggers the same operation on
# the dependency libraries, include something like
#
# $(eval $(call deps_tgt_templ,target,lib_deps))
# target: ... $(target_deps) ...
#	recipe

define deps_tgt_templ
$(1)_deps = $$(addprefix $(1)_,$$($(2)))

$(1)_%:
	$(MAKE) -C ../$$(@:$(1)_%=lib%) $(1)

endef


#
# Debug build
#

# Debug extra compiler flags

debug_cflags = -Wall -g3 
debug_cflags += -DDEBUG_LVL=3 
debug_cflags += -DMEM_DEBUG   
debug_cflags += -DXCHAR_BYTES=4
ifdef print_mem
debug_cflags += -DMEM_DEBUG_PRINT_ALL
endif

#$(debug_build_dir):
#	mkdir -p $@

$(debug_build_dir)/%.o: %.c
	$(CC) -c $(INCLUDE_CFLAGS) $(debug_cflags) $(CFLAGS) $< -o $@

# Recursively build library and its prerequisite libraries for debugging

$(eval $(call deps_tgt_templ,debug_lib_build,lib_deps))

debug_lib_objs = $(patsubst %.c,$(debug_build_dir)/%.o,$(lib_srcs))

$(debug_lib_objs): | $(debug_build_dir)

.PHONY: debug_lib_build

debug_lib_build: deps $(debug_lib_build_deps) $(debug_lib_objs) ;


# Add tests to debug build

debug_test_objs = $(patsubst %.c,$(debug_build_dir)/%.o,$(test_srcs))

$(debug_test_objs): | $(debug_build_dir)

.PHONY: debug_test_build 
debug_test_build: deps $(debug_test_objs) ;

# Wrap debug build as libraries + tests

debug_strict_deps_objs = $(foreach lib, $(strict_deps), \
	$(patsubst %.c, ../lib$(lib)/$(debug_build_dir)/%.o,\
		$(notdir $(shell find ../lib$(lib)/$(src_dir) -name '*.c'))))

.PHONY: debug  
debug: debug_lib_build debug_test_build 
	$(CC) $(INCLUDE_CFLAGS) $(debug_cflags) $(CFLAGS) $(debug_strict_deps_objs) \
	$(debug_build_dir)/*.o -lm -o $(debug_build_dir)/debug



#
# Test build
#

# Test extra compiler flags

test_cflags = -O3 
test_cflags += -DDEBUG_LVL=1
test_cflags += -DXCHAR_BYTES=4

#$(test_build_dir):
#	mkdir -p $@

$(test_build_dir)/%.o: %.c
	$(CC) -c $(INCLUDE_CFLAGS) $(test_cflags) $(CFLAGS) $< -o $@

# Recursively build library and its prerequisite libraries for testing

$(eval $(call deps_tgt_templ,test_lib_build,lib_deps))

test_lib_objs = $(patsubst %.c,$(test_build_dir)/%.o,$(lib_srcs))

$(test_lib_objs): | $(test_build_dir)

.PHONY: test_lib_build

test_lib_build: deps $(test_lib_build_deps) $(test_lib_objs) ;


# Add tests to the test build

test_test_objs = $(patsubst %.c,$(test_build_dir)/%.o,$(test_srcs))

$(test_test_objs): | $(test_build_dir)

.PHONY: test_test_build 
test_test_build: deps $(test_test_objs) ;

# Wrap test build as libraries + tests

test_strict_deps_objs = $(foreach lib, $(strict_deps), \
	$(patsubst %.c, ../lib$(lib)/$(test_build_dir)/%.o,\
		$(notdir $(shell find ../lib$(lib)/$(src_dir) -name '*.c'))))

.PHONY: test  
test: test_lib_build test_test_build 
	$(CC) $(INCLUDE_CFLAGS) $(test_cflags) $(CFLAGS) $(test_strict_deps_objs) \
	$(test_build_dir)/*.o -lm -o $(test_build_dir)/test


#
# Static library build and installation
# 

# static lib build extra compiler flags

static_cflags =  -O3
static_cflags += -DDEBUG_LVL=1 
static_cflags += -DXCHAR_BYTES=4


$(static_build_dir)/%.o: %.c
	$(CC) -c $(INCLUDE_CFLAGS) $(static_cflags) $(CFLAGS) $< -o $@


# Recursively build library and its prerequisite libraries 

$(eval $(call deps_tgt_templ,staticlib_build,lib_deps))

static_objs = $(patsubst %.c,$(static_build_dir)/%.o,$(lib_srcs))

$(static_objs): | $(static_build_dir)

staticlib_name = lib$(lib_name).a
staticlib_path = $(static_build_dir)/$(staticlib_name)

dep_objs=$(patsubst %.c, %.o,\
	$(addprefix ../lib$(1)/$(static_build_dir)/,\
	$(notdir $(shell find ../lib$(1)/$(src_dir) -name *.c))))

.PHONY: staticlib_build
staticlib_build: deps $(staticlib_build_deps) $(static_objs) ;
	ar rcs $(staticlib_path)\
		$(foreach l,$(strict_deps),$(call dep_objs,$(l)))\
		$(static_objs)


$(eval $(call deps_tgt_templ,staticlib_install,lib_deps))

.PHONY: staticlib_install
staticlib_install: $(staticlib_install_deps) | $(lib_include_dir) $(install_dir)
	@echo Installing headers to $(lib_include_dir)
	cp -p $(lib_hdr_paths) $(lib_include_dir)
	@echo installing static library to $(install_dir)
	cp -p $(staticlib_path) $(install_dir)
	@echo done

.PHONY: staticlib
staticlib: staticlib_build staticlib_install

$(eval $(call deps_tgt_templ,staticlib_uninstall,lib_deps))

.PHONY: staticlib_uninstall
staticlib_uninstall: $(staticlib_uninstall_deps)
	@echo Removing lib$(lib_name) headers
	$(RM) $(addprefix $(lib_include_dir)/,$(lib_hdrs))
	-rmdir $(lib_include_dir)
	@echo Removing lib$(lib_name) archive 
	$(RM) $(addprefix $(install_dir)/,$(staticlib_name))


#
# Shared library build and installation
#

# static lib build extra compiler flags

shared_cflags =  -O3
shared_cflags += -DDEBUG_LVL=1 
shared_cflags += -DXCHAR_BYTES=4
shared_cflags += -fpic

#$(shared_build_dir):
#	mkdir -p $@

$(shared_build_dir)/%.o: %.c
	$(CC) -c $(INCLUDE_CFLAGS) $(shared_cflags) $(CFLAGS) $< -o $@


# Recursively build library and its prerequisite libraries 

$(eval $(call deps_tgt_templ,sharedlib_build,lib_deps))

shared_objs = $(patsubst %.c,$(shared_build_dir)/%.o,$(lib_srcs))

$(shared_objs): | $(shared_build_dir)

sharedlib_linkname = lib$(lib_name).so
sharedlib_soname = $(sharedlib_linkname).$(lib_major)
sharedlib_realname = $(sharedlib_soname).$(lib_minor).$(lib_release)
sharedlib_path = $(shared_build_dir)/$(sharedlib_realname)

.PHONY: sharedlib_build
sharedlib_build: deps $(sharedlib_build_deps) $(shared_objs) ;
	$(CC) -shared -Wl,-soname,$(sharedlib_soname) \
		-o $(sharedlib_path) \
		$(shared_objs) -lc


$(eval $(call deps_tgt_templ,sharedlib_install,lib_deps))

.PHONY: sharedlib_install
sharedlib_install: $(sharedlib_install_deps) | $(lib_include_dir) $(install_dir)
	@echo Installing headers to $(lib_include_dir)
	cp -p $(lib_hdr_paths) $(lib_include_dir)
	@echo installing shared library to $(install_dir)
	cp -p $(sharedlib_path) $(install_dir)
	ldconfig
	cd $(install_dir) && ln -sf $(sharedlib_soname) $(sharedlib_linkname)
	@echo done


.PHONY: sharedlib
sharedlib: sharedlib_build sharedlib_install

$(eval $(call deps_tgt_templ,sharedlib_uninstall,lib_deps))

.PHONY: sharedlib_uninstall
sharedlib_uninstall: $(sharedlib_uninstall_deps)
	@echo Removing lib$(lib_name) headers
	$(RM) $(addprefix $(lib_include_dir)/,$(lib_hdrs))
	-rmdir $(lib_include_dir)
	@echo Removing lib$(lib_name) archive 
	$(RM) $(addprefix $(install_dir)/,$(sharedlib_realname))
	ldconfig
	cd $(install_dir) && $(RM) $(sharedlib_linkname)
	@echo done


.PHONY: info
info: 
	@echo This is library $(lib_name)


.PHONY: clean
clean: 
	rm -rf $(build_dir)