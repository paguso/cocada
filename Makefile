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
 
all_valid_targets := clean clone debug staticlib_build staticlib_install staticlib_uninstall staticlib sharedlib_build sharedlib_install sharedlib_uninstall sharedlib doc clean_doc help
all_libs := cocada cocadaapp cocadastrproc cocadabio cocadasketch
all_valid_args := $(all_valid_targets) $(all_libs) all
invalid_args := $(filter-out $(all_valid_args), $(MAKECMDGOALS))

ifeq (all, $(filter all, $(MAKECMDGOALS)))
	do_all = true 
	informed_libs = $(all_libs)
else
	informed_libs = $(strip $(filter $(all_libs), $(MAKECMDGOALS)))
endif 

informed_targets = $(strip $(filter $(all_valid_targets), $(MAKECMDGOALS)))

lib_dirs = $(addprefix, ./lib, $(target_libs))

.PHONY: help
help:
	@echo This is COCADA
	@echo
	@echo Usage:
	@echo 
	@echo make \<target\> [\<libs\>] [make options] 
	@echo 
	@echo where
	@echo 
	@echo - \<target\> is one in \'$(all_valid_targets)\'
	@echo - \<libs\> is any valid subset of \'$(all_libs)\' or \'all\' for the complete set 
	@echo - [make options] are valid GNU Make options, including environment variable definitions
	@echo 
	@echo Available targets:
	@echo
	@echo - clean [\<libs\>]: wipeout all library builds 
	@echo - clone [\<libs\>]: copy library sources
	@echo - debug [\<libs\>]: build library and tests for debugging
	@echo - staticlib_build [\<libs\>]: build static libraries
	@echo - staticlib_install [\<libs\>]: install built static libraries
	@echo - staticlib_uninstall [\<libs\>]: uninstall static libraries
	@echo - staticlib [\<libs\>]: build and install static libraries 
	@echo - sharedlib_build [\<libs\>]: build shared libraries
	@echo - sharedlib_install [\<libs\>]: install built shared libraries
	@echo - sharedlib_uninstall [\<libs\>]: uninstall shared libraries
	@echo - sharedlib [\<libs\>]: build and install shared libraries 
	@echo - fmt: format library source files 
	@echo - doc: generate API documentation for all libraries
	@echo - doc_clean: wipeout API documentation for all libraries
	@echo - help: prints this message
	@echo
	@echo "See README.md for more instructions"

.PHONY: check_call 
check_call:
ifdef invalid_args
	$(error Don't know what to do with $(invalid_args). Try 'make help')
else ifeq (,$(sort $(informed_targets)))
	$(error No valid target given. Try 'make help')
else ifneq (1,$(words $(sort $(informed_targets))))
	$(error Multiple targets given. Try 'make help')
else
	@:
endif

%: check_call
ifdef invalid_args
	@echo Don\'t know what to do with $(invalid_args)
	@echo Try make help
else 
	@:
endif

.PHONY: all 
all:
	@:


define multilib_target_template

$(1)_libs = $$(addprefix $(1)_,$$(informed_libs))

$(1)_%:
	$$(MAKE) -C ./$$(@:$(1)_%=lib%) $(1) 

.PHONY: $(1)
$(1): check_call $$($(1)_libs);

endef


$(foreach tgt,$(filter-out help doc clean_doc,$(all_valid_targets)), $(eval $(call multilib_target_template,$(tgt))))


doc_dir = apidoc
doxygen_cfg := cocada.doxy

cocada_doxygen_input = $(patsubst %,lib%/src,$(all_libs))

# API documentation

.PHONY: doc clean_doc
doc:
	COCADA_DOXYGEN_INPUT='$(cocada_doxygen_input)' doxygen $(doxygen_cfg)

clean_doc:
	$(RM) -r $(doc_dir)	

head_license_cmd := headlicense.sh
fmt_cmd := astyle
fmt_options := --style=kr --indent=tab --recursive -n

# code formatting 

fmt:
	$(head_license_cmd) $(filter-out $(thrdpty_paths), $(src_paths)) $(filter-out $(thrdpty_paths), $(head_paths)) $(test_src_paths) $(test_head_paths)
	$(fmt_cmd) $(fmt_options) $(src_dir)/*.$(src_ext),*.$(head_ext) $(test_dir)/*.$(src_ext),*.$(head_ext)
	
