#include "game_server.h"

#include "debug.h"

int main() {
    if (!debug__init_module()) {
        return 1;
    }

    const uint32_t game_server_port = 3200;
    game_server_t game_server = game_server__create(game_server_port);

    const double target_fps = 20.0;
    game_server__run(game_server, target_fps);

    game_server__destroy(game_server);

    debug__deinit_module();

    return 0;
}
