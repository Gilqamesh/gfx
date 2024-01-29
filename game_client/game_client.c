#include "game_client.h"

#include "tp.h"
#include "system.h"
#include "helper_macros.h"
#include "debug.h"
#include "game.h"
#include "packet.h"
#include "gfx.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "game_client_impl.c"

game_client_t game_client__create(game_client_config_t config, uint16_t client_port, const char* server_ip, uint16_t server_port) {
    if (!gfx__init()) {
        return 0;
    }

    tp_socket_t tp_socket;
    if (!tp_socket__create(&tp_socket, SOCKET_TYPE_UDP, client_port)) {
        return 0;
    }
    debug__write_and_flush(
        DEBUG_MODULE_GAME_CLIENT, DEBUG_INFO,
        "created udp socket on port %u", client_port
    );

    network_addr_t server_addr;
    if (!network_addr__create(&server_addr, server_ip, server_port)) {
        return false;
    }

    if (!tp_socket__connect(&tp_socket, server_addr)) {
        tp_socket__destroy(&tp_socket);
        return 0;
    }

    const uint32_t sent_packets_queue_size = config.max_time_after_packet_is_lost * config.max_target_fps * 1.5;
    sent_packet_t* sent_packets_queue = calloc(1, sent_packets_queue_size * sizeof(*sent_packets_queue));
    if (!sent_packets_queue) {
        return 0;
    }

    uint32_t monitors_size = 0;
    monitor_t* monitors = gfx__get_monitors(&monitors_size);
    ASSERT(monitors_size > 0);
    window_t window = window__create(monitors[0], "Main Window", 800, 600);
    if (!window) {
        return 0;
    }

    game_t game_state = game__create(window);
    if (!game_state) {
        return 0;
    }

    game_client_t result = calloc(1, sizeof(*result));
    if (!result) {
        tp_socket__destroy(&tp_socket);
        return 0;
    }

    result->game_state = game_state;

    memcpy(&result->config, &config, sizeof(result->config));

    result->tp_socket = tp_socket;
    memcpy(&result->connection.addr, &server_addr, sizeof(result->connection.addr));
    
    result->frame_info_sample_size = 128;
    result->frame_info_sample = malloc(result->frame_info_sample_size * sizeof(*result->frame_info_sample));

    result->sent_packets_queue_size = sent_packets_queue_size;
    result->sent_packets_queue = sent_packets_queue;

    result->window = window;

    game_client__push_stage(result, &loop_stage__collect_previous_frame_info);
    // todo: hot reload stage
    // game_client__push_stage(result, &loop_stage__reload_game_dll);
    game_client__push_stage(result, &loop_stage__poll_inputs);
    game_client__push_stage(result, &loop_stage__update_loop);
    game_client__push_stage(result, &loop_stage__render);
    game_client__push_stage(result, &loop_stage__sleep_till_end_of_frame);

    return result;
}

void game_client__destroy(game_client_t self) {
    window__destroy(self->window);

    tp_socket__destroy(&self->tp_socket);

    gfx__deinit();

    free(self);
}

void game_client__run(game_client_t self, double target_fps) {
    debug__write_and_flush(DEBUG_MODULE_GAME_CLIENT, DEBUG_INFO, "target fps: %lf", target_fps);

    debug__set_message_type_availability(_DEBUG_MODULE_SIZE, DEBUG_NET, false);

    self->time_frame_expected    = 1.0 / target_fps;
    self->time_game_update_fixed = game__update_upper_bound(self->game_state);
    ASSERT(self->time_game_update_fixed < self->time_frame_expected);
    system__init();

    ASSERT(self->loop_stages_top > 0);
    bool stage_failed = false;
    while (!stage_failed) {
        loop_stage_t* loop_stage = 0;
        for (uint32_t stage_id = 0; stage_id < self->loop_stages_top; ++stage_id) {
            loop_stage = &self->loop_stages[stage_id];
            const double loop_stage_time_start = system__get_time();
            loop_stage->time_start = loop_stage_time_start;
            if (stage_id > 0) {
                self->loop_stages[stage_id - 1].time_elapsed = loop_stage_time_start - self->loop_stages[stage_id - 1].time_start;
            }
            loop_stage = &self->loop_stages[stage_id];
            if (!loop_stage->loop_stage__execute(loop_stage, self)) {
                stage_failed = true;
                break ;
            }
        }
        loop_stage->time_elapsed = system__get_time() - loop_stage->time_start;
    }
}

double game_client__time(game_client_t self) {
    (void) self;
    return system__get_time();
}
