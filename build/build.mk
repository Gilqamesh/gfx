build_src = \
	$(build_dir)/build.c \
	$(build_dir)/build_driver.c
build_cflags = $(common_cflags) -Icommon
build_lflags = 
build_obj = $(build_src:.c=.o)
build_dps = $(build_src:.c=.d)

$(build_dir)/%.o: $(build_dir)/%.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(build_cflags)

$(build_bin): $(build_obj)
	$(cc) -o $@ $^ $(build_lflags)

-include $(build_dps)
