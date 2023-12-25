struct         loop_stage;
struct         frame_info;
struct         network_packet;
typedef struct frame_info     frame_info_t;
typedef struct loop_stage     loop_stage_t;
typedef struct network_packet network_packet_t;

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
    bool     (*loop_stage__execute)(struct loop_stage* self, game_server_t game_server);
};

typedef struct client {
    bool           connected;
    network_addr_t addr;
    uint32_t       sequence_id;
} client_t;

struct game_server {
    udp_socket_t   udp_socket;

    uint32_t       sequence_id;
    client_t       client;

    game_t         game_state;

    uint32_t      current_frame;
    double        time_lost;
    double        frames_lost;
    double        time_game_update_fixed;
    double        time_update_to_process;
    uint32_t      loop_stages_top;
    uint32_t      loop_stages_size;
    loop_stage_t* loop_stages;
    frame_info_t  previous_frame_info;
    uint32_t      frame_info_sample_index_tail;
    uint32_t      frame_info_sample_index_head;
    uint32_t      frame_info_sample_size;
    frame_info_t* frame_info_sample;
};

static bool loop_stage__collect_previous_frame_info(loop_stage_t* self, game_server_t game_server);
static bool loop_stage__poll_inputs(loop_stage_t* self, game_server_t game_server);
static bool loop_stage__update_loop(loop_stage_t* self, game_server_t game_server);
static bool loop_stage__sleep_till_end_of_frame(loop_stage_t* self, game_server_t game_server);
static void game_server__sample_prev_frame(game_server_t self);
static void game_server__push_stage(game_server_t self, bool (*stage_fn)(struct loop_stage* self, game_server_t game_server));

static bool loop_stage__collect_previous_frame_info(loop_stage_t* self, game_server_t game_server) {
    game_server->previous_frame_info.time_start         = game_server->previous_frame_info.time_end;
    game_server->previous_frame_info.time_end           = self->time_start;
    game_server->previous_frame_info.elapsed_time       = game_server->previous_frame_info.time_end - game_server->previous_frame_info.time_start;
    game_server->previous_frame_info.time_render_actual = game_server->previous_frame_info.time_render_actual;

    const double expected_time_lost_this_frame = game_server->previous_frame_info.elapsed_time - game_server->previous_frame_info.time_frame_expected;
    game_server->time_lost              += expected_time_lost_this_frame;
    game_server->frames_lost            += expected_time_lost_this_frame / game_server->previous_frame_info.time_frame_expected;
    game_server->time_update_to_process += game_server->previous_frame_info.elapsed_time;

    game_server__sample_prev_frame(game_server);

    int64_t seconds_since_loop_start = (int64_t) game_server->previous_frame_info.time_end;
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
        int32_t sample_index = (int32_t) game_server->frame_info_sample_index_tail;
        uint32_t frame_samples_count = 0;
        while (sample_index != (int32_t) game_server->frame_info_sample_index_head) {
            time_frame_actual_avg    += game_server->frame_info_sample[sample_index].elapsed_time;
            time_frame_expected_avg  += game_server->frame_info_sample[sample_index].time_frame_expected;
            time_update_actual_avg   += game_server->frame_info_sample[sample_index].time_update_actual;
            time_render_actual_avg   += game_server->frame_info_sample[sample_index].time_render_actual;
            number_of_updates_avg    += game_server->frame_info_sample[sample_index].number_of_updates;
            if (sample_index == (int32_t) game_server->frame_info_sample_size - 1) {
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

            debug__write("Frame #%u", game_server->current_frame);
            debug__write("Time lost:   %lf", game_server->time_lost);
            debug__write("Frames lost: %lf", game_server->frames_lost);
            debug__write("Frame avg info across %u frames", frame_samples_count);
            debug__write("  Game updates:            %lf", number_of_updates_avg);
            debug__write("  Time:");
            debug__write("    Total:                 %lfs", game_server->previous_frame_info.time_end);
            debug__write("    Game update actual:    %lfus", time_update_actual_avg * 1000000);
            debug__write("    Game update fixed:     %lfus", game_server->time_game_update_fixed * 1000000);
            debug__write("    Render actual:         %lfus", time_render_actual_avg * 1000000);
            debug__write("    Frame actual:          %lfms, %lffps", time_frame_actual_avg * 1000.0, 1.0 / time_frame_actual_avg);
            debug__write("    Frame expected:        %lfms, %lffps", time_frame_expected_avg * 1000.0, 1.0 / time_frame_expected_avg);
            debug__write("    Left to process:       %lfms", (game_server->time_update_to_process - game_server->previous_frame_info.elapsed_time) * 1000.0);
            debug__flush(DEBUG_MODULE_GAME_SERVER, DEBUG_INFO);
        }
    }

    return true;
}

static bool loop_stage__poll_inputs(loop_stage_t* self, game_server_t game_server) {
    (void) self;

    packet_t packet;
    uint32_t received_data_len = 0;
    network_addr_t sender_addr;
    if (udp_socket__get_data(&game_server->udp_socket, &packet, sizeof(packet), &received_data_len, &sender_addr)) {
        if (received_data_len == sizeof(packet)) {
            if (game_server->client.connected) {
                if (
                    sender_addr.addr == game_server->client.addr.addr &&
                    sender_addr.port == game_server->client.addr.port
                ) {
                    // received packet is from client
                    debug__write_and_flush(
                        DEBUG_MODULE_GAME_SERVER, DEBUG_INFO,
                        "received packet from client, remote seq id: %u, local seq id: %u",
                        packet.sequence_id, game_server->sequence_id
                    );
                    // update their connection
                    game_server->client.sequence_id = packet.sequence_id;
                } else {
                    // packet is not from client -> discard packet
                    debug__write_and_flush(
                        DEBUG_MODULE_GAME_SERVER, DEBUG_INFO,
                        "discarded packet as it is not from the connected client, received from: <todo with inet_ntop>"
                    );
                }
            } else {
                // accept new connection
                game_server->client.connected = true;
                game_server->client.addr.addr = sender_addr.addr;
                game_server->client.addr.port = sender_addr.port;
                game_server->client.sequence_id = packet.sequence_id;
                char msg[128] = { 0 };
                snprintf(msg, ARRAY_SIZE(msg), "Welcome to the server!\n");
                udp_socket__send_data_to(&game_server->udp_socket, msg, strlen(msg), &sender_addr);
            }
        }
    }

    if (game_server->client.connected) {
        // disconnect client that hasn't sent packets in a long time
        const uint32_t max_sequence_id_diff = 128;
        uint32_t sequence_id_diff = 0;
        if (game_server->sequence_id > game_server->client.sequence_id) {
            sequence_id_diff = game_server->sequence_id - game_server->client.sequence_id;
        } else {
            // our sequence_id wrapper around, but client's hasn't yet
            sequence_id_diff = game_server->client.sequence_id - game_server->sequence_id;
        }
        if (sequence_id_diff >= max_sequence_id_diff) {
            game_server->client.connected = false;
            char msg[128] = { 0 };
            snprintf(msg, ARRAY_SIZE(msg), "Sorry you were away for too long. :(\n");
            udp_socket__send_data_to(&game_server->udp_socket, msg, strlen(msg), &sender_addr);
        }
    }

    ++game_server->sequence_id;

    // todo: poll inputs
    // note: a client can manipulate the server such as shut it down, restart, etc.?

    // if (window__get_should_close(game_server->window)) {
    //     return false;
    // }

    return true;
}

static bool loop_stage__update_loop(loop_stage_t* self, game_server_t game_server) {
    game_server->previous_frame_info.time_update_actual = 0.0;
    game_server->previous_frame_info.number_of_updates  = 0;
    if (game_server->time_update_to_process == 0) {
        return true;
    }

    game_server->previous_frame_info.number_of_updates = game_server->time_update_to_process / game_server->time_game_update_fixed;
    game_server->time_update_to_process -= game_server->previous_frame_info.number_of_updates * game_server->time_game_update_fixed;
    for (uint32_t game_updates_count = 0; game_updates_count < game_server->previous_frame_info.number_of_updates; ++game_updates_count) {
        game__update(game_server->game_state, game_server->time_game_update_fixed);
    }
    double time_end = system__get_time();
    game_server->previous_frame_info.time_update_actual = (time_end - self->time_start) / game_server->previous_frame_info.number_of_updates;

    return true;
}

static bool loop_stage__sleep_till_end_of_frame(loop_stage_t* self, game_server_t game_server) {
    const double time_mark_end_frame     = game_server->loop_stages[0].time_start + game_server->previous_frame_info.time_frame_expected;
    const double time_till_end_of_frame  = time_mark_end_frame - self->time_start;
    if (time_till_end_of_frame > 0.0) {
        system__sleep(time_till_end_of_frame);
    }

    ++game_server->current_frame;

    return true;
}

static void game_server__sample_prev_frame(game_server_t self) {
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

static void game_server__push_stage(game_server_t self, bool (*stage_fn)(struct loop_stage* self, game_server_t game_server)) {
    ARRAY_ENSURE_TOP(self->loop_stages, self->loop_stages_top, self->loop_stages_size);
    self->loop_stages[self->loop_stages_top].loop_stage__execute = stage_fn;
    ++self->loop_stages_top;
}
