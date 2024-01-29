struct         loop_stage;
struct         frame_info;
struct         sent_packet;
typedef struct loop_stage  loop_stage_t;
typedef struct frame_info  frame_info_t;
typedef struct sent_packet sent_packet_t;

struct frame_info {
    double       elapsed_time;
    double       time_update_actual;
    double       time_render_actual;
    double       time_start;
    double       time_end;
    uint32_t     number_of_updates;
};

struct loop_stage {
    double   time_start;
    double   time_elapsed;
    bool     (*loop_stage__execute)(struct loop_stage* self, game_client_t game_client);
};

struct sent_packet {
    double   time;
    uint32_t sequence_id;
};

struct game_client {
    game_client_config_t config;
    tp_socket_t          tp_socket;

    window_t       window;

    connection_t   connection;
    seq_id_t       sequence_id;

    uint32_t       sent_packets_queue_head;
    uint32_t       sent_packets_queue_tail;
    uint32_t       sent_packets_queue_size;
    sent_packet_t* sent_packets_queue;

    game_t         game_state;

    uint32_t       current_frame;
    double         time_lost;
    double         frames_lost;
    double         time_game_update_fixed;
    double         time_update_to_process;
    double         time_frame_expected;
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
static bool loop_stage__render(loop_stage_t* self, game_client_t game_client);
static bool loop_stage__sleep_till_end_of_frame(loop_stage_t* self, game_client_t game_client);

static void game_client__sample_prev_frame(game_client_t self);
static void game_client__push_stage(game_client_t self, bool (*stage_fn)(struct loop_stage* self, game_client_t game_client));
static void game_client__ack_packet(game_client_t self, connection_t* connection, packet_t* packet, double time);
static void game_client__connection_accept(
    game_client_t self, connection_t* connection, network_addr_t sender_addr,
    packet_t* packet, double time
);
static void game_client__accept_packet(game_client_t self, connection_t* connection, packet_t* packet, double time);
static void game_client__receive_packets(game_client_t self, double time);
static void game_client__send_packet(game_client_t self, double time);

static bool sent_packet__is_acked(sent_packet_t* self);

static void connection__check_for_lost_packets(connection_t* connection, uint32_t left_shift);

static bool loop_stage__collect_previous_frame_info(loop_stage_t* self, game_client_t game_client) {
    game_client->previous_frame_info.time_start         = game_client->previous_frame_info.time_end;
    game_client->previous_frame_info.time_end           = self->time_start;
    game_client->previous_frame_info.elapsed_time       = game_client->previous_frame_info.time_end - game_client->previous_frame_info.time_start;
    game_client->previous_frame_info.time_render_actual = game_client->previous_frame_info.time_render_actual;

    const double expected_time_lost_this_frame = game_client->previous_frame_info.elapsed_time - game_client->time_frame_expected;
    game_client->time_lost              += expected_time_lost_this_frame;
    game_client->frames_lost            += expected_time_lost_this_frame / game_client->time_frame_expected;
    game_client->time_update_to_process += game_client->previous_frame_info.elapsed_time;

    game_client__sample_prev_frame(game_client);

    int64_t seconds_since_loop_start = (int64_t) game_client->previous_frame_info.time_end;
    static int64_t seconds_last_info_printed;
    // (void) seconds_last_info_printed;
    if (seconds_since_loop_start > seconds_last_info_printed) {
    // if (0) {
        seconds_last_info_printed = seconds_since_loop_start;
        double time_frame_actual_avg             = 0.0;
        double time_update_actual_avg            = 0.0;
        double time_render_actual_avg            = 0.0;
        double number_of_updates_avg             = 0.0;
        int32_t sample_index = (int32_t) game_client->frame_info_sample_index_tail;
        uint32_t frame_samples_count = 0;
        while (sample_index != (int32_t) game_client->frame_info_sample_index_head) {
            frame_info_t* frame_info = &game_client->frame_info_sample[sample_index];
            time_frame_actual_avg    += frame_info->elapsed_time;
            time_update_actual_avg   += frame_info->time_update_actual;
            time_render_actual_avg   += frame_info->time_render_actual;
            number_of_updates_avg    += frame_info->number_of_updates;
            if (sample_index == (int32_t) game_client->frame_info_sample_size - 1) {
                sample_index = 0;
            } else {
                ++sample_index;
            }
            ++frame_samples_count;
        }
        if (frame_samples_count > 0) {
            time_frame_actual_avg    /= frame_samples_count;
            time_update_actual_avg   /= frame_samples_count;
            time_render_actual_avg   /= frame_samples_count;
            number_of_updates_avg    /= frame_samples_count;

            debug__lock();

            debug__writeln("Frame #%u", game_client->current_frame);
            debug__writeln("Time lost:   %lf", game_client->time_lost);
            debug__writeln("Frames lost: %lf", game_client->frames_lost);
            debug__writeln("Frame avg info across %u frames", frame_samples_count);
            debug__writeln("  Game updates:            %lf", number_of_updates_avg);
            debug__writeln("  Time:");
            debug__writeln("    Total:                 %lfs", game_client->previous_frame_info.time_end);
            debug__writeln("    Game update actual:    %lfus", time_update_actual_avg * 1000000.0);
            debug__writeln("    Game update fixed:     %lfus", game_client->time_game_update_fixed * 1000000.0);
            debug__writeln("    Render actual:         %lfus", time_render_actual_avg * 1000000.0);
            debug__writeln("    Frame actual:          %lfms, %lffps", time_frame_actual_avg * 1000.0, 1.0 / time_frame_actual_avg);
            debug__writeln("    Frame expected:        %lfms, %lffps", game_client->time_frame_expected * 1000.0, 1.0 / game_client->time_frame_expected);
            debug__writeln("    Left to process:       %lfms", (game_client->time_update_to_process - game_client->previous_frame_info.elapsed_time) * 1000.0);
            connection_t* connection = &game_client->connection;
            if (connection->connected) {
                debug__writeln("  Connection:");
                debug__writeln("    Addr:                  %u:%u", connection->addr.addr, connection->addr.port);
                debug__writeln("    Connected at:          %lf", connection->time_connected);
                debug__writeln("    Time last seen:        %lf", connection->time_last_seen);
                debug__writeln("    Seq id:                %u", connection->sequence_id);
                debug__writeln("    Ack bitfield: ");
                debug__write_ack_bitfield_raw(connection->ack_bitfield);
                debug__write_raw("\n");
                debug__writeln("    Packets dropped:       %u", connection->packets_dropped);
                debug__writeln("    RTT:                   %lfms", connection->rtt * 1000.0);
            }
            debug__flush(DEBUG_MODULE_GAME_CLIENT, DEBUG_INFO);

            debug__unlock();
        }
    }

    return true;
}

static bool loop_stage__poll_inputs(loop_stage_t* self, game_client_t game_client) {
    gfx__poll_events();
    if (window__get_should_close(game_client->window)) {
        return false;
    }
    game__frame_start(game_client->game_state);

    game_client__receive_packets(game_client, self->time_start);
    game_client__send_packet(game_client, self->time_start);

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

static bool loop_stage__render(loop_stage_t* self, game_client_t game_client) {
    ASSERT(game_client->time_update_to_process < game_client->time_frame_expected);
    const double render_interpolation_factor = 1.0 - game_client->time_update_to_process / game_client->time_frame_expected;
    ASSERT(render_interpolation_factor >= 0.0 && render_interpolation_factor <= 1.0);
    game__render(game_client->game_state, render_interpolation_factor);
    window__swap_buffers(game_client->window);

    game_client->previous_frame_info.time_render_actual = system__get_time() - self->time_start;

    return true;
}

static bool loop_stage__sleep_till_end_of_frame(loop_stage_t* self, game_client_t game_client) {
    const double time_mark_end_frame     = game_client->loop_stages[0].time_start + game_client->time_frame_expected;
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

static void game_client__ack_packet(game_client_t self, connection_t* connection, packet_t* packet, double time) {
    const uint32_t local_seq_id_delta = sequence_id__delta(self->sequence_id, packet->ack);
    if (local_seq_id_delta < self->sent_packets_queue_size) {
        const uint32_t packet_index_to_ack = (self->sent_packets_queue_head + self->sent_packets_queue_size - local_seq_id_delta) % self->sent_packets_queue_size;
        sent_packet_t* packet_to_ack = &self->sent_packets_queue[packet_index_to_ack];
        ASSERT(packet_to_ack->sequence_id == packet->ack);
        if (!sent_packet__is_acked(packet_to_ack)) {
            double rtt = time - self->previous_frame_info.elapsed_time - packet_to_ack->time;
            packet_to_ack->time = 0.0;
            if (connection->rtt == 0.0) {
                connection->rtt = rtt;
            } else {
                const double percentage_to_move = 0.1;
                connection->rtt = (1.0 - percentage_to_move) * connection->rtt + percentage_to_move * rtt;
            }
            // debug__write_and_flush(DEBUG_MODULE_GAME_CLIENT, DEBUG_NET, "RTT: %lfms", self->rtt * 1000.0);

            // todo: discard packet
            // if (rtt > 1.0) {
            //     // discard packet
            // }
            // ASSERT(rtt >= 0.0);
        }
    }
}

static void game_client__connection_accept(
    game_client_t self, connection_t* connection, network_addr_t sender_addr,
    packet_t* packet, double time
) {
    ASSERT(!self->connection.connected);

    connection->addr            = sender_addr;
    connection->time_last_seen  = time;
    connection->sequence_id     = packet->sequence_id;
    connection->ack_bitfield    = -1;
    connection->time_connected  = time;
    connection->rtt             = 0.0;
    connection->packets_dropped = 0;
    connection->connected       = true;

    debug__lock();

    debug__writeln("client connected to the server: %u:%u", sender_addr.addr, sender_addr.port);
    debug__flush(DEBUG_MODULE_GAME_CLIENT, DEBUG_NET);

    debug__unlock();
}

static void game_client__accept_packet(game_client_t self, connection_t* connection, packet_t* packet, double time) {
    connection->time_last_seen = time;

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

    game_client__ack_packet(self, connection, packet, time);

    debug__lock();

    debug__write_raw("RECV PACKET: ");
    debug__write_packet_raw(packet);
    debug__flush(DEBUG_MODULE_GAME_CLIENT, DEBUG_NET);

    debug__unlock();
}

static void game_client__receive_packets(game_client_t self, double time) {
    packet_t packet;
    uint32_t received_data_len = 0;
    network_addr_t sender_addr;
    // todo: process a limited amount
    while (tp_socket__get_data(&self->tp_socket, &packet, sizeof(packet), &received_data_len, &sender_addr)) {
        if (received_data_len == sizeof(packet)) {
            if (network_addr__is_same(&sender_addr, &self->connection.addr)) {
                if (!self->connection.connected) {
                    game_client__connection_accept(self, &self->connection, sender_addr, &packet, time);
                }
                game_client__accept_packet(self, &self->connection, &packet, time);
            } else {
                // packet is not from server -> discard packet
                debug__write_and_flush(
                    DEBUG_MODULE_GAME_CLIENT, DEBUG_NET,
                    "discarded packet as it is not from the server, received from: <todo with inet_ntop>"
                );
            }
        } else {
            debug__write_and_flush(
                DEBUG_MODULE_GAME_CLIENT, DEBUG_NET,
                "unknown packet size received: %u, expected: %u",
                received_data_len, sizeof(packet)
            );
        }
    }
}

static void game_client__send_packet(game_client_t self, double time) {
    packet_t packet = {
        .sequence_id = self->sequence_id,
        .ack = self->connection.sequence_id,
        .ack_bitfield = self->connection.ack_bitfield
    };

    ASSERT(self->sent_packets_queue_head < self->frame_info_sample_size);
    sent_packet_t* sent_packet = &self->sent_packets_queue[self->sent_packets_queue_head++];
    sent_packet->sequence_id = packet.sequence_id;
    sent_packet->time = time;

    if (self->sent_packets_queue_head == self->sent_packets_queue_size) {
        self->sent_packets_queue_head = 0;
    }
    if (self->sent_packets_queue_head == self->sent_packets_queue_tail) {
        ++self->sent_packets_queue_tail;
        if (self->sent_packets_queue_tail == self->sent_packets_queue_size) {
            self->sent_packets_queue_tail = 0;
        }
    }

    tp_socket__send_data(&self->tp_socket, &packet, sizeof(packet));

    debug__lock();

    debug__write_raw("SENT PACKET: ");
    debug__write_packet_raw(&packet);
    debug__flush(DEBUG_MODULE_GAME_CLIENT, DEBUG_NET);
    
    debug__unlock();

    ++self->sequence_id;
}

static bool sent_packet__is_acked(sent_packet_t* self) {
    return self->time == 0.0;
}

static void connection__check_for_lost_packets(connection_t* connection, uint32_t left_shift) {
    // todo: early exit if connection hasn't been alive for long enough to determine if packets have been lost

    if (connection->ack_bitfield != 0) {
        uint32_t bit_index_end = 0;
        if (left_shift > (sizeof(connection->ack_bitfield) << 3)) {
            bit_index_end = sizeof(connection->ack_bitfield) << 3;
            ++connection->packets_dropped;
            debug__write_and_flush(DEBUG_MODULE_GAME_CLIENT, DEBUG_NET, "LOST PACKET: %u", connection->sequence_id);
        } else {
            bit_index_end = left_shift;
        }

        for (uint32_t bit_index = 0; bit_index < bit_index_end; ++bit_index) {
            const uint32_t bit_mask_index = ((sizeof(connection->ack_bitfield) << 3) - 1 - bit_index);
            const uint32_t bit_mask = 1 << bit_mask_index;
            if ((connection->ack_bitfield & bit_mask) == 0) {
                ++connection->packets_dropped;
                debug__write_and_flush(DEBUG_MODULE_GAME_CLIENT, DEBUG_NET, "LOST PACKET: %u", sequence_id__sub(connection->sequence_id, bit_mask_index + 1));
            }
        }
    }
}
