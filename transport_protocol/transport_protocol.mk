transport_protocol_dir     := $(dir $(lastword $(MAKEFILE_LIST)))
include $(wildcard $(transport_protocol_dir)*/*.mk)

transport_protocol_src     := $(wildcard $(transport_protocol_dir)*.c)
transport_protocol_dps     := $(patsubst %.c, %.d, $(transport_protocol_src))
transport_protocol_obj     := $(patsubst %.c, %.o, $(transport_protocol_src))
transport_protocol_cflags  := $(common_cflags)
transport_protocol_lflags  := 
transport_protocol_obj_dps := $(transport_protocol_obj)
transport_protocol_iflags  := -I$(transport_protocol_dir)

$(transport_protocol_dir)/%.o: $(transport_protocol_dir)/%.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(transport_protocol_cflags) $(transport_protocol_iflags)

-include $(transport_protocol_dps)
