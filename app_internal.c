typedef struct image {
    uint8_t* data;
    uint32_t w;
    uint32_t h;
} image_t;

typedef struct frame_sample {
    double   elapsed_time;
    double   delay;
    uint32_t number_of_updates;
    double   time_render_actual;
    double   time_update_actual;
} frame_sample_t;

typedef struct frame_info {
    uint32_t        frame_sample_top;
    uint32_t        frame_sample_size;
    frame_sample_t* frame_sample;

    double   time_start;
    double   time_end;
} frame_info_t;

struct app {
    image_t   window_icon_image;

    cursor_t  cursor;
    image_t   cursor_image;

    uint32_t  current_frame;
    window_t  window;
    window_t  debug_window;
    monitor_t monitor;

    double time_frame_expected;
    double time_update_to_process;
    double time_update_expected;
    double time_update_actual;
    double time_render_actual;
    double time_start;

    frame_info_t prev_frame_info;
};

static void app__pre_update(app_t self);
static void app__update_loop(app_t self);
static void app__update(app_t self);
static void app__render(app_t self);
static void array__ensure_top(void** array, uint32_t array_top, uint32_t* array_size, uint32_t element_size);
static void app__button_default_action_fps_lock_inc(void* user_pointer);
static void app__button_default_action_fps_lock_dec(void* user_pointer);
static bool app__load_images(app_t self);
static void app__collect_previous_frame_info(app_t self);

static void app__pre_update(app_t self) {
    (void) self;

    attached_buffer_color__clearfv(
        (void*)0,
        sin(system__get_time()) * 0.5f + 0.5f,
        cos(system__get_time()) * 0.5f + 0.5f,
        cos(system__get_time() + 0.25f) * 0.5f + 0.5f,
        1.0f
    );
    // gl__clear_color(0.9f, 0.0f, 0.0f, 1.0f, BUFFER_COLOR);
}

static void app__update_loop(app_t self) {
    while (self->time_update_expected <= self->time_update_actual) {
        const double new_time_update_expected = self->time_update_expected * 1.5;
        debug__write_and_flush(
            DEBUG_MODULE_APP, DEBUG_INFO,
            "adjusted expected updates: %lf -> %lf [updates / s]",
            1.0 / self->time_update_expected, 1.0 / new_time_update_expected
        );
        self->time_update_expected = new_time_update_expected;
    }
    ASSERT(self->time_update_actual < self->time_update_expected);

    array__ensure_top((void**) &self->prev_frame_info.frame_sample, self->prev_frame_info.frame_sample_top, &self->prev_frame_info.frame_sample_size, sizeof(self->prev_frame_info.frame_sample[0]));
    self->prev_frame_info.frame_sample[self->prev_frame_info.frame_sample_top].number_of_updates = 0;
    while (self->time_update_to_process >= self->time_update_expected) {
        app__update(self);
        self->time_update_to_process -= self->time_update_expected;
    }
}

static void app__update(app_t self) {

    double time_start = system__get_time();

    ++self->prev_frame_info.frame_sample[self->prev_frame_info.frame_sample_top].number_of_updates;

    // simulate update
    system__usleep(2230.0);

    double time_end = system__get_time();

    self->time_update_actual = time_end - time_start;

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
    //                 first_col_len, self->current_frame, col_pad, ' ',
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

static void app__render(app_t self) {
    double time_render_start = system__get_time();

    double how_far_in_frame_are_we_at_the_point_of_rendering = self->time_update_to_process / self->time_update_expected;
    (void) how_far_in_frame_are_we_at_the_point_of_rendering;
    // todo: it can be > 1.0, because we earlied out from update loop, so just take the fractional part and interpolate with that
    // ASSERT(how_far_in_frame_are_we_at_the_point_of_rendering >= 0.0 && how_far_in_frame_are_we_at_the_point_of_rendering <= 1.0);

    // interpolate between previous and current physics state based on 'how_far_in_frame_are_we_at_the_point_of_rendering',
    // source: https://gafferongames.com/post/fix_your_timestep/

    // simulate render
    system__usleep(5000);

    window__swap_buffers(self->window);

    double time_render_end = system__get_time();

    self->time_render_actual = time_render_end - time_render_start;
}

static void array__ensure_top(void** array, uint32_t array_top, uint32_t* array_size, uint32_t element_size) {
    if (array_top == *array_size) {
        if (*array_size == 0) {
            *array_size = 8;
            *array = malloc(*array_size * element_size);
        } else {
            *array_size <<= 1;
            *array = realloc(*array, *array_size * element_size);
        }
    }
    ASSERT(array_top < *array_size);
}

static void app__button_default_action_fps_lock_inc(void* user_pointer) {
    app_t app = (app_t) user_pointer;

    const double current_fps = 1.0 / app->time_frame_expected;
    const double new_fps     = current_fps + 1.0;
    app->time_frame_expected = 1.0 / new_fps;

    debug__write_and_flush(DEBUG_MODULE_APP, DEBUG_INFO, "changed: fps - %lf -> %lf", current_fps, new_fps);
}

static void app__button_default_action_fps_lock_dec(void* user_pointer) {
    app_t app = (app_t) user_pointer;

    const double current_fps = 1.0 / app->time_frame_expected;
    if (current_fps <= 1.0) {
        return ;
    }

    const double new_fps     = current_fps - 1.0;
    app->time_frame_expected = 1.0 / new_fps;

    debug__write_and_flush(DEBUG_MODULE_APP, DEBUG_INFO, "changed: fps - %lf -> %lf", current_fps, new_fps);
}

static bool app__load_images(app_t self) {
    int number_of_channels_per_pixel;
    self->window_icon_image.data = stbi_load("assets/icon.png", (int32_t*) &self->window_icon_image.w, (int32_t*) &self->window_icon_image.h, &number_of_channels_per_pixel, 0);
    if (!self->window_icon_image.data) {
        return false;
    }
    self->cursor_image.data = stbi_load("assets/cursor.png", (int32_t*) &self->cursor_image.w, (int32_t*) &self->cursor_image.h, &number_of_channels_per_pixel, 0);
    if (!self->cursor_image.data) {
        return false;
    }

    return true;
}

static void app__collect_previous_frame_info(app_t self) {
    self->prev_frame_info.time_start = self->prev_frame_info.time_end;
    self->prev_frame_info.time_end   = system__get_time();
    self->time_update_to_process     += self->prev_frame_info.time_end - self->prev_frame_info.time_start;
    
    const double time_mark_cur_frame        = self->time_start + self->current_frame * self->time_frame_expected;
    const double expected_total_time_passed = time_mark_cur_frame - self->time_start;
    const double actual_total_time_passed   = self->prev_frame_info.time_end - self->time_start;
    const double total_delay                = actual_total_time_passed - expected_total_time_passed;

    array__ensure_top((void**) &self->prev_frame_info.frame_sample, self->prev_frame_info.frame_sample_top, &self->prev_frame_info.frame_sample_size, sizeof(self->prev_frame_info.frame_sample[0]));
    self->prev_frame_info.frame_sample[self->prev_frame_info.frame_sample_top].delay        = (self->prev_frame_info.time_end - self->prev_frame_info.time_start) - self->time_frame_expected;
    self->prev_frame_info.frame_sample[self->prev_frame_info.frame_sample_top].elapsed_time = self->prev_frame_info.time_end - self->prev_frame_info.time_start;
    self->prev_frame_info.frame_sample[self->prev_frame_info.frame_sample_top].time_render_actual = self->time_render_actual;
    self->prev_frame_info.frame_sample[self->prev_frame_info.frame_sample_top].time_update_actual = self->time_update_actual;
    ++self->prev_frame_info.frame_sample_top;

    if (
        ((1.0 / self->time_frame_expected) > 60 && self->current_frame % 100 == 0) ||
        (self->current_frame % (uint32_t) (1.0 / self->time_frame_expected) == 0)
    ) {
        double delay_avg              = 0.0;
        double elapsed_time_avg       = 0.0;
        double number_of_updates_avg  = 0.0;
        double time_update_actual_avg = 0.0;
        double time_render_actual_avg = 0.0;
        for (uint32_t sample_index = 0; sample_index < self->prev_frame_info.frame_sample_top; ++sample_index) {
            delay_avg              += self->prev_frame_info.frame_sample[sample_index].delay;
            elapsed_time_avg       += self->prev_frame_info.frame_sample[sample_index].elapsed_time;
            number_of_updates_avg  += self->prev_frame_info.frame_sample[sample_index].number_of_updates;
            time_update_actual_avg += self->prev_frame_info.frame_sample[sample_index].time_update_actual;
            time_render_actual_avg += self->prev_frame_info.frame_sample[sample_index].time_render_actual;
        }
        delay_avg              /= self->prev_frame_info.frame_sample_top;
        elapsed_time_avg       /= self->prev_frame_info.frame_sample_top;
        number_of_updates_avg  /= self->prev_frame_info.frame_sample_top;
        time_update_actual_avg /= self->prev_frame_info.frame_sample_top;
        time_render_actual_avg /= self->prev_frame_info.frame_sample_top;

        debug__write("Frame #%u", self->current_frame);
        debug__write("Frame info across %u frames", self->prev_frame_info.frame_sample_top);
        debug__write("Number of updates:   %lf", number_of_updates_avg);
        debug__write("Time update actual:  %lfus", time_update_actual_avg * 1000000);
        debug__write("Time render actual:  %lfus", time_render_actual_avg * 1000000);
        debug__write("Actual frame time:   %lfms, %lffps", elapsed_time_avg * 1000.0, 1.0 / elapsed_time_avg);
        debug__write("Expected frame time: %lfms, %lffps", self->time_frame_expected * 1000.0, 1.0 / self->time_frame_expected);
        debug__write("Time to process:     %lfms", self->time_update_to_process * 1000.0);
        debug__write("Expected total time: %lfus", expected_total_time_passed * 1000000.0);
        debug__write("Actual total time:   %lfus", actual_total_time_passed   * 1000000.0);
        debug__write("Total delay:         %lfus", total_delay * 1000000.0);
        debug__write("Avg delay:           %lfus", delay_avg * 1000000.0);
        debug__write("Lost frames:         %lf", actual_total_time_passed / self->time_frame_expected - self->current_frame);
        debug__flush(DEBUG_MODULE_APP, DEBUG_INFO);

        self->prev_frame_info.frame_sample_top = 0;
    }
}
