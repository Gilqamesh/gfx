#ifndef GFX_H
# define GFX_H

# include <stdbool.h>
# include <stdint.h>

/********************************************************************************
 * Module API
 ********************************************************************************/

bool gfx__init();
void gfx__deinit();

/********************************************************************************
 * Window API
 ********************************************************************************/

struct         gfx_window;
typedef struct gfx_window* gfx_window_t;

gfx_window_t gfx_window__create(const char* title, uint32_t width, uint32_t height);
void gfx_window__destroy(gfx_window_t self);

/**
 * @brief Defines the transformation from normalized device coordinates to window coordinates
*/
void gfx_window__set_viewport(gfx_window_t self, int32_t x, int32_t y, uint32_t width, uint32_t height);

/**
 * @brief Defines the scissor box in window coordinates
*/
void gfx_window__set_scissor(gfx_window_t self, int32_t x, int32_t y, uint32_t width, uint32_t height);

void gfx_window__render(gfx_window_t self);

#endif // GFX_H
