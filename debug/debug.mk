debug_dir     := $(dir $(lastword $(MAKEFILE_LIST)))
include $(wildcard $(debug_dir)*/*.mk)

debug_src     := $(wildcard $(debug_dir)*.c)
debug_dps     := $(patsubst %.c, %.d, $(debug_src))
debug_obj     := $(patsubst %.c, %.o, $(debug_src))
debug_cflags  := $(common_cflags)
debug_lflags  := 
debug_obj_dps := $(debug_obj)
debug_iflags  := -I$(debug_dir)

$(debug_dir)/%.o: $(debug_dir)/%.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(debug_cflags) $(debug_iflags)

-include $(debug_dps)
