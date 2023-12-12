#include "app.h"

#include "debug.h"
#include "helper_macros.h"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"

#include <string.h>

// @param how_far_in_frame_are_we_at_the_point_of_rendering 0.0 (right at the previous frame), 1.0 (right at the next frame)
static void render(double how_far_in_frame_are_we_at_the_point_of_rendering) {
    (void) how_far_in_frame_are_we_at_the_point_of_rendering;

    // interpolate between previous and current physics state based on 'how_far_in_frame_are_we_at_the_point_of_rendering',
    // source: https://gafferongames.com/post/fix_your_timestep/
}

static void app__update(app_t* self) {
    (void) self;

    // const char* first_col_name = "Frame";
    // const char* second_col_name = "Button               ";
    // const char* third_col_name = "Transitions";
    // const char* forth_col_name = "Repeats";
    // const char* fifth_col_name = "State    ";
    // const uint32_t first_col_len = strlen(first_col_name);
    // const uint32_t second_col_len = strlen(second_col_name);
    // const uint32_t third_col_len = strlen(third_col_name);
    // const uint32_t forth_col_len = strlen(forth_col_name);
    // const uint32_t fifth_col_len = strlen(fifth_col_name);
    // const uint32_t col_pad = 4;

    // if (window__received_button_input(self->window)) {
    //     debug__write(
    //         "%-*.*s%*c"
    //         "%-*.*s%*c"
    //         "%-*.*s%*c"
    //         "%-*.*s%*c"
    //         "%-*.*s",
    //         first_col_len, first_col_len, first_col_name, col_pad, ' ',
    //         second_col_len, second_col_len, second_col_name, col_pad, ' ',
    //         third_col_len, third_col_len, third_col_name, col_pad, ' ',
    //         forth_col_len, forth_col_len, forth_col_name, col_pad, ' ',
    //         fifth_col_len, fifth_col_len, fifth_col_name
    //     );
    //     for (uint32_t i = 0; i < sizeof(self->window->buttons) / sizeof(self->window->buttons[0]); ++i) {
    //         uint32_t n_of_transitions = window__button_n_of_transitions(self->window, i);
    //         uint32_t n_of_repeats = window__button_n_of_repeats(self->window, i);
    //         if (n_of_transitions > 0 || n_of_repeats > 0) {
    //             debug__write(
    //                 "%-*u%*c"
    //                 "%-*.*s%*c"
    //                 "%-*u%*c"
    //                 "%-*u%*c"
    //                 "%-.*s",
    //                 first_col_len, self->frame_count, col_pad, ' ',
    //                 second_col_len, second_col_len, button__to_str(i), col_pad, ' ',
    //                 third_col_len, n_of_transitions, col_pad, ' ',
    //                 forth_col_len, n_of_repeats, col_pad, ' ',
    //                 fifth_col_len, window__button_is_down(self->window, i) ? "Pressed" : "Released"
    //             );
    //         }
    //     }
    //     debug__flush(DEBUG_MODULE_APP, DEBUG_INFO);
    // }
}

bool app__create(app_t* self, int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    memset(self, 0, sizeof(*self));

    int number_of_channels_per_pixel;
    self->window_icon.data = stbi_load("assets/icon.png", (int32_t*) &self->window_icon.w, (int32_t*) &self->window_icon.h, &number_of_channels_per_pixel, 0);
    if (!self->window_icon.data) {
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

    self->monitor = monitors[0];
    self->window = window__create(self->monitor, "Title", 100, 300);
    if (!self->window) {
        glfw__deinit();
        return false;
    }

    window__set_icon(self->window, self->window_icon.data, self->window_icon.w, self->window_icon.h);

    return true;
}

void app__destroy(app_t* self) {
    window__destroy(self->window);
    glfw__deinit();

    if (self->window_icon.data) {
        stbi_image_free(self->window_icon.data);
    }
}

void app__run(app_t* self) {
    window__set_current_window(self->window);

    const uint32_t min_w = 100;
    const uint32_t min_h = 100;
    const uint32_t max_w = 1000;
    const uint32_t max_h = 1000;
    window__set_size_limit(self->window, min_w, min_h, max_w, max_h);
    window__set_content_area_size(self->window, max_w / 2, max_h / 2);
    // window__set_fullscreen(self->window, true);

    window__set_window_opacity(self->window, 0.89);

    uint32_t time_samples_top = 0;
    double time_samples[30] = { 0 };

    const double target_fps = 500.0;
    debug__write_and_flush(DEBUG_MODULE_APP, DEBUG_INFO, "target fps: %lf", target_fps);

    double time_to_process = 0.0;

    // todo: measure this on the current machine
    const double game_seconds_to_update = 1 / (target_fps + 50.0);
    // note: add this, otherwise the game time can't catch up to the real time
    const double game_seconds_per_update_epsilon = 0.00005;
    const double game_seconds_per_update = game_seconds_to_update + game_seconds_per_update_epsilon;

    double time_previous = glfw__get_time_s();
    while (!window__get_should_close(self->window)) {
        double time_start = glfw__get_time_s();
        double time_elapsed = time_start - time_previous;
        time_previous = time_start;
        time_to_process += time_elapsed;

        time_samples[time_samples_top++] = time_elapsed;
        if (time_samples_top == ARRAY_SIZE(time_samples)) {
            time_samples_top = 0;
            double time_samples_average = 0.0;
            for (uint32_t time_samples_index = 0; time_samples_index < ARRAY_SIZE(time_samples); ++time_samples_index) {
                time_samples_average += time_samples[time_samples_index];
            }
            time_samples_average /= ARRAY_SIZE(time_samples);
            printf("Average frame time across %lu frames: %lfms, equates to: %lffps\n", ARRAY_SIZE(time_samples), time_samples_average * 1000.0, 1.0 / time_samples_average);
        }

        glfw__poll_events();

        while (time_to_process >= game_seconds_per_update) {
            app__update(self);
            time_to_process -= game_seconds_per_update;
        }

        render(time_to_process / game_seconds_per_update);

        window__swap_buffers(self->window);

        double time_end = glfw__get_time_s();
        while (target_fps * (time_end - time_start) < 1.0) {
            // usleep(0);
            time_end = glfw__get_time_s();
        }

        ++self->frame_count;
    }
}
