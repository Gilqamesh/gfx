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

struct game_server {
    game_server_config_t config;
    tp_socket_t          tp_socket;

    seq_id_t             sequence_id;

    // todo: better lookup into connections based on network addr
    uint32_t       connections_size;
    uint32_t       connections_fill;
    connection_t*  connections;

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
static void game_server__receive_packets(game_server_t self, double time);
static void game_server__send_packets(game_server_t self);
static void game_server__disconnect_connection(game_server_t self, connection_t* connection);
static void game_server__connection__accept(
    game_server_t self, connection_t* connection, network_addr_t sender_addr,
    packet_t* packet, double time
);
static void game_server__accept_packet(game_server_t self, connection_t* connection, packet_t* packet, double time);

static void connection__check_for_lost_packets(connection_t* connection, uint32_t left_shift);

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

    game_server__receive_packets(game_server, self->time_start);
    game_server__send_packets(game_server);

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

static void game_server__disconnect_connection(game_server_t self, connection_t* connection) {
    ASSERT(self->connections_fill > 0);
    --self->connections_fill;

    connection->connected = false;
    debug__write("client disconnected from the server: %u:%u", connection->addr.addr, connection->addr.port);
    debug__write("last known packet from them: %u, local sequence id: %u", connection->sequence_id, self->sequence_id);
    debug__write("available connections: %u", self->connections_size - self->connections_fill);
    debug__flush(DEBUG_MODULE_GAME_SERVER, DEBUG_NET);
}

static void game_server__connection__accept(
    game_server_t self, connection_t* connection, network_addr_t sender_addr,
    packet_t* packet, double time
) {
    ASSERT(self->connections_fill < self->connections_size);
    ++self->connections_fill;

    connection->addr           = sender_addr;
    connection->time_last_sent = time;
    connection->sequence_id    = packet->sequence_id;
    connection->ack_bitfield   = -1;
    connection->time_connected = time;
    connection->connected      = true;

    debug__write("client connected to the server: %u:%u", sender_addr.addr, sender_addr.port);
    debug__write("available connections left: %u", self->connections_size - self->connections_fill);
    debug__flush(DEBUG_MODULE_GAME_SERVER, DEBUG_NET);
}

static void game_server__accept_packet(game_server_t self, connection_t* connection, packet_t* packet, double time) {
    (void) self;

    connection->time_last_sent = time;

    if (sequence_id__is_more_recent(packet->sequence_id, connection->sequence_id)) {
        const uint32_t connection_seq_id_delta = sequence_id__delta(packet->sequence_id, connection->sequence_id);
        connection__check_for_lost_packets(connection, connection_seq_id_delta);
        uint32_t new_ack_bitfield = 0;
        if (connection_seq_id_delta <= (sizeof(connection->ack_bitfield) << 3)) {
            ASSERT(connection_seq_id_delta > 0);
            new_ack_bitfield |= (connection->ack_bitfield << connection_seq_id_delta);
            new_ack_bitfield |= (1 << (connection_seq_id_delta - 1));
        }

        connection->ack_bitfield = new_ack_bitfield;
        connection->sequence_id = packet->sequence_id;
    } else if (connection->sequence_id != packet->sequence_id) {
        const uint32_t connection_seq_id_delta = sequence_id__delta(connection->sequence_id, packet->sequence_id);
        if (connection_seq_id_delta <= (sizeof(connection->ack_bitfield) << 3)) {
            connection->ack_bitfield |= (1 << (connection_seq_id_delta - 1));
        }
    }

    debug__write_raw("RECV PACKET: ");
    debug__write_packet_raw(packet);
    debug__flush(DEBUG_MODULE_GAME_SERVER, DEBUG_NET);
}

static void game_server__receive_packets(game_server_t self, double time) {
    packet_t packet;
    uint32_t received_data_len = 0;
    network_addr_t sender_addr;
    // todo: process a limited amount
    while (tp_socket__get_data(&self->tp_socket, &packet, sizeof(packet), &received_data_len, &sender_addr)) {
        if (received_data_len == sizeof(packet)) {
            bool package_accepted = false;
            connection_t* free_connection = 0;
            for (uint32_t connection_index = 0; connection_index < self->connections_size; ++connection_index) {
                connection_t* connection = &self->connections[connection_index];
                if (connection->connected) {
                    if (network_addr__is_same(&sender_addr, &connection->addr)) {
                        game_server__accept_packet(self, connection, &packet, time);
                        package_accepted = true;
                        break ;
                    }
                } else {
                    free_connection = connection;
                }
            }

            if (!package_accepted && free_connection) {
                game_server__connection__accept(self, free_connection, sender_addr, &packet, time);
            }
        } else {
            debug__write_and_flush(
                DEBUG_MODULE_GAME_SERVER, DEBUG_NET,
                "unknown packet size received: %u, expected: %u",
                received_data_len, sizeof(packet)
            );
        }

        // system__usleep(2000);
    }

    for (uint32_t connection_index = 0; connection_index < self->connections_size; ++connection_index) {
        connection_t* connection = &self->connections[connection_index];
        if (connection->connected) {
            if (connection->time_last_sent + self->config.max_time_for_disconnect < time) {
                game_server__disconnect_connection(self, connection);
            }
        }
    }
}

static void game_server__send_packets(game_server_t self) {
    for (uint32_t connection_index = 0; connection_index < self->connections_size; ++connection_index) {
        connection_t* connection = &self->connections[connection_index];
        if (connection->connected) {
            packet_t packet = {
                .sequence_id = self->sequence_id,
                .ack = connection->sequence_id,
                .ack_bitfield = connection->ack_bitfield
            };

            if (self->sequence_id % 7 != 0) {
                tp_socket__send_data_to(&self->tp_socket, &packet, sizeof(packet), connection->addr);
                debug__write_raw("SENT PACKET: ");
                debug__write_packet_raw(&packet);
                debug__flush(DEBUG_MODULE_GAME_SERVER, DEBUG_NET);
            }
        }
    }
    ++self->sequence_id;
}

static void connection__check_for_lost_packets(connection_t* connection, uint32_t left_shift) {
    if (connection->ack_bitfield != 0) {
        uint32_t bit_index_end = 0;
        if (left_shift > (sizeof(connection->ack_bitfield) << 3)) {
            bit_index_end = sizeof(connection->ack_bitfield) << 3;
            debug__write_and_flush(DEBUG_MODULE_GAME_CLIENT, DEBUG_NET, "LOST PACKET: %u", connection->sequence_id);
        } else {
            bit_index_end = left_shift;
        }

        for (uint32_t bit_index = 0; bit_index < bit_index_end; ++bit_index) {
            const uint32_t bit_mask_index = ((sizeof(connection->ack_bitfield) << 3) - 1 - bit_index);
            const uint32_t bit_mask = 1 << bit_mask_index;
            if ((connection->ack_bitfield & bit_mask) == 0) {
                debug__write_and_flush(DEBUG_MODULE_GAME_CLIENT, DEBUG_NET, "LOST PACKET: %u", sequence_id__sub(connection->sequence_id, bit_mask_index + 1));
            }
        }
    }
}
