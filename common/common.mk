common_dir     := $(dir $(lastword $(MAKEFILE_LIST)))
include $(wildcard $(common_dir)*/*.mk)

common_src     := $(wildcard $(common_dir)*.c)
common_dps     := $(patsubst %.c, %.d, $(common_src))
common_obj     := $(patsubst %.c, %.o, $(common_src))
common_cflags  := 
common_lflags  := 
common_obj_dps := $(common_obj)
common_iflags  := -I$(common_dir)

ifeq ($(build_mode), debug)
	common_cflags += -DDEBUG -g -O0 -Wall -Wextra -Werror
else ifeq ($(build_mode), release)
	common_cflags += -DRELEASE -Wno-unused-function -O3
else
	$(error build mode can either be debug or release)
endif

$(common_dir)/%.o: $(common_dir)/%.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(common_cflags) $(common_iflags)

-include $(common_dps)
