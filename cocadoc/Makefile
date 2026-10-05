src_dir = ./src
test_src_dir = ./test

cocada_base = ../
cocada_src_paths = $(shell find  $(cocada_base)/libcocada/src -name '*.c') $(shell find  $(cocada_base)/libcocadaapp/src -name '*.c') 
cocada_src_dirs = $(sort $(dir $(cocada_src_paths)))
cocada_srcs = $(notdir $(cocada_src_paths))
cocada_hdr_paths = $(shell find  $(cocada_base)/libcocada/src -name '*.h') $(shell find  $(cocada_base)/libcocadaapp/src -name '*.h') 
cocada_hdr_dirs = $(sort $(dir $(cocada_hdr_paths)))
cocada_hdrs = $(notdir $(cocada_hdr_paths))

src_paths = $(shell find $(src_dir) -name '*.c')
src_dirs = $(sort $(dir $(src_paths)))
srcs = $(notdir $(src_paths))
hdr_paths = $(shell find $(src_dir) -name '*.h')
hdr_dirs = $(sort $(dir $(hdr_paths)))
hdrs = $(notdir $(hdr_paths))

test_src_paths = $(shell find $(test_src_dir) -name '*.c')
test_src_dirs = $(sort $(dir $(test_src_paths)))
test_srcs = $(notdir $(test_src_paths))
test_hdr_paths = $(shell find $(test_src_dir) -name '*.h')
test_hdr_dirs = $(sort $(dir $(test_hdr_paths)))
test_hdrs = $(notdir $(test_hdr_paths))

VPATH += $(cocada_src_dirs)
VPATH += $(src_dirs)
VPATH += $(test_src_dirs)

# Build output dirs

build_dir = ./build
debug_build_dir = $(build_dir)/debug
test_build_dir = $(build_dir)/test
release_build_dir = $(build_dir)/release


# Make directory targets

$(debug_build_dir) $(test_build_dir) $(release_build_dir):
	mkdir -p $@ 

# 
# Common compilation options and targets
#

INCLUDE_CFLAGS = $(addprefix -I, $(cocada_hdr_dirs) $(hdr_dirs) $(test_hdr_dirs)) 

# generate header dependencies (.d files), so that objects are rebuilt
# when a header they include changes
DEP_CFLAGS = -MMD -MP

debug_cflags =  -O3
debug_cflags += -DDEBUG_LVL=1 
debug_cflags += -DXCHAR_BYTES=4
debug_cflags += -fpic


$(debug_build_dir)/%.o: %.c
	$(CC) -c $(INCLUDE_CFLAGS) $(DEP_CFLAGS) $(debug_cflags) $< -o $@


debug_srcs = $(cocada_srcs) $(srcs)
debug_objs = $(patsubst %.c,$(debug_build_dir)/%.o,$(debug_srcs))

.PHONY: build
build: $(debug_build_dir) $(debug_objs) 
	$(CC) $(debug_objs) -lm -o $(debug_build_dir)/cocadoc


#
# Unit tests: cocada + cocadoc sources (minus main.c) + tests
#

test_cflags = -Wall -g
test_cflags += -DDEBUG_LVL=3
test_cflags += -DMEM_DEBUG
test_cflags += -DXCHAR_BYTES=4

$(test_build_dir)/%.o: %.c
	$(CC) -c $(INCLUDE_CFLAGS) $(DEP_CFLAGS) $(test_cflags) $(CFLAGS) $< -o $@

test_all_srcs = $(cocada_srcs) $(filter-out main.c, $(srcs)) $(test_srcs)
test_objs = $(patsubst %.c,$(test_build_dir)/%.o,$(test_all_srcs))

$(test_objs): | $(test_build_dir)

.PHONY: test
test: $(test_objs)
	$(CC) $(test_objs) -lm -o $(test_build_dir)/test

-include $(debug_objs:.o=.d) $(test_objs:.o=.d)

.PHONY: clean
clean: 
	rm -rf $(debug_build_dir)/* $(test_build_dir)/*

