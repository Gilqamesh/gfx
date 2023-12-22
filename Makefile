#Configurable options

# binaries
build_bin = a.out
binaries  = $(build_bin)

# host compiler
cc = gcc
#TODO: embed this for compilation of 'build' module

# # vulkan, opengl
# gfx_backend = vulkan

# debug, release
build_mode = debug

#Non-configurable options

ifeq ($(build_mode), debug)
	common_cflags += -DDEBUG -g -O0 -Wall -Wextra -Werror
else ifeq ($(build_mode), release)
	common_cflags += -DRELEASE -Wno-unused-function -O3
else
	$(error build mode can either be debug or release)
endif

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
