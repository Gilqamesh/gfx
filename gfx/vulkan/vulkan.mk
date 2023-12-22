vulkan_dir     := $(dir $(lastword $(MAKEFILE_LIST)))
include $(wildcard $(vulkan_dir)*/*.mk)

vulkan_src     := $(wildcard $(vulkan_dir)*.c)
vulkan_cflags  := $(common_cflags)
vulkan_lflags  := $(glfw_lflags) $(debug_lflags)
vulkan_dps     := $(patsubst %.c, %.d, $(vulkan_src))
vulkan_obj     := $(patsubst %.c, %.o, $(vulkan_src))
vulkan_obj_dps := $(vulkan_obj) $(glfw_obj_dps) $(debug_obj_dps)
vulkan_iflags  := -I$(vulkan_dir) $(glfw_iflags) $(debug_iflags)

$(vulkan_dir)/%.o: $(vulkan_dir)/%.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(vulkan_cflags) $(vulkan_iflags)

-include $(vulkan_dps)
