game_dir     := $(dir $(lastword $(MAKEFILE_LIST)))
include $(wildcard $(game_dir)*/*.mk)

game_src     := $(wildcard $(game_dir)*.c)
game_dps     := $(patsubst %.c, %.d, $(game_src))
game_obj     := $(patsubst %.c, %.o, $(game_src))
game_cflags  := $(common_cflags)
game_lflags  := $(gfx_lflags) $(debug_lflags) $(common_lflags)
game_obj_dps := $(game_obj) $(gfx_obj_dps) $(debug_obj_dps) $(common_obj_dps)
game_iflags  := -I$(game_dir) $(gfx_iflags) $(debug_iflags) $(common_iflags)

$(game_dir)/%.o: $(game_dir)/%.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(game_cflags) $(game_iflags)

-include $(game_dps)
