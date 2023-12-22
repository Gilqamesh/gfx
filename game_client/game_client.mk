game_client_dir     := $(dir $(lastword $(MAKEFILE_LIST)))
include $(wildcard $(game_client_dir)*/*.mk)

game_client_src     := $(wildcard $(game_client_dir)*.c)
game_client_dps     := $(patsubst %.c, %.d, $(game_client_src))
game_client_obj     := $(patsubst %.c, %.o, $(game_client_src))
game_client_cflags  := $(common_cflags)
game_client_lflags  := 
game_client_obj_dps := $(game_client_obj)
game_client_iflags  := -I$(game_client_dir)

$(game_client_dir)/%.o: $(game_client_dir)/%.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(game_client_cflags) $(game_client_iflags)

-include $(game_client_dps)
