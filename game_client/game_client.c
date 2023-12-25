#include "game_client.h"

#include "udp.h"
#include "system.h"
#include "helper_macros.h"
#include "debug.h"
#include "game.h"
#include "packet.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "game_client_impl.c"

game_client_t game_client__create(const char* server_ip, uint16_t server_port) {
    udp_socket_t udp_socket;
    if (!udp_socket__create(&udp_socket, server_port)) {
        return 0;
    }

    network_addr_t server_addr;
    if (!network_addr__create(&server_addr, server_ip, server_port)) {
        return false;
    }

    if (!udp_socket__connect(&udp_socket, server_addr)) {
        udp_socket__destroy(&udp_socket);
        return 0;
    }

    game_t game_state = game__create();
    (void) game_state;

    game_client_t result = calloc(1, sizeof(*result));
    if (!result) {
        udp_socket__destroy(&udp_socket);
        return 0;
    }

    result->udp_socket = udp_socket;
    memcpy(&result->server_addr, &server_addr, sizeof(result->server_addr));
    
    result->frame_info_sample_size = 128;
    result->frame_info_sample = malloc(result->frame_info_sample_size * sizeof(*result->frame_info_sample));

    game_client__push_stage(result, &loop_stage__collect_previous_frame_info);
    game_client__push_stage(result, &loop_stage__poll_inputs);
    game_client__push_stage(result, &loop_stage__update_loop);
    game_client__push_stage(result, &loop_stage__sleep_till_end_of_frame);

    return result;
}

void game_client__destroy(game_client_t self) {
    udp_socket__destroy(&self->udp_socket);

    free(self);
}

void game_client__run(game_client_t self, double target_fps) {
    debug__write_and_flush(DEBUG_MODULE_GAME_CLIENT, DEBUG_INFO, "target fps: %lf", target_fps);

    self->previous_frame_info.time_frame_expected = 1.0 / target_fps;
    self->time_game_update_fixed                  = game__update_upper_bound(self->game_state);
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
