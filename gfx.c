#include "gfx.h"

# if defined(OPENGL)
#  include "gl.h"
# elif defined(VULKAN)
#  include "vulkan.h"
# else
#  error "undefined backend, must either be OPENGL or VULKAN"
# endif

# include "gfx_math.h"

bool gfx__init(gfx_symbol_loader_t symbol_loader, window_t window) {
#if defined(OPENGL)
    (void) window;
    return gl__init(symbol_loader);
#elif defined(VULKAN)
    (void) symbol_loader;

    return vk__init(window);
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif
}

void gfx__deinit() {
#if defined(OPENGL)
    gl__deinit();
#elif defined(VULKAN)
    vk__deinit();
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif
}

void gfx__viewport(int32_t x, int32_t y, uint32_t width, uint32_t height) {
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

void gfx__render() {
#if defined(OPENGL)
#elif defined(VULKAN)
    vk__render();
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif
}
