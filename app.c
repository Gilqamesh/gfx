#include "app.h"

#include "debug.h"
#include "helper_macros.h"
#include "system.h"
#include "glfw.h"
#include "gl.h"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"

#include <string.h>

#include "app_internal.c"

app_t app__create(int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    app_t result = calloc(1, sizeof(*result));
    memset(result, 0, sizeof(*result));

    system__init();

    if (!app__load_images(result)) {
        return false;
    }

    if (!glfw__init()) {
        return false;
    }

    uint32_t number_of_monitors;
    monitor_t* monitors = monitor__get_monitors(&number_of_monitors);
    if (number_of_monitors == 0) {
        glfw__deinit();
        return false;
    }

    result->monitor = monitors[0];
    result->window = window__create(result->monitor, "Title", 100, 300);
    if (!result->window) {
        glfw__deinit();
        return false;
    }

    result->debug_window = window__create(result->monitor, "Debug window", 100, 300);

    window__button_register_action(result->window, BUTTON_FPS_LOCK_INC, (void*) result, app__button_default_action_fps_lock_inc);
    window__button_register_action(result->window, BUTTON_FPS_LOCK_DEC, (void*) result, app__button_default_action_fps_lock_dec);

    window__set_icon(result->window, result->window_icon_image.data, result->window_icon_image.w, result->window_icon_image.h);

    result->cursor = cursor__create(result->cursor_image.data, result->cursor_image.w, result->cursor_image.h);
    window__set_cursor(result->window, result->cursor);

    return result;
}

void app__destroy(app_t self) {
    window__destroy(self->window);
    glfw__deinit();

    if (self->window_icon_image.data) {
        stbi_image_free(self->window_icon_image.data);
    }
}

void app__run(app_t self) {
    debug__set_message_module_availability(DEBUG_MODULE_APP, false);
    debug__set_message_module_availability(DEBUG_MODULE_GLFW, false);

    window__set_current_window(self->debug_window);
    window__set_window_opacity(self->debug_window, 0.89);
    window__destroy(self->debug_window);

    window__set_current_window(self->window);

    const uint32_t min_w = 100;
    const uint32_t min_h = 100;
    const uint32_t max_w = 1000;
    const uint32_t max_h = 1000;
    window__set_size_limit(self->window, min_w, min_h, max_w, max_h);
    window__set_content_area_size(self->window, max_w / 2, max_h / 2);
    // window__set_fullscreen(self->window, true);

    window__set_window_opacity(self->window, 0.89);

    const double target_fps = 60.0;
    self->time_frame_expected = 1.0 / target_fps;
    debug__write_and_flush(DEBUG_MODULE_APP, DEBUG_INFO, "target fps: %lf", target_fps);

    self->time_update_expected     = 1.0 / 100.0;
    self->time_update_actual       = self->time_update_expected;
    self->time_start               = system__get_time();
    self->prev_frame_info.time_end = self->time_start;
    self->time_update_to_process   = 0.0;
    while (!window__get_should_close(self->window)) {
        // app__collect_previous_frame_info(self);
        (void) app__collect_previous_frame_info;

        glfw__poll_events();

        if (window__get_should_close(self->window)) {
            break ;
        }

        app__pre_update(self);

        app__update_loop(self);

        app__render(self);

        const double time_mark_end_frame     = self->time_start + (self->current_frame + 1) * self->time_frame_expected;
        const double time_till_end_of_frame  = time_mark_end_frame - system__get_time();
        if (time_till_end_of_frame > 0.0) {
            system__sleep(time_till_end_of_frame);
        }

        ++self->current_frame;
    }
}
