gfx_dir     := $(dir $(lastword $(MAKEFILE_LIST)))
include $(wildcard $(gfx_dir)*/*.mk)

gfx_src     := $(wildcard $(gfx_dir)*.c)
gfx_cflags  := $(common_cflags)
gfx_dps     := $(patsubst %.c, %.d, $(gfx_src))
gfx_obj     := $(patsubst %.c, %.o, $(gfx_src))
gfx_lflags  := $(glfw_lflags)
gfx_obj_dps := $(gfx_obj) $(glfw_obj_dps)
gfx_iflags  := -I$(gfx_dir) $(glfw_iflags)

ifeq ($(gfx_backend), opengl)
	gfx_cflags  += -DOPENGL
	gfx_lflags  += $(gl_lflags)
	gfx_obj_dps += $(gl_obj_dps)
else ifeq ($(gfx_backend), vulkan)
	gfx_cflags  += -DVULKAN
	gfx_lflags  += $(vulkan_lflags)
	gfx_obj_dps += $(vulkan_obj_dps)
else
	$(error gfx_backend must either be set to "opengl" or "vulkan")
endif

$(gfx_dir)/%.o: $(gfx_dir)/%.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(gfx_cflags) $(gfx_iflags)

-include $(gfx_dps)
