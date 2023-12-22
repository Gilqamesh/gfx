typedef struct gfx {
    gfx_window_t current_gfx_window;
} gfx_t;

static gfx_t gfx;

struct gfx_window {
    window_t window;
};

static void gfx_window__set_current(gfx_window_t self) {
    if (gfx.current_gfx_window != self) {
        window__set_current_window(self->window);
        gfx.current_gfx_window = self->window;
    }
}
