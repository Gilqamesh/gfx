#ifndef GFX_H
# define GFX_H

# include <stdbool.h>
# include <stdint.h>

typedef void* (*gfx_symbol_loader_t)(const char*);

bool gfx__init(gfx_symbol_loader_t symbol_loader);
void gfx__deinit();

void gfx__viewport(int32_t x, int32_t y, uint32_t width, uint32_t height);

#endif // GFX_H
