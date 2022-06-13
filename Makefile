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
 
all_valid_targets := clone debug staticlib help
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
	@echo make \<target\> \<libs\> [make options] 
	@echo 
	@echo where
	@echo 
	@echo - \<target\> is exactly one in \'$(all_valid_targets)\'
	@echo - \<libs\> is any valid subset of \'$(all_libs)\' or \'all\' for the complete set 
	@echo - [make options] are valid GNU Make options, including environment variable definitions
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

# Clone

define multilib_target_template

$(1)_libs = $$(addprefix $(1)_,$$(informed_libs))

$(1)_%:
	$$(MAKE) -C ./$$(@:$(1)_%=lib%) $(1) 

.PHONY: $(1)
$(1): check_call $$($(1)_libs);

endef

$(foreach tgt,clone debug staticlib, $(eval $(call multilib_target_template,$(tgt))))
