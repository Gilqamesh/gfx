game_server_dir     := $(dir $(lastword $(MAKEFILE_LIST)))
include $(wildcard $(game_server_dir)*/*.mk)

game_server_src     := $(wildcard $(game_server_dir)*.c)
game_server_dps     := $(patsubst %.c, %.d, $(game_server_src))
game_server_obj     := $(patsubst %.c, %.o, $(game_server_src))
game_server_cflags  := $(common_cflags)
game_server_lflags  := $(common_lflags) $(debug_lflags) $(game_lflags)
game_server_obj_dps := $(game_server_obj) $(common_obj_dps) $(debug_obj_dps) $(game_obj_dps)
game_server_iflags  := -I$(game_server_dir) $(common_iflags) $(debug_iflags) $(game_iflags)

$(game_server_dir)/%.o: $(game_server_dir)/%.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(game_server_cflags) $(game_server_iflags)

-include $(game_server_dps)
