struct         loop_stage;
struct         frame_info;
typedef struct loop_stage loop_stage_t;
typedef struct frame_info frame_info_t;

struct frame_info {
    double   elapsed_time;
    double   time_update_actual;
    double   time_render_actual;
    double   time_start;
    double   time_end;
    double   time_frame_expected;
    uint32_t number_of_updates;
};

struct loop_stage {
    double   time_start;
    double   time_elapsed;
    bool     (*loop_stage__execute)(struct loop_stage* self, game_client_t game_client);
};

struct game_client {
    udp_socket_t   udp_socket;

    network_addr_t server_addr;
    uint32_t       sequence_id;

    game_t         game_state;

    uint32_t       current_frame;
    double         time_lost;
    double         frames_lost;
    double         time_game_update_fixed;
    double         time_update_to_process;
    uint32_t       loop_stages_top;
    uint32_t       loop_stages_size;
    loop_stage_t*  loop_stages;
    frame_info_t   previous_frame_info;
    uint32_t       frame_info_sample_index_tail;
    uint32_t       frame_info_sample_index_head;
    uint32_t       frame_info_sample_size;
    frame_info_t*  frame_info_sample;
};

static bool loop_stage__collect_previous_frame_info(loop_stage_t* self, game_client_t game_client);
static bool loop_stage__poll_inputs(loop_stage_t* self, game_client_t game_client);
static bool loop_stage__update_loop(loop_stage_t* self, game_client_t game_client);
static bool loop_stage__sleep_till_end_of_frame(loop_stage_t* self, game_client_t game_client);
static void game_client__sample_prev_frame(game_client_t self);
static void game_client__push_stage(game_client_t self, bool (*stage_fn)(struct loop_stage* self, game_client_t game_client));

static bool loop_stage__collect_previous_frame_info(loop_stage_t* self, game_client_t game_client) {
    game_client->previous_frame_info.time_start         = game_client->previous_frame_info.time_end;
    game_client->previous_frame_info.time_end           = self->time_start;
    game_client->previous_frame_info.elapsed_time       = game_client->previous_frame_info.time_end - game_client->previous_frame_info.time_start;
    game_client->previous_frame_info.time_render_actual = game_client->previous_frame_info.time_render_actual;

    const double expected_time_lost_this_frame = game_client->previous_frame_info.elapsed_time - game_client->previous_frame_info.time_frame_expected;
    game_client->time_lost              += expected_time_lost_this_frame;
    game_client->frames_lost            += expected_time_lost_this_frame / game_client->previous_frame_info.time_frame_expected;
    game_client->time_update_to_process += game_client->previous_frame_info.elapsed_time;

    game_client__sample_prev_frame(game_client);

    int64_t seconds_since_loop_start = (int64_t) game_client->previous_frame_info.time_end;
    static int64_t seconds_last_info_printed;
    (void) seconds_last_info_printed;
    // if (seconds_since_loop_start > seconds_last_info_printed) {
    if (0) {
        seconds_last_info_printed = seconds_since_loop_start;
        double time_frame_actual_avg    = 0.0;
        double time_frame_expected_avg  = 0.0;
        double time_update_actual_avg   = 0.0;
        double time_render_actual_avg   = 0.0;
        double number_of_updates_avg    = 0.0;
        int32_t sample_index = (int32_t) game_client->frame_info_sample_index_tail;
        uint32_t frame_samples_count = 0;
        while (sample_index != (int32_t) game_client->frame_info_sample_index_head) {
            time_frame_actual_avg    += game_client->frame_info_sample[sample_index].elapsed_time;
            time_frame_expected_avg  += game_client->frame_info_sample[sample_index].time_frame_expected;
            time_update_actual_avg   += game_client->frame_info_sample[sample_index].time_update_actual;
            time_render_actual_avg   += game_client->frame_info_sample[sample_index].time_render_actual;
            number_of_updates_avg    += game_client->frame_info_sample[sample_index].number_of_updates;
            if (sample_index == (int32_t) game_client->frame_info_sample_size - 1) {
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

            debug__write("Frame #%u", game_client->current_frame);
            debug__write("Time lost:   %lf", game_client->time_lost);
            debug__write("Frames lost: %lf", game_client->frames_lost);
            debug__write("Frame avg info across %u frames", frame_samples_count);
            debug__write("  Game updates:            %lf", number_of_updates_avg);
            debug__write("  Time:");
            debug__write("    Total:                 %lfs", game_client->previous_frame_info.time_end);
            debug__write("    Game update actual:    %lfus", time_update_actual_avg * 1000000);
            debug__write("    Game update fixed:     %lfus", game_client->time_game_update_fixed * 1000000);
            debug__write("    Render actual:         %lfus", time_render_actual_avg * 1000000);
            debug__write("    Frame actual:          %lfms, %lffps", time_frame_actual_avg * 1000.0, 1.0 / time_frame_actual_avg);
            debug__write("    Frame expected:        %lfms, %lffps", time_frame_expected_avg * 1000.0, 1.0 / time_frame_expected_avg);
            debug__write("    Left to process:       %lfms", (game_client->time_update_to_process - game_client->previous_frame_info.elapsed_time) * 1000.0);
            debug__flush(DEBUG_MODULE_GAME_CLIENT, DEBUG_INFO);
        }
    }

    return true;
}

static void game_client__receive_packet(game_client_t self) {
    packet_t packet;
    uint32_t received_data_len = 0;
    network_addr_t sender_addr;
    if (udp_socket__get_data(&self->udp_socket, &packet, sizeof(packet), &received_data_len, &sender_addr)) {
        if (received_data_len == sizeof(packet)) {
            if (
                sender_addr.addr == self->server_addr.addr &&
                sender_addr.port == self->server_addr.port
            ) {
                // received packet is from the server
                debug__write_and_flush(
                    DEBUG_MODULE_GAME_SERVER, DEBUG_INFO,
                    "received packet from server, remote seq id: %u, local seq id: %u",
                    packet.sequence_id, self->sequence_id
                );
                // update local sequence id to the server
                self->sequence_id = packet.sequence_id;
            } else {
                // packet is not from server -> discard packet
                debug__write_and_flush(
                    DEBUG_MODULE_GAME_SERVER, DEBUG_INFO,
                    "discarded packet as it is not from the server, received from: <todo with inet_ntop>"
                );
            }
        } else {
            debug__write_and_flush(
                DEBUG_MODULE_GAME_CLIENT, DEBUG_INFO,
                "unknown packet size received: %u, expected: %u",
                received_data_len, sizeof(packet)
            );
        }
    }
}

static void game_client__send_packet(game_client_t self) {
    packet_t packet = {
        .sequence_id = self->sequence_id
    };
    udp_socket__send_data(&self->udp_socket, &packet, sizeof(packet));
}

static bool loop_stage__poll_inputs(loop_stage_t* self, game_client_t game_client) {
    (void) self;

    game_client__receive_packet(game_client);
    game_client__send_packet(game_client);

    ++game_client->sequence_id;

    return true;
}

static bool loop_stage__update_loop(loop_stage_t* self, game_client_t game_client) {
    game_client->previous_frame_info.time_update_actual = 0.0;
    game_client->previous_frame_info.number_of_updates  = 0;
    if (game_client->time_update_to_process == 0) {
        return true;
    }

    game_client->previous_frame_info.number_of_updates = game_client->time_update_to_process / game_client->time_game_update_fixed;
    game_client->time_update_to_process -= game_client->previous_frame_info.number_of_updates * game_client->time_game_update_fixed;
    for (uint32_t game_updates_count = 0; game_updates_count < game_client->previous_frame_info.number_of_updates; ++game_updates_count) {
        game__update(game_client->game_state, game_client->time_game_update_fixed);
    }
    double time_end = system__get_time();
    game_client->previous_frame_info.time_update_actual = (time_end - self->time_start) / game_client->previous_frame_info.number_of_updates;

    return true;
}

static bool loop_stage__sleep_till_end_of_frame(loop_stage_t* self, game_client_t game_client) {
    const double time_mark_end_frame     = game_client->loop_stages[0].time_start + game_client->previous_frame_info.time_frame_expected;
    const double time_till_end_of_frame  = time_mark_end_frame - self->time_start;
    if (time_till_end_of_frame > 0.0) {
        system__sleep(time_till_end_of_frame);
    }

    ++game_client->current_frame;

    return true;
}

static void game_client__sample_prev_frame(game_client_t self) {
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

static void game_client__push_stage(game_client_t self, bool (*stage_fn)(struct loop_stage* self, game_client_t game_client)) {
    ARRAY_ENSURE_TOP(self->loop_stages, self->loop_stages_top, self->loop_stages_size);
    self->loop_stages[self->loop_stages_top].loop_stage__execute = stage_fn;
    ++self->loop_stages_top;
}
