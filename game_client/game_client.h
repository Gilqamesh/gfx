#ifndef GAME_CLIENT_H
# define GAME_CLIENT_H

# include <stdint.h>

struct         game_client;
struct         game_client_config;
typedef struct game_client*       game_client_t;
typedef struct game_client_config game_client_config_t;

struct game_client_config {
    const double max_time_after_packet_is_lost;
    const double max_target_fps;
};

game_client_t game_client__create(game_client_config_t config, uint16_t client_port, const char* server_ip, uint16_t server_port);
void game_client__destroy(game_client_t self);

void game_client__run(game_client_t self, double target_fps);

/**
 * @brief Get server time in seconds since it has been running
*/
double game_client__time(game_client_t self);

#endif // GAME_CLIENT_H
