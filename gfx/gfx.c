#include "gfx.h"

#include "glfw.h"

/**
 * TODO: remove this file and create gfx_vulkan and gfx_gl, both implementing gfx.h
*/

# if defined(OPENGL)
#  include "gl.h"
# elif defined(VULKAN)
#  include "vulkan.h"
# else
#  error "undefined backend, must either be OPENGL or VULKAN"
# endif

# include "gfx_math.h"

# include <stdlib.h>

# include "gfx_impl.c"

bool gfx__init() {
#if defined(OPENGL)
#elif defined(VULKAN)
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif
    memset(&gfx, 0, sizeof(gfx));

    return true;
}

void gfx__deinit() {
#if defined(OPENGL)
#elif defined(VULKAN)
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif

    ASSERT(!gfx.current_gfx_window);
}

gfx_window_t gfx_window__create(const char* title, uint32_t width, uint32_t height) {
    uint32_t number_of_monitors = 0;
    monitor_t* monitors = monitor__get_monitors(&number_of_monitors);
    if (number_of_monitors == 0) {
        // todo: notify client
        return 0;
    }

#if defined(OPENGL)
#elif defined(VULKAN)
    // The window surface cannot be shared with another API
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif

    window_t window = window__create(monitors[0], title, width, height);

#if defined(OPENGL)
    if (!gl__init_context(window)) {
        window__destroy(window);
        return 0;
    }
#elif defined(VULKAN)
    if (!vk__init(window)) {
        window__destroy(window);
        return 0;
    }
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif

    gfx_window_t result = calloc(1, sizeof(*result));
    if (!result) {
        window__destroy(window);
        return 0;
    }

    result->window = window;

    gfx_window__set_current(result);

    return result;
}

void gfx_window__destroy(gfx_window_t self) {
    gfx_window__set_current(self);

    window__destroy(self->window);
#if defined(OPENGL)
#elif defined(VULKAN)
    vk__deinit();
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif

    free(self);

    gfx.current_gfx_window = 0;
}

void gfx_window__set_viewport(gfx_window_t self, int32_t x, int32_t y, uint32_t width, uint32_t height) {
    gfx_window__set_current(self);

#if defined(OPENGL)
    gl__viewport(x, y, width, height);
#elif defined(VULKAN)
    (void) x;
    (void) y;
    (void) width;
    (void) height;
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif
}

void gfx_window__set_scissor(gfx_window_t self, int32_t x, int32_t y, uint32_t width, uint32_t height) {
    gfx_window__set_current(self);

#if defined(OPENGL)
    gl__scissor(x, y, width, height);
#elif defined(VULKAN)
    (void) x;
    (void) y;
    (void) width;
    (void) height;
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif
}

void gfx_window__render(gfx_window_t self) {
    gfx_window__set_current(self);

#if defined(OPENGL)
    window__swap_buffers(self->window);
#elif defined(VULKAN)
    vk__render();
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif
}
