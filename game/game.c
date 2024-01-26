#include "game.h"

#include "debug.h"
#include "helper_macros.h"
#include "system.h"
#include "file.h"
#include "vec_math.h"

#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"

//! TODO: use gfx.h API instead of this, but currently experimenting with opengl, and idk what the API could look like for vulkan
#include "gl/gl.h"

#include "game_internal.c"

game_t game__create() {
    game_t result = calloc(1, sizeof(*result));
    if (!result) {
        return 0;
    }

    result->position._[2] = -3.0f;

    // todo: game__update tests to measure the upper-bound for game__update_upper_bound?

    debug__write_and_flush(DEBUG_MODULE_GAME, DEBUG_INFO, "loading images...");

    if (!game__load_images(result)) {
        return 0;
    }

    if (!game__load_shaders(result)) {
        return 0;
    }

    if (!game__init_textures(result)) {
        return 0;
    }

    result->cursor = cursor__create(result->cursor_image.data, result->cursor_image.w, result->cursor_image.h);
    if (!result->cursor) {
        return 0;
    }

    debug__write_and_flush(DEBUG_MODULE_GAME, DEBUG_INFO, "loading game objects...");

    if (!game__init_game_objects(result)) {
        return 0;
    }

    debug__write_and_flush(DEBUG_MODULE_GAME, DEBUG_INFO, "finished initializing");

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

void game__frame_start(game_t self) {
    (void) self;
    
    attached_buffer_color__clearfv(0, 0.9f, 0.1f, 0.1f, 1.0f);
}

void game__customize_window(game_t self, window_t window) {
    self->window = window;
    window__set_icon(window, self->window_icon_image.data, self->window_icon_image.w, self->window_icon_image.h);
    window__set_cursor(window, self->cursor);
}

double game__update_upper_bound(game_t self) {
    (void) self;
    
    return 120.0 / 1000000.0;
}

void game__update(game_t self, controller_t* window_controller, double s) {
    self->time += s;

    // todo: because of fixed time update, this is always the same, so can be cached
    const float move_speed_per_second = 1.0f;
    const float move_speed = move_speed_per_second * s;


    if (controller__button_is_down(window_controller, BUTTON_SPACE)) {
        self->position._[1] += move_speed;
    }
    if (controller__button_is_down(window_controller, BUTTON_LCTRL)) {
        self->position._[1] -= move_speed;
    }

    system__usleep(100);
}

void game__render(game_t self, double factor) {
    geometry_object__draw(
        &self->geometry,
        &self->shader,
        vertex_stream_specification(
            PRIMITIVE_TYPE_TRIANGLE,
            // PRIMITIVE_TYPE_POINT,
            3, 0,
            5, 0
        ),
        self
    );
    (void) factor;
}
