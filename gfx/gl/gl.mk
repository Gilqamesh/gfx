gl_dir     := $(dir $(lastword $(MAKEFILE_LIST)))
include $(wildcard $(gl_dir)*/*.mk)

gl_src     := $(wildcard $(gl_dir)*.c) $(gl_dir)/third_party/glad/src/glad.c
gl_cflags  := $(common_cflags) -I$(gl_dir)/third_party/glad/include
gl_lflags  := $(glfw_lflags) $(debug_lflags)
gl_dps     := $(patsubst %.c, %.d, $(gl_src))
gl_obj     := $(patsubst %.c, %.o, $(gl_src))
gl_obj_dps := $(gl_obj) $(glfw_module_obj_dps) $(debug_obj_dps)
gl_iflags  := -I$(gl_dir) $(glfw_iflags) $(debug_iflags)

$(gl_dir)third_party/glad/src/glad.o: $(gl_dir)third_party/glad/src/glad.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(gl_cflags)

$(gl_dir)%.o: $(gl_dir)%.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(gl_cflags) $(gl_iflags)

-include $(gl_dps)
