gfx_backend = vulkan
# gfx_backend = opengl

build_mode = debug
# build_mode = release

bin = a.out
test_bin = test.out
cc = gcc

ifeq ($(gfx_backend), opengl)
	gfx_src = gl.c glad/src/glad.c
	gfx_cflags = -Iglad/include -DOPENGL
	gfx_lflags = gl
else ifeq ($(gfx_backend), vulkan)
	gfx_src = vulkan.c
	gfx_cflags = -DVULKAN
	gfx_lflags = vulkan
else
	$(error gfx_backend must either be set to "opengl" or "vulkan")
endif

common_src = \
	app.c \
	glfw.c \
	debug.c \
	buffer.c \
	system.c \
	gfx.c \
	game.c \
	$(gfx_src)
src = \
	$(common_src) \
	main.c
tst_src = \
	$(common_src) \
	test.c
obj = $(src:.c=.o)
test_obj = $(tst_src:.c=.o)
dps = $(src:.c=.d)
test_dps = $(tst_src:.c=.d)
cflags = $(shell pkg-config --cflags glfw3) -Iglfw/includes $(gfx_cflags)
lflags = $(shell pkg-config --static --libs glfw3 $(gfx_lflags))

ifeq ($(build_mode), debug)
	cflags += -DDEBUG -g -O0 -Wall -Wextra -Werror
else ifeq ($(build_mode), release)
	cflags += -DRELEASE -Wno-unused-function -O3
else
	$(error build mode can either be debug or release)
endif

.PHONY: all
all: $(bin)

.PHONY: clean
clean:
	- rm -f $(obj) $(test_obj) $(bin) $(test_bin) $(dps) $(test_dps)

.PHONY: re
re: clean
re: all

.PHONY: test 
test: $(test_bin)

$(test_bin): $(test_obj)
	$(cc) -o $(test_bin) $^ $(lflags)

%.o: %.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(cflags)

$(bin): $(obj)
	$(cc) -o $(bin) $^ $(lflags)

-include $(dps)
