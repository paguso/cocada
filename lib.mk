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
 

.PHONY: help

help: 
	@echo "choose the appropriate target to build"


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
#lib_hdrs = $(notdir $(lib_hdr_paths))

test_src_paths = $(shell find $(test_src_dir) -name '*.c')
test_src_dirs = $(sort $(dir $(test_src_paths)))
test_srcs = $(notdir $(test_src_paths))
test_hdr_paths = $(shell find $(test_src_dir) -name '*.h')
test_hdr_dirs = $(sort $(dir $(test_hdr_paths)))
#test_hdrs = $(notdir $(test_hdr_paths))

VPATH += $(lib_src_dirs)
VPATH += $(test_src_dirs)

# Output dirs

build_dir = ./build
debug_build_dir = $(build_dir)/debug
release_build_dir = $(build_dir)/release
target_build_dir = $(build_dir)/$(build_type)


# 
# Common compilation options and targets
#


INCLUDE_CFLAGS = $(addprefix -I, $(lib_hdr_dirs) $(test_hdr_dirs) $(all_deps_hdr_dirs)) 

$(build_dir):
	mkdir -p $@ 

#
# Recursively collects library dependencies 
#

deps_file = $(build_dir)/lib.deps 

export dep_chain += $(lib_name)

chain_deps_files = $(patsubst %,../lib%/$(deps_file), $(dep_chain))

$(deps_file): | $(build_dir)
#$(file > $(deps_file))
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
# Debug build
#

# Debug extra compiler flags

debug_cflags = -Wall -g3
debug_cflags += -DDEBUG_LVL=3 
debug_cflags += -DMEM_DEBUG   
debug_cflags += -DXCHAR_BYTESIZE=4

$(debug_build_dir):
	mkdir -p $@

$(debug_build_dir)/%.o: %.c
	$(CC) -c $(INCLUDE_CFLAGS) $(debug_cflags) $(CFLAGS) $< -o $@

# Recursively build library and its prerequisite libraries for debugging

debug_lib_deps = $(addprefix debug_lib_build_,$(lib_deps))

debug_lib_build_%: 
	cd ../$(@:debug_lib_build_%=lib%) && $(MAKE) debug_lib_build

debug_lib_objs = $(patsubst %.c,$(debug_build_dir)/%.o,$(lib_srcs))

$(debug_lib_objs): | $(debug_build_dir)

.PHONY: debug_lib_build

debug_lib_build: deps $(debug_lib_deps) $(debug_lib_objs) ;


# Add tests to debug build

debug_test_objs = $(patsubst %.c,$(debug_build_dir)/%.o,$(test_srcs))

$(debug_test_objs): | $(debug_build_dir)

.PHONY: debug_test_build 
debug_test_build: deps $(debug_test_objs) ;

debug_strict_deps_objs = $(foreach lib, $(strict_deps), \
	$(patsubst %.c, ../lib$(lib)/$(debug_build_dir)/%.o,\
		$(notdir $(shell find ../lib$(lib)/$(src_dir) -name '*.c'))))

# Wrap debug build as libraries + tests

.PHONY: debug release 
debug: debug_lib_build debug_test_build 
	$(CC) $(INCLUDE_CFLAGS) $(debug_cflags) $(CFLAGS) $(debug_strict_deps_objs) $(debug_build_dir)/*.o -lm -o $(debug_build_dir)/debug


#
# Release build
#

# Release extra compiler flags

relase_cflags =  -O3
relase_cflags += -DDEBUG_LVL=1 
relase_cflags += -DXCHAR_BYTESIZE=4

$(release_build_dir):
	mkdir -p $@

$(release_build_dir)/%.o: %.c
	$(CC) -c $(INCLUDE_CFLAGS) $(relase_cflags) $(CFLAGS) $< -o $@


# Recursively build library and its prerequisite libraries 

release_lib_deps = $(addprefix release_lib_build_,$(lib_deps))

release_lib_build_%: 
	cd ../$(@:release_lib_build_%=lib%) && $(MAKE) release_lib_build

release_lib_objs = $(patsubst %.c,$(release_build_dir)/%.o,$(lib_srcs))

$(release_lib_objs): | $(release_build_dir)

.PHONY: release_lib_build
release_lib_build: deps $(release_lib_deps) $(release_lib_objs) ;


.PHONY: release
release: release_lib_build;


#
# Static lib build and installation
#

staticlib_name = lib$(lib_name).a
staticlib_deps = $(addprefix staticlib_build_, $(lib_deps))

staticlib_build_%:
	cd ../$(@:staticlib_build_%=lib%) && $(MAKE) staticlib_build

dep_objs=$(patsubst %.c, %.o,\
	$(addprefix ../lib$(1)/$(release_build_dir)/,\
	$(notdir $(shell find ../lib$(1)/$(src_dir) -name *.c))))

staticlib_build: deps $(staticlib_deps) release 
	ar rcs $(build_dir)/lib$(lib_name).a\
		$(foreach lib,$(strict_deps),$(call dep_objs,$(lib)))\
		$(release_lib_objs)


include_dir = /usr/local/include/$(lib_name)
static_install_dir = /usr/local/lib/

$(include_dir):
	sudo mkdir -p $@

$(static_install_dir):
	sudo mkdir -p $@


staticlib_deps_install = $(addprefix staticlib_install_, $(lib_deps))

staticlib_install_%:
	cd ../$(@:staticlib_install_%=lib%) && $(MAKE) staticlib_install

staticlib_install: $(staticlib_deps_install) $(include_dir) $(install_dir)
	@echo Installing headers to $(include_dir)
	sudo $(RM) $(include_dir)/*
	sudo cp -p $(lib_hdr_paths) $(include_dir)
	@echo installing static library to $(static_install_dir)
	sudo cp -p $(build_dir)/*.a $(static_install_dir)
	@echo done

.PHONY: staticlib
staticlib: staticlib_build staticlib_install





.PHONY: info
info: 
	@echo This library is $(lib_name)

clean: 
	rm -rf $(build_dir)