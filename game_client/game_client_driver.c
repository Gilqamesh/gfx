#include "game_client.h"

#include "debug.h"

int main() {
    if (!debug__init_module()) {
        return 1;
    }

    game_client_config_t game_client_config = {
        .max_target_fps = 60,
        .max_time_after_packet_is_lost = 1.0
    };

    const uint32_t game_client_port = 3100;
    const uint32_t game_server_port = 3300;
    const char* game_server_ip = "127.0.0.1";
    // const char* game_server_ip = "192.168.0.227";
    // const char* game_server_ip = "192.168.0.186";
    // const char* game_server_ip = "172.28.221.172";
    game_client_t game_client = game_client__create(game_client_config, game_client_port, game_server_ip, game_server_port);
    if (!game_client) {
        return 2;
    }

    const double target_fps = 5.0;
    game_client__run(game_client, target_fps);

    game_client__destroy(game_client);

    debug__deinit_module();

    return 0;
}
