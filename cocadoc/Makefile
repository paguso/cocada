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
release_build_dir = $(build_dir)/release


# Make directory targets

$(debug_build_dir) $(release_build_dir):
	mkdir -p $@ 

# 
# Common compilation options and targets
#

INCLUDE_CFLAGS = $(addprefix -I, $(cocada_hdr_dirs) $(hdr_dirs) $(test_hdr_dirs)) 

debug_cflags =  -O3
debug_cflags += -DDEBUG_LVL=1 
debug_cflags += -DXCHAR_BYTES=4
debug_cflags += -fpic


$(debug_build_dir)/%.o: %.c
	$(CC) -c $(INCLUDE_CFLAGS) $(debug_cflags) $< -o $@


debug_srcs = $(cocada_srcs) $(srcs) $(test_srcs)
debug_objs = $(patsubst %.c,$(debug_build_dir)/%.o,$(debug_srcs))

.PHONY: build
build: $(debug_build_dir) $(debug_objs) 
	$(CC) $(debug_build_dir)/*.o -lm -o $(debug_build_dir)/debug

.PHONY: clean
clean: 
	rm -rf $(debug_build_dir)/*

