glfw_dir     := $(dir $(lastword $(MAKEFILE_LIST)))
include $(wildcard $(glfw_dir)*/*.mk)

glfw_src     := $(wildcard $(glfw_dir)*.c)
glfw_dps     := $(patsubst %.c, %.d, $(glfw_src))
glfw_obj     := $(patsubst %.c, %.o, $(glfw_src))
glfw_cflags  := $(common_cflags) $(shell pkg-config --cflags glfw3) -I$(glfw_dir)/third_party/includes
ifeq ($(gfx_backend), opengl)
	glfw_pkg_lflags := gl
else ifeq ($(gfx_backend), vulkan)
	glfw_pkg_lflags := vulkan
else
	$(error gfx_backend must either be set to "opengl" or "vulkan")
endif
glfw_lflags  := $(shell pkg-config --static --libs glfw3 $(glfw_pkg_lflags)) $(debug_obj_dps)
glfw_obj_dps := $(glfw_obj) $(debug_obj_dps)
glfw_iflags  := -I$(glfw_dir) $(debug_iflags)

$(glfw_dir)/%.o: $(glfw_dir)/%.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(glfw_cflags) $(glfw_iflags)

-include $(glfw_dps)

# todo: add $(glfw_dir)/third_party as dependency
