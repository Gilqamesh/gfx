#ifndef GAME_SERVER_H
# define GAME_SERVER_H

# include <stdint.h>

struct         game_server;
struct         game_server_config; 
typedef struct game_server*       game_server_t;
typedef struct game_server_config game_server_config_t;

struct game_server_config {
    /**
     * Time after clients are disconnected if we haven't seen a package from them
    */
    double max_time_for_disconnect;
};

game_server_t game_server__create(game_server_config_t config, uint16_t port);
void game_server__destroy(game_server_t self);

void game_server__run(game_server_t self, double target_fps);

/**
 * @brief Get server time in seconds since it has been running
*/
double game_server__time(game_server_t self);

#endif // GAME_SERVER_H
