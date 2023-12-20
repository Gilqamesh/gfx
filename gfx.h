#ifndef GFX_H
# define GFX_H

# include <stdbool.h>
# include <stdint.h>

# include "glfw.h"

typedef void* (*gfx_symbol_loader_t)(const char*);

bool gfx__init(gfx_symbol_loader_t symbol_loader, window_t window);
void gfx__deinit();

void gfx__viewport(int32_t x, int32_t y, uint32_t width, uint32_t height);

void gfx__render();

#endif // GFX_H
