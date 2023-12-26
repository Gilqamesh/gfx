#include "game_server.h"

#include "debug.h"

int main() {
    if (!debug__init_module()) {
        return 1;
    }

    game_server_config_t game_server_config = {
        .max_time_for_disconnect = 1.0
    };

    const uint32_t game_server_port = 3300;
    game_server_t game_server = game_server__create(game_server_config, game_server_port);
    if (!game_server) {
        return 2;
    }

    const double target_fps = 3;
    game_server__run(game_server, target_fps);

    game_server__destroy(game_server);

    debug__deinit_module();

    return 0;
}
