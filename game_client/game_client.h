#ifndef GAME_CLIENT_H
# define GAME_CLIENT_H

# include <stdint.h>

struct         game_client;
typedef struct game_client* game_client_t;

game_client_t game_client__create(const char* server_ip, uint16_t server_port);
void game_client__destroy(game_client_t self);

void game_client__run(game_client_t self, double target_fps);

/**
 * @brief Get server time in seconds since it has been running
*/
double game_client__time(game_client_t self);

#endif // GAME_CLIENT_H
