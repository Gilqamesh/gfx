typedef struct frame_info {
    double   elapsed_time;
    double   time_update_actual;
    double   time_render_actual;
    double   time_start;
    double   time_end;
    double   time_frame_expected;
    uint32_t number_of_updates;
} frame_info_t;

typedef struct loop_stage {
    double   time_start;
    double   time_elapsed;
    bool     (*loop_stage__execute)(struct loop_stage* self, app_t app);
} loop_stage_t;

struct app {
    game_t game;

    uint32_t   current_frame;
    window_t   window;
    window_t   debug_window;
    monitor_t  monitor;

    uint32_t      loop_stages_top;
    uint32_t      loop_stages_size;
    loop_stage_t* loop_stages;

    double time_lost;
    double frames_lost;

    double time_game_update_fixed;
    double time_update_to_process;

    frame_info_t  previous_frame_info;
    uint32_t      frame_info_sample_index_tail;
    uint32_t      frame_info_sample_index_head;
    uint32_t      frame_info_sample_size;
    frame_info_t* frame_info_sample;
};

static bool loop_stage__collect_previous_frame_info(loop_stage_t* self, app_t app);
static bool loop_stage__pre_update(loop_stage_t* self, app_t app);
static bool loop_stage__render(loop_stage_t* self, app_t app);
static bool loop_stage__update_loop(loop_stage_t* self, app_t app);
static bool loop_stage__sleep_till_end_of_frame(loop_stage_t* self, app_t app);
static void app__button_default_action_fps_lock_inc(void* user_pointer);
static void app__button_default_action_fps_lock_dec(void* user_pointer);
static void app__sample_prev_frame(app_t self);
static void app__push_stage(app_t self, bool (*stage_fn)(struct loop_stage* self, app_t app));

static bool loop_stage__collect_previous_frame_info(loop_stage_t* self, app_t app) {
    app->previous_frame_info.time_start         = app->previous_frame_info.time_end;
    app->previous_frame_info.time_end           = self->time_start;
    app->previous_frame_info.elapsed_time       = app->previous_frame_info.time_end - app->previous_frame_info.time_start;
    app->previous_frame_info.time_render_actual = app->previous_frame_info.time_render_actual;

    const double expected_time_lost_this_frame = app->previous_frame_info.elapsed_time - app->previous_frame_info.time_frame_expected;
    app->time_lost              += expected_time_lost_this_frame;
    app->frames_lost            += expected_time_lost_this_frame / app->previous_frame_info.time_frame_expected;
    app->time_update_to_process += app->previous_frame_info.elapsed_time;

    app__sample_prev_frame(app);

    int64_t seconds_since_loop_start = (int64_t) app->previous_frame_info.time_end;
    static int64_t seconds_last_info_printed;
    if (seconds_since_loop_start > seconds_last_info_printed) {
        seconds_last_info_printed = seconds_since_loop_start;
        double time_frame_actual_avg    = 0.0;
        double time_frame_expected_avg  = 0.0;
        double time_update_actual_avg   = 0.0;
        double time_render_actual_avg   = 0.0;
        double number_of_updates_avg    = 0.0;
        int32_t sample_index = (int32_t) app->frame_info_sample_index_tail;
        uint32_t frame_samples_count = 0;
        while (sample_index != (int32_t) app->frame_info_sample_index_head) {
            time_frame_actual_avg    += app->frame_info_sample[sample_index].elapsed_time;
            time_frame_expected_avg  += app->frame_info_sample[sample_index].time_frame_expected;
            time_update_actual_avg   += app->frame_info_sample[sample_index].time_update_actual;
            time_render_actual_avg   += app->frame_info_sample[sample_index].time_render_actual;
            number_of_updates_avg    += app->frame_info_sample[sample_index].number_of_updates;
            if (sample_index == (int32_t) app->frame_info_sample_size - 1) {
                sample_index = 0;
            } else {
                ++sample_index;
            }
            ++frame_samples_count;
        }
        if (frame_samples_count > 0) {
            time_frame_actual_avg    /= frame_samples_count;
            time_frame_expected_avg  /= frame_samples_count;
            time_update_actual_avg   /= frame_samples_count;
            time_render_actual_avg   /= frame_samples_count;
            number_of_updates_avg    /= frame_samples_count;

            debug__write("Frame #%u", app->current_frame);
            debug__write("Time lost:   %lf", app->time_lost);
            debug__write("Frames lost: %lf", app->frames_lost);
            debug__write("Frame avg info across %u frames", frame_samples_count);
            debug__write("  Game updates:            %lf", number_of_updates_avg);
            debug__write("  Time:");
            debug__write("    Total:                 %lfs", app->previous_frame_info.time_end);
            debug__write("    Game update actual:    %lfus", time_update_actual_avg * 1000000);
            debug__write("    Game update fixed:     %lfus", app->time_game_update_fixed * 1000000);
            debug__write("    Render actual:         %lfus", time_render_actual_avg * 1000000);
            debug__write("    Frame actual:          %lfms, %lffps", time_frame_actual_avg * 1000.0, 1.0 / time_frame_actual_avg);
            debug__write("    Frame expected:        %lfms, %lffps", time_frame_expected_avg * 1000.0, 1.0 / time_frame_expected_avg);
            debug__write("    Left to process:       %lfms", (app->time_update_to_process - app->previous_frame_info.elapsed_time) * 1000.0);
            debug__flush(DEBUG_MODULE_APP, DEBUG_INFO);
        }
    }

    return true;
}

static bool loop_stage__pre_update(loop_stage_t* self, app_t app) {
    (void) self;

    glfw__poll_events();

    if (window__get_should_close(app->window)) {
        return false;
    }

    return true;

    // attached_buffer_color__clearfv((void*)0, 0.5f, 0.3f, 0.5f, 1.0f);
    // attached_buffer_color__clearfv((void*)0, 1.0f, 1.0f, 1.0f, 1.0f);
    // gl__clear_color(0.9f, 0.0f, 0.0f, 1.0f, BUFFER_COLOR);
}

static bool loop_stage__update_loop(loop_stage_t* self, app_t app) {
    app->previous_frame_info.time_update_actual = 0.0;
    app->previous_frame_info.number_of_updates  = 0;
    if (app->time_update_to_process == 0) {
        return true;
    }

    app->previous_frame_info.number_of_updates = app->time_update_to_process / app->time_game_update_fixed;
    app->time_update_to_process -= app->previous_frame_info.number_of_updates * app->time_game_update_fixed;
    for (uint32_t game_updates_count = 0; game_updates_count < app->previous_frame_info.number_of_updates; ++game_updates_count) {
        game__update(app->game, app->time_game_update_fixed);
    }
    double time_end = system__get_time();
    app->previous_frame_info.time_update_actual = (time_end - self->time_start) / app->previous_frame_info.number_of_updates;

    return true;
}

static bool loop_stage__render(loop_stage_t* self, app_t app) {
    double how_far_in_frame_are_we_at_the_point_of_rendering = app->time_update_to_process / app->time_game_update_fixed;
    (void) how_far_in_frame_are_we_at_the_point_of_rendering;
    // todo: it can be > 1.0, because we earlied out from update loop, so just take the fractional part and interpolate with that
    // ASSERT(how_far_in_frame_are_we_at_the_point_of_rendering >= 0.0 && how_far_in_frame_are_we_at_the_point_of_rendering <= 1.0);

    // interpolate between previous and current physics state based on 'how_far_in_frame_are_we_at_the_point_of_rendering',
    // source: https://gafferongames.com/post/fix_your_timestep/

    gfx__render();
    system__usleep(15000);

    // window__swap_buffers(app->window);

    double time_render_end = system__get_time();

    app->previous_frame_info.time_render_actual = time_render_end - self->time_start;
    return true;
}

static bool loop_stage__sleep_till_end_of_frame(loop_stage_t* self, app_t app) {
    const double time_mark_end_frame     = app->loop_stages[0].time_start + app->previous_frame_info.time_frame_expected;
    const double time_till_end_of_frame  = time_mark_end_frame - self->time_start;
    if (time_till_end_of_frame > 0.0) {
        system__sleep(time_till_end_of_frame);
    }

    ++app->current_frame;

    return true;
}

static void app__button_default_action_fps_lock_inc(void* user_pointer) {
    app_t app = (app_t) user_pointer;

    const double current_fps = 1.0 / app->previous_frame_info.time_frame_expected;
    const double new_fps     = current_fps + 1.0;
    app->previous_frame_info.time_frame_expected = 1.0 / new_fps;
    app->frame_info_sample_index_head = 0;
    app->frame_info_sample_index_head = app->frame_info_sample_index_tail;

    debug__write_and_flush(DEBUG_MODULE_APP, DEBUG_INFO, "changed: fps - %lf -> %lf", current_fps, new_fps);
}

static void app__button_default_action_fps_lock_dec(void* user_pointer) {
    app_t app = (app_t) user_pointer;

    const double current_fps = 1.0 / app->previous_frame_info.time_frame_expected;
    if (current_fps <= 1.5) {
        return ;
    }

    const double new_fps = current_fps - 1.0;
    app->previous_frame_info.time_frame_expected = 1.0 / new_fps;
    app->frame_info_sample_index_head = 0;
    app->frame_info_sample_index_head = app->frame_info_sample_index_tail;

    debug__write_and_flush(DEBUG_MODULE_APP, DEBUG_INFO, "changed: fps - %lf -> %lf", current_fps, new_fps);
}

static void app__sample_prev_frame(app_t self) {
    ASSERT(self->frame_info_sample_index_head < self->frame_info_sample_size);
    frame_info_t* frame_info_sample = &self->frame_info_sample[self->frame_info_sample_index_head++];
    memcpy(frame_info_sample, &self->previous_frame_info, sizeof(self->previous_frame_info));
    if (self->frame_info_sample_index_head == self->frame_info_sample_size) {
        self->frame_info_sample_index_head = 0;
    }
    if (self->frame_info_sample_index_head == self->frame_info_sample_index_tail) {
        ++self->frame_info_sample_index_tail;
        if (self->frame_info_sample_index_tail == self->frame_info_sample_size) {
            self->frame_info_sample_index_tail = 0;
        }
    }
    ASSERT(self->frame_info_sample_index_tail != self->frame_info_sample_index_head);

}

static void app__push_stage(app_t self, bool (*loop_stage__execute)(struct loop_stage* self, app_t app)) {
    ARRAY_ENSURE_TOP(self->loop_stages, self->loop_stages_top, self->loop_stages_size);
    self->loop_stages[self->loop_stages_top].loop_stage__execute = loop_stage__execute;
    ++self->loop_stages_top;
}
