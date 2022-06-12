all_valid_targets := debug staticlib help
all_libs := cocada cocadastrproc cocadabio cocadasketch
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
	@echo Usage:
	@echo 
	@echo make \<target\> \<libs\> 
	@echo 
	@echo where
	@echo 
	@echo - \<target\> is exactly one in \'$(all_valid_targets)\'
	@echo - \<libs\> is any valid subset of \'$(all_libs)\' or \'all\' for the complete set 
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

debug_libs = $(addprefix debug_,$(informed_libs))

debug_%:
	$(MAKE) -C ./$(@:debug_%=lib%) debug 

.PHONY: debug
debug: check_call $(debug_libs);


staticlib_libs = $(addprefix staticlib_,$(informed_libs))

staticlib_%:
	$(MAKE) -C ./$(@:staticlib_%=lib%) staticlib 

.PHONY: staticlib
staticlib: check_call $(staticlib_libs);