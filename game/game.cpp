#include "game.h"

#include "debug.h"
#include "helper_macros.h"
#include "system.h"
#include "file.h"

#include <stdlib.h>

#define GLM_ENABLE_EXPERIMENTAL
#include "third_party/glm/glm/glm.hpp"
#include "third_party/glm/glm/gtc/matrix_transform.hpp"
#include "third_party/glm/glm/gtc/type_ptr.hpp"
#include "third_party/glm/glm/gtc/quaternion.hpp"
#include "third_party/glm/glm/gtx/quaternion.hpp"
#include "third_party/glm/glm/gtx/rotate_vector.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"

//! TODO: use gfx.h API instead of this, but currently experimenting with opengl, and idk what the API could look like for vulkan
#include "gl/gl.h"

#include "game_internal.cpp"

game_t game__create(window_t window) {
    debug__set_message_type_availability(DEBUG_MODULE_GFX, DEBUG_INFO, false);
    debug__set_message_type_availability(DEBUG_MODULE_GAME_CLIENT, DEBUG_INFO, false);

    game_t result = (game_t) calloc(1, sizeof(*result));
    if (!result) {
        return 0;
    }

    result->orientation = glm::vec3(0.0f, 0.0f, -1.0f);
    result->position[2] = 3.0f;
    result->up = glm::vec3(0.0f, 1.0f, 0.0f);
    result->window = window;

    uint32_t number_of_controllers = 0;
    controller_t* controllers = gfx__get_controllers(&number_of_controllers);
    ASSERT(number_of_controllers);
    result->controller = controllers[0];

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

    window__set_icon(result->window, result->window_icon_image.data, result->window_icon_image.w, result->window_icon_image.h);
    window__set_cursor(result->window, result->cursor);

    controller__get_cursor_pos(window__get_controller(result->window), &result->p_prev_cursorx, &result->p_prev_cursory);
    controller__get_cursor_pos(window__get_controller(result->window), &result->p_cur_cursorx, &result->p_cur_cursory);

    window__set_cursor_state(result->window, CURSOR_DISABLED);

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
    self->p_prev_cursorx = self->p_cur_cursorx;
    self->p_prev_cursory = self->p_cur_cursory;
    controller__get_cursor_pos(window__get_controller(self->window), &self->p_cur_cursorx, &self->p_cur_cursory);
    
    // attached_buffer_depth__clearfv(0, 1.0f, 1.0f, 1.0f, 1.0f);
    attached_buffer_color__clearfv(0, 0.9f, 0.1f, 0.1f, 1.0f);
}

double game__update_upper_bound(game_t self) {
    (void) self;
    
    return 120.0 / 1000000.0;
}

void game__update(game_t self, double s) {
    self->time += s;

    // todo: because of fixed time update, this is always the same, so can be cached
    const float move_speed_per_second = 5.0f;
    const float move_speed = move_speed_per_second * s;

    glm::vec3 side = glm::normalize(glm::cross(self->up, self->orientation));
    glm::vec3 up = glm::normalize(glm::cross(self->orientation, side));

    const float roll_speed_per_second = 2.0f;
    const float roll_speed = roll_speed_per_second * s;

    uint32_t window_width;
    uint32_t window_height;
    window__get_windowed_state_content_area(self->window, 0, 0, &window_width, &window_height);

    if (controller__is_connected(self->controller)) {
        controller_t controller = self->controller;
        if (controller__button_is_down(controller, BUTTON_GAMEPAD_AXIS_RIGHT_TRIGGER)) {
            self->position += controller__button_value(controller, BUTTON_GAMEPAD_AXIS_RIGHT_TRIGGER) * move_speed * self->up;
        }
        if (controller__button_is_down(controller, BUTTON_GAMEPAD_AXIS_LEFT_TRIGGER)) {
            self->position -= controller__button_value(controller, BUTTON_GAMEPAD_AXIS_LEFT_TRIGGER) * move_speed * self->up;
        }
        if (controller__button_is_down(controller, BUTTON_GAMEPAD_AXIS_LEFT_Y)) {
            self->position -= controller__button_value(controller, BUTTON_GAMEPAD_AXIS_LEFT_Y) * move_speed * self->orientation;
        }
        if (controller__button_is_down(controller, BUTTON_GAMEPAD_AXIS_LEFT_X)) {
            self->position -= controller__button_value(controller, BUTTON_GAMEPAD_AXIS_LEFT_X) * move_speed * side;
        }

        if (controller__button_is_down(controller, BUTTON_GAMEPAD_AXIS_RIGHT_Y)) {
            const float pitch_per_second = (float) controller__button_value(controller, BUTTON_GAMEPAD_AXIS_RIGHT_Y);
            const float pitch = 2.5f * pitch_per_second * s;
            const glm::vec3 new_orientation = glm::normalize(glm::rotate(self->orientation, pitch, side));
            if (fabs(glm::dot(self->up, new_orientation)) < 0.9f) {
                self->orientation = new_orientation;
            }
        }
        if (controller__button_is_down(controller, BUTTON_GAMEPAD_AXIS_RIGHT_X)) {
            const float jaw_per_second = (float) controller__button_value(controller, BUTTON_GAMEPAD_AXIS_RIGHT_X);
            const float jaw = -2.5f * jaw_per_second * s;
            self->orientation = glm::normalize(glm::rotate(self->orientation, jaw, up));
        }


        if (controller__button_is_down(controller, BUTTON_GAMEPAD_LEFT_BUMPER)) {
            self->up = glm::normalize(glm::rotate(self->up, -roll_speed, self->orientation));
        }
        if (controller__button_is_down(controller, BUTTON_GAMEPAD_RIGHT_BUMPER)) {
            self->up = glm::normalize(glm::rotate(self->up, roll_speed, self->orientation));
        }
    }

    if (controller__is_connected(window__get_controller(self->window))) {
        controller_t controller = window__get_controller(self->window);
        if (controller__button_is_down(controller, BUTTON_SPACE)) {
            self->position += move_speed * self->up;
        }
        if (controller__button_is_down(controller, BUTTON_LCTRL)) {
            self->position -= move_speed * self->up;
        }
        if (controller__button_is_down(controller, BUTTON_A)) {
            self->position += move_speed * side;
        }
        if (controller__button_is_down(controller, BUTTON_D)) {
            self->position -= move_speed * side;
        }
        if (controller__button_is_down(controller, BUTTON_S)) {
            self->position -= move_speed * self->orientation;
        }
        if (controller__button_is_down(controller, BUTTON_W)) {
            self->position += move_speed * self->orientation;
        }

        int32_t dpcursorx = self->p_cur_cursorx - self->p_prev_cursorx;
        int32_t dpcursory = self->p_cur_cursory - self->p_prev_cursory;
        const float jaw_per_second = (float) -dpcursorx;
        const float pitch_per_second = (float) dpcursory;
        const float jaw = jaw_per_second * s;
        const float pitch = pitch_per_second * s;

        if (controller__button_is_down(controller, BUTTON_Q)) {
            self->up = glm::normalize(glm::rotate(self->up, -roll_speed, self->orientation));
        }
        if (controller__button_is_down(controller, BUTTON_E)) {
            self->up = glm::normalize(glm::rotate(self->up, roll_speed, self->orientation));
        }

        if (jaw) {
            self->orientation = glm::normalize(glm::rotate(self->orientation, jaw, up));
        }
        if (pitch) {
            const glm::vec3 new_orientation = glm::normalize(glm::rotate(self->orientation, pitch, side));
            if (fabs(glm::dot(self->up, new_orientation)) < 0.9f) {
                self->orientation = new_orientation;
            }
        }
    }

    self->player_vp =
        glm::perspective(glm::radians(60.0f), (float) window_width / (float) window_height, 0.01f, 100.0f) *
        glm::lookAt(self->position, self->position + self->orientation, self->up);

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
