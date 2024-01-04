#Configurable options

# binaries
build_bin = b.out
binaries  = $(build_bin)

# host compiler
cc = gcc
#TODO: embed this for compilation of 'build' module

# # vulkan, opengl
gfx_backend := opengl
# gfx_backend := vulkan

# debug, release
build_mode = debug

#Non-configurable options

common_dir := common
common_src := \
	$(common_dir)/hash_set.c \
	$(common_dir)/hash_map.c \
	$(common_dir)/libc.c\
	$(common_dir)/file.c\
	$(common_dir)/str_builder.c
common_dps := $(common_src:.c=.d)
common_obj := $(common_src:.c=.o)
common_cflags := -I$(common_dir)
common_lflags :=
ifeq ($(build_mode), debug)
	common_cflags += -DDEBUG -g -O0 -Wall -Wextra -Werror
else ifeq ($(build_mode), release)
	common_cflags += -DRELEASE -Wno-unused-function -O3
else
	$(error build mode can either be debug or release)
endif

$(common_dir)/%.o: $(common_dir)/%.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(common_cflags)

.PHONY: all
all: build

.PHONY: clean
clean:
	- rm $(build_obj) $(build_bin) $(build_dps)

.PHONY: re
re: clean
re: all

build_dir = build
include $(build_dir)/*.mk

.PHONY: build
build: $(build_bin)

.PHONY:	debug
debug: ## display local make variables defined
	@$(foreach V, $(sort $(.VARIABLES)), \
		$(if $(filter-out environment% default automatic,\
			$(origin $V)), \
			$(warning $V = $($V) )) \
	)

-include $(common_dps)