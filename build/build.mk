build_src := \
	$(common_src) \
	$(build_dir)/build.c \
	$(build_dir)/build_driver.c
build_cflags := \
	$(common_cflags) \
	-DGLFW_CFLAGS="\"$(shell pkg-config --cflags glfw3)\"" \
	-DGLFW_LFLAGS="\"$(shell pkg-config --static --libs glfw3)\""
build_lflags := 
build_obj := $(build_src:.c=.o)
build_dps := $(build_src:.c=.d)

ifeq ($(gfx_backend), opengl)
	build_cflags += \
		-DOPENGL \
		-DGFX_BACKEND_CFLAGS="\"-DOPENGL\"" \
		-DGFX_BACKEND_LFLAGS="\"$(shell pkg-config --static --libs gl)\""
	
else ifeq ($(gfx_backend), vulkan)
	build_cflags += \
		-DVULKAN \
		-DGFX_BACKEND_CFLAGS="\"-DVULKAN\"" \
		-DGFX_BACKEND_LFLAGS="\"$(shell pkg-config --static --libs vulkan)\""
else
	$(error gfx_backend must either be set to "opengl" or "vulkan")
endif

$(build_dir)/%.o: $(build_dir)/%.c
	$(cc) -c $< -o $@ -MMD -MP -MF $(<:.c=.d) $(build_cflags)

$(build_bin): $(build_obj)
	$(cc) -o $@ $^ $(build_lflags)

-include $(build_dps)
