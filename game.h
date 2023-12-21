#ifndef GAME_H
# define GAME_H

# include "glfw.h"

# include <stdbool.h>

struct         game;
typedef struct game* game_t;

game_t game__create(window_t window);
void game__destroy(game_t self);

/**
 * @returns The real time (in seconds) it takes at most to do one game__update
 * @note Necessary for fixed time step across multiple clients
 * @note If too low (lower than the average time to do a single game__update), the game loop won't catch up to real-time
 * @note Or it could catch by the game lowering its own time to take a single game__update, for example by lowering the game's LOD
*/
double game__update_upper_bound(game_t self);

/**
 * @note Should be fairly deterministic in terms of how much a call takes to work best with the fixed time step
*/
void game__update(game_t self, double s);

#endif // GAME_H
