#include "game.h"

#include "gfx.h"
#include "debug.h"
#include "helper_macros.h"
#include "system.h"

#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"

#include "game_internal.c"

game_t game__create() {
    game_t result = calloc(1, sizeof(*result));

    if (!game__load_images(result)) {
        game__destroy(result);
        return 0;
    }

    // window__set_icon(result->window, result->window_icon_image.data, result->window_icon_image.w, result->window_icon_image.h);

    result->cursor = cursor__create(result->cursor_image.data, result->cursor_image.w, result->cursor_image.h);
    if (!result->cursor) {
        game__destroy(result);
        return 0;
    }
    // window__set_cursor(result->window, result->cursor);

    // todo: game__update tests to measure the upper-bound for game__update_upper_bound?

    return result;
}

void game__destroy(game_t self) {
    if (self->window_icon_image.data) {
        stbi_image_free(self->window_icon_image.data);
    }
    if (self->cursor_image.data) {
        stbi_image_free(self->cursor_image.data);
    }
    if (self->cursor) {
        cursor__destroy(self->cursor);
    }
    free(self);
}

double game__update_upper_bound(game_t self) {
    (void) self;
    
    return 120.0 / 1000000.0;
}

void game__update(game_t self, double s) {

    system__usleep(100);

    (void) self;
    (void) s;
}
