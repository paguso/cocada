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

INCLUDE_CFLAGS = $(addprefix -I, $(lib_hdr_dirs) $(test_hdr_dirs) $(all_deps_hdr_dirs)) 

ALL_CFLAGS = $(INCLUDE_CFLAGS) $(CFLAGS)

$(build_dir):
	mkdir -p $(build_dir)

.PHONY: help

help: 
	@echo "choose the appropriate target to build"

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
	@echo "grab_deps for $(lib_name) chain is $(dep_chain)"
	@echo "(foreach f, $(chain_deps_files), (shell echo -n "$(lib_name) " >> (f)))"
	$(foreach f, $(chain_deps_files), $(shell echo -n "$(lib_name) " >> $(f)))

grab_deps_%:
	cd ../$(@:grab_deps_%=lib%) && $(MAKE) grab_deps
	
deps: reset_deps grab_deps ;
	@echo "INCLUDES $(all_deps_hdr_dirs)"

all_deps = $(sort $(shell cat $(deps_file)))
all_deps_hdr_paths = $(foreach lib, $(all_deps), $(shell find ../lib$(lib)/$(src_dir) -name '*.h'))
all_deps_hdr_dirs = $(sort $(dir $(all_deps_hdr_paths)))
strict_deps = $(filter-out $(lib_name), $(all_deps))
#req_hdrs = $(notdir $(req_hdr_paths))


#
# Debug build
#

# Debug compiler extra flags

DEBUG_CFLAGS = -Wall -g3
DEBUG_CFLAGS += -DDEBUG_LVL=3 
DEBUG_CFLAGS += -DMEM_DEBUG   
DEBUG_CFLAGS += -DXCHAR_BYTESIZE=4

# Recursively build library and its prerequisite libraries for debugging

$(debug_build_dir):
	mkdir -p $@

$(debug_build_dir)/%.o: %.c
	$(CC) -c $(ALL_CFLAGS) $< -o $@

debug_lib_deps = $(addprefix debug_lib_build_,$(lib_deps))

debug_lib_build_%: 
	cd ../$(@:debug_lib_build_%=lib%) && $(MAKE) debug_lib_build

debug_lib_objs = $(patsubst %.c,$(debug_build_dir)/%.o,$(lib_srcs))

$(debug_lib_objs): | $(debug_build_dir)

.PHONY: debug_lib_build

debug_lib_build: CFLAGS += $(DEBUG_CFLAGS)
debug_lib_build: deps $(debug_lib_deps) $(debug_lib_objs) ;


# Finish debug build by adding tests

debug_test_objs = $(patsubst %.c,$(debug_build_dir)/%.o,$(test_srcs))

$(debug_test_objs): | $(debug_build_dir)

.PHONY: debug_test_build 
debug_test_build: CFLAGS += $(DEBUG_CFLAGS)
debug_test_build: deps $(debug_test_objs) ;


strict_deps_objs = $(foreach lib, $(strict_deps), \
	$(patsubst %.c, ../lib$(lib)/build/debug/%.o,\
		$(notdir $(shell find ../lib$(lib)/src -name '*.c'))))

.PHONY: debug 
debug: debug_lib_build debug_test_build 
	$(CC) $(CFLAGS) $(strict_deps_objs) $(debug_build_dir)/*.o -lm -o $(debug_build_dir)/debug


clean: 
	rm -rf $(build_dir)

